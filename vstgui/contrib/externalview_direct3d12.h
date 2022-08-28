// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "externalview_hwnd.h"

#include <d3d12.h>
#include <dcomp.h>
#include <dxgi1_4.h>
#include <wrl.h>

#include <mutex>

#ifdef _MSC_VER
#pragma comment(lib, "dcomp.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#endif

//------------------------------------------------------------------------
namespace VSTGUI {
//------------------------------------------------------------------------

inline void ThrowIfFailed (HRESULT hr)
{
	if (FAILED (hr))
	{
		throw;
	}
}

//------------------------------------------------------------------------
namespace ExternalView {

//------------------------------------------------------------------------
struct IDirect3D12View
{
	virtual ~IDirect3D12View () noexcept = default;

	virtual ID3D12CommandQueue* getCommandQueue () const = 0;
	virtual ID3D12CommandAllocator* getCommandAllocator () const = 0;
	virtual IDXGISwapChain3* getSwapChain () const = 0;
	virtual ID3D12Device* getDevice () const = 0;

	virtual INT getFrameIndex () const = 0;
	virtual void setFrameIndex (INT index) = 0;

	virtual void render () = 0;
};

//------------------------------------------------------------------------
struct IDirect3D12Renderer
{
	virtual bool init (IDirect3D12View* view) = 0;
	virtual void render () = 0;
	virtual void beforeSizeUpdate () = 0;
	virtual void onSizeUpdate (IntSize newSize, double scaleFactor) = 0;
	virtual void onAttach () = 0;
	virtual void onRemove () = 0;

	virtual uint32_t getFrameCount () = 0;
};
using Direct3D12RendererPtr = std::shared_ptr<IDirect3D12Renderer>;

//------------------------------------------------------------------------
struct Direct3D12View : public ExternalHWNDBase,
						IDirect3D12View
{
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	Direct3D12View (HINSTANCE instance, const Direct3D12RendererPtr& renderer)
	: Base (instance), m_renderer (renderer)
	{
	}

private:
	ID3D12CommandQueue* getCommandQueue () const { return m_commandQueue.Get (); }
	ID3D12CommandAllocator* getCommandAllocator () const { return m_commandAllocator.Get (); }
	IDXGISwapChain3* getSwapChain () const { return m_swapChain.Get (); }
	ID3D12Device* getDevice () const { return m_device.Get (); }
	INT getFrameIndex () const { return m_frameIndex; }
	void setFrameIndex (INT index) { m_frameIndex = index; }

	void render () override
	{
		waitForPreviousFrame ();
		Guard g (mutex);
		m_renderer->render ();
	}

	bool attach (void* parent, PlatformViewType parentViewType) override
	{
		if (Base::attach (parent, parentViewType))
		{
			try
			{
				if (m_renderer->init (this))
				{
					initPipeline ();
					m_renderer->onAttach ();
				}
			}
			catch (...)
			{
				freeResources ();
			}
			return true;
		}
		return false;
	}

	bool remove () override
	{
		Guard g (mutex);
		freeResources ();
		return Base::remove ();
	}

	void setViewSize (IntRect frame, IntRect visible) override
	{
		Guard g (mutex);
		Base::setViewSize (frame, visible);
		m_visibleRect = visible;
		updateSizes ();
	}

	void setContentScaleFactor (double factor) override
	{
		Guard g (mutex);
		m_scaleFactor = factor;
		updateSizes ();
	}

	void updateSizes ()
	{
		if (m_commandQueue)
		{
			if (m_size.width == m_visibleRect.size.width &&
				m_size.height == m_visibleRect.size.height)
				return;
			m_size = m_visibleRect.size;
			waitForPreviousFrame ();
			m_renderer->beforeSizeUpdate ();
			ThrowIfFailed (m_dcompVisual->SetContent (nullptr));
			ThrowIfFailed (m_swapChain->ResizeBuffers (
				m_renderer->getFrameCount (), static_cast<UINT> (m_size.width),
				static_cast<UINT> (m_size.height), DXGI_FORMAT_R8G8B8A8_UNORM,
				DXGI_SWAP_EFFECT_FLIP_DISCARD));
			ThrowIfFailed (m_dcompVisual->SetContent (m_swapChain.Get ()));
			m_renderer->onSizeUpdate (m_size, m_scaleFactor);
			ThrowIfFailed (m_dcompDevice->Commit ());
		}
	}

	void freeResources ()
	{
		waitForPreviousFrame ();
		CloseHandle (m_fenceEvent);

		m_renderer->onRemove ();

		m_commandAllocator.Reset ();
		m_commandQueue.Reset ();

		m_dcompDevice.Reset ();
		m_dcompTarget.Reset ();
		m_dcompVisual.Reset ();

		m_swapChain.Reset ();
		m_device.Reset ();
	}

	void initPipeline ()
	{
#if defined(_DEBUG)
		// Enable the D3D12 debug layer.
		{
			ComPtr<ID3D12Debug> debugController;
			if (SUCCEEDED (D3D12GetDebugInterface (IID_PPV_ARGS (&debugController))))
			{
				debugController->EnableDebugLayer ();
			}
		}
#endif
		ComPtr<IDXGIFactory4> factory;
		ThrowIfFailed (CreateDXGIFactory1 (IID_PPV_ARGS (&factory)));

		ComPtr<IDXGIAdapter1> hardwareAdapter;
		getHardwareAdapter (factory.Get (), &hardwareAdapter);
		if (!hardwareAdapter)
		{
			throw;
		}

		ThrowIfFailed (D3D12CreateDevice (hardwareAdapter.Get (), D3D_FEATURE_LEVEL_11_0,
										  IID_PPV_ARGS (&m_device)));

		// Describe and create the command queue.
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		ThrowIfFailed (m_device->CreateCommandQueue (&queueDesc, IID_PPV_ARGS (&m_commandQueue)));

		try
		{
			createSwapChain (factory.Get ());
			setupDirectComposition ();
		}
		catch (...)
		{
			auto exc = std::current_exception ();
			throw (exc);
		}

		ThrowIfFailed (
			factory->MakeWindowAssociation (container.getHWND (), DXGI_MWA_NO_ALT_ENTER));

		ThrowIfFailed (m_device->CreateCommandAllocator (D3D12_COMMAND_LIST_TYPE_DIRECT,
														 IID_PPV_ARGS (&m_commandAllocator)));

		// Create synchronization objects and wait until assets have been uploaded to the GPU.
		ThrowIfFailed (m_device->CreateFence (0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS (&m_fence)));
		m_fenceValue = 1;

		// Create an event handle to use for frame synchronization.
		m_fenceEvent = CreateEvent (nullptr, FALSE, FALSE, nullptr);
		if (m_fenceEvent == nullptr)
		{
			ThrowIfFailed (HRESULT_FROM_WIN32 (GetLastError ()));
		}
	}

	void createSwapChain (IDXGIFactory4* factory)
	{
		// Describe and create the swap chain.
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = m_renderer->getFrameCount ();
		swapChainDesc.Width = static_cast<UINT> (m_size.width);
		swapChainDesc.Height = static_cast<UINT> (m_size.height);
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;

		ComPtr<IDXGISwapChain1> swapChain;
		ThrowIfFailed (factory->CreateSwapChainForComposition (
			m_commandQueue.Get (), // Swap chain needs the queue so that it can force a flush on it.
			&swapChainDesc, nullptr, &swapChain));

		ThrowIfFailed (swapChain.As (&m_swapChain));
	}

	void setupDirectComposition ()
	{
		// Create the DirectComposition device
		ThrowIfFailed (DCompositionCreateDevice (
			nullptr, IID_PPV_ARGS (m_dcompDevice.ReleaseAndGetAddressOf ())));

		// Create a DirectComposition target associated with the window (pass in hWnd here)
		ThrowIfFailed (m_dcompDevice->CreateTargetForHwnd (
			container.getHWND (), true, m_dcompTarget.ReleaseAndGetAddressOf ()));

		// Create a DirectComposition "visual"
		ThrowIfFailed (m_dcompDevice->CreateVisual (m_dcompVisual.ReleaseAndGetAddressOf ()));

		// Associate the visual with the swap chain
		ThrowIfFailed (m_dcompVisual->SetContent (m_swapChain.Get ()));

		// Set the visual as the root of the DirectComposition target's composition tree
		ThrowIfFailed (m_dcompTarget->SetRoot (m_dcompVisual.Get ()));
		ThrowIfFailed (m_dcompDevice->Commit ());
	}

	void getHardwareAdapter (IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter) const
	{
		ComPtr<IDXGIAdapter1> adapter;
		*ppAdapter = nullptr;

		for (UINT adapterIndex = 0;
			 DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1 (adapterIndex, &adapter);
			 ++adapterIndex)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1 (&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// Don't select the Basic Render Driver adapter.
				// If you want a software adapter, pass in "/warp" on the command line.
				continue;
			}

			// Check to see if the adapter supports Direct3D 12, but don't create the
			// actual device yet.
			if (SUCCEEDED (D3D12CreateDevice (adapter.Get (), D3D_FEATURE_LEVEL_11_0,
											  _uuidof(ID3D12Device), nullptr)))
			{
				break;
			}
		}

		*ppAdapter = adapter.Detach ();
	}

	void waitForPreviousFrame ()
	{
		if (!m_commandQueue)
			return;

		// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
		// This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
		// sample illustrates how to use fences for efficient resource usage and to
		// maximize GPU utilization.

		// Signal and increment the fence value.
		const UINT64 fence = m_fenceValue;
		ThrowIfFailed (m_commandQueue->Signal (m_fence.Get (), fence));
		m_fenceValue++;

		// Wait until the previous frame is finished.
		if (m_fence->GetCompletedValue () < fence)
		{
			ThrowIfFailed (m_fence->SetEventOnCompletion (fence, m_fenceEvent));
			WaitForSingleObject (m_fenceEvent, INFINITE);
		}

		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex ();
	}

	using Mutex = std::recursive_mutex;
	using Guard = std::lock_guard<Mutex>;

	Mutex mutex;

	IntSize m_size {100, 100};
	IntRect m_visibleRect {};
	double m_scaleFactor {1.};

	UINT m_frameIndex {0};

	// Synchronization objects.
	HANDLE m_fenceEvent {nullptr};
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue {0};

	ComPtr<IDXGISwapChain3> m_swapChain;

	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;

	// DirectComposition objects.
	ComPtr<IDCompositionDevice> m_dcompDevice;
	ComPtr<IDCompositionTarget> m_dcompTarget;
	ComPtr<IDCompositionVisual> m_dcompVisual;

	Direct3D12RendererPtr m_renderer;
};

//------------------------------------------------------------------------
} // ExternalView
} // VSTGUI

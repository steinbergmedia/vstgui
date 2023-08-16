// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "externalview_hwnd.h"

#include <d3d12.h>
#include <dcomp.h>
#include <dxgi1_4.h>
#include <wrl.h>
#include <comdef.h>

#include <mutex>

#ifdef _MSC_VER
#pragma comment(lib, "dcomp.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#endif

//------------------------------------------------------------------------
namespace VSTGUI {
//------------------------------------------------------------------------

//------------------------------------------------------------------------
struct Win32Exception : std::exception
{
	explicit Win32Exception (HRESULT hr) : _hr (hr)
	{
		FormatMessageA (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
							FORMAT_MESSAGE_IGNORE_INSERTS,
						NULL, hr, MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&_errorStr, 0,
						NULL);
	}

	~Win32Exception () noexcept
	{
		if (_errorStr)
			LocalFree ((HLOCAL)_errorStr);
	}

	const char* what () const noexcept override { return _errorStr; }

	HRESULT hr () const noexcept { return _hr; }

private:
	HRESULT _hr;
	char* _errorStr {nullptr};
};

inline void ThrowIfFailed (HRESULT hr)
{
	if (FAILED (hr))
	{
		throw Win32Exception (hr);
	}
}

//------------------------------------------------------------------------
namespace ExternalView {

//------------------------------------------------------------------------
struct IDirect3D12View
{
	virtual ~IDirect3D12View () noexcept = default;

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
	virtual ~IDirect3D12Renderer () noexcept = default;

	virtual bool init (IDirect3D12View* view) = 0;
	virtual void render (ID3D12CommandQueue* queue) = 0;
	virtual void beforeSizeUpdate () = 0;
	virtual void onSizeUpdate (IntSize newSize, double scaleFactor) = 0;
	virtual void onAttach () = 0;
	virtual void onRemove () = 0;

	virtual uint32_t getFrameCount () const = 0;
};

//------------------------------------------------------------------------
using Direct3D12RendererPtr = std::shared_ptr<IDirect3D12Renderer>;

//------------------------------------------------------------------------
struct GPUFence
{
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	GPUFence () = default;
	GPUFence (ID3D12Device* device, UINT64 initialValue = 0)
	{
		ThrowIfFailed (device->CreateFence (0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS (&m_fence)));
		m_event = CreateEvent (nullptr, FALSE, FALSE, nullptr);
		if (m_event == nullptr)
		{
			ThrowIfFailed (HRESULT_FROM_WIN32 (GetLastError ()));
		}
		m_value = initialValue;
	}
	~GPUFence () noexcept
	{
		if (m_event)
			CloseHandle (m_event);
	}

	GPUFence& operator=(GPUFence&& o) noexcept
	{
		m_event = o.m_event;
		m_fence = o.m_fence;
		m_value = o.m_value;
		o.m_event = nullptr;
		o.m_fence.Reset ();
		o.m_value = {};
		return *this;
	}

	void wait (ID3D12CommandQueue* queue)
	{
		if (m_fence == nullptr)
			return;

		const auto value = m_value;
		ThrowIfFailed (queue->Signal (m_fence.Get (), value));
		m_value++;
		if (m_fence->GetCompletedValue () < value)
		{
			ThrowIfFailed (m_fence->SetEventOnCompletion (value, m_event));
			WaitForSingleObject (m_event, INFINITE);
		}
	}

	HANDLE m_event {nullptr};
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_value {0};
};

//------------------------------------------------------------------------
struct Direct3D12View : public ExternalHWNDBase,
						IDirect3D12View
{
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	Direct3D12View (HINSTANCE instance, const Direct3D12RendererPtr& renderer,
					ComPtr<IDXGIFactory4> factory = nullptr, ComPtr<ID3D12Device> device = nullptr, ComPtr<ID3D12CommandQueue> commandQueue = nullptr)
	: Base (instance), m_renderer (renderer), m_factory (factory), m_device (device), m_commandQueue (commandQueue)
	{
		vstgui_assert ((factory && device) || (!factory && !device), "Either both factory and device are provided or none of both!");
		vstgui_assert (commandQueue ? device : true, "If a command queue is provided, the device must also be provided!");
	}

	static std::shared_ptr<Direct3D12View> make (HINSTANCE instance,
												 const Direct3D12RendererPtr& renderer,
												 ComPtr<IDXGIFactory4> factory = nullptr,
												 ComPtr<ID3D12Device> device = nullptr,
												 ComPtr<ID3D12CommandQueue> queue = nullptr)
	{
		return std::make_shared<Direct3D12View> (instance, renderer, factory, device, queue);
	}

	void render () override { doRender (); }

	Direct3D12RendererPtr& getRenderer () { return m_renderer; }
	const Direct3D12RendererPtr& getRenderer () const { return m_renderer; }

private:
	ID3D12CommandAllocator* getCommandAllocator () const { return m_commandAllocator.Get (); }
	IDXGISwapChain3* getSwapChain () const { return m_swapChain.Get (); }
	ID3D12Device* getDevice () const { return m_device.Get (); }
	INT getFrameIndex () const { return m_frameIndex; }
	void setFrameIndex (INT index) { m_frameIndex = index; }

	void doRender ()
	{
		if (mutex.try_lock ())
		{
			if (m_commandQueue)
			{
				HRESULT result = S_FALSE;
				try
				{
					waitForPreviousFrame ();
					m_renderer->render (m_commandQueue.Get ());
					result = getSwapChain ()->Present (1, 0);
					ThrowIfFailed (result);
				}
				catch (const Win32Exception& e)
				{
					try
					{
						freeResources ();
					}
					catch (...)
					{
					}
					throw (e);
				}
			}
			mutex.unlock ();
		}
	}

	bool attach (void* parent, PlatformViewType parentViewType) override
	{
		if (Base::attach (parent, parentViewType))
		{
			try
			{
				if (m_renderer->init (this))
				{
					init ();
					m_renderer->onAttach ();
				}
			}
			catch (...)
			{
				auto reasonHR = m_device->GetDeviceRemovedReason ();
				Win32Exception e (reasonHR);
				freeResources ();
			}
			return true;
		}
		return false;
	}

	bool remove () override
	{
		Guard g (mutex);
		waitForPreviousFrame ();
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
		if (m_swapChain)
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
		m_fence = {};

		try
		{
			m_renderer->onRemove ();
		}
		catch (...)
		{
		}

		m_commandAllocator.Reset ();
		m_commandQueue.Reset ();

		m_dcompDevice.Reset ();
		m_dcompTarget.Reset ();
		m_dcompVisual.Reset ();

		m_swapChain.Reset ();
		m_device.Reset ();
	}

	void init ()
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
		if (!m_factory)
			ThrowIfFailed (CreateDXGIFactory1 (IID_PPV_ARGS (&m_factory)));
		if (m_device == nullptr)
		{
			ComPtr<IDXGIAdapter1> hardwareAdapter;
			getHardwareAdapter (m_factory.Get (), &hardwareAdapter);
			if (!hardwareAdapter)
			{
				throw;
			}

			ThrowIfFailed (D3D12CreateDevice (hardwareAdapter.Get (), D3D_FEATURE_LEVEL_11_0,
											  IID_PPV_ARGS (&m_device)));
		}
		if (!m_commandQueue)
		{
			// Describe and create the command queue.
			D3D12_COMMAND_QUEUE_DESC queueDesc = {};
			queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
			queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			ThrowIfFailed (m_device->CreateCommandQueue (&queueDesc, IID_PPV_ARGS (&m_commandQueue)));
		}

		try
		{
			createSwapChain (m_factory.Get ());
			setupDirectComposition ();
		}
		catch (...)
		{
			auto exc = std::current_exception ();
			throw (exc);
		}

		ThrowIfFailed (
			m_factory->MakeWindowAssociation (container.getHWND (), DXGI_MWA_NO_ALT_ENTER));

		ThrowIfFailed (m_device->CreateCommandAllocator (D3D12_COMMAND_LIST_TYPE_DIRECT,
														 IID_PPV_ARGS (&m_commandAllocator)));

		m_fence = GPUFence (m_device.Get (), 1);
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

	static void getHardwareAdapter (IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter)
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
		if (!m_commandQueue || !m_swapChain)
			return;

		// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
		// This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
		// sample illustrates how to use fences for efficient resource usage and to
		// maximize GPU utilization.

		m_fence.wait (m_commandQueue.Get ());
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
	GPUFence m_fence;

	ComPtr<IDXGIFactory4> m_factory;

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

// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "direct3dwindow.h"
#include "direct3dshader.h"
#include "vstgui/contrib/externalview_hwnd.h"
#include "vstgui/lib/cexternalview.h"
#include "vstgui/lib/platform/win32/win32factory.h"
#include "vstgui/lib/cvstguitimer.h"
#include "vstgui/standalone/include/helpers/uidesc/customization.h"
#include "vstgui/standalone/include/helpers/windowcontroller.h"
#include "vstgui/standalone/include/iuidescwindow.h"
#include "vstgui/uidescription/delegationcontroller.h"
#include "vstgui/uidescription/iuidescription.h"
#include "vstgui/uidescription/uiattributes.h"

#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <dcomp.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>

#ifdef _MSC_VER
#pragma comment(lib, "dcomp.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")
#endif

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

using Microsoft::WRL::ComPtr;

inline void ThrowIfFailed (HRESULT hr)
{
	if (FAILED (hr))
	{
		throw;
	}
}

struct CD3DX12_DEFAULT
{
};
extern const DECLSPEC_SELECTANY CD3DX12_DEFAULT D3D12_DEFAULT;

inline UINT8 D3D12GetFormatPlaneCount (_In_ ID3D12Device* pDevice, DXGI_FORMAT Format) noexcept
{
	D3D12_FEATURE_DATA_FORMAT_INFO formatInfo = {Format, 0};
	if (FAILED (pDevice->CheckFeatureSupport (D3D12_FEATURE_FORMAT_INFO, &formatInfo,
											  sizeof (formatInfo))))
	{
		return 0;
	}
	return formatInfo.PlaneCount;
}

//------------------------------------------------------------------------------------------------
inline constexpr UINT D3D12CalcSubresource (UINT MipSlice, UINT ArraySlice, UINT PlaneSlice,
											UINT MipLevels, UINT ArraySize) noexcept
{
	return MipSlice + ArraySlice * MipLevels + PlaneSlice * MipLevels * ArraySize;
}

struct CD3DX12_BLEND_DESC : public D3D12_BLEND_DESC
{
	CD3DX12_BLEND_DESC () = default;
	explicit CD3DX12_BLEND_DESC (const D3D12_BLEND_DESC& o) noexcept : D3D12_BLEND_DESC (o) {}
	explicit CD3DX12_BLEND_DESC (CD3DX12_DEFAULT) noexcept
	{
		AlphaToCoverageEnable = FALSE;
		IndependentBlendEnable = FALSE;
		const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc = {
			FALSE,
			FALSE,
			D3D12_BLEND_ONE,
			D3D12_BLEND_ZERO,
			D3D12_BLEND_OP_ADD,
			D3D12_BLEND_ONE,
			D3D12_BLEND_ZERO,
			D3D12_BLEND_OP_ADD,
			D3D12_LOGIC_OP_NOOP,
			D3D12_COLOR_WRITE_ENABLE_ALL,
		};
		for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
			RenderTarget[i] = defaultRenderTargetBlendDesc;
	}
};

struct CD3DX12_HEAP_PROPERTIES : public D3D12_HEAP_PROPERTIES
{
	CD3DX12_HEAP_PROPERTIES () = default;
	explicit CD3DX12_HEAP_PROPERTIES (const D3D12_HEAP_PROPERTIES& o) noexcept
	: D3D12_HEAP_PROPERTIES (o)
	{
	}
	CD3DX12_HEAP_PROPERTIES (D3D12_CPU_PAGE_PROPERTY cpuPageProperty,
							 D3D12_MEMORY_POOL memoryPoolPreference, UINT creationNodeMask = 1,
							 UINT nodeMask = 1)
	noexcept
	{
		Type = D3D12_HEAP_TYPE_CUSTOM;
		CPUPageProperty = cpuPageProperty;
		MemoryPoolPreference = memoryPoolPreference;
		CreationNodeMask = creationNodeMask;
		VisibleNodeMask = nodeMask;
	}
	explicit CD3DX12_HEAP_PROPERTIES (D3D12_HEAP_TYPE type, UINT creationNodeMask = 1,
									  UINT nodeMask = 1) noexcept
	{
		Type = type;
		CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		CreationNodeMask = creationNodeMask;
		VisibleNodeMask = nodeMask;
	}
	bool IsCPUAccessible () const noexcept
	{
		return Type == D3D12_HEAP_TYPE_UPLOAD || Type == D3D12_HEAP_TYPE_READBACK ||
			   (Type == D3D12_HEAP_TYPE_CUSTOM &&
				(CPUPageProperty == D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE ||
				 CPUPageProperty == D3D12_CPU_PAGE_PROPERTY_WRITE_BACK));
	}
};

struct CD3DX12_RESOURCE_DESC : public D3D12_RESOURCE_DESC
{
	CD3DX12_RESOURCE_DESC () = default;
	explicit CD3DX12_RESOURCE_DESC (const D3D12_RESOURCE_DESC& o) noexcept : D3D12_RESOURCE_DESC (o)
	{
	}
	CD3DX12_RESOURCE_DESC (D3D12_RESOURCE_DIMENSION dimension, UINT64 alignment, UINT64 width,
						   UINT height, UINT16 depthOrArraySize, UINT16 mipLevels,
						   DXGI_FORMAT format, UINT sampleCount, UINT sampleQuality,
						   D3D12_TEXTURE_LAYOUT layout, D3D12_RESOURCE_FLAGS flags)
	noexcept
	{
		Dimension = dimension;
		Alignment = alignment;
		Width = width;
		Height = height;
		DepthOrArraySize = depthOrArraySize;
		MipLevels = mipLevels;
		Format = format;
		SampleDesc.Count = sampleCount;
		SampleDesc.Quality = sampleQuality;
		Layout = layout;
		Flags = flags;
	}
	static inline CD3DX12_RESOURCE_DESC
		Buffer (const D3D12_RESOURCE_ALLOCATION_INFO& resAllocInfo,
				D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE) noexcept
	{
		return CD3DX12_RESOURCE_DESC (D3D12_RESOURCE_DIMENSION_BUFFER, resAllocInfo.Alignment,
									  resAllocInfo.SizeInBytes, 1, 1, 1, DXGI_FORMAT_UNKNOWN, 1, 0,
									  D3D12_TEXTURE_LAYOUT_ROW_MAJOR, flags);
	}
	static inline CD3DX12_RESOURCE_DESC
		Buffer (UINT64 width, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
				UINT64 alignment = 0) noexcept
	{
		return CD3DX12_RESOURCE_DESC (D3D12_RESOURCE_DIMENSION_BUFFER, alignment, width, 1, 1, 1,
									  DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
									  flags);
	}
	static inline CD3DX12_RESOURCE_DESC Tex1D (
		DXGI_FORMAT format, UINT64 width, UINT16 arraySize = 1, UINT16 mipLevels = 0,
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
		D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN, UINT64 alignment = 0) noexcept
	{
		return CD3DX12_RESOURCE_DESC (D3D12_RESOURCE_DIMENSION_TEXTURE1D, alignment, width, 1,
									  arraySize, mipLevels, format, 1, 0, layout, flags);
	}
	static inline CD3DX12_RESOURCE_DESC Tex2D (
		DXGI_FORMAT format, UINT64 width, UINT height, UINT16 arraySize = 1, UINT16 mipLevels = 0,
		UINT sampleCount = 1, UINT sampleQuality = 0,
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
		D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN, UINT64 alignment = 0) noexcept
	{
		return CD3DX12_RESOURCE_DESC (D3D12_RESOURCE_DIMENSION_TEXTURE2D, alignment, width, height,
									  arraySize, mipLevels, format, sampleCount, sampleQuality,
									  layout, flags);
	}
	static inline CD3DX12_RESOURCE_DESC Tex3D (
		DXGI_FORMAT format, UINT64 width, UINT height, UINT16 depth, UINT16 mipLevels = 0,
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
		D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN, UINT64 alignment = 0) noexcept
	{
		return CD3DX12_RESOURCE_DESC (D3D12_RESOURCE_DIMENSION_TEXTURE3D, alignment, width, height,
									  depth, mipLevels, format, 1, 0, layout, flags);
	}
	inline UINT16 Depth () const noexcept
	{
		return (Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D ? DepthOrArraySize : 1);
	}
	inline UINT16 ArraySize () const noexcept
	{
		return (Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE3D ? DepthOrArraySize : 1);
	}
	inline UINT8 PlaneCount (_In_ ID3D12Device* pDevice) const noexcept
	{
		return D3D12GetFormatPlaneCount (pDevice, Format);
	}
	inline UINT Subresources (_In_ ID3D12Device* pDevice) const noexcept
	{
		return MipLevels * ArraySize () * PlaneCount (pDevice);
	}
	inline UINT CalcSubresource (UINT MipSlice, UINT ArraySlice, UINT PlaneSlice) noexcept
	{
		return D3D12CalcSubresource (MipSlice, ArraySlice, PlaneSlice, MipLevels, ArraySize ());
	}
};

struct CD3DX12_RESOURCE_BARRIER : public D3D12_RESOURCE_BARRIER
{
	CD3DX12_RESOURCE_BARRIER () = default;
	explicit CD3DX12_RESOURCE_BARRIER (const D3D12_RESOURCE_BARRIER& o) noexcept
	: D3D12_RESOURCE_BARRIER (o)
	{
	}
	static inline CD3DX12_RESOURCE_BARRIER
		Transition (_In_ ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateBefore,
					D3D12_RESOURCE_STATES stateAfter,
					UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
					D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE) noexcept
	{
		CD3DX12_RESOURCE_BARRIER result = {};
		D3D12_RESOURCE_BARRIER& barrier = result;
		result.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		result.Flags = flags;
		barrier.Transition.pResource = pResource;
		barrier.Transition.StateBefore = stateBefore;
		barrier.Transition.StateAfter = stateAfter;
		barrier.Transition.Subresource = subresource;
		return result;
	}
	static inline CD3DX12_RESOURCE_BARRIER Aliasing (_In_ ID3D12Resource* pResourceBefore,
													 _In_ ID3D12Resource* pResourceAfter) noexcept
	{
		CD3DX12_RESOURCE_BARRIER result = {};
		D3D12_RESOURCE_BARRIER& barrier = result;
		result.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
		barrier.Aliasing.pResourceBefore = pResourceBefore;
		barrier.Aliasing.pResourceAfter = pResourceAfter;
		return result;
	}
	static inline CD3DX12_RESOURCE_BARRIER UAV (_In_ ID3D12Resource* pResource) noexcept
	{
		CD3DX12_RESOURCE_BARRIER result = {};
		D3D12_RESOURCE_BARRIER& barrier = result;
		result.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		barrier.UAV.pResource = pResource;
		return result;
	}
};

struct CD3DX12_CPU_DESCRIPTOR_HANDLE : public D3D12_CPU_DESCRIPTOR_HANDLE
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE () = default;
	explicit CD3DX12_CPU_DESCRIPTOR_HANDLE (const D3D12_CPU_DESCRIPTOR_HANDLE& o) noexcept
	: D3D12_CPU_DESCRIPTOR_HANDLE (o)
	{
	}
	CD3DX12_CPU_DESCRIPTOR_HANDLE (CD3DX12_DEFAULT) noexcept { ptr = 0; }
	CD3DX12_CPU_DESCRIPTOR_HANDLE (_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other,
								   INT offsetScaledByIncrementSize)
	noexcept
	{
		InitOffsetted (other, offsetScaledByIncrementSize);
	}
	CD3DX12_CPU_DESCRIPTOR_HANDLE (_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other,
								   INT offsetInDescriptors, UINT descriptorIncrementSize)
	noexcept
	{
		InitOffsetted (other, offsetInDescriptors, descriptorIncrementSize);
	}
	CD3DX12_CPU_DESCRIPTOR_HANDLE& Offset (INT offsetInDescriptors,
										   UINT descriptorIncrementSize) noexcept
	{
		ptr = SIZE_T (INT64 (ptr) + INT64 (offsetInDescriptors) * INT64 (descriptorIncrementSize));
		return *this;
	}
	CD3DX12_CPU_DESCRIPTOR_HANDLE& Offset (INT offsetScaledByIncrementSize) noexcept
	{
		ptr = SIZE_T (INT64 (ptr) + INT64 (offsetScaledByIncrementSize));
		return *this;
	}
	bool operator== (_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other) const noexcept
	{
		return (ptr == other.ptr);
	}
	bool operator!= (_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other) const noexcept
	{
		return (ptr != other.ptr);
	}
	CD3DX12_CPU_DESCRIPTOR_HANDLE& operator= (const D3D12_CPU_DESCRIPTOR_HANDLE& other) noexcept
	{
		ptr = other.ptr;
		return *this;
	}

	inline void InitOffsetted (_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base,
							   INT offsetScaledByIncrementSize) noexcept
	{
		InitOffsetted (*this, base, offsetScaledByIncrementSize);
	}

	inline void InitOffsetted (_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base,
							   INT offsetInDescriptors, UINT descriptorIncrementSize) noexcept
	{
		InitOffsetted (*this, base, offsetInDescriptors, descriptorIncrementSize);
	}

	static inline void InitOffsetted (_Out_ D3D12_CPU_DESCRIPTOR_HANDLE& handle,
									  _In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base,
									  INT offsetScaledByIncrementSize) noexcept
	{
		handle.ptr = SIZE_T (INT64 (base.ptr) + INT64 (offsetScaledByIncrementSize));
	}

	static inline void InitOffsetted (_Out_ D3D12_CPU_DESCRIPTOR_HANDLE& handle,
									  _In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base,
									  INT offsetInDescriptors,
									  UINT descriptorIncrementSize) noexcept
	{
		handle.ptr = SIZE_T (INT64 (base.ptr) +
							 INT64 (offsetInDescriptors) * INT64 (descriptorIncrementSize));
	}
};

//------------------------------------------------------------------------
struct Direct3DView : public ExternalView::ExternalHWNDBase
{
	using Base::Base;

	bool attach (void* parent, PlatformViewType parentViewType) override
	{
		if (Base::attach (parent, parentViewType))
		{
			try
			{
				initPipeline ();
				loadAssets ();
				doRender ();
				timer = makeOwned<CVSTGUITimer> ([this] (auto) { doRender (); }, 1);
			}
			catch (...)
			{
				onDestroy ();
			}
			return true;
		}
		return false;
	}

	bool remove () override
	{
		timer = nullptr;
		onDestroy ();
		return Base::remove ();
	}

	void setViewSize (IntRect frame, IntRect visible) override
	{
		Base::setViewSize (frame, visible);
		visibleRect = visible;
		updateSizes ();
	}

	void setContentScaleFactor (double factor) override
	{
		scaleFactor = factor;
		updateSizes ();
	}

	void updateSizes ()
	{
		if (m_width == visibleRect.size.width && m_height == visibleRect.size.height)
			return;
		m_width = visibleRect.size.width;
		m_height = visibleRect.size.height;
		m_viewport = {0.f, 0.f, static_cast<float> (visibleRect.size.width),
					  static_cast<float> (visibleRect.size.height)};
		m_scissorRect = {0, 0, static_cast<LONG> (visibleRect.size.width),
						 static_cast<LONG> (visibleRect.size.height)};
		if (m_commandQueue)
		{
			waitForPreviousFrame ();
			ThrowIfFailed (m_dcompVisual->SetContent (nullptr));
			freeFrameResources ();
			ThrowIfFailed (m_swapChain->ResizeBuffers (FrameCount, m_width, m_height,
													   DXGI_FORMAT_R8G8B8A8_UNORM,
													   DXGI_SWAP_EFFECT_FLIP_DISCARD));
			ThrowIfFailed (m_dcompVisual->SetContent (m_swapChain.Get ()));
			createFrameResources ();
			doRender ();
			ThrowIfFailed (m_dcompDevice->Commit ());
		}
	}

	void getHardwareAdapter (IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter)
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

		// Describe and create the swap chain.
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = FrameCount;
		swapChainDesc.Width = m_width;
		swapChainDesc.Height = m_height;
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
		//------------------------------------------------------------------
		// Set up DirectComposition
		//------------------------------------------------------------------

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

		//------------------------------------------------------------------
		// DirectComposition setup end
		//------------------------------------------------------------------

		// This sample does not support fullscreen transitions.
		ThrowIfFailed (
			factory->MakeWindowAssociation (container.getHWND (), DXGI_MWA_NO_ALT_ENTER));

		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex ();

		// Create descriptor heaps.
		{
			// Describe and create a render target view (RTV) descriptor heap.
			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
			rtvHeapDesc.NumDescriptors = FrameCount;
			rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			ThrowIfFailed (
				m_device->CreateDescriptorHeap (&rtvHeapDesc, IID_PPV_ARGS (&m_rtvHeap)));

			// Describe and create a shader resource view (SRV) heap for the texture.
			D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
			srvHeapDesc.NumDescriptors = 1;
			srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			ThrowIfFailed (
				m_device->CreateDescriptorHeap (&srvHeapDesc, IID_PPV_ARGS (&m_srvHeap)));

			m_rtvDescriptorSize =
				m_device->GetDescriptorHandleIncrementSize (D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		}

		createFrameResources ();

		ThrowIfFailed (m_device->CreateCommandAllocator (D3D12_COMMAND_LIST_TYPE_DIRECT,
														 IID_PPV_ARGS (&m_commandAllocator)));
	}

	void freeFrameResources ()
	{
		for (UINT n = 0; n < FrameCount; n++)
			m_renderTargets[n].Reset ();
		m_frameIndex = 0;
	}

	void createFrameResources ()
	{
		// Create frame resources.
		{
			auto rtvHandle = (m_rtvHeap->GetCPUDescriptorHandleForHeapStart ());

			// Create a RTV for each frame.
			for (UINT n = 0; n < FrameCount; n++)
			{
				ThrowIfFailed (m_swapChain->GetBuffer (n, IID_PPV_ARGS (&m_renderTargets[n])));
				m_device->CreateRenderTargetView (m_renderTargets[n].Get (), nullptr, rtvHandle);
				rtvHandle.ptr =
					SIZE_T (INT64 (rtvHandle.ptr) + INT64 (1) * INT64 (m_rtvDescriptorSize));
			}
		}
	}

	// Load the sample assets.
	void loadAssets ()
	{
		// Create an empty root signature.
		{
			D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {
				0, nullptr, 0, nullptr,
				D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT};

			ComPtr<ID3DBlob> signature;
			ComPtr<ID3DBlob> error;
			ThrowIfFailed (D3D12SerializeRootSignature (
				&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
			ThrowIfFailed (m_device->CreateRootSignature (0, signature->GetBufferPointer (),
														  signature->GetBufferSize (),
														  IID_PPV_ARGS (&m_rootSignature)));
		}

		// Create the pipeline state, which includes compiling and loading shaders.
		{
			ComPtr<ID3DBlob> vertexShader;
			ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
			// Enable better shader debugging with the graphics debugging tools.
			UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
			UINT compileFlags = 0;
#endif

			ThrowIfFailed (D3DCompile (shaderCode, strlen (shaderCode), nullptr, nullptr, nullptr,
									   "VSMain", "vs_5_0", compileFlags, 0, &vertexShader,
									   nullptr));
			ThrowIfFailed (D3DCompile (shaderCode, strlen (shaderCode), nullptr, nullptr, nullptr,
									   "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

			// Define the vertex input layout.
			D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
				{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
				 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
				{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,
				 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

			// Describe and create the graphics pipeline state object (PSO).
			D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
			psoDesc.InputLayout = {inputElementDescs, _countof (inputElementDescs)};
			psoDesc.pRootSignature = m_rootSignature.Get ();
			psoDesc.VS = {vertexShader.Get ()->GetBufferPointer (),
						  vertexShader.Get ()->GetBufferSize ()};
			psoDesc.PS = {pixelShader.Get ()->GetBufferPointer (),
						  pixelShader.Get ()->GetBufferSize ()};
			psoDesc.RasterizerState = {D3D12_FILL_MODE_SOLID,
									   D3D12_CULL_MODE_BACK,
									   FALSE,
									   D3D12_DEFAULT_DEPTH_BIAS,
									   D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
									   D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
									   TRUE,
									   FALSE,
									   FALSE,
									   0,
									   D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF};
			psoDesc.BlendState = CD3DX12_BLEND_DESC (D3D12_DEFAULT);
			psoDesc.DepthStencilState.DepthEnable = FALSE;
			psoDesc.DepthStencilState.StencilEnable = FALSE;
			psoDesc.SampleMask = UINT_MAX;
			psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			psoDesc.NumRenderTargets = 1;
			psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			psoDesc.SampleDesc.Count = 1;
			ThrowIfFailed (
				m_device->CreateGraphicsPipelineState (&psoDesc, IID_PPV_ARGS (&m_pipelineState)));
		}

		// Create the command list.
		ThrowIfFailed (m_device->CreateCommandList (
			0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get (), m_pipelineState.Get (),
			IID_PPV_ARGS (&m_commandList)));

		// Command lists are created in the recording state, but there is nothing
		// to record yet. The main loop expects it to be closed, so close it now.
		ThrowIfFailed (m_commandList->Close ());

		updateAndUploadVertexBuffer ();

		// Create synchronization objects and wait until assets have been uploaded to the GPU.
		{
			ThrowIfFailed (
				m_device->CreateFence (0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS (&m_fence)));
			m_fenceValue = 1;

			// Create an event handle to use for frame synchronization.
			m_fenceEvent = CreateEvent (nullptr, FALSE, FALSE, nullptr);
			if (m_fenceEvent == nullptr)
			{
				ThrowIfFailed (HRESULT_FROM_WIN32 (GetLastError ()));
			}

			// Wait for the command list to execute; we are reusing the same command
			// list in our main loop but for now, we just want to wait for setup to
			// complete before continuing.
			waitForPreviousFrame ();
		}
	}

	void updateAndUploadVertexBuffer ()
	{
		// Create the vertex buffer.
		{
			// Define the geometry for a triangle.
			Vertex triangleVertices[] = {{{0.0f, 0.5f, 0.0f}, colorRight},
										 {{0.5f, -0.5f, 0.0f}, colorLeft},
										 {{-0.5f, -0.5f, 0.0f}, colorTop}};

			const UINT vertexBufferSize = sizeof (triangleVertices);

			// Note: using upload heaps to transfer static data like vert buffers is not
			// recommended. Every time the GPU needs it, the upload heap will be marshalled
			// over. Please read up on Default Heap usage. An upload heap is used here for
			// code simplicity and because there are very few verts to actually transfer.
			ThrowIfFailed (m_device->CreateCommittedResource (
				&CD3DX12_HEAP_PROPERTIES (D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer (vertexBufferSize),
				D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS (&m_vertexBuffer)));

			// Copy the triangle data to the vertex buffer.
			UINT8* pVertexDataBegin;
			D3D12_RANGE readRange {0, 0}; // We do not intend to read from this resource on the CPU.
			ThrowIfFailed (
				m_vertexBuffer->Map (0, &readRange, reinterpret_cast<void**> (&pVertexDataBegin)));
			memcpy (pVertexDataBegin, triangleVertices, sizeof (triangleVertices));
			m_vertexBuffer->Unmap (0, nullptr);

			// Initialize the vertex buffer view.
			m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress ();
			m_vertexBufferView.StrideInBytes = sizeof (Vertex);
			m_vertexBufferView.SizeInBytes = vertexBufferSize;
		}
	}

	void updateColors ()
	{
		++frameCounter;
		colorTop.x = (1.f + std::sin (frameCounter * 0.013f)) * 0.5f;
		colorTop.y = (1.f + std::sin (frameCounter * 0.021f)) * 0.5f;
		colorTop.z = (1.f + std::sin (frameCounter * 0.037f)) * 0.5f;

		colorLeft.x = (1.f + std::sin (frameCounter * 0.031f)) * 0.5f;
		colorLeft.y = (1.f + std::sin (frameCounter * 0.021f)) * 0.5f;
		colorLeft.z = (1.f + std::sin (frameCounter * 0.011f)) * 0.5f;

		colorRight.x = (1.f + std::sin (frameCounter * 0.025f)) * 0.5f;
		colorRight.y = (1.f + std::sin (frameCounter * 0.012f)) * 0.5f;
		colorRight.z = (1.f + std::sin (frameCounter * 0.031f)) * 0.5f;
	}

	void doRender ()
	{
		waitForPreviousFrame ();

		updateColors ();
		updateAndUploadVertexBuffer ();

		// Record all the commands we need to render the scene into the command list.
		populateCommandList ();

		// Execute the command list.
		ID3D12CommandList* ppCommandLists[] = {m_commandList.Get ()};
		m_commandQueue->ExecuteCommandLists (_countof (ppCommandLists), ppCommandLists);

		// Present the frame.
		ThrowIfFailed (m_swapChain->Present (1, 0));

	}

	void onDestroy ()
	{
		waitForPreviousFrame ();

		CloseHandle (m_fenceEvent);

		m_vertexBuffer.Reset ();

		m_dcompDevice.Reset ();
		m_dcompTarget.Reset ();
		m_dcompVisual.Reset ();

		m_swapChain.Reset ();
		m_device.Reset ();
		for (auto i = 0; i < FrameCount; ++i)
			m_renderTargets[i].Reset ();
		m_commandAllocator.Reset ();
		m_commandQueue.Reset ();
		m_rootSignature.Reset ();
		m_rtvHeap.Reset ();
		m_srvHeap.Reset ();
		m_pipelineState.Reset ();
		m_commandList.Reset ();
	}

	void populateCommandList ()
	{
		// Command list allocators can only be reset when the associated
		// command lists have finished execution on the GPU; apps should use
		// fences to determine GPU execution progress.
		ThrowIfFailed (m_commandAllocator->Reset ());

		// However, when ExecuteCommandList() is called on a particular command
		// list, that command list can then be reset at any time and must be before
		// re-recording.
		ThrowIfFailed (m_commandList->Reset (m_commandAllocator.Get (), m_pipelineState.Get ()));

		// Set necessary state.
		m_commandList->SetGraphicsRootSignature (m_rootSignature.Get ());
		m_commandList->RSSetViewports (1, &m_viewport);
		m_commandList->RSSetScissorRects (1, &m_scissorRect);

		// Indicate that the back buffer will be used as a render target.
		m_commandList->ResourceBarrier (
			1, &CD3DX12_RESOURCE_BARRIER::Transition (m_renderTargets[m_frameIndex].Get (),
													  D3D12_RESOURCE_STATE_PRESENT,
													  D3D12_RESOURCE_STATE_RENDER_TARGET));

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle (m_rtvHeap->GetCPUDescriptorHandleForHeapStart (),
												 m_frameIndex, m_rtvDescriptorSize);
		m_commandList->OMSetRenderTargets (1, &rtvHandle, FALSE, nullptr);

		// Record commands.
		const float clearColor[] = {0.0f, 0.2f, 0.4f, 1.0f};
		m_commandList->ClearRenderTargetView (rtvHandle, clearColor, 0, nullptr);
		m_commandList->IASetPrimitiveTopology (D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_commandList->IASetVertexBuffers (0, 1, &m_vertexBufferView);
		m_commandList->DrawInstanced (3, 1, 0, 0);

		// Indicate that the back buffer will now be used to present.
		m_commandList->ResourceBarrier (
			1, &CD3DX12_RESOURCE_BARRIER::Transition (m_renderTargets[m_frameIndex].Get (),
													  D3D12_RESOURCE_STATE_RENDER_TARGET,
													  D3D12_RESOURCE_STATE_PRESENT));

		ThrowIfFailed (m_commandList->Close ());
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

	static constexpr const UINT FrameCount {2};
	UINT m_width {100};
	UINT m_height {100};
	float m_aspectRatio {1.f};

	// DirectComposition objects.
	ComPtr<IDCompositionDevice> m_dcompDevice;
	ComPtr<IDCompositionTarget> m_dcompTarget;
	ComPtr<IDCompositionVisual> m_dcompVisual;

	// Pipeline objects.
	D3D12_VIEWPORT m_viewport {};
	D3D12_RECT m_scissorRect {};
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	UINT m_rtvDescriptorSize {0};

	// Synchronization objects.
	UINT m_frameIndex {0};
	HANDLE m_fenceEvent {nullptr};
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue {0};

	// App objects
	ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
	};

	IntRect visibleRect {};
	float scaleFactor {1.};

	DirectX::XMFLOAT4 colorTop {0.f, 0.f, 1.f, 1.f};
	DirectX::XMFLOAT4 colorLeft {0.f, 1.f, 0.f, 1.f};
	DirectX::XMFLOAT4 colorRight {1.f, 0.f, 0.f, 1.f};
	uint32_t frameCounter {0};
	SharedPointer<CVSTGUITimer> timer;
};

//------------------------------------------------------------------------
struct Direct3DController : DelegationController
{
	using DelegationController::DelegationController;

	CView* createView (const UIAttributes& attributes, const IUIDescription* description) override
	{
		if (auto viewName = attributes.getAttributeValue (IUIDescription::kCustomViewName))
		{
			if (*viewName == "Direct3DView")
			{
				if (auto view = std::make_shared<Direct3DView> (
						getPlatformFactory ().asWin32Factory ()->getInstance ()))
				{
					return new CExternalView ({}, view);
				}
			}
		}
		return DelegationController::createView (attributes, description);
	}
};

//------------------------------------------------------------------------
WindowPtr makeNewDirect3DExampleWindow ()
{
	auto customization = UIDesc::Customization::make ();
	customization->addCreateViewControllerFunc ("Direct3DController", [] (auto, auto parent, auto) {
		return new Direct3DController (parent);
	});

	UIDesc::Config config;
	config.uiDescFileName = "direct3dwindow.uidesc";
	config.viewName = "view";
	config.windowConfig.type = WindowType::Document;
	config.windowConfig.style.close ().size ().border ();
	config.windowConfig.title = "Direct3D Example";
	config.customization = customization;

	return UIDesc::makeWindow (config);
}

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI

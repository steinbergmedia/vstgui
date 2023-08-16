// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "direct3dwindow.h"
#include "direct3dshader.h"
#include "vstgui/contrib/externalview_direct3d12.h"
#include "vstgui/lib/cexternalview.h"
#include "vstgui/lib/cvstguitimer.h"
#include "vstgui/lib/platform/win32/win32factory.h"
#include "vstgui/standalone/include/helpers/uidesc/customization.h"
#include "vstgui/standalone/include/helpers/windowcontroller.h"
#include "vstgui/standalone/include/iuidescwindow.h"
#include "vstgui/uidescription/delegationcontroller.h"
#include "vstgui/uidescription/iuidescription.h"
#include "vstgui/uidescription/uiattributes.h"

#include <D3Dcompiler.h>
#include <DirectXMath.h>

#include <thread>

#define VSTGUI_USE_THREADED_DIRECT3D12_EXAMPLE 1

#ifdef _MSC_VER
#pragma comment(lib, "d3dcompiler.lib")
#endif

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

using Microsoft::WRL::ComPtr;

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
struct ExampleRenderer : public ExternalView::IDirect3D12Renderer
{
	static constexpr uint32_t FrameCount = 2;

	ExternalView::IDirect3D12View* m_view;

	bool init (ExternalView::IDirect3D12View* view) override
	{
		m_view = view;
		return true;
	}

	void render (ID3D12CommandQueue* queue) override { doRender (queue); }

	void beforeSizeUpdate () override { freeFrameResources (); }

	void onSizeUpdate (ExternalView::IntSize newSize, double scaleFactor) override
	{
		m_viewport = {0.f, 0.f, static_cast<float> (newSize.width),
					  static_cast<float> (newSize.height)};
		m_scissorRect = {0, 0, static_cast<LONG> (newSize.width),
						 static_cast<LONG> (newSize.height)};

		createFrameResources ();
		m_view->render ();
	}

	void onAttach () override
	{
		createFrameResources ();
		loadAssets ();
		m_view->render ();
#if VSTGUI_USE_THREADED_DIRECT3D12_EXAMPLE
		stopRenderThread = false;
		renderThread = std::thread ([this] () {
			while (!stopRenderThread)
			{
				try
				{
					m_view->render ();
				}
				catch (...)
				{
					stopRenderThread = true;
				}
				std::this_thread::sleep_for (std::chrono::milliseconds (1));
			}
		});
#else
		timer = makeOwned<CVSTGUITimer> ([this] (auto) { m_view->render (); }, 16);
#endif
	}
	void onRemove () override
	{
#if VSTGUI_USE_THREADED_DIRECT3D12_EXAMPLE
		stopRenderThread = true;
		if (renderThread.joinable ())
			renderThread.join ();
#else
		timer = nullptr;
#endif
		freeFrameResources ();
		onDestroy ();
	}

	uint32_t getFrameCount () const override { return FrameCount; }

	void freeFrameResources ()
	{
		for (UINT n = 0; n < FrameCount; n++)
			m_renderTargets[n].Reset ();
		m_view->setFrameIndex (0);
	}

	void createFrameResources ()
	{
		// Create descriptor heaps.
		{
			// Describe and create a render target view (RTV) descriptor heap.
			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
			rtvHeapDesc.NumDescriptors = FrameCount;
			rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			ThrowIfFailed (m_view->getDevice ()->CreateDescriptorHeap (&rtvHeapDesc,
																	   IID_PPV_ARGS (&m_rtvHeap)));

			m_rtvDescriptorSize = m_view->getDevice ()->GetDescriptorHandleIncrementSize (
				D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		}
		// Create frame resources.
		{
			auto rtvHandle = (m_rtvHeap->GetCPUDescriptorHandleForHeapStart ());

			// Create a RTV for each frame.
			for (UINT n = 0; n < FrameCount; n++)
			{
				ThrowIfFailed (
					m_view->getSwapChain ()->GetBuffer (n, IID_PPV_ARGS (&m_renderTargets[n])));
				m_view->getDevice ()->CreateRenderTargetView (m_renderTargets[n].Get (), nullptr,
															  rtvHandle);
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
			ThrowIfFailed (m_view->getDevice ()->CreateRootSignature (
				0, signature->GetBufferPointer (), signature->GetBufferSize (),
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
			constexpr D3D12_BLEND_DESC DefaultBlendDesc = {
				FALSE,
				FALSE,
				{FALSE, FALSE, D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
				 D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD, D3D12_LOGIC_OP_NOOP,
				 D3D12_COLOR_WRITE_ENABLE_ALL}};
			psoDesc.BlendState = DefaultBlendDesc;
			psoDesc.DepthStencilState.DepthEnable = FALSE;
			psoDesc.DepthStencilState.StencilEnable = FALSE;
			psoDesc.SampleMask = UINT_MAX;
			psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			psoDesc.NumRenderTargets = 1;
			psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			psoDesc.SampleDesc.Count = 1;
			ThrowIfFailed (m_view->getDevice ()->CreateGraphicsPipelineState (
				&psoDesc, IID_PPV_ARGS (&m_pipelineState)));
		}

		// Create the command list.
		ThrowIfFailed (m_view->getDevice ()->CreateCommandList (
			0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_view->getCommandAllocator (),
			m_pipelineState.Get (), IID_PPV_ARGS (&m_commandList)));

		// Command lists are created in the recording state, but there is nothing
		// to record yet. The main loop expects it to be closed, so close it now.
		ThrowIfFailed (m_commandList->Close ());

		updateAndUploadVertexBuffer ();
	}

	void updateAndUploadVertexBuffer ()
	{
		// Create the vertex buffer.

		// Define the geometry for a triangle.
		Vertex triangleVertices[] = {{{0.0f, 0.8f, 0.0f}, colorRight},
									 {{0.8f, -0.8f, 0.0f}, colorLeft},
									 {{-0.8f, -0.8f, 0.0f}, colorTop}};

		const UINT vertexBufferSize = sizeof (triangleVertices);

		// Note: using upload heaps to transfer static data like vert buffers is not
		// recommended. Every time the GPU needs it, the upload heap will be marshalled
		// over. Please read up on Default Heap usage. An upload heap is used here for
		// code simplicity and because there are very few verts to actually transfer.
		auto heapProps = CD3DX12_HEAP_PROPERTIES (D3D12_HEAP_TYPE_UPLOAD);
		auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer (vertexBufferSize);
		ThrowIfFailed (m_view->getDevice ()->CreateCommittedResource (
			&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, IID_PPV_ARGS (&m_vertexBuffer)));

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

	void doRender (ID3D12CommandQueue* queue)
	{
		updateColors ();
		updateAndUploadVertexBuffer ();

		// Record all the commands we need to render the scene into the command list.
		populateCommandList ();

		// Execute the command list.
		ID3D12CommandList* ppCommandLists[] = {m_commandList.Get ()};
		queue->ExecuteCommandLists (_countof (ppCommandLists), ppCommandLists);
	}

	void onDestroy ()
	{
		m_vertexBuffer.Reset ();

		for (auto i = 0; i < FrameCount; ++i)
			m_renderTargets[i].Reset ();
		m_rootSignature.Reset ();
		m_rtvHeap.Reset ();
		m_pipelineState.Reset ();
		m_commandList.Reset ();
	}

	void populateCommandList ()
	{
		// Command list allocators can only be reset when the associated
		// command lists have finished execution on the GPU; apps should use
		// fences to determine GPU execution progress.
		ThrowIfFailed (m_view->getCommandAllocator ()->Reset ());

		// However, when ExecuteCommandList() is called on a particular command
		// list, that command list can then be reset at any time and must be before
		// re-recording.
		ThrowIfFailed (
			m_commandList->Reset (m_view->getCommandAllocator (), m_pipelineState.Get ()));

		// Set necessary state.
		m_commandList->SetGraphicsRootSignature (m_rootSignature.Get ());
		m_commandList->RSSetViewports (1, &m_viewport);
		m_commandList->RSSetScissorRects (1, &m_scissorRect);

		// Indicate that the back buffer will be used as a render target.
		auto transition = CD3DX12_RESOURCE_BARRIER::Transition (
			m_renderTargets[m_view->getFrameIndex ()].Get (), D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_commandList->ResourceBarrier (1, &transition);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle (m_rtvHeap->GetCPUDescriptorHandleForHeapStart (),
												 m_view->getFrameIndex (), m_rtvDescriptorSize);
		m_commandList->OMSetRenderTargets (1, &rtvHandle, FALSE, nullptr);

		// Record commands.
		const float clearColor[] = {0.0f, 0.2f, 0.4f, 1.0f};
		m_commandList->ClearRenderTargetView (rtvHandle, clearColor, 0, nullptr);
		m_commandList->IASetPrimitiveTopology (D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_commandList->IASetVertexBuffers (0, 1, &m_vertexBufferView);
		m_commandList->DrawInstanced (3, 1, 0, 0);

		// Indicate that the back buffer will now be used to present.
		transition = CD3DX12_RESOURCE_BARRIER::Transition (
			m_renderTargets[m_view->getFrameIndex ()].Get (), D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT);
		m_commandList->ResourceBarrier (1, &transition);

		ThrowIfFailed (m_commandList->Close ());
	}

	float m_aspectRatio {1.f};

	// Pipeline objects.
	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	UINT m_rtvDescriptorSize {0};

	// App objects
	ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	D3D12_VIEWPORT m_viewport {};
	D3D12_RECT m_scissorRect {};

	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
	};

	DirectX::XMFLOAT4 colorTop {0.f, 0.f, 1.f, 1.f};
	DirectX::XMFLOAT4 colorLeft {0.f, 1.f, 0.f, 1.f};
	DirectX::XMFLOAT4 colorRight {1.f, 0.f, 0.f, 1.f};
	uint32_t frameCounter {0};
#if VSTGUI_USE_THREADED_DIRECT3D12_EXAMPLE
	bool stopRenderThread {true};
	std::thread renderThread;
#else
	SharedPointer<CVSTGUITimer> timer;
#endif
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
				auto renderer = std::make_shared<ExampleRenderer> ();
				if (auto view = ExternalView::Direct3D12View::make (
						getPlatformFactory ().asWin32Factory ()->getInstance (), renderer))
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

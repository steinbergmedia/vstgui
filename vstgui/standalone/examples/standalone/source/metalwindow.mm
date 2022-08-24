#import "metaltypes.h"
#import "metalwindow.h"
#import "vstgui/contrib/externalview_metal.h"
#import "vstgui/lib/cexternalview.h"
#import "vstgui/standalone/include/helpers/uidesc/customization.h"
#import "vstgui/standalone/include/helpers/windowcontroller.h"
#import "vstgui/standalone/include/iuidescwindow.h"
#import "vstgui/uidescription/delegationcontroller.h"
#import "vstgui/uidescription/iuidescription.h"
#import "vstgui/uidescription/uiattributes.h"

#import <simd/simd.h>
#import "metalshader.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

//------------------------------------------------------------------------
struct ExampleMetalRenderer : ExternalView::IMetalRenderer
{
	id<MTLDevice> _device {MTLCreateSystemDefaultDevice ()};
	id<MTLRenderPipelineState> _pipelineState {nullptr};
	id<MTLCommandQueue> _commandQueue {nullptr};
	MTLRenderPassDescriptor* _drawableRenderDescriptor;
	vector_uint2 _viewportSize {};

	simd::float4 colorTop {0, 0, 1, 1};
	simd::float4 colorLeft {0, 1, 0, 1};
	simd::float4 colorRight {1, 0, 0, 1};

	uint64_t frameCounter {0};

	ExternalView::IMetalView* _metalView {nullptr};
	CVDisplayLinkRef _displayLink {nullptr};

#if !__has_feature(objc_arc)
	~ExampleMetalRenderer () noexcept
	{
		[_pipelineState release];
		[_commandQueue release];
		[_drawableRenderDescriptor release];
		[_device release];
	}
#endif

	bool init (ExternalView::IMetalView* metalView, CAMetalLayer* metalLayer) override
	{
		NSError* error = nullptr;
		auto defaultLibrary =
			[_device newLibraryWithSource:[NSString stringWithUTF8String:shaderCode]
								  options:nil
									error:&error];
		if (error)
			return false;

		auto vertexFunction = [defaultLibrary newFunctionWithName:@"vertexShader"];
		auto fragmentFunction = [defaultLibrary newFunctionWithName:@"fragmentShader"];
		if (!vertexFunction || !fragmentFunction)
			return false;

		// Configure a pipeline descriptor that is used to create a pipeline state.
		auto pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
		pipelineStateDescriptor.label = @"Simple Pipeline";
		pipelineStateDescriptor.vertexFunction = vertexFunction;
		pipelineStateDescriptor.fragmentFunction = fragmentFunction;
		pipelineStateDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;

		_pipelineState = [_device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor
																 error:&error];

#if !__has_feature(objc_arc)
		[pipelineStateDescriptor release];
		[defaultLibrary release];
		[vertexFunction release];
		[fragmentFunction release];
#endif
		if (error)
			return false;

		// Create the command queue
		_commandQueue = [_device newCommandQueue];
		_drawableRenderDescriptor = [MTLRenderPassDescriptor new];
		_drawableRenderDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
		_drawableRenderDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
		_drawableRenderDescriptor.colorAttachments[0].clearColor = MTLClearColorMake (0, 0, 0, 0);

		metalLayer.device = _device;
		_metalView = metalView;
		return true;
	}

	void onSizeUpdate (int32_t width, int32_t height, double scaleFactor) override
	{
		_viewportSize.x = width * scaleFactor;
		_viewportSize.y = height * scaleFactor;
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

	static CVReturn displayLinkRender (CVDisplayLinkRef displayLink, const CVTimeStamp* now,
									   const CVTimeStamp* outputTime, CVOptionFlags flagsIn,
									   CVOptionFlags* flagsOut, void* displayLinkContext)
	{
		auto Self = reinterpret_cast<ExampleMetalRenderer*> (displayLinkContext);
		Self->updateColors ();
		Self->_metalView->render ();
	}

	void onAttached () override {}

	void onRemoved () override
	{
		CVDisplayLinkStop (_displayLink);
		CVDisplayLinkRelease (_displayLink);
		_displayLink = nullptr;
	}

	void onScreenChanged (NSScreen* screen) override
	{
		if (_displayLink)
			onRemoved ();
		auto result = CVDisplayLinkCreateWithActiveCGDisplays (&_displayLink);
		if (result != kCVReturnSuccess)
			return;
		result = CVDisplayLinkSetOutputCallback (_displayLink, displayLinkRender, this);
		if (result != kCVReturnSuccess)
			return;
		auto displayID = static_cast<CGDirectDisplayID> (
			[screen.deviceDescription[@"NSScreenNumber"] unsignedIntValue]);
		result = CVDisplayLinkSetCurrentCGDisplay (_displayLink, displayID);
		if (result != kCVReturnSuccess)
			return;
		CVDisplayLinkStart (_displayLink);
	}

	void draw (id<CAMetalDrawable> drawable) override
	{
		if (!_pipelineState)
			return;

		float width = _viewportSize.x * 0.5;
		float height = _viewportSize.y * 0.5;
		const AAPLVertex triangleVertices[] = {
			// 2D positions,    RGBA colors
			{{width, -height}, colorRight},
			{{-width, -height}, colorLeft},
			{{0, height}, colorTop},
		};

		// Create a new command buffer for each render pass to the current drawable.
		auto commandBuffer = [_commandQueue commandBuffer];
		commandBuffer.label = @"MyCommand";

		_drawableRenderDescriptor.colorAttachments[0].texture = drawable.texture;

		// Create a render command encoder.
		auto renderEncoder =
			[commandBuffer renderCommandEncoderWithDescriptor:_drawableRenderDescriptor];
		renderEncoder.label = @"MyRenderEncoder";

		// Set the region of the drawable to draw into.
		[renderEncoder setViewport:(MTLViewport) {0.0, 0.0, static_cast<double> (_viewportSize.x),
												  static_cast<double> (_viewportSize.y), 0.0, 1.0}];

		[renderEncoder setRenderPipelineState:_pipelineState];

		// Pass in the parameter data.
		[renderEncoder setVertexBytes:triangleVertices
							   length:sizeof (triangleVertices)
							  atIndex:AAPLVertexInputIndexVertices];

		[renderEncoder setVertexBytes:&_viewportSize
							   length:sizeof (_viewportSize)
							  atIndex:AAPLVertexInputIndexViewportSize];

		// Draw the triangle.
		[renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];

		[renderEncoder endEncoding];

		// Schedule a present once the framebuffer is complete using the current drawable.
		[commandBuffer presentDrawable:drawable];

		// Finalize rendering here & push the command buffer to the GPU.
		[commandBuffer commit];
	}
};

//------------------------------------------------------------------------
struct MetalController : DelegationController
{
	using DelegationController::DelegationController;

	CView* createView (const UIAttributes& attributes, const IUIDescription* description) override
	{
		if (auto viewName = attributes.getAttributeValue (IUIDescription::kCustomViewName))
		{
			if (*viewName == "MetalView")
			{
				auto renderer = std::make_shared<ExampleMetalRenderer> ();
				if (auto metalView = ExternalView::MetalView::make (renderer))
				{
					return new CExternalView ({}, metalView);
				}
			}
		}
		return DelegationController::createView (attributes, description);
	}
};

//------------------------------------------------------------------------
WindowPtr makeNewMetalExampleWindow ()
{
	auto customization = UIDesc::Customization::make ();
	customization->addCreateViewControllerFunc (
		"MetalController", [] (auto, auto parent, auto) { return new MetalController (parent); });

	UIDesc::Config config;
	config.uiDescFileName = "metalwindow.uidesc";
	config.viewName = "view";
	config.windowConfig.type = WindowType::Document;
	config.windowConfig.style.close ().size ().border ();
	config.windowConfig.title = "Metal Example";
	config.customization = customization;

	return UIDesc::makeWindow (config);
}

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI

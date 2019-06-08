// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#import "touchbarsupport.h"

#ifdef MAC_OS_X_VERSION_10_12 // works only when building with the 10.12 SDK !

#import "vstgui/standalone/include/helpers/valuelistener.h"
#import <Cocoa/Cocoa.h>

//------------------------------------------------------------------------
@interface MandelbrotTouchbarSliderBinding : NSObject
@property (retain) NSSlider* slider;
@property VSTGUI::Standalone::ValuePtr value;
@end

//------------------------------------------------------------------------
namespace Mandelbrot {

using namespace VSTGUI;
using namespace VSTGUI::Standalone;

//------------------------------------------------------------------------
class TouchBarMaker : public ITouchBarCreator, public ValueListenerAdapter
{
public:
	TouchBarMaker (const ValuePtr& value) : value (value)
	{
		value->registerListener (this);
	}

	~TouchBarMaker () override
	{
		value->unregisterListener (this);
	}

	void* createTouchBar () override
	{
		auto touchbar = [NSTouchBar new];
		sliderBinding = [MandelbrotTouchbarSliderBinding new];
		sliderBinding.value = value;
		sliderBinding.slider.doubleValue = value->getValue ();
		auto itemID = @"MandelBrotMaxIterationsID";
		touchbar.defaultItemIdentifiers = @[itemID];
		auto item = [[NSCustomTouchBarItem alloc] initWithIdentifier:itemID];
		item.view = sliderBinding.slider;
		touchbar.templateItems = [NSSet setWithObject:item];
		return touchbar;
	}

	void onPerformEdit (IValue&, IValue::Type newValue) override
	{
		if (sliderBinding)
		{
			sliderBinding.slider.doubleValue = newValue;
		}
	}

	ValuePtr value;
	MandelbrotTouchbarSliderBinding* sliderBinding {nil};
};
	
//------------------------------------------------------------------------
void installTouchbarSupport (IPlatformFrameTouchBarExtension* tbExt, const ValuePtr& value)
{
	tbExt->setTouchBarCreator (owned<ITouchBarCreator> (new TouchBarMaker (value)));
}
	
//------------------------------------------------------------------------
} // Mandelbrot


//------------------------------------------------------------------------
@implementation MandelbrotTouchbarSliderBinding

//------------------------------------------------------------------------
- (instancetype)init
{
	self = [super init];
	self->_slider = [NSSlider sliderWithTarget:self action:@selector(onSliderAction:)];
	return self;
}

//------------------------------------------------------------------------
- (void)onSliderAction:(id)sender
{
	self.value->performEdit ([sender doubleValue]);
}

//------------------------------------------------------------------------
@end

#endif

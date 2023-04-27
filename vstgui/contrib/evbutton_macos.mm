// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#import "evbutton.h"
#import "externalview_nsview.h"
#import "../lib/platform/mac/macstring.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace ExternalView {

//------------------------------------------------------------------------
struct ButtonDelegate : RuntimeObjCClass<ButtonDelegate>
{
	using ActionCallback = std::function<void ()>;

	static constexpr const auto ActionCallbackVarName = "ActionCallback";

	static id allocAndInit (ActionCallback&& actionCallback)
	{
		id obj = Base::alloc ();
		initWithCallbacks (obj, std::move (actionCallback));
		return obj;
	}

	static Class CreateClass ()
	{
		return ObjCClassBuilder ()
			.init ("ButtonDelegate", [NSObject class])
			.addMethod (@selector (onAction:), onAction)
			.addIvar<ActionCallback> (ActionCallbackVarName)
			.finalize ();
	}

	static id initWithCallbacks (id self, ActionCallback&& actionCallback)
	{
		if ((self = makeInstance (self).callSuper<id (), id> (@selector (init))))
		{
			auto instance = makeInstance (self);
			if (auto var = instance.getVariable<ActionCallback> (ActionCallbackVarName))
				var->set (actionCallback);
		}
		return self;
	}

	static void onAction (id self, SEL cmd, id sender)
	{
		if (auto var = makeInstance (self).getVariable<ActionCallback> (ActionCallbackVarName))
		{
			const auto& callback = var->get ();
			if (callback)
				callback ();
		}
	}
};

//------------------------------------------------------------------------
struct Button::Impl : ExternalNSViewBase<NSButton>,
					  IControlViewExtension
{
	using Base::Base;

	id delegate {nil};
	EditCallbacks callbacks {};

#if !__has_feature(objc_arc)
	~Impl () noexcept
	{
		if (delegate)
			[delegate release];
	}
#endif

	bool setValue (double value) override
	{
		if (value < 0.5)
			view.state = NSControlStateValueOff;
		else if (value == 0.5)
			view.state = NSControlStateValueMixed;
		else
			view.state = NSControlStateValueOn;
		return true;
	}

	bool setEditCallbacks (const EditCallbacks& editCallbacks) override
	{
		callbacks = editCallbacks;
		return true;
	}
};

//------------------------------------------------------------------------
Button::Button (Type type, const UTF8String& inTitle)
{
	NSString* title = fromUTF8String<NSString*> (inTitle);
	ButtonDelegate::ActionCallback actionCallback = [this] () {
		double value = 0.;
		switch (impl->view.state)
		{
			case NSControlStateValueOn:
				value = 1.;
				break;
			case NSControlStateValueOff:
				value = 0.;
				break;
			case NSControlStateValueMixed:
				value = 0.5;
				break;
		}
		if (impl->callbacks.beginEdit)
			impl->callbacks.beginEdit ();
		if (impl->callbacks.performEdit)
			impl->callbacks.performEdit (value);
		if (impl->callbacks.endEdit)
			impl->callbacks.endEdit ();
	};
	NSButton* button = {};
	switch (type)
	{
		case Type::Checkbox:
		{
			button = [NSButton checkboxWithTitle:title target:nullptr action:nullptr];
			break;
		}
		case Type::Push:
		{
			button = [NSButton buttonWithTitle:title target:nullptr action:nullptr];
			[button setButtonType:NSButtonTypeMomentaryLight];
			actionCallback = [this] () {
				if (impl->callbacks.beginEdit)
					impl->callbacks.beginEdit ();
				if (impl->callbacks.performEdit)
					impl->callbacks.performEdit (1.);
				if (impl->callbacks.endEdit)
					impl->callbacks.endEdit ();
				if (impl->callbacks.beginEdit)
					impl->callbacks.beginEdit ();
				if (impl->callbacks.performEdit)
					impl->callbacks.performEdit (0.);
				if (impl->callbacks.endEdit)
					impl->callbacks.endEdit ();
			};
			break;
		}
		case Type::OnOff:
		{
			button = [NSButton buttonWithTitle:title target:nullptr action:nullptr];
			[button setButtonType:NSButtonTypePushOnPushOff];
			break;
		}
		case Type::Radio:
		{
			button = [NSButton radioButtonWithTitle:title target:nullptr action:nullptr];
			break;
		}
	}
	[button sizeToFit];
	impl = std::make_unique<Impl> (button);
	impl->delegate = ButtonDelegate::allocAndInit (std::move (actionCallback));
	impl->view.target = impl->delegate;
	impl->view.action = @selector (onAction:);
	[impl->container addSubview:impl->view];
	[button retain];
}

//------------------------------------------------------------------------
Button::~Button () noexcept = default;

//------------------------------------------------------------------------
bool Button::platformViewTypeSupported (PlatformViewType type)
{
	return impl->platformViewTypeSupported (type);
}

//------------------------------------------------------------------------
bool Button::attach (void* parent, PlatformViewType parentViewType)
{
	return impl->attach (parent, parentViewType);
}

//------------------------------------------------------------------------
bool Button::remove () { return impl->remove (); }

//------------------------------------------------------------------------
void Button::setViewSize (IntRect frame, IntRect visible)
{
	static constexpr const NSControlSize controlSizes[] = {NSControlSizeRegular, NSControlSizeSmall,
														   NSControlSizeMini};
	for (auto i = 0; i < std::size (controlSizes); i++)
	{
		impl->view.controlSize = controlSizes[i];
		auto size = [impl->view sizeThatFits:NSMakeSize (frame.size.width, frame.size.height)];
		if (size.height <= frame.size.height)
			break;
	}
	impl->setViewSize (frame, visible);
}

//------------------------------------------------------------------------
void Button::setContentScaleFactor (double scaleFactor)
{
	impl->setContentScaleFactor (scaleFactor);
}

//------------------------------------------------------------------------
void Button::setMouseEnabled (bool state) { impl->setMouseEnabled (state); }

//------------------------------------------------------------------------
void Button::takeFocus () { impl->takeFocus (); }

//------------------------------------------------------------------------
void Button::looseFocus () { impl->looseFocus (); }

//------------------------------------------------------------------------
void Button::setTookFocusCallback (const TookFocusCallback& callback)
{
	impl->setTookFocusCallback (callback);
}

//------------------------------------------------------------------------
bool Button::setValue (double value) { return impl->setValue (value); }

//------------------------------------------------------------------------
bool Button::setEditCallbacks (const EditCallbacks& callbacks)
{
	return impl->setEditCallbacks (callbacks);
}

//------------------------------------------------------------------------
} // ExternalView
} // VSTGUI

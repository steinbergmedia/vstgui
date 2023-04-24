// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#import "datepicker.h"
#import "externalview_nsview.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace ExternalView {

//------------------------------------------------------------------------
struct DatePickerDelegate : RuntimeObjCClass<DatePickerDelegate>
{
	using DoneCallback = std::function<void ()>;
	using ValidateCallback = std::function<void (NSDate**, NSTimeInterval*)>;

	static constexpr const auto DoneCallbackVarName = "DoneCallback";
	static constexpr const auto ValidateCallbackVarName = "ValidateCallback";

	static id allocAndInit (DoneCallback&& doneCallback, ValidateCallback&& callback)
	{
		id obj = Base::alloc ();
		initWithCallbacks (obj, std::move (doneCallback), std::move (callback));
		return obj;
	}

	static Class CreateClass ()
	{
		return ObjCClassBuilder ()
			.init ("DatePickerDelegate", [NSObject class])
			.addProtocol ("NSDatePickerCellDelegate")
			.addMethod (@selector (datePickerCell:validateProposedDateValue:timeInterval:),
						validate)
			.addMethod (@selector (complete:), complete)
			.addIvar<ValidateCallback> (ValidateCallbackVarName)
			.addIvar<DoneCallback> (DoneCallbackVarName)
			.finalize ();
	}

	static id initWithCallbacks (id self, DoneCallback&& doneCallback, ValidateCallback&& callback)
	{
		if ((self = makeInstance (self).callSuper<id (), id> (@selector (init))))
		{
			auto instance = makeInstance (self);
			if (auto var = instance.getVariable<DoneCallback> (DoneCallbackVarName))
				var->set (doneCallback);
			if (auto var = instance.getVariable<ValidateCallback> (ValidateCallbackVarName))
				var->set (callback);
		}
		return self;
	}

	static void complete (id self, SEL cmd, id sender)
	{
		if (auto var = makeInstance (self).getVariable<DoneCallback> (DoneCallbackVarName))
		{
			const auto& callback = var->get ();
			if (callback)
				callback ();
		}
	}

	static void validate (id self, SEL cmd, NSDatePickerCell* datePickerCell,
						  NSDate* _Nonnull* _Nonnull proposedDateValue,
						  NSTimeInterval* _Nullable proposedTimeInterval)
	{
		if (auto var = makeInstance (self).getVariable<ValidateCallback> (ValidateCallbackVarName))
		{
			const auto& callback = var->get ();
			if (callback)
				callback (proposedDateValue, proposedTimeInterval);
		}
	}
};

//------------------------------------------------------------------------
struct DatePicker::Impl : ExternalNSViewBase<NSDatePicker>
{
	using Base::Base;

	id delegate {nil};
	ChangeCallback changeCallback;

#if !__has_feature(objc_arc)
	~Impl () noexcept
	{
		if (delegate)
			[delegate release];
	}
#endif
};

//------------------------------------------------------------------------
DatePicker::DatePicker ()
{
	impl = std::make_unique<Impl> ([[NSDatePicker alloc] initWithFrame: {0., 0., 10., 10.}]);
	impl->view.datePickerStyle = NSDatePickerStyleTextField;
	impl->view.datePickerMode = NSDatePickerModeSingle;
	impl->view.datePickerElements = NSDatePickerElementFlagYearMonthDay;
	if (@available (macOS 10.15.4, *))
		impl->view.presentsCalendarOverlay = YES;
	impl->view.dateValue = [NSDate date];
	impl->view.calendar = [NSCalendar currentCalendar];
	[impl->container addSubview:impl->view];

	impl->delegate = DatePickerDelegate::allocAndInit (
		[impl = impl.get ()] () {
			if (impl->changeCallback)
			{
				auto dateValue = impl->view.dateValue;
				auto calendar = impl->view.calendar;
				auto components = [calendar
					components:NSCalendarUnitYear | NSCalendarUnitMonth | NSCalendarUnitDay
					  fromDate:dateValue];
				Date date;
				date.day = static_cast<int32_t> (components.day);
				date.month = static_cast<int32_t> (components.month);
				date.year = static_cast<int32_t> (components.year);
				impl->changeCallback (date);
			}
		},
		[] (NSDate** date, NSTimeInterval* time) {
			// TODO: add validation mechanism
		});
	impl->view.delegate = impl->delegate;
	impl->view.target = impl->delegate;
	impl->view.action = @selector (complete:);
}

//------------------------------------------------------------------------
DatePicker::~DatePicker () noexcept {}

//------------------------------------------------------------------------
void DatePicker::setDate (Date date)
{
	auto calendar = impl->view.calendar;
	auto dateComponents = [NSDateComponents new];
	dateComponents.calendar = calendar;
	dateComponents.day = date.day;
	dateComponents.month = date.month;
	dateComponents.year = date.year;
	impl->view.dateValue = [calendar dateFromComponents:dateComponents];
}

//------------------------------------------------------------------------
void DatePicker::setChangeCallback (const ChangeCallback& callback)
{
	impl->changeCallback = callback;
}

//------------------------------------------------------------------------
bool DatePicker::platformViewTypeSupported (PlatformViewType type)
{
	return impl->platformViewTypeSupported (type);
}

//------------------------------------------------------------------------
bool DatePicker::attach (void* parent, PlatformViewType parentViewType)
{
	return impl->attach (parent, parentViewType);
}

//------------------------------------------------------------------------
bool DatePicker::remove () { return impl->remove (); }

//------------------------------------------------------------------------
void DatePicker::setViewSize (IntRect frame, IntRect visible)
{
	impl->setViewSize (frame, visible);
}

//------------------------------------------------------------------------
void DatePicker::setContentScaleFactor (double scaleFactor)
{
	impl->setContentScaleFactor (scaleFactor);
}

//------------------------------------------------------------------------
void DatePicker::setMouseEnabled (bool state) { impl->setMouseEnabled (state); }

//------------------------------------------------------------------------
void DatePicker::takeFocus () { impl->takeFocus (); }

//------------------------------------------------------------------------
void DatePicker::looseFocus () { impl->looseFocus (); }

//------------------------------------------------------------------------
void DatePicker::setTookFocusCallback (const TookFocusCallback& callback)
{
	impl->setTookFocusCallback (callback);
}

//------------------------------------------------------------------------
} // ExternalView
} // VSTGUI

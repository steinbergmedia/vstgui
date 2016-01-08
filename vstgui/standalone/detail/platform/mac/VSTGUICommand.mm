#import "VSTGUICommand.h"

//------------------------------------------------------------------------
@implementation VSTGUICommand

- (const VSTGUI::Standalone::Detail::CommandWithKey&)command { return self->_cmd; }

@end

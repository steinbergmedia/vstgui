#pragma once

#import <Foundation/Foundation.h>
#import "../../application.h"

//------------------------------------------------------------------------
@interface VSTGUICommand : NSObject
@property VSTGUI::Standalone::Detail::CommandWithKey cmd;

- (const VSTGUI::Standalone::Detail::CommandWithKey&)command;
@end

#pragma once

#import "../../application.h"
#import <Foundation/Foundation.h>

//------------------------------------------------------------------------
@interface VSTGUICommand : NSObject
@property VSTGUI::Standalone::Detail::CommandWithKey cmd;

- (const VSTGUI::Standalone::Detail::CommandWithKey&)command;
@end

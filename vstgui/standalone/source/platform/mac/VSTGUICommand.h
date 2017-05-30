// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#import "../../application.h"
#import <Foundation/Foundation.h>

//------------------------------------------------------------------------
@interface VSTGUICommand : NSObject
@property VSTGUI::Standalone::Detail::CommandWithKey cmd;

- (const VSTGUI::Standalone::Detail::CommandWithKey&)command;
@end

// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "fwd.h"
#include <functional>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
/** %asynchronous tasks
 *	@ingroup standalone
 */
namespace Async {

using Task = std::function<void ()>;

//------------------------------------------------------------------------
/** Asynchronous context. */
enum class Context
{
	/** Main thread context. */
	Main,
	/** Background thread context. */
	Background
};

//------------------------------------------------------------------------
/** Schedule a task to be performed asynchronous either on a background thread or on the main
 *	thread.
 *
 *	Can be called from any thread, but should not be called from realtime constraint threads as it
 *	may involves locks and memory allocations
 *
 *	@param context	background or main thread
 *	@param task		task to be performed
 */
void perform (Context context, Task&& task);

//------------------------------------------------------------------------
} // Async
} // Standalone
} // VSTGUI

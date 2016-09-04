#pragma once

#include "fwd.h"
#include <functional>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Async {

using Task = std::function<void ()>;

//------------------------------------------------------------------------
/** Asynchronous Context */
enum class Context
{
	Main,
	Background
};

//------------------------------------------------------------------------
/** Perform a task asynchronously.
 *
 *	Schedule a task to be performed asynchronous either on a background thread or on the main
 *	thread.
 *
 *	Can be called from any thread.
 *
 *	@param context	background or main thread
 *	@param task		task to be performed
 */
void perform (Context context, Task&& task);

//------------------------------------------------------------------------
} // Async
} // Standalone
} // VSTGUI

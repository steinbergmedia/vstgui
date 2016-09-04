#include "../../../iasync.h"
#include <dispatch/dispatch.h>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Async {

//------------------------------------------------------------------------
void perform (Context context, Task&& task)
{
	dispatch_queue_t queue;
	switch (context)
	{
		case Context::Main:
		{
			queue = dispatch_get_main_queue ();
			break;
		}
		case Context::Background:
		{
			queue = dispatch_get_global_queue (QOS_CLASS_UTILITY, 0);
			break;
		}
	}
	dispatch_async (queue, [task = std::move (task)] () {
		// execute
		task ();
	});
}

//------------------------------------------------------------------------
} // Async
} // Standalone
} // VSTGUI

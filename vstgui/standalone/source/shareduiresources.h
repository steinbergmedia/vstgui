#pragma once

#include "../../uidescription/uidescriptionfwd.h"
#include "../../lib/optional.h"
#include "../include/ishareduiresources.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Detail {

//------------------------------------------------------------------------
const ISharedUIResources& getSharedUIResources ();

//------------------------------------------------------------------------
SharedPointer<UIDescription> getSharedUIDescription ();

//------------------------------------------------------------------------
void cleanupSharedUIResources ();

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
#if VSTGUI_LIVE_EDITING
void saveSharedUIDescription ();

//------------------------------------------------------------------------
struct IEditFileMap
{
	virtual void set (const std::string& filename, const std::string& absolutePath) = 0;
	virtual Optional<const char*> get (const std::string& filename) const = 0;
};

//------------------------------------------------------------------------
IEditFileMap& getEditFileMap ();

//------------------------------------------------------------------------
enum class UIDescCheckFilePathResult
{
	exists,
	newPathSet,
	cancel
};

//------------------------------------------------------------------------
UIDescCheckFilePathResult checkAndUpdateUIDescFilePath (
    UIDescription& uiDesc, CFrame* _frame,
    UTF8StringPtr notFoundText = "The uidesc file location cannot be found.");

//------------------------------------------------------------------------
bool initUIDescAsNew (UIDescription& uiDesc, CFrame* _frame);

#endif // VSTGUI_LIVE_EDITING

//------------------------------------------------------------------------
} // Detail
} // Standalone
} // VSTGUI

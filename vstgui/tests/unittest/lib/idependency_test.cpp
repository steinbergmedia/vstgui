// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../unittests.h"

#if VSTGUI_ENABLE_DEPRECATED_METHODS
#include "../../../lib/private/disabledeprecatedmessage.h"

#include "../../../lib/idependency.h"

namespace VSTGUI {

class DependentObject : public CBaseObject, public IDependency
{
public:
	DependentObject () : notifyCalledCount (0) {}

	CMessageResult notify (CBaseObject* sender, IdStringPtr message) override
	{
		notifyCalledCount++;
		return kMessageNotified;
	}

	int32_t notifyCalledCount;
};

class TestObject : public CBaseObject, public IDependency
{
public:
	TestObject () {}
};

TEST_CASE (IDependencyTest, SimpleDependency)
{
	DependentObject dObj;
	TestObject tObj;
	tObj.addDependency (&dObj);
	tObj.changed ("Test");
	EXPECT (dObj.notifyCalledCount == 1)
	tObj.removeDependency (&dObj);
}

TEST_CASE (IDependencyTest, SimpleDeferedDependency)
{
	DependentObject dObj;
	TestObject tObj;
	tObj.addDependency (&dObj);
	tObj.deferChanges (true);
	tObj.changed ("Test");
	EXPECT (dObj.notifyCalledCount == 0)
	tObj.deferChanges (false);
	EXPECT (dObj.notifyCalledCount == 1)
	tObj.removeDependency (&dObj);
}

TEST_CASE (IDependencyTest, SimpleDeferChanges)
{
	DependentObject dObj;
	TestObject tObj;
	tObj.addDependency (&dObj);
	{
		IDependency::DeferChanges df (&tObj);
		IdStringPtr messageID = "Test";
		tObj.changed (messageID);
		tObj.changed (messageID);
		EXPECT (dObj.notifyCalledCount == 0)
	}
	EXPECT (dObj.notifyCalledCount == 1)
	tObj.removeDependency (&dObj);
}

} // VSTGUI

#include "../../../lib/private/enabledeprecatedmessage.h"
#endif // VSTGUI_ENABLE_DEPRECATED_METHODS

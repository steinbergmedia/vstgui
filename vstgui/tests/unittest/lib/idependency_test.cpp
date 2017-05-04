// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../unittests.h"
#include "../../../lib/idependency.h"

namespace VSTGUI {

class DependentObject : public CBaseObject, public IDependency
{
public:
	DependentObject ()
	: notifyCalledCount (0)
	{
		
	}
	
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
	TestObject ()
	{
		
	}

};

TESTCASE(IDependencyTest,

	TEST(simpleDependency,
		DependentObject dObj;
		TestObject tObj;
		tObj.addDependency (&dObj);
		tObj.changed ("Test");
		EXPECT(dObj.notifyCalledCount == 1)
		tObj.removeDependency(&dObj);
	);

	TEST(simpleDeferedDependency,
		DependentObject dObj;
		TestObject tObj;
		tObj.addDependency (&dObj);
		tObj.deferChanges (true);
		tObj.changed ("Test");
		EXPECT(dObj.notifyCalledCount == 0)
		tObj.deferChanges (false);
		EXPECT(dObj.notifyCalledCount == 1)
		tObj.removeDependency (&dObj);
	);

	TEST(simpleDeferChanges,
		DependentObject dObj;
		TestObject tObj;
		tObj.addDependency (&dObj);
		{
			IDependency::DeferChanges df (&tObj);
			tObj.changed ("Test");
			tObj.changed ("Test");
			EXPECT(dObj.notifyCalledCount == 0)
		}
		EXPECT(dObj.notifyCalledCount == 1)
		tObj.removeDependency (&dObj);
	);
);

} // namespace

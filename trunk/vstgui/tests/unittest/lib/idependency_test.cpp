//
//  idependency_test.cpp
//  vstgui
//
//  Created by Arne Scheffler on 5/29/13.
//
//

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
	
	CMessageResult notify (CBaseObject* sender, IdStringPtr message)
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

	TEST(simpleDependencyTest,
		DependentObject dObj;
		TestObject tObj;
		tObj.addDependency (&dObj);
		tObj.changed ("Test");
		EXPECT(dObj.notifyCalledCount == 1)
		tObj.removeDependency(&dObj);
	);

	TEST(simpleDeferedDependencyTest,
		DependentObject dObj;
		TestObject tObj;
		tObj.addDependency (&dObj);
		tObj.deferChanges (true);
		tObj.changed ("Test");
		EXPECT(dObj.notifyCalledCount == 0)
		tObj.deferChanges (false);
		EXPECT(dObj.notifyCalledCount == 1)
	);

	TEST(simpleDeferChangesTest,
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
	);
);

} // namespace

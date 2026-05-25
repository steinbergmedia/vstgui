// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../unittests.h"
#include "../../../lib/weakptr.h"

namespace VSTGUI {
namespace {

//------------------------------------------------------------------------
struct WeakableObject : public NonAtomicReferenceCounted,
						public WeakPointerSupport<WeakableObject>
{
	int32_t value {0};
	explicit WeakableObject (int32_t v = 0) : value (v) {}
	WeakPointer<WeakableObject> weakFromThisPublic () { return shared (this); }

	virtual int32_t getValue () const { return value; }
};

struct WeakableObject2 : public NonAtomicReferenceCounted,
						 public WeakPointerSupport<WeakableObject2>
{
	int32_t value {0};
	explicit WeakableObject2 (int32_t v = 0) : value (v) {}
	WeakPointer<WeakableObject2> weakFromThisPublic () { return shared (this); }

	virtual int32_t getValue () const { return value; }
};

//------------------------------------------------------------------------
struct WeakObject2 : public WeakableObject
{
	using WeakableObject::WeakableObject;

	int32_t getValue () const override { return 100; }
};

//------------------------------------------------------------------------
SharedPointer<WeakableObject> makeTestObject (int32_t v = 42)
{
	return makeShared<WeakableObject> (v);
}

} // anonymous

TEST_CASE (WeakPointerTest, ConstructFromSharedPointerAndLock)
{
	auto obj = makeTestObject (123);
	WeakPointer<WeakableObject> wp {obj};
	EXPECT (wp.expired () == false);
	auto locked = wp.lock ();
	EXPECT (locked != nullptr);
	EXPECT (locked->value == 123);
}

TEST_CASE (WeakPointerTest, ExpireOnDestruction)
{
	WeakPointer<WeakableObject> wp;
	{
		auto obj = makeTestObject (11);
		wp = obj;
		EXPECT (wp.expired () == false);
	}
	EXPECT (wp.expired () == true);
	auto locked = wp.lock ();
	EXPECT (locked == nullptr);
}

TEST_CASE (WeakPointerTest, CopyConstructAndAssign)
{
	auto obj = makeTestObject (7);
	WeakPointer<WeakableObject> wp1 {obj};
	WeakPointer<WeakableObject> wp2 {wp1};
	EXPECT (wp1.expired () == false);
	EXPECT (wp2.expired () == false);
	EXPECT (wp1.lock ().get () == wp2.lock ().get ());

	WeakPointer<WeakableObject> wp3;
	wp3 = wp1;
	EXPECT (wp3.expired () == false);
	EXPECT (wp1.lock ().get () == wp3.lock ().get ());
}

TEST_CASE (WeakPointerTest, AssignFromSharedPointer)
{
	WeakPointer<WeakableObject> wp;
	{
		auto obj = makeTestObject (5);
		wp = obj;
		EXPECT (wp.expired () == false);
		EXPECT (wp.lock ()->value == 5);
	}
	EXPECT (wp.expired ());
}

TEST_CASE (WeakPointerTest, Reset)
{
	auto obj = makeTestObject (9);
	WeakPointer<WeakableObject> wp {obj};
	EXPECT (wp.expired () == false);
	wp.reset ();
	EXPECT (wp.expired () == true);
	EXPECT (wp.lock () == nullptr);
}

TEST_CASE (WeakPointerTest, Swap)
{
	auto obj1 = makeTestObject (1);
	auto obj2 = makeTestObject (2);
	WeakPointer<WeakableObject> wp1 {obj1};
	WeakPointer<WeakableObject> wp2 {obj2};
	wp1.swap (wp2);
	EXPECT (wp1.lock ()->value == 2);
	EXPECT (wp2.lock ()->value == 1);
}

TEST_CASE (WeakPointerTest, EqualityOperator)
{
	auto obj = makeTestObject (3);
	WeakPointer<WeakableObject> a {obj};
	WeakPointer<WeakableObject> b {obj};
	EXPECT ((a.lock () == b.lock ()) == true);
	auto obj2 = makeTestObject (4);
	WeakPointer<WeakableObject> c {obj2};
	EXPECT ((a.lock () == c.lock ()) == false);
}

TEST_CASE (WeakPointerTest, WeakFromThis)
{
	auto obj = makeTestObject (77);
	WeakPointer<WeakableObject> wp = obj->weakFromThisPublic ();
	EXPECT (wp.expired () == false);
	EXPECT (wp.lock ()->value == 77);
	obj.reset ();
	EXPECT (wp.expired () == true);
}

TEST_CASE (WeakPointerTest, MoveConstruct)
{
	auto obj = makeTestObject (88);
	WeakPointer<WeakableObject> src {obj};
	EXPECT (src.expired () == false);

	WeakPointer<WeakableObject> dst {std::move (src)};

	EXPECT (src.expired () == true);
	EXPECT (dst.expired () == false);
	auto locked = dst.lock ();
	EXPECT (locked != nullptr);
	EXPECT (locked->value == 88);
}

TEST_CASE (WeakPointerTest, MoveAssign)
{
	auto obj1 = makeTestObject (101);
	auto obj2 = makeTestObject (202);

	WeakPointer<WeakableObject> a {obj1};
	WeakPointer<WeakableObject> b {obj2};

	// Move-assign: transfer b into a
	a = std::move (b);

	// Source should be expired after move
	EXPECT (b.expired () == true);

	// Destination should now reference obj2
	EXPECT (a.expired () == false);
	auto locked = a.lock ();
	EXPECT (locked != nullptr);
	EXPECT (locked->value == 202);

	EXPECT (obj1->weakFromThisPublic ().lock ()->value == 101);
}

#if 1
TEST_CASE (WeakPointerTest, Inheritance)
{
	auto obj = makeShared<WeakObject2> (1);
	auto obj2 = obj.cast<WeakableObject> ();
	WeakPointer<WeakObject2> weakPtr = obj2;
	auto objPtr = weakPtr.lock ();
	EXPECT (objPtr != nullptr);
	EXPECT (objPtr->getValue () == 100);
	auto obj3 = makeShared<WeakableObject2> (1);
}
#endif

} // VSTGUI

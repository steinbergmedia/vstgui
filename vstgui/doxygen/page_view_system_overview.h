/**

@page the_view_system The view system overview

A view is a rectangular section which is contained in a window.
It is responsible for handling user events and drawing to the screen.
In VSTGUI this is the CView class.  
Another aspect of a view is that it can be part of a parent view.
The main parent view is always a CFrame object. It is itself a view and it is a
view container which can contain one or more child views.  
If you need to group multiple views you can use the CViewContainer class.
It is also a view but it can have child views.

@section inherit_from_cview Inherit from CView

If you want to draw custom data which cannot be presented via the included
CView subclasses you need to create a new subclass of CView:

~~~~~~~~~~~~~{.cpp}
class MyView : public CView
{
public:
  MyView (const CRect& size) : CView (size) {}
};
~~~~~~~~~~~~~

If you want to draw some custom stuff, you need to override the draw method:

~~~~~~~~~~~~~{.cpp}
class MyView : public CView
{
public:
  MyView (const CRect& size) : CView (size) {}
  void draw (CDrawContext* context) override
  {}
};
~~~~~~~~~~~~~

As you see, the draw method has a CDrawContext argument. The draw context has all the methods to
draw stuff into the view. Below I just show you a very simple thing, drawing a green line.

~~~~~~~~~~~~~{.cpp}
void MyView::draw (CDrawContext* context) override
{
  context->setFrameColor (kGreenColor);
  context->drawLine (CPoint (0,0), CPoint (10, 10));
}
~~~~~~~~~~~~~

Now a little strange concept of VSTGUI is that the draw context is not automatically adjusted to the
position of the view, so that if the position of the view is at x=50 and y=50, you will not see the line
(because it is drawn from 0,0 to 10,10 and that is outside the views boundaries).
So you need to offset your drawing by the position of the view.

~~~~~~~~~~~~~{.cpp}
void MyView::draw (CDrawContext* context) override
{
  auto viewPos = getViewSize ().getTopLeft ();
  CDrawContext::Transform t (*context, CGraphicsTransform ().translate (viewPos));

  context->setFrameColor (kGreenColor);
  context->drawLine (CPoint (0,0), CPoint (10, 10));
}
~~~~~~~~~~~~~

If CDrawContext::Transform is not defined in your VSTGUI version you have a version older than 4.2
and you need to offset the points yourself.

If you have a complex view which takes a long time to draw, you can also override the CView::drawRect (CDrawContext*, const CRect&) method
and only draw the region which needs to be drawn.

@section handling_mouse_events Handling mouse events

Next you may want to add user interactions via mouse events.  
For this you have to override two or three methods depending on if you want to track mouse movement or just want to get a mouse click.
For a simple mouse click you have to do this :

~~~~~~~~~~~~~{.cpp}
class MyView : public CView
{
public:
  CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override
  {
    return kMouseEventHandled; // needed to get the onMouseUp call
  }
  CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override
  {
    if (buttons.isLeftButton () && getViewSize ().pointInside (where))
      doMouseClick ();
    return kMouseEventHandled;
  }
  void doMouseClick () {}
};
~~~~~~~~~~~~~

If a user clicks inside MyView the doMouseClick() method is called.  

If you need to track mouse movement in your view you have to additionally override the onMouseMoved method:

~~~~~~~~~~~~~{.cpp}
class MyView : public CView
{
public:
  CMouseEventResult onMouseMove (CPoint& where, const CButtonState& buttons) override
  {
    return kMouseEventHandled;
  }
};
~~~~~~~~~~~~~

This method is always called if the mouse moves inside your view, even if no buttons are down.  
So if you want to track the mouse only if the left button is down you have to check for it with buttons.isLeftButton () inside that method.

*/
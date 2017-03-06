//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "cairopath.h"
#include "../../cgradient.h"
#include "../../cgraphicstransform.h"
#include "cairocontext.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Cairo {

//------------------------------------------------------------------------
Path::Path (const ContextHandle& cr) noexcept : cr (cr)
{
}

//------------------------------------------------------------------------
Path::~Path () noexcept
{
	dirty ();
}

//------------------------------------------------------------------------
CGradient* Path::createGradient (double color1Start, double color2Start,
								 const VSTGUI::CColor& color1, const VSTGUI::CColor& color2)
{
	return CGradient::create (color1Start, color2Start, color1, color2);
}

//------------------------------------------------------------------------
bool Path::hitTest (const CPoint& p, bool evenOddFilled, CGraphicsTransform* transform)
{
#warning TODO: Implementation (use cairo_in_fill)
	(void)p;
	(void)evenOddFilled;
	(void)transform;
	return false;
}

//------------------------------------------------------------------------
CPoint Path::getCurrentPosition ()
{
	CPoint p;
	if (auto cPath = getPath (cr))
	{
		cairo_save (cr);
		cairo_new_path (cr);
		cairo_append_path (cr, cPath);
		cairo_get_current_point (cr, &p.x, &p.y);
		cairo_restore (cr);
	}
	return p;
}

//------------------------------------------------------------------------
CRect Path::getBoundingBox ()
{
	CRect r;
	if (auto cPath = getPath (cr))
	{
		cairo_save (cr);
		cairo_new_path (cr);
		cairo_append_path (cr, cPath);
		CPoint p1, p2;
		cairo_path_extents (cr, &p1.x, &p1.y, &p2.x, &p2.y);
		cairo_restore (cr);
		r.setTopLeft (p1);
		r.setBottomRight (p2);
	}
	return r;
}

//------------------------------------------------------------------------
void Path::dirty ()
{
	if (path)
	{
		cairo_path_destroy (path);
		path = nullptr;
	}
}

//------------------------------------------------------------------------
cairo_path_t* Path::getPath (const ContextHandle& handle, const CGraphicsTransform* alignTm)
{
	if (alignTm)
		dirty ();
	if (!path)
	{
		cairo_new_path (handle);
		for (auto& e : elements)
		{
			switch (e.type)
			{
				case Element::Type::kBeginSubpath:
				{
					cairo_new_sub_path (handle);
					if (alignTm)
					{
						auto p = pixelAlign (*alignTm,
											 CPoint {e.instruction.point.x, e.instruction.point.y});
						cairo_move_to (handle, p.x - 0.5, p.y - 0.5);
					}
					else
						cairo_move_to (handle, e.instruction.point.x, e.instruction.point.y);
					break;
				}
				case Element::Type::kCloseSubpath:
				{
					cairo_close_path (handle);
					break;
				}
				case Element::Type::kLine:
				{
					if (alignTm)
					{
						auto p = pixelAlign (*alignTm,
											 CPoint {e.instruction.point.x, e.instruction.point.y});
						cairo_line_to (handle, p.x - 0.5, p.y - 0.5);
					}
					else
						cairo_line_to (handle, e.instruction.point.x, e.instruction.point.y);
					break;
				}
				case Element::Type::kBezierCurve:
				{
					cairo_curve_to (handle, e.instruction.curve.control1.x,
									e.instruction.curve.control1.y, e.instruction.curve.control2.x,
									e.instruction.curve.control2.y, e.instruction.curve.end.x,
									e.instruction.curve.end.y);
					break;
				}
				case Element::Type::kRect:
				{
					if (alignTm)
					{
						auto r = pixelAlign (
							*alignTm, CRect {e.instruction.rect.left, e.instruction.rect.top,
											 e.instruction.rect.right, e.instruction.rect.bottom});
						cairo_rectangle (handle, r.left - 0.5, r.top - 0.5, r.getWidth (),
										 r.getHeight ());
					}
					else
					{
						cairo_rectangle (handle, e.instruction.rect.left, e.instruction.rect.top,
										 e.instruction.rect.right - e.instruction.rect.left,
										 e.instruction.rect.bottom - e.instruction.rect.top);
					}
					break;
				}
				case Element::Type::kEllipse: {
#warning TODO: Implementation Element::Type::kEllipse
					break;
				}
				case Element::Type::kArc:
				{
					auto radiusX =
						(e.instruction.arc.rect.right - e.instruction.arc.rect.left) / 2.;
					auto radiusY =
						(e.instruction.arc.rect.bottom - e.instruction.arc.rect.top) / 2.;

					auto centerX = static_cast<double> (e.instruction.arc.rect.left + radiusX);
					auto centerY = static_cast<double> (e.instruction.arc.rect.top + radiusY);

					double startAngle = radians (e.instruction.arc.startAngle);
					double endAngle = radians (e.instruction.arc.endAngle);
					if (radiusX != radiusY)
					{
						startAngle = atan2 (sin (startAngle) * radiusX, cos (startAngle) * radiusY);
						endAngle = atan2 (sin (endAngle) * radiusX, cos (endAngle) * radiusY);
					}
					cairo_matrix_t matrix;
					cairo_get_matrix (handle, &matrix);
					cairo_translate (handle, centerX, centerY);
					cairo_scale (handle, radiusX, radiusY);
					if (e.instruction.arc.clockwise)
					{
						cairo_arc (handle, 0, 0, 1, startAngle, endAngle);
					}
					else
					{
						cairo_arc_negative (handle, 0, 0, 1, startAngle, endAngle);
					}
					cairo_set_matrix (handle, &matrix);
					break;
				}
			}
		}
		path = cairo_copy_path (handle);
		cairo_new_path (handle); // clear path
	}
	return path;
}

//------------------------------------------------------------------------
} // Cairo
} // VSTGUI

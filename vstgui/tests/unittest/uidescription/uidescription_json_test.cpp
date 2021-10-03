// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../lib/cbitmap.h"
#include "../../../lib/ccolor.h"
#include "../../../lib/cgradient.h"
#include "../../../lib/cviewcontainer.h"
#include "../../../uidescription/detail/uiviewcreatorattributes.h"
#include "../../../uidescription/uiattributes.h"
#include "../../../uidescription/uicontentprovider.h"
#include "uidescription_test_helper.h"

namespace VSTGUI {
using namespace UIDescriptionTesting;

namespace {

//------------------------------------------------------------------------
static constexpr auto defaultSafeFlags = UIDescription::kWriteImagesIntoUIDescFile;

constexpr auto emptyUIDesc = R"(
{
	"vstgui-ui-description": {
		"version": "1"
	}
}
)";

constexpr auto colorNodesUIDesc = R"(
{
	"vstgui-ui-description": {
		"version": "1",
		"colors": {
			"c1": "#000000ff",
			"c2": "#ffffffff",
			"c3": "#ff000064",
			"c4": "#00ff0096",
			"c5": "#ff00ff64"
		}
	}
}
)";

constexpr auto fontNodesUIDesc = R"(
{
	"vstgui-ui-description": {
		"version": "1",
		"fonts": {
			"f1": {
				"font-name": "Arial",
				"size": "8"
			},
			"f2": {
				"bold": "true",
				"font-name": "Arial",
				"size": "8"
			},
			"f3": {
				"font-name": "Arial",
				"italic": "true",
				"size": "8"
			},
			"f4": {
				"font-name": "Arial",
				"size": "8",
				"underline": "true"
			},
			"f5": {
				"font-name": "Arial",
				"size": "8",
				"strike-through": "true"
			},
			"f6": {
				"alternative-font-names": "Arial, Courier",
				"font-name": "bla",
				"size": "8"
			}
		}
	}
}
)";

constexpr auto bitmapNodesUIDesc = R"(
{
	"vstgui-ui-description": {
		"version": "1",
		"bitmaps": {
			"b1": {
				"path": "b1.png"
			},
			"b1#2.0x": {
				"path": "b1#2.0x.png",
				"scale-factor": "2"
			}
		}
	}
}
)";

constexpr auto tagNodesUIDesc = R"(
{
	"vstgui-ui-description": {
		"version": "1",
		"control-tags": {
			"t1": "1234",
			"t2": "4321",
			"t3": "'mytg'"
		}
	}
}
)";

constexpr auto calculateTagNodesUIDesc = R"(
{
	"vstgui-ui-description": {
		"version": "1",
		"control-tags": {
			"t1": "1+2"
		}
	}
}
)";

constexpr auto gradientNodesUIDesc = R"(
{
	"vstgui-ui-description": {
		"version": "1",
		"gradients": {
			"g1": [
				{
					"rgba": "#000000ff",
					"start": "0"
				},
				{
					"rgba": "#ff0000ff",
					"start": "0.5"
				},
				{
					"rgba": "#ffffffff",
					"start": "1"
				}
			]
		}
	}
}
)";

constexpr auto variableNodesUIDesc = R"(
{
	"vstgui-ui-description": {
		"version": "1",
		"variables": {
			"v1": "10",
			"v2": "string",
			"v3": "string",
			"v4": "20.5",
			"v5": "2*var.v1",
			"v6": ""
		}
	}
}
)";

constexpr auto withAllNodesUIDesc = R"({
	"vstgui-ui-description": {
		"version": "1",
		"variables": {
			"test": "10",
			"test": "this is a string"
		},
		"bitmaps": {
			"b1": {
				"path": "b1.png",
				"data": {
					"encoding": "base64",
					"data": "iVBORw0KGgoAAAANSUhEUgAAAAwAAAAMCAYAAABWdVznAAABe2lDQ1BJQ0MgUHJvZmlsZQAAKJF9kE0rRFEYx38zXjNkwcLC4jaG1RCjvGyUmYSaxTRGedvcueZFmXG7c4VsLJTtFCU23hZ8AjYWylopRUrKVyA20vUcQ+OlPHXO+Z3nPM+/5/zB7ddNc7a0HTJZ24oOBrWx8Qmt4gEX5XjQ8OpGzuyPRMJIfJ0/4+VaqiWuWpXW3/d/wzOdyBngqhTuM0zLFh4SblqwTcVKr96SoYRXFKcKvKE4XuCjj5pYNCR8KqwZaX1a+E7Yb6StDLiVvi/+rSb1jTOz88bnPOon1Yns6IicXlmN5IgySFC8GGaAEF100Ct7F60EaJMbdmLRVs2hOXPJmkmlba1fnEhow1mjza8F2ju6Qfn6269ibm4Xep6hJF/MxTfhZA0abos53w7UrsLxualb+keqRJY7mYTHQ6gZh7pLqJrMJTsDhR9VB6Hs3nGemqFiHd7yjvO65zhv+9IsHp1lCx59anFwA7FlCF/A1ja0iHbt1Dv7WWccXX/QZQAAAExJREFUKBVjZEAALSAzDMFFYa0C8q6BRJhQhBEcUSAThIkGDUCVIIwBcNmAoRAmMKoBFhL4aEagJLYYhkXaazTNq1jQBGBcdIUwcQYAOGIGVqwWW9EAAAAASUVORK5CYII="
				}
			},
			"b1#2.0x": {
				"path": "b1#2.0x.png",
				"scale-factor": "2"
			},
			"dataBitmap": {
				"path": "dataBitmap.png"
			}
		},
		"fonts": {
			"f1": {
				"font-name": "Arial",
				"size": "8"
			},
			"f2": {
				"bold": "true",
				"font-name": "Arial",
				"size": "8"
			}
		},
		"colors": {
			"c1": "#000000ff",
			"c2": "#ffffffff",
			"c3": "#ff000064"
		},
		"gradients": {
			"g1": [
				{
					"rgba": "#000000ff",
					"start": "0"
				},
				{
					"rgba": "#ff0000ff",
					"start": "0.5"
				},
				{
					"rgba": "#ffffffff",
					"start": "1"
				}
			]
		},
		"control-tags": {
			"t1": "1234",
			"t2": "4321"
		}
	}
})";

constexpr auto createViewUIDesc = R"(
{
	"vstgui-ui-description": {
		"version": "1",
		"templates": {
			"view": {
				"attributes": {
					"background-color": "~ TransparentCColor",
					"background-color-draw-style": "filled and stroked",
					"class": "CViewContainer",
					"mouse-enabled": "true",
					"opacity": "1",
					"origin": "0, 0",
					"size": "400, 235",
					"transparent": "false"
				},
				"children": {
					"CView": {
						"attributes": {
							"class": "CView",
							"mouse-enabled": "true",
							"opacity": "1",
							"origin": "4, 10",
							"size": "392, 40",
							"transparent": "false"
						}
					}
				}
			}
		}
	}
}
)";

constexpr auto restoreViewUIDesc = R"(
{
	"vstgui-ui-description": {
		"version": "1",
		"templates": {
			"view": {
				"attributes": {
					"background-color": "~ TransparentCColor",
					"background-color-draw-style": "filled and stroked",
					"class": "CViewContainer",
					"mouse-enabled": "true",
					"opacity": "1",
					"origin": "0, 0",
					"size": "400, 235",
					"transparent": "false"
				},
				"children": {
					"CViewContainer": {
						"attributes": {
							"class": "CViewContainer",
							"mouse-enabled": "true",
							"opacity": "1",
							"origin": "4, 10",
							"size": "392, 40",
							"transparent": "false"
						}
					},
					"CViewContainer": {
						"attributes": {
							"class": "CViewContainer",
							"mouse-enabled": "true",
							"opacity": "1",
							"origin": "4, 10",
							"size": "392, 40",
							"transparent": "false"
						},
						"children": {
							"CView": {
								"attributes": {
									"class": "CView",
									"mouse-enabled": "true",
									"opacity": "1",
									"origin": "4, 10",
									"size": "392, 40",
									"transparent": "false"
								}
							}
						}
					}
				}
			}
		}
	}
}
)";

#ifndef _MSC_VER
constexpr auto completeExample = R"(
{
	"vstgui-ui-description": {
		"version": "1",
		"variables": {},
		"bitmaps": {
			"animation_knob": {
				"path": "animation_knob.png"
			},
			"FrameBackground": {
				"nineparttiled-offsets": "10, 20, 10, 10",
				"path": "FrameBackground.png"
			},
			"TabController": {
				"path": "TabController.png"
			},
			"horizontal_slider_back": {
				"path": "horizontal_slider_back.bmp"
			},
			"onoff_button": {
				"path": "onoff_button.bmp"
			},
			"rocker_switch": {
				"path": "rocker_switch.bmp"
			},
			"slider_handle": {
				"path": "slider_handle.bmp"
			},
			"switch_horizontal": {
				"path": "switch_horizontal.bmp"
			},
			"switch_vertical": {
				"path": "switch_vertical.bmp"
			},
			"vertical_slider_back": {
				"path": "vertical_slider_back.bmp"
			},
			"vumeter_back": {
				"path": "vumeter_back.bmp"
			},
			"vumeter_front": {
				"path": "vumeter_front.bmp"
			}
		},
		"fonts": {},
		"colors": {},
		"gradients": {
			"Default TextButton Gradient": [
				{
					"rgba": "#dcdcdcff",
					"start": "0"
				},
				{
					"rgba": "#b4b4b4ff",
					"start": "1"
				}
			],
			"Default TextButton Gradient Highlighted": [
				{
					"rgba": "#b4b4b4ff",
					"start": "0"
				},
				{
					"rgba": "#646464ff",
					"start": "1"
				}
			]
		},
		"control-tags": {
			"Switch": "20000"
		},
		"custom": {
			"UIViewInspector": {
				"windowSize": "71, 194, 471, 709"
			},
			"UIViewHierarchyBrowser": {
				"windowSize": "513, 194, 813, 694"
			},
			"FocusDrawing": {},
			"UIGridController": {},
			"UITemplateController": {
				"SelectedTemplate": "tab1"
			},
			"UIEditController": {
				"EditViewScale": "1",
				"EditorSize": "0, 0, 1244, 755",
				"SplitViewSize_0_0": "0.8228882833787466433150825650955084711313",
				"SplitViewSize_0_1": "0.1498637602179836436633308949240017682314",
				"SplitViewSize_1_0": "0.480926430517711167578198683258960954845",
				"SplitViewSize_1_1": "0.5122615803814714041664046817459166049957",
				"SplitViewSize_2_0": "0.7033762057877813722583937305898871272802",
				"SplitViewSize_2_1": "0.2926045016077170601853651987767079845071",
				"Version": "1"
			},
			"UIAttributesController": {},
			"UIViewCreatorDataSource": {
				"SelectedRow": "20"
			}
		},
		"templates": {
			"view": {
				"attributes": {
					"autosize": "left right top bottom ",
					"background-color": "~ WhiteCColor",
					"background-color-draw-style": "filled and stroked",
					"class": "CViewContainer",
					"maxSize": "300, 500",
					"minSize": "150, 300",
					"mouse-enabled": "true",
					"opacity": "1",
					"origin": "0, 0",
					"size": "300, 500",
					"transparent": "false"
				},
				"children": {
					"UIViewSwitchContainer": {
						"attributes": {
							"animation-style": "fade",
							"animation-time": "120",
							"background-color": "~ BlackCColor",
							"background-color-draw-style": "filled and stroked",
							"class": "UIViewSwitchContainer",
							"mouse-enabled": "true",
							"opacity": "1",
							"origin": "10, 10",
							"size": "280, 480",
							"transparent": "false"
						}
					}
				}
			},
			"tab1": {
				"attributes": {
					"autosize": "left right top bottom ",
					"background-color": "~ BlackCColor",
					"background-color-draw-style": "filled and stroked",
					"class": "CViewContainer",
					"mouse-enabled": "true",
					"opacity": "1",
					"origin": "0, 0",
					"size": "280, 480",
					"transparent": "true"
				},
				"children": {
					"CLayeredViewContainer": {
						"attributes": {
							"background-color": "~ BlackCColor",
							"background-color-draw-style": "filled and stroked",
							"class": "CLayeredViewContainer",
							"mouse-enabled": "true",
							"opacity": "1",
							"origin": "10, 20",
							"size": "100, 100",
							"transparent": "false",
							"z-index": "0"
						}
					},
					"CShadowViewContainer": {
						"attributes": {
							"background-color": "~ BlackCColor",
							"background-color-draw-style": "filled and stroked",
							"bitmap": "0",
							"class": "CShadowViewContainer",
							"mouse-enabled": "true",
							"opacity": "1",
							"origin": "130, 20",
							"shadow-blur-size": "4",
							"shadow-intensity": "0.3",
							"shadow-offset": "0, 0",
							"size": "200, 200",
							"transparent": "false"
						}
					},
					"CScrollView": {
						"attributes": {
							"auto-drag-scrolling": "false",
							"auto-hide-scrollbars": "false",
							"background-color": "~ BlackCColor",
							"background-color-draw-style": "filled and stroked",
							"bordered": "true",
							"class": "CScrollView",
							"container-size": "200, 200",
							"follow-focus-view": "false",
							"horizontal-scrollbar": "true",
							"mouse-enabled": "true",
							"opacity": "1",
							"origin": "40, 240",
							"overlay-scrollbars": "false",
							"scrollbar-background-color": "#ffffffc8",
							"scrollbar-frame-color": "~ BlackCColor",
							"scrollbar-scroller-color": "~ BlueCColor",
							"scrollbar-width": "16",
							"size": "100, 100",
							"transparent": "false",
							"vertical-scrollbar": "true"
						}
					},
					"CRowColumnView": {
						"attributes": {
							"animate-view-resizing": "false",
							"background-color": "~ BlackCColor",
							"background-color-draw-style": "filled and stroked",
							"class": "CRowColumnView",
							"equal-size-layout": "left-top",
							"margin": "0,0,0,0",
							"mouse-enabled": "true",
							"opacity": "1",
							"origin": "60, 360",
							"row-style": "true",
							"size": "100, 100",
							"spacing": "0",
							"transparent": "false",
							"view-resize-animation-time": "200"
						}
					},
					"CSplitView": {
						"attributes": {
							"background-color": "~ BlackCColor",
							"background-color-draw-style": "filled and stroked",
							"class": "CSplitView",
							"mouse-enabled": "true",
							"opacity": "1",
							"orientation": "horizontal",
							"origin": "160, 280",
							"resize-method": "last",
							"separator-width": "10",
							"size": "100, 100",
							"transparent": "false"
						}
					}
				}
			},
			"tab2": {
				"attributes": {
					"autosize": "left right top bottom row ",
					"background-color": "~ BlackCColor",
					"background-color-draw-style": "filled and stroked",
					"class": "CViewContainer",
					"mouse-enabled": "true",
					"opacity": "1",
					"origin": "0, 0",
					"size": "280, 480",
					"transparent": "true"
				},
				"children": {
					"CAnimKnob": {
						"attributes": {
							"angle-range": "270",
							"angle-start": "135",
							"background-offset": "0, 0",
							"circle-drawing": "false",
							"class": "CAnimKnob",
							"corona-color": "~ WhiteCColor",
							"corona-dash-dot": "false",
							"corona-drawing": "false",
							"corona-from-center": "false",
							"corona-inset": "0",
							"corona-inverted": "false",
							"corona-outline": "false",
							"default-value": "0.5",
							"handle-color": "~ WhiteCColor",
							"handle-line-width": "1",
							"handle-shadow-color": "~ GreyCColor",
							"height-of-one-image": "0",
							"max-value": "1",
							"min-value": "0",
							"mouse-enabled": "true",
							"opacity": "1",
							"origin": "10, 10",
							"size": "20, 20",
							"sub-pixmaps": "0",
							"transparent": "false",
							"value-inset": "0",
							"wheel-inc-value": "0.1",
							"zoom-factor": "1.5"
						}
					},
					"CAnimationSplashScreen": {
						"attributes": {
							"animation-index": "0",
							"animation-time": "500",
							"background-offset": "0, 0",
							"class": "CAnimationSplashScreen",
							"default-value": "0.5",
							"max-value": "1",
							"min-value": "0",
							"mouse-enabled": "true",
							"opacity": "1",
							"origin": "50, 10",
							"size": "20, 20",
							"splash-origin": "0, 0",
							"splash-size": "0, 0",
							"transparent": "false",
							"wheel-inc-value": "0.1"
						}
					},
					"CCheckBox": {
						"attributes": {
							"autosize-to-fit": "false",
							"background-offset": "0, 0",
							"boxfill-color": "~ WhiteCColor",
							"boxframe-color": "~ BlackCColor",
							"checkmark-color": "~ RedCColor",
							"class": "CCheckBox",
							"default-value": "0.5",
							"draw-crossbox": "false",
							"font": "~ SystemFont",
							"font-color": "~ WhiteCColor",
							"max-value": "1",
							"min-value": "0",
							"mouse-enabled": "true",
							"opacity": "1",
							"origin": "110, 10",
							"size": "100, 20",
							"title": "Title",
							"transparent": "false",
							"wheel-inc-value": "0.1"
						}
					},
					"CControl": {
						"attributes": {
							"background-offset": "0, 0",
							"class": "CControl",
							"default-value": "0.5",
							"max-value": "1",
							"min-value": "0",
							"mouse-enabled": "true",
							"opacity": "1",
							"origin": "20, 50",
							"size": "20, 20",
							"transparent": "false",
							"wheel-inc-value": "0.1"
						}
					},
					"CGradientView": {
						"attributes": {
							"class": "CGradientView",
							"draw-antialiased": "true",
							"frame-color": "~ BlackCColor",
							"frame-width": "1",
							"gradient-angle": "0",
							"gradient-style": "linear",
							"mouse-enabled": "true",
							"opacity": "1",
							"origin": "70, 50",
							"radial-center": "0.5, 0.5",
							"radial-radius": "1",
							"round-rect-radius": "5",
							"size": "100, 100",
							"transparent": "false"
						}
					},
					"CHorizontalSwitch": {
						"attributes": {
							"background-offset": "0, 0",
							"class": "CHorizontalSwitch",
							"default-value": "0",
							"height-of-one-image": "0",
							"max-value": "1",
							"min-value": "0",
							"mouse-enabled": "true",
							"opacity": "1",
							"origin": "20, 160",
							"size": "20, 20",
							"sub-pixmaps": "0",
							"transparent": "false",
							"wheel-inc-value": "0.1"
						}
					},
					"CKickButton": {
						"attributes": {
							"background-offset": "0, 0",
							"class": "CKickButton",
							"default-value": "0.5",
							"height-of-one-image": "0",
							"max-value": "1",
							"min-value": "0",
							"mouse-enabled": "true",
							"opacity": "1",
							"origin": "70, 150",
							"size": "20, 20",
							"sub-pixmaps": "0",
							"transparent": "false",
							"wheel-inc-value": "0.1"
						}
					},
					"CKnob": {
						"attributes": {
							"angle-range": "270",
							"angle-start": "135",
							"background-offset": "0, 0",
							"circle-drawing": "false",
							"class": "CKnob",
							"corona-color": "~ WhiteCColor",
							"corona-dash-dot": "false",
							"corona-drawing": "false",
							"corona-from-center": "false",
							"corona-inset": "0",
							"corona-inverted": "false",
							"corona-outline": "false",
							"default-value": "0.5",
							"handle-color": "~ WhiteCColor",
							"handle-line-width": "1",
							"handle-shadow-color": "~ GreyCColor",
							"max-value": "1",
							"min-value": "0",
							"mouse-enabled": "true",
							"opacity": "1",
							"origin": "120, 150",
							"size": "20, 20",
							"transparent": "false",
							"value-inset": "3",
							"wheel-inc-value": "0.1",
							"zoom-factor": "1.5"
						}
					},
					"CMovieBitmap": {
						"attributes": {
							"background-offset": "0, 0",
							"class": "CMovieBitmap",
							"default-value": "0.5",
							"height-of-one-image": "0",
							"max-value": "1",
							"min-value": "0",
							"mouse-enabled": "true",
							"opacity": "1",
							"origin": "170, 160",
							"size": "20, 20",
							"sub-pixmaps": "0",
							"transparent": "false",
							"wheel-inc-value": "0.1"
						}
					},
					"CMovieButton": {
						"attributes": {
							"background-offset": "0, 0",
							"class": "CMovieButton",
							"default-value": "0.5",
							"height-of-one-image": "0",
							"max-value": "1",
							"min-value": "0",
							"mouse-enabled": "true",
							"opacity": "1",
							"origin": "30, 200",
							"size": "20, 20",
							"sub-pixmaps": "0",
							"transparent": "false",
							"wheel-inc-value": "0.1"
						}
					},
					"COnOffButton": {
						"attributes": {
							"background-offset": "0, 0",
							"class": "COnOffButton",
							"default-value": "0.5",
							"max-value": "1",
							"min-value": "0",
							"mouse-enabled": "true",
							"opacity": "1",
							"origin": "80, 190",
							"size": "20, 20",
							"transparent": "false",
							"wheel-inc-value": "0.1"
						}
					},
					"COptionMenu": {
						"attributes": {
							"back-color": "~ BlackCColor",
							"background-offset": "0, 0",
							"class": "COptionMenu",
							"default-value": "0.5",
							"font": "~ NormalFont",
							"font-antialias": "true",
							"font-color": "~ WhiteCColor",
							"frame-color": "~ BlackCColor",
							"frame-width": "1",
							"max-value": "1.84467e+19",
							"menu-check-style": "false",
							"menu-popup-style": "false",
							"min-value": "0",
							"mouse-enabled": "true",
							"opacity": "1",
							"origin": "140, 190",
							"round-rect-radius": "6",
							"shadow-color": "~ RedCColor",
							"size": "20, 20",
							"style-3D-in": "false",
							"style-3D-out": "false",
							"style-no-draw": "false",
							"style-no-frame": "false",
							"style-no-text": "false",
							"style-round-rect": "false",
							"style-shadow-text": "false",
							"text-alignment": "center",
							"text-inset": "0, 0",
							"text-rotation": "0",
							"transparent": "false",
							"value-precision": "2",
							"wheel-inc-value": "0.1"
						}
					},
					"CParamDisplay": {
						"attributes": {
							"back-color": "~ BlackCColor",
							"background-offset": "0, 0",
							"class": "CParamDisplay",
							"default-value": "0.5",
							"font": "~ NormalFont",
							"font-antialias": "true",
							"font-color": "~ WhiteCColor",
							"frame-color": "~ BlackCColor",
							"frame-width": "1",
							"max-value": "1",
							"min-value": "0",
							"mouse-enabled": "true",
							"opacity": "1",
							"origin": "30, 230",
							"round-rect-radius": "6",
							"shadow-color": "~ RedCColor",
							"size": "20, 20",
							"style-3D-in": "false",
							"style-3D-out": "false",
							"style-no-draw": "false",
							"style-no-frame": "false",
							"style-no-text": "false",
							"style-round-rect": "false",
							"style-shadow-text": "false",
							"text-alignment": "center",
							"text-inset": "0, 0",
							"text-rotation": "0",
							"transparent": "false",
							"value-precision": "2",
							"wheel-inc-value": "0.1"
						}
					},
					"CRockerSwitch": {
						"attributes": {
							"background-offset": "0, 0",
							"class": "CRockerSwitch",
							"default-value": "0.5",
							"height-of-one-image": "0",
							"max-value": "1",
							"min-value": "-1",
							"mouse-enabled": "true",
							"opacity": "1",
							"origin": "70, 230",
							"size": "20, 20",
							"sub-pixmaps": "3",
							"transparent": "false",
							"wheel-inc-value": "0.1"
						}
					},
					"CSegmentButton": {
						"attributes": {
							"background-offset": "0, 0",
							"class": "CSegmentButton",
							"default-value": "0.5",
							"font": "~ NormalFont",
							"frame-color": "~ BlackCColor",
							"frame-width": "1",
							"icon-text-margin": "0",
							"max-value": "1",
							"min-value": "0",
							"mouse-enabled": "true",
							"opacity": "1",
							"origin": "120, 230",
							"round-radius": "5",
							"segment-names": "Segment 1,Segment 2,Segment 3,Segment 4",
							"size": "200, 20",
							"style": "horizontal",
							"text-alignment": "center",
							"text-color": "~ BlackCColor",
							"text-color-highlighted": "~ WhiteCColor",
							"transparent": "false",
							"wheel-inc-value": "0.1"
						}
					},
					"CSlider": {
						"attributes": {
							"background-offset": "0, 0",
							"bitmap-offset": "0, 0",
							"class": "CSlider",
							"default-value": "0.5",
							"draw-back": "false",
							"draw-back-color": "~ WhiteCColor",
							"draw-frame": "false",
							"draw-frame-color": "~ WhiteCColor",
							"draw-value": "false",
							"draw-value-color": "~ WhiteCColor",
							"draw-value-from-center": "false",
							"draw-value-inverted": "false",
							"handle-offset": "0, 0",
							"max-value": "1",
							"min-value": "0",
							"mode": "free click",
							"mouse-enabled": "true",
							"opacity": "1",
							"orientation": "horizontal",
							"origin": "20, 270",
							"reverse-orientation": "false",
							"size": "20, 20",
							"transparent": "false",
							"transparent-handle": "true",
							"wheel-inc-value": "0.1",
							"zoom-factor": "10"
						}
					},
					"CTextButton": {
						"attributes": {
							"background-offset": "0, 0",
							"class": "CTextButton",
							"default-value": "0.5",
							"font": "~ SystemFont",
							"frame-color": "~ BlackCColor",
							"frame-color-highlighted": "~ BlackCColor",
							"frame-width": "1",
							"gradient": "Default TextButton Gradient",
							"gradient-highlighted": "Default TextButton Gradient Highlighted",
							"icon-position": "left",
							"icon-text-margin": "0",
							"kick-style": "true",
							"max-value": "1",
							"min-value": "0",
							"mouse-enabled": "true",
							"opacity": "1",
							"origin": "60, 270",
							"round-radius": "6",
							"size": "100, 20",
							"text-alignment": "center",
							"text-color": "~ BlackCColor",
							"text-color-highlighted": "~ WhiteCColor",
							"transparent": "false",
							"wheel-inc-value": "0.1"
						}
					},
					"CTextEdit": {
						"attributes": {
							"back-color": "~ BlackCColor",
							"background-offset": "0, 0",
							"class": "CTextEdit",
							"default-value": "0.5",
							"font": "~ NormalFont",
							"font-antialias": "true",
							"font-color": "~ WhiteCColor",
							"frame-color": "~ BlackCColor",
							"frame-width": "1",
							"immediate-text-change": "false",
							"max-value": "1",
							"min-value": "0",
							"mouse-enabled": "true",
							"opacity": "1",
							"origin": "180, 270",
							"round-rect-radius": "6",
							"shadow-color": "~ RedCColor",
							"size": "100, 20",
							"style-3D-in": "false",
							"style-3D-out": "false",
							"style-doubleclick": "false",
							"style-no-draw": "false",
							"style-no-frame": "false",
							"style-no-text": "false",
							"style-round-rect": "false",
							"style-shadow-text": "false",
							"text-alignment": "center",
							"text-inset": "0, 0",
							"text-rotation": "0",
							"transparent": "false",
							"value-precision": "2",
							"wheel-inc-value": "0.1"
						}
					},
					"CTextLabel": {
						"attributes": {
							"back-color": "~ BlackCColor",
							"background-offset": "0, 0",
							"class": "CTextLabel",
							"default-value": "0.5",
							"font": "~ NormalFont",
							"font-antialias": "true",
							"font-color": "~ WhiteCColor",
							"frame-color": "~ BlackCColor",
							"frame-width": "1",
							"max-value": "1",
							"min-value": "0",
							"mouse-enabled": "true",
							"opacity": "1",
							"origin": "20, 300",
							"round-rect-radius": "6",
							"shadow-color": "~ RedCColor",
							"size": "100, 20",
							"style-3D-in": "false",
							"style-3D-out": "false",
							"style-no-draw": "false",
							"style-no-frame": "false",
							"style-no-text": "false",
							"style-round-rect": "false",
							"style-shadow-text": "false",
							"text-alignment": "center",
							"text-inset": "0, 0",
							"text-rotation": "0",
							"transparent": "false",
							"value-precision": "2",
							"wheel-inc-value": "0.1"
						}
					},
					"CVerticalSwitch": {
						"attributes": {
							"background-offset": "0, 0",
							"class": "CVerticalSwitch",
							"default-value": "0",
							"height-of-one-image": "0",
							"max-value": "1",
							"min-value": "0",
							"mouse-enabled": "true",
							"opacity": "1",
							"origin": "20, 330",
							"size": "20, 20",
							"sub-pixmaps": "0",
							"transparent": "false",
							"wheel-inc-value": "0.1"
						}
					},
					"CView": {
						"attributes": {
							"class": "CView",
							"mouse-enabled": "true",
							"opacity": "1",
							"origin": "50, 330",
							"size": "20, 20",
							"transparent": "false"
						}
					},
					"CVuMeter": {
						"attributes": {
							"background-offset": "0, 0",
							"class": "CVuMeter",
							"decrease-step-value": "0.1",
							"default-value": "0.5",
							"max-value": "1",
							"min-value": "0",
							"mouse-enabled": "true",
							"num-led": "100",
							"opacity": "1",
							"orientation": "vertical",
							"origin": "90, 330",
							"size": "20, 20",
							"transparent": "false",
							"wheel-inc-value": "0.1"
						}
					},
					"CXYPad": {
						"attributes": {
							"back-color": "~ BlackCColor",
							"background-offset": "0, 0",
							"class": "CXYPad",
							"default-value": "0.5",
							"font": "~ NormalFont",
							"font-antialias": "true",
							"font-color": "~ WhiteCColor",
							"frame-color": "~ BlackCColor",
							"frame-width": "1",
							"max-value": "2",
							"min-value": "0",
							"mouse-enabled": "true",
							"opacity": "1",
							"origin": "140, 330",
							"round-rect-radius": "6",
							"shadow-color": "~ RedCColor",
							"size": "100, 20",
							"style-3D-in": "false",
							"style-3D-out": "false",
							"style-no-draw": "false",
							"style-no-frame": "false",
							"style-no-text": "false",
							"style-round-rect": "false",
							"style-shadow-text": "false",
							"text-alignment": "center",
							"text-inset": "0, 0",
							"text-rotation": "0",
							"transparent": "false",
							"value-precision": "2",
							"wheel-inc-value": "0.1"
						}
					}
				}
			}
		}
	}
}
)";
#endif

constexpr auto sharedResourcesUIDesc = R"(
{
	"vstgui-ui-description": {
		"version": "1",
		"bitmaps": {
			"b1": {
				"path": "b1.png",
				"data": {
					"encoding": "base64",
					"data": "iVBORw0KGgoAAAANSUhEUgAAAAwAAAAMCAYAAABWdVznAAABe2lDQ1BJQ0MgUHJvZmlsZQAAKJF9kE0rRFEYx38zXjNkwcLC4jaG1RCjvGyUmYSaxTRGedvcueZFmXG7c4VsLJTtFCU23hZ8AjYWylopRUrKVyA20vUcQ+OlPHXO+Z3nPM+/5/zB7ddNc7a0HTJZ24oOBrWx8Qmt4gEX5XjQ8OpGzuyPRMJIfJ0/4+VaqiWuWpXW3/d/wzOdyBngqhTuM0zLFh4SblqwTcVKr96SoYRXFKcKvKE4XuCjj5pYNCR8KqwZaX1a+E7Yb6StDLiVvi/+rSb1jTOz88bnPOon1Yns6IicXlmN5IgySFC8GGaAEF100Ct7F60EaJMbdmLRVs2hOXPJmkmlba1fnEhow1mjza8F2ju6Qfn6269ibm4Xep6hJF/MxTfhZA0abos53w7UrsLxualb+keqRJY7mYTHQ6gZh7pLqJrMJTsDhR9VB6Hs3nGemqFiHd7yjvO65zhv+9IsHp1lCx59anFwA7FlCF/A1ja0iHbt1Dv7WWccXX/QZQAAAExJREFUKBVjZEAALSAzDMFFYa0C8q6BRJhQhBEcUSAThIkGDUCVIIwBcNmAoRAmMKoBFhL4aEagJLYYhkXaazTNq1jQBGBcdIUwcQYAOGIGVqwWW9EAAAAASUVORK5CYII="
				}
			}
		},
		"fonts": {
			"f1": {
				"font-name": "Arial",
				"size": "8"
			}
		},
		"colors": {
			"c1": "#000000ff"
		},
		"gradients": {
			"g1": [
				{
					"rgba": "#000000ff",
					"start": "0"
				},
				{
					"rgba": "#ff0000ff",
					"start": "0.5"
				},
				{
					"rgba": "#ffffffff",
					"start": "1"
				}
			]
		}
	}
}
)";

} // anonymous

using StringPtrList = std::list<const std::string*>;

TEST_CASE (UIDescriptionJSONTests, ParseEmpty)
{
	MemoryContentProvider provider (emptyUIDesc, static_cast<uint32_t> (strlen (emptyUIDesc)));
	UIDescription desc (&provider);
	EXPECT (desc.parse () == true);
	EXPECT (desc.getGradient ("t") == nullptr);
	EXPECT (desc.getBitmap ("b") == nullptr);
	EXPECT (desc.getFont ("f") == nullptr);
	EXPECT (desc.getTagForName ("t") == -1);
	CColor c;
	EXPECT (desc.getColor ("c", c) == false);
	EXPECT (desc.getControlListener ("t") == nullptr);
	EXPECT (desc.getController () == nullptr);
}

TEST_CASE (UIDescriptionJSONTests, Colors)
{
	MemoryContentProvider provider (colorNodesUIDesc,
	                                static_cast<uint32_t> (strlen (colorNodesUIDesc)));
	UIDescription desc (&provider);
	EXPECT (desc.parse () == true);
	CColor c;
	EXPECT (desc.getColor ("c1", c));
	EXPECT (c == CColor (0, 0, 0, 255));
	EXPECT (desc.getColor ("c2", c));
	EXPECT (c == CColor (255, 255, 255, 255));
	EXPECT (desc.getColor ("c3", c));
	EXPECT (c == CColor (255, 0, 0, 100));
	EXPECT (desc.getColor ("c4", c));
	EXPECT (c == CColor (0, 255, 0, 150));
	EXPECT (desc.getColor ("c5", c));
	EXPECT (c == CColor (255, 0, 255, 100));

	StringPtrList names;
	desc.collectColorNames (names);
	uint32_t numNames = 0;
	for (auto& name : names)
	{
		if (name->at (0) != '~')
			numNames++;
	}
	EXPECT (numNames == 5);

	desc.changeColor ("c5", CColor (0, 255, 0, 255));
	EXPECT (desc.getColor ("c5", c));
	EXPECT (c == CColor (0, 255, 0, 255));

	desc.changeColor ("added color node", CColor (1, 2, 3, 4));
	EXPECT (desc.hasColorName ("added color node"));

	auto name = desc.lookupColorName (CColor (0, 255, 0, 255));
	EXPECT (name == std::string ("c5"));
	desc.changeColorName ("c5", "new color");
	EXPECT (desc.hasColorName ("new color"));
	desc.removeColor ("new color");
	EXPECT (desc.getColor ("new color", c) == false);
}

TEST_CASE (UIDescriptionJSONTests, Fonts)
{
	MemoryContentProvider provider (fontNodesUIDesc,
	                                static_cast<uint32_t> (strlen (fontNodesUIDesc)));
	UIDescription desc (&provider);
	EXPECT (desc.parse () == true);
	EXPECT (desc.hasFontName ("f1"));
	auto font = desc.getFont ("f1");
	EXPECT (font->getName () == std::string ("Arial"));
	EXPECT (font->getSize () == 8);
	EXPECT (font->getStyle () == kNormalFace);
	font = desc.getFont ("f2");
	EXPECT (font->getName () == std::string ("Arial"));
	EXPECT (font->getSize () == 8);
	EXPECT (font->getStyle () == kBoldFace);
	font = desc.getFont ("f3");
	EXPECT (font->getName () == std::string ("Arial"));
	EXPECT (font->getSize () == 8);
	EXPECT (font->getStyle () == kItalicFace);
	font = desc.getFont ("f4");
	EXPECT (font->getName () == std::string ("Arial"));
	EXPECT (font->getSize () == 8);
	EXPECT (font->getStyle () == kUnderlineFace);
	font = desc.getFont ("f5");
	EXPECT (font->getName () == std::string ("Arial"));
	EXPECT (font->getSize () == 8);
	EXPECT (font->getStyle () == kStrikethroughFace);
	font = desc.getFont ("f6");
	EXPECT (font->getName () == std::string ("Arial"));
	EXPECT (font->getSize () == 8);
	std::string altFontNames;
	EXPECT (desc.getAlternativeFontNames ("f5", altFontNames) == false);
	EXPECT (desc.getAlternativeFontNames ("f6", altFontNames));
	EXPECT (altFontNames == "Arial, Courier");
	desc.changeAlternativeFontNames ("f6", "Courier");
	desc.getAlternativeFontNames ("f6", altFontNames);
	EXPECT (altFontNames == "Courier");
	auto name = desc.lookupFontName (font);
	EXPECT (name == std::string ("f6"));
	StringPtrList names;
	desc.collectFontNames (names);
	uint32_t numNames = 0;
	for (auto& n : names)
	{
		if (n->at (0) != '~')
			numNames++;
	}
	EXPECT (numNames == 6);
	desc.changeFontName ("f1", "font");
	EXPECT (desc.hasFontName ("font"));
	auto newFont = owned (new CFontDesc (*font));
	desc.changeFont ("font", newFont);
	desc.changeFont ("font2", newFont);
	EXPECT (desc.getFont ("font") == newFont);
	EXPECT (desc.getFont ("font2") == newFont);
	desc.removeFont ("font");
	EXPECT (desc.hasFontName ("font") == false);
}

TEST_CASE (UIDescriptionJSONTests, Bitmaps)
{
	MemoryContentProvider provider (bitmapNodesUIDesc,
	                                static_cast<uint32_t> (strlen (bitmapNodesUIDesc)));
	UIDescription desc (&provider);
	EXPECT (desc.parse () == true);
	EXPECT (desc.hasBitmapName ("b1"));
	auto bitmap = desc.getBitmap ("b1");
	EXPECT (bitmap);
	auto name = desc.lookupBitmapName (bitmap);
	EXPECT (name == std::string ("b1"));
	StringPtrList names;
	desc.collectBitmapNames (names);
	EXPECT (names.size () == 2);
	desc.changeBitmapName ("b1", "new bitmap");
	EXPECT (desc.hasBitmapName ("new bitmap"));
	desc.removeBitmap ("new bitmap");
	EXPECT (desc.hasBitmapName ("new bitmap") == false);
	CRect ninePartTiledOffset (10, 10, 10, 10);
	desc.changeBitmap ("added bitmap node", "path to bitmap", &ninePartTiledOffset);
	EXPECT (desc.hasBitmapName ("added bitmap node"));
	bitmap = desc.getBitmap ("added bitmap node");
	EXPECT (dynamic_cast<CNinePartTiledBitmap*> (bitmap));
	auto& offsets = dynamic_cast<CNinePartTiledBitmap*> (bitmap)->getPartOffsets ();
	EXPECT (offsets.left == 10 && offsets.top == 10 && offsets.right == 10 && offsets.bottom == 10);
	desc.changeBitmap ("added bitmap node", "added bitmap node", nullptr);
	bitmap = desc.getBitmap ("added bitmap node");
	EXPECT (dynamic_cast<CNinePartTiledBitmap*> (bitmap) == nullptr);
}

TEST_CASE (UIDescriptionJSONTests, Tags)
{
	MemoryContentProvider provider (tagNodesUIDesc,
	                                static_cast<uint32_t> (strlen (tagNodesUIDesc)));
	UIDescription desc (&provider);
	EXPECT (desc.parse () == true);
	EXPECT (desc.hasTagName ("t1"));
	EXPECT (desc.getTagForName ("t1") == 1234);
	EXPECT (desc.getTagForName ("t3") == 1836676199);
	auto name = desc.lookupControlTagName (1234);
	EXPECT (name == std::string ("t1"));
	StringPtrList names;
	desc.collectControlTagNames (names);
	EXPECT (names.size () == 3);
	desc.changeTagName ("t1", "control tag");
	EXPECT (desc.hasTagName ("control tag"));
	desc.changeControlTagString ("control tag", "4567 - 5");
	EXPECT (desc.getTagForName ("control tag") == 4562);
	std::string tagString;
	EXPECT (desc.getControlTagString ("control not existing", tagString) == false);
	EXPECT (desc.getControlTagString ("control tag", tagString));
	EXPECT (tagString == "4567 - 5");
	desc.removeTag ("control tag");
	EXPECT (desc.hasTagName ("control tag") == false);
	desc.changeControlTagString ("new control tag", "2*2", true);
	EXPECT (desc.getTagForName ("new control tag") == 4);
}

TEST_CASE (UIDescriptionJSONTests, LookupTagsCalculateTag)
{
	MemoryContentProvider provider (calculateTagNodesUIDesc,
	                                static_cast<uint32_t> (strlen (calculateTagNodesUIDesc)));
	UIDescription desc (&provider);
	EXPECT (desc.parse () == true);
	auto name = desc.lookupControlTagName (3);
	EXPECT (name);
	EXPECT (std::string (name) == "t1");
}

TEST_CASE (UIDescriptionJSONTests, Gradient)
{
	MemoryContentProvider provider (gradientNodesUIDesc,
	                                static_cast<uint32_t> (strlen (gradientNodesUIDesc)));
	UIDescription desc (&provider);
	EXPECT (desc.parse () == true);
	EXPECT (desc.hasGradientName ("g1"));
	auto gradient = desc.getGradient ("g1");
	const auto& colorStops = gradient->getColorStops ();
	EXPECT (colorStops.size () == 3);
	auto it = colorStops.find (0.);
	EXPECT (it != colorStops.end ());
	EXPECT (it->second == CColor (0, 0, 0, 255));
	it = colorStops.find (0.5);
	EXPECT (it != colorStops.end ());
	EXPECT (it->second == CColor (255, 0, 0, 255));
	it = colorStops.find (1.);
	EXPECT (it != colorStops.end ());
	EXPECT (it->second == CColor (255, 255, 255, 255));
	auto name = desc.lookupGradientName (gradient);
	EXPECT (name == std::string ("g1"));
	StringPtrList names;
	desc.collectGradientNames (names);
	EXPECT (names.size () == 1);
	desc.changeGradientName ("g1", "gradient");
	EXPECT (desc.hasGradientName ("gradient"));
	EXPECT (desc.hasGradientName ("g1") == false);
	auto newGradient = owned (CGradient::create (0., 1., kWhiteCColor, kBlackCColor));
	desc.changeGradient ("gradient", newGradient);
	EXPECT (desc.getGradient ("gradient") == newGradient);
	desc.changeGradient ("gradientnew", newGradient);
	EXPECT (desc.hasGradientName ("gradientnew"));
	desc.removeGradient ("gradientnew");
	EXPECT (desc.hasGradientName ("gradientnew") == false);
}

TEST_CASE (UIDescriptionJSONTests, Variables)
{
	MemoryContentProvider provider (variableNodesUIDesc,
	                                static_cast<uint32_t> (strlen (variableNodesUIDesc)));
	UIDescription desc (&provider);
	EXPECT (desc.parse () == true);
	double value;
	EXPECT (desc.getVariable ("v1", value));
	EXPECT (value == 10.);
	std::string strValue;
	EXPECT (desc.getVariable ("v2", strValue));
	EXPECT (strValue == "string");
	EXPECT (desc.getVariable ("v3", strValue));
	EXPECT (strValue == "string");
	EXPECT (desc.getVariable ("v4", value));
	EXPECT (value == 20.5);
	EXPECT (desc.getVariable ("v5", value));
	EXPECT (value == 20.);
	EXPECT (desc.getVariable ("v6", strValue));
	EXPECT (strValue == "");
}

TEST_CASE (UIDescriptionJSONTests, Calculations)
{
	MemoryContentProvider provider (tagNodesUIDesc,
	                                static_cast<uint32_t> (strlen (tagNodesUIDesc)));
	UIDescription desc (&provider);
	EXPECT (desc.parse () == true);
	double value;
	EXPECT (desc.calculateStringValue ("1", value));
	EXPECT (value == 1.);
	EXPECT (desc.calculateStringValue ("1+1", value));
	EXPECT (value == 2.);
	EXPECT (desc.calculateStringValue ("(1+1)*2", value));
	EXPECT (value == 4.);
	EXPECT (desc.calculateStringValue ("(1+1)*2-(3/3 + (0.5+0.5))", value));
	EXPECT (value == 2.);
	EXPECT (desc.calculateStringValue ("tag.t1 - 4", value));
	EXPECT (value == 1230.);
	EXPECT (desc.calculateStringValue ("(1+5*3", value) == false);
	EXPECT (desc.calculateStringValue ("tag.unknown - 4", value) == false);
	EXPECT (desc.calculateStringValue ("var.unknown", value) == false);
	EXPECT (desc.calculateStringValue ("unknown", value) == false);
}

TEST_CASE (UIDescriptionJSONTests, WriteToStream)
{
	std::string str (withAllNodesUIDesc);
	MemoryContentProvider provider (str.data (), static_cast<uint32_t> (str.size ()));
	SaveUIDescription desc (&provider);
	EXPECT (desc.parse () == true);
	CMemoryStream outputStream (1024, 1024, false);
	EXPECT (desc.saveToStream (outputStream, defaultSafeFlags, nullptr));
	outputStream.end ();
	std::string result (reinterpret_cast<const char*> (outputStream.getBuffer ()));
	EXPECT (result.size () == str.size ());
	EXPECT (result == str);
}

TEST_CASE (UIDescriptionJSONTests, GetViewAttributes)
{
	MemoryContentProvider provider (createViewUIDesc,
	                                static_cast<uint32_t> (strlen (createViewUIDesc)));
	UIDescription desc (&provider);
	EXPECT (desc.parse () == true);

	auto attributes = desc.getViewAttributes ("view");
	EXPECT (attributes);
	auto classAttr = attributes->getAttributeValue (UIViewCreator::kAttrClass);
	EXPECT (classAttr);
	EXPECT (*classAttr == "CViewContainer");
	attributes = desc.getViewAttributes ("view not existing");
	EXPECT (attributes == nullptr);
}

TEST_CASE (UIDescriptionJSONTests, CollectTemplateViewNames)
{
	MemoryContentProvider provider (createViewUIDesc,
	                                static_cast<uint32_t> (strlen (createViewUIDesc)));
	UIDescription desc (&provider);
	EXPECT (desc.parse () == true);

	StringPtrList names;
	desc.collectTemplateViewNames (names);
	EXPECT (names.size () == 1);
	EXPECT (*names.front () == std::string ("view"));
}

TEST_CASE (UIDescriptionJSONTests, DuplicateTemplate)
{
	MemoryContentProvider provider (createViewUIDesc,
	                                static_cast<uint32_t> (strlen (createViewUIDesc)));
	UIDescription desc (&provider);
	EXPECT (desc.parse () == true);

	StringPtrList names;
	desc.collectTemplateViewNames (names);
	EXPECT (desc.duplicateTemplate ("view not existing", "viewcopy") == false);
	EXPECT (desc.duplicateTemplate ("view", "viewcopy"));
	EXPECT (desc.addNewTemplate ("view", nullptr) == false);
	names.clear ();
	desc.collectTemplateViewNames (names);
	EXPECT (names.size () == 2);
	EXPECT (*names.front () == std::string ("view"));
	EXPECT (*names.back () == std::string ("viewcopy"));
}

TEST_CASE (UIDescriptionJSONTests, ChangeTemplateName)
{
	MemoryContentProvider provider (createViewUIDesc,
	                                static_cast<uint32_t> (strlen (createViewUIDesc)));
	UIDescription desc (&provider);
	EXPECT (desc.parse () == true);

	EXPECT (desc.duplicateTemplate ("view", "viewcopy"));
	EXPECT (desc.changeTemplateName ("viewcopy", "copyOfView"));
	StringPtrList names;
	desc.collectTemplateViewNames (names);
	EXPECT (*names.back () == std::string ("copyOfView"));
}

TEST_CASE (UIDescriptionJSONTests, GetTemplateNameFromView)
{
	MemoryContentProvider provider (createViewUIDesc,
	                                static_cast<uint32_t> (strlen (createViewUIDesc)));
	UIDescription desc (&provider);
	EXPECT (desc.parse () == true);

	Controller controller;
	auto view = owned (desc.createView ("view", &controller));
	std::string name;
	desc.getTemplateNameFromView (view, name);
	EXPECT (name == "view");
}

TEST_CASE (UIDescriptionJSONTests, RemoveTemplate)
{
	MemoryContentProvider provider (createViewUIDesc,
	                                static_cast<uint32_t> (strlen (createViewUIDesc)));
	UIDescription desc (&provider);
	EXPECT (desc.parse () == true);

	EXPECT (desc.removeTemplate ("view which does not exist") == false);
	EXPECT (desc.removeTemplate ("view"));
}

TEST_CASE (UIDescriptionJSONTests, AddNewTemplate)
{
	MemoryContentProvider provider (createViewUIDesc,
	                                static_cast<uint32_t> (strlen (createViewUIDesc)));
	UIDescription desc (&provider);
	EXPECT (desc.parse () == true);

	auto a = makeOwned<UIAttributes> ();
	a->setAttribute (UIViewCreator::kAttrClass, "CViewContainer");
	EXPECT (desc.addNewTemplate ("addNewTemplate", a));
	StringPtrList names;
	desc.collectTemplateViewNames (names);
	EXPECT (*names.back () == std::string ("addNewTemplate"));
}

TEST_CASE (UIDescriptionJSONTests, StoreRestoreViews)
{
	MemoryContentProvider provider (createViewUIDesc,
	                                static_cast<uint32_t> (strlen (createViewUIDesc)));
	UIDescription desc (&provider);
	EXPECT (desc.parse () == true);

	Controller controller;
	auto view = owned (desc.createView ("view", &controller));
	EXPECT (view);

	CMemoryStream memoryStream (1024, 1024, false);
	std::list<SharedPointer<CView>> restoredView;

	UIAttributes customAttributes;
	customAttributes.setAttribute ("Test", "Value");

	EXPECT (desc.storeViews ({view.cast<CViewContainer> ()->getView (0)}, memoryStream,
	                         &customAttributes));
	memoryStream.rewind ();

	UIAttributes* customAttributesRestored = nullptr;
	EXPECT (desc.restoreViews (memoryStream, restoredView, &customAttributesRestored));
	EXPECT (customAttributesRestored);
	EXPECT (*customAttributesRestored->getAttributeValue ("Test") == "Value");
}

TEST_CASE (UIDescriptionJSONTests, StoreRestoreViewsAttached)
{
	MemoryContentProvider provider (restoreViewUIDesc,
	                                static_cast<uint32_t> (strlen (restoreViewUIDesc)));
	UIDescription desc (&provider);
	EXPECT (desc.parse () == true);

	Controller controller;
	auto view = owned (desc.createView ("view", &controller));
	EXPECT (view);

	CMemoryStream memoryStream (1024, 1024, false);
	std::list<SharedPointer<CView>> restoredView;

	auto parentContainer = owned (new CViewContainer (CRect (0, 0, 10, 10)));
	view->attached (parentContainer);
	auto viewToRestore = SharedPointer<CView> (view.cast<CViewContainer> ()->getView (1));
	viewToRestore = viewToRestore.cast<CViewContainer> ()->getView (0);

	memoryStream.rewind ();
	EXPECT (desc.storeViews ({viewToRestore}, memoryStream));
	memoryStream.rewind ();
	restoredView.clear ();
	EXPECT (desc.restoreViews (memoryStream, restoredView));
	view->removed (parentContainer);
}

TEST_CASE (UIDescriptionJSONTests, UpdateViewDescription)
{
	MemoryContentProvider provider (createViewUIDesc,
	                                static_cast<uint32_t> (strlen (createViewUIDesc)));
	UIDescription desc (&provider);
	EXPECT (desc.parse () == true);
	Controller controller;
	auto view = owned (desc.createView ("view", &controller));
	EXPECT (view);
	EXPECT (view->getTransparency () == false);
	view->setTransparency (true);
	desc.updateViewDescription ("view", view);
	auto attr = desc.getViewAttributes ("view");
	bool value;
	EXPECT (attr->getBooleanAttribute ("transparent", value));
	EXPECT (value == true);
}

TEST_CASE (UIDescriptionJSONTests, CustomAttributes)
{
	MemoryContentProvider provider (createViewUIDesc,
	                                static_cast<uint32_t> (strlen (createViewUIDesc)));
	UIDescription desc (&provider);
	EXPECT (desc.parse () == true);
	auto attr = desc.getCustomAttributes ("Test", false);
	EXPECT (attr == nullptr);
	attr = desc.getCustomAttributes ("Test", true);
	EXPECT (attr);
	EXPECT (desc.getCustomAttributes ("Test", false) == attr);
	EXPECT (desc.setCustomAttributes ("Test", nullptr) == false);
}

#ifndef _MSC_VER
TEST_CASE (UIDescriptionJSONTests, Listeners)
{
	MemoryContentProvider provider (completeExample,
	                                static_cast<uint32_t> (strlen (completeExample)));
	UIDescription desc (&provider);
	EXPECT (desc.parse () == true);

	DescriptionListenerMock mok (UIDescTestCase::TagChanged);
	desc.registerListener (&mok);

	desc.changeControlTagString ("NewTag", "5", true);
	EXPECT (mok.callCount () == 1);
	desc.changeControlTagString ("NewTag", "5", false);
	EXPECT (mok.callCount () == 2);
	desc.changeTagName ("NewTag", "NewTagNew");
	EXPECT (mok.callCount () == 3);
	desc.removeTag ("NewTagNew");
	EXPECT (mok.callCount () == 4);

	mok.setTestCase (UIDescTestCase::ColorChanged);
	EXPECT (mok.callCount () == 0);

	CColor newColor;
	desc.changeColor ("NewColor", newColor);
	EXPECT (mok.callCount () == 1);
	desc.changeColor ("NewColor", newColor);
	EXPECT (mok.callCount () == 2);
	desc.changeColorName ("NewColor", "NewColorNew");
	EXPECT (mok.callCount () == 3);
	desc.removeColor ("NewColorNew");
	EXPECT (mok.callCount () == 4);

	mok.setTestCase (UIDescTestCase::FontChanged);
	EXPECT (mok.callCount () == 0);

	auto font = makeOwned<CFontDesc> ();
	desc.changeFont ("NewFont", font);
	EXPECT (mok.callCount () == 1);
	desc.changeFont ("NewFont", font);
	EXPECT (mok.callCount () == 2);
	desc.changeFontName ("NewFont", "NewFontNew");
	EXPECT (mok.callCount () == 3);
	desc.changeAlternativeFontNames ("NewFontNew", "Hack, Menlo");
	EXPECT (mok.callCount () == 4);
	desc.removeFont ("NewFontNew");
	EXPECT (mok.callCount () == 5);

	mok.setTestCase (UIDescTestCase::BitmapChanged);
	EXPECT (mok.callCount () == 0);

	auto bitmap = makeOwned<CBitmap> (CPoint (10, 10));
	desc.changeBitmap ("NewBitmap", "bitmappath");
	EXPECT (mok.callCount () == 1);
	desc.changeBitmap ("NewBitmap", "bitmappath");
	EXPECT (mok.callCount () == 2);
	desc.changeBitmapName ("NewBitmap", "NewBitmapNew");
	EXPECT (mok.callCount () == 3);
	desc.removeBitmap ("NewBitmapNew");
	EXPECT (mok.callCount () == 4);

	mok.setTestCase (UIDescTestCase::GradientChanged);
	EXPECT (mok.callCount () == 0);

	auto gradient = owned (CGradient::create (0., 0., newColor, newColor));
	desc.changeGradient ("NewGradient", gradient);
	EXPECT (mok.callCount () == 1);
	desc.changeGradient ("NewGradient", gradient);
	EXPECT (mok.callCount () == 2);
	desc.changeGradientName ("NewGradient", "NewGradientNew");
	EXPECT (mok.callCount () == 3);
	desc.removeGradient ("NewGradientNew");
	EXPECT (mok.callCount () == 4);

	mok.setTestCase (UIDescTestCase::TemplateChanged);
	EXPECT (mok.callCount () == 0);

	desc.addNewTemplate ("NewTemplate", makeOwned<UIAttributes> ());
	EXPECT (mok.callCount () == 1);
	desc.changeTemplateName ("NewTemplate", "NewTemplateNew");
	EXPECT (mok.callCount () == 2);
	desc.duplicateTemplate ("NewTemplateNew", "NewTemplateNewDup");
	EXPECT (mok.callCount () == 3);
	desc.removeTemplate ("NewTemplateNew");
	EXPECT (mok.callCount () == 4);
	desc.removeTemplate ("NewTemplateNewDup");
	EXPECT (mok.callCount () == 5);

	desc.unregisterListener (&mok);
}
#endif // _MSC_VER

TEST_CASE (UIDescriptionJSONTests, FocusSettings)
{
	MemoryContentProvider provider (emptyUIDesc, static_cast<uint32_t> (strlen (emptyUIDesc)));
	UIDescription desc (&provider);
	EXPECT (desc.parse () == true);

	FocusDrawingSettings fd;
	fd.enabled = true;
	fd.width = 1.5;
	fd.colorName = "FocusColor";

	desc.setFocusDrawingSettings (fd);
	auto fd2 = desc.getFocusDrawingSettings ();
	EXPECT (!(fd != fd2));
}

TEST_CASE (UIDescriptionJSONTests, SharedResources)
{
	MemoryContentProvider provider (emptyUIDesc, static_cast<uint32_t> (strlen (emptyUIDesc)));
	UIDescription desc (&provider);
	EXPECT (desc.parse () == true);

	CColor color1;
	CColor color2;
	EXPECT (desc.getColor ("c1", color1) == false);
	EXPECT (desc.getFont ("f1") == nullptr);
	EXPECT (desc.getGradient ("g1") == nullptr);
	EXPECT (desc.getBitmap ("b1") == nullptr);

	MemoryContentProvider resProvider (sharedResourcesUIDesc,
	                                   static_cast<uint32_t> (strlen (sharedResourcesUIDesc)));
	UIDescription resDesc (&resProvider);
	EXPECT (resDesc.parse () == true);

	desc.setSharedResources (&resDesc);
	EXPECT (desc.getSharedResources () == &resDesc);
	EXPECT (desc.getColor ("c1", color1) == true);
	EXPECT (resDesc.getColor ("c1", color2) == true);
	EXPECT (color1 == color2);
	EXPECT (desc.getFont ("f1") != nullptr);
	EXPECT (desc.getFont ("f1") == resDesc.getFont ("f1"));
	EXPECT (desc.getGradient ("g1") != nullptr);
	EXPECT (desc.getGradient ("g1") == resDesc.getGradient ("g1"));
	EXPECT (desc.getBitmap ("b1") != nullptr);
	EXPECT (desc.getBitmap ("b1") == resDesc.getBitmap ("b1"));

	desc.setSharedResources (nullptr);
}

#if 0
TEST_CASE (UIDescriptionJSONTests, CompleteExample)
{
	MemoryContentProvider provider (completeExample, strlen (completeExample));
	SaveUIDescription desc (&provider);
	EXPECT (desc.parse () == true);
	auto view = owned (desc.createView ("view", nullptr));
	view = owned (desc.createView ("tab1", nullptr));
	view = owned (desc.createView ("tab2", nullptr));
	CMemoryStream outputStream (1024, 1024, false);
	EXPECT (desc.saveToStream (outputStream, SaveUIDescription::kWriteImagesIntoXMLFile, nullptr));
}
#endif

} // VSTGUI

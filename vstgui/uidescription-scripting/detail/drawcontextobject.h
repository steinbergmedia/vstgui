// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "scriptobject.h"
#include "../../lib/vstguifwd.h"
#include "../../uidescription/uidescriptionfwd.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace ScriptingInternal {

//------------------------------------------------------------------------
struct DrawContextObject : ScriptObject,
						   TJS::IScriptVarLifeTimeObserver
{
	DrawContextObject ();
	~DrawContextObject () noexcept override;

	void setDrawContext (CDrawContext* context, IUIDescription* uiDesc);

	void onDestroy (CScriptVar* v) override;

private:
	// CanvasRenderingContext2D API
	// clang-format off
/*
	fillStyle: string | CanvasGradient | CanvasPattern;
	filter: string
    font: string;
	fontKerning: string;
	fontStretch: string;
	fontVariantCaps: string;
    globalAlpha: number;
    globalCompositionOperation: string;
    lineCap: string;
    lineDashOffset: number;
    lineJoin: string;
    lineWidth: number;
    miterLimit: number;
    shadowBlur: number;
    shadowColor: string;
    shadowOffsetX: number;
    shadowOffsetY: number;
    strokeStyle: string;
    textAlign: string;
    textBaseline: string;
    arc: (x: number, y: number, r: number, sAngle: number, eAngle: number, counterClockwise?: boolean) => void;
    arcTo: (x1: number, y1: number, x2: number, y2: number, r: number) => void;
    beginPath: () => void;
    bezierCurveTo: (cp1x: number, cp1y: number, cp2x: number, cp2y: number, x: number, y: number) => void;
    clearRect: (x: number, y: number, width: number, height: number) => void;
    clip: () => void;
    closePath: () => void;
    createImageData: (width: number, height: number, imageData: ImageData) => void;
    createLinearGradient: (x0: number, yo: number, x1: number, y1: number) => CanvasGradient;
    createPattern: () => CanvasPattern;
    createRadialGradient: (x0: number, y0: number, r0: number, x1: number, y1: number, r1: number) => CanvasGradient;
    drawFocusIfNeeded: (html: HTMLElement) => void;
    drawImage: (image: Image,dx: number,dy: number,sx?: number,sy?: number,sWidth?: number,sHeight?: number,dWidth?: number,dHeight?: number) => void;
    ellipse: (x: number, y: number, radiusX: number, radiusY: number, rotation: number, startAngle: number, endAngle: number, anticlockwise?: boolean) => void;
    fill: (Path2D?: Path2D, fillRule?: any) => void;
    fillRect: (x: number, y: number, width: number, height: number) => void;
    fillText: (text: string, x: number, y: number, maxWidth?: number) => void;
    getImageData: (sx: number, sy: number, sw: number, sh: number) => Promise<ImageData>;
    getLineDash: () => number[];
    isPointInPath: (x: number, y: number, fillRule: any, path: Path2D) => boolean;
    isPointInStroke: (x: number, y: number, path: Path2D) => boolean;
    lineTo: (x: number, y: number) => void;
    measureText: (text: string) => any;
    moveTo: (x: number, y: number) => void;
    putImageData: (imageData: ImageData, dx: number, dy: number, dirtyX?: number, dirtyY?: number, dirtyWidth?: number, dirtyHeight?: number) => void;
    quadraticCurveTo: (cpx: number, cpy: number, x: number, y: number) => void;
    rect: (x: number, y: number, width: number, height: number) => void;
	reset: () => void
	resetTransform: () => void
	restore: () => void;
    rotate: (angle: number) => void;
	roundRect: (x: number, y: number, width: number, height: number, radii: number) => void
    save: () => void;
    scale: (x: number, y: number) => void;
    setLineDash: (segments: number[]) => void;
    setTransform: (a: number, b: number, c: number, d: number, e: number, f: number) => void;
    stroke: (path?: Path2D) => void;
    strokeRect: (x: number, y: number, width: number, height: number) => void;
    strokeText: (text: string, x: number, y: number, maxWidth?: number) => void;
    transform: (a: number, b: number, c: number, d: number, e: number, f: number) => void;
    translate: (x: number, y: number) => void;
*/
	// clang-format on

	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
} // ScriptingInternal
} // VSTGUI

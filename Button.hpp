//
//  Button.hpp  --  the one clickable widget the menus need.
//
#ifndef BUTTON_HPP
#define BUTTON_HPP


// ---------------------------------------------------------------------------
//  Visual Studio IntelliSense only -- this block is invisible to the compiler.
//
//  Every module is included from iMain.cpp in dependency order and the compiler
//  sees each one exactly once, so these includes are NOT needed to build. The
//  editor, however, parses this header on its own, and without them it marks
//  perfectly valid symbols in red. __INTELLISENSE__ is defined by the editor's
//  parser and never by cl.exe, so this costs the build nothing.
//
//  iGraphics.h has no include guard of its own and defines
//  STB_IMAGE_IMPLEMENTATION, so it is pulled in behind a sentinel to keep the
//  editor from processing it twice.
// ---------------------------------------------------------------------------
#ifdef __INTELLISENSE__
#ifndef IG_INTELLISENSE_BASE
#define IG_INTELLISENSE_BASE
#include "iGraphics.h"
#endif
#include "Config.hpp"
#include "Draw.hpp"
#endif
struct Button {
	double      x, y, w, h;
	const char *label;
};

bool buttonContains(const Button &b, int mx, int my)
{
	return mx >= b.x && mx <= b.x + b.w &&
	       my >= b.y && my <= b.y + b.h;
}

// "selected" covers both mouse hover and keyboard highlight -- they look the
// same on purpose, so the two input styles never disagree about what is active.
void buttonDraw(const Button &b, bool selected)
{
	if (selected) {
		dFillRectA(b.x, b.y, b.w, b.h, C_ACCENT, 0.92);
		dRectOutlineA(b.x, b.y, b.w, b.h, C_WHITE, 0.85, 2.0);

		dSetColor(C_INK);
		dText(b.x + 26, b.y + b.h / 2 - 6, b.label, GLUT_BITMAP_HELVETICA_18);

		// a small chevron so the highlight reads at a glance
		dSetColor(C_INK);
		dText(b.x + 11, b.y + b.h / 2 - 6, ">", GLUT_BITMAP_HELVETICA_18);
	} else {
		dFillRectA(b.x, b.y, b.w, b.h, C_PANEL, 0.62);
		dRectOutlineA(b.x, b.y, b.w, b.h, C_ACCENT_DIM, 0.55, 1.0);

		dSetColor(C_TEXT);
		dText(b.x + 26, b.y + b.h / 2 - 6, b.label, GLUT_BITMAP_HELVETICA_18);
	}
}

#endif // BUTTON_HPP

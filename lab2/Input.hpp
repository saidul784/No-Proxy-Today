//
//  Input.hpp  --  edge-triggered keyboard on top of iGraphics' level-triggered
//                 isKeyPressed() / isSpecialKeyPressed().
//
//  iGraphics only tells us whether a key is DOWN right now. fixedUpdate() runs
//  every 16 ms, so a menu driven straight off isKeyPressed() would scroll
//  through every item in a fraction of a second. We keep last frame's state and
//  report the moment a key goes from up to down.
//
//  Contract: call inputEndFrame() as the LAST thing in fixedUpdate().
//
#ifndef INPUT_HPP
#define INPUT_HPP


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
#endif
static int gPrevKey[256]     = { 0 };
static int gPrevSpecial[256] = { 0 };

// True only on the frame the key goes down.
bool keyJustPressed(unsigned char key)
{
	return isKeyPressed(key) != 0 && gPrevKey[key] == 0;
}

bool specialKeyJustPressed(unsigned char key)
{
	return isSpecialKeyPressed(key) != 0 && gPrevSpecial[key] == 0;
}

void inputEndFrame()
{
	for (int i = 0; i < 256; i++) {
		gPrevKey[i]     = isKeyPressed((unsigned char)i);
		gPrevSpecial[i] = isSpecialKeyPressed((unsigned char)i);
	}
}

// Forgets all held keys. Call on a screen change so a key held down during the
// transition does not immediately fire on the new screen.
void inputClear()
{
	for (int i = 0; i < 256; i++) {
		gPrevKey[i]     = 1;
		gPrevSpecial[i] = 1;
	}
}

#endif // INPUT_HPP

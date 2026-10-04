//
//  NameEntry.hpp  --  who is about to play, asked once between picking a level
//                     and the level actually starting.
//
//  The level chosen on the campaign map is held PENDING here. Nothing about it
//  starts -- no entry function, no timer, no attempt -- until the name is
//  confirmed, so backing out leaves no trace and sets no record.
//
//  Text entry rides the existing edge-triggered keyboard: every printable key
//  is polled once per frame through keyJustPressed(), which is the same thing
//  the menus already do, so there is no second input system.
//
#ifndef NAMEENTRY_HPP
#define NAMEENTRY_HPP


// ---------------------------------------------------------------------------
//  Visual Studio IntelliSense only -- this block is invisible to the compiler.
// ---------------------------------------------------------------------------
#ifdef __INTELLISENSE__
#ifndef IG_INTELLISENSE_BASE
#define IG_INTELLISENSE_BASE
#include "iGraphics.h"
#endif
#include "Config.hpp"
#include "Draw.hpp"
#include "Input.hpp"
#include "Assets.hpp"
#include "GameState.hpp"
#include "Button.hpp"
#include "HighScores.hpp"
#endif
#include <string.h>

// 0, 1 or 2 -- the level waiting on a name. -1 when nothing is pending.
static int  gPendingLevel = -1;
static char gNameBuf[HS_NAME_MAX + 1] = "";
static int  gNameLen = 0;
static double gNameCaret = 0.0;      // caret blink
static double gNameError = 0.0;      // how long the inline message stays up

static const char *kLevelTitle[3] = {
	"LEVEL 01  -  MOHANAGAR TO HATIRJHEEL",
	"LEVEL 02  -  THE RIVER AND THE PIRATE SHIP",
	"LEVEL 03  -  KUNIPARA TO AUST"
};

// Called from the campaign map instead of setState(STATE_LEVELnn).
void nameEntryBegin(int levelIndex)
{
	gPendingLevel = levelIndex;
	setState(STATE_NAME_ENTRY);
}

void nameEntryEnter()
{
	// Prefill with whoever played last this session, and put the caret at the
	// end so it can simply be typed over or extended.
	hsTrimCopy(gPlayerName, gNameBuf, sizeof(gNameBuf));
	gNameLen = (int)strlen(gNameBuf);
	gNameCaret = 0.0;
	gNameError = 0.0;
}

// ---------------------------------------------------------------------------
//  Layout
// ---------------------------------------------------------------------------
static const double NE_W = 620.0;
static const double NE_H = 300.0;

static double neX() { return (WIN_W - NE_W) * 0.5; }
static double neY() { return (WIN_H - NE_H) * 0.5; }

static Button neStartButton()
{
	Button b;
	b.w = 180.0; b.h = 44.0;
	b.x = neX() + NE_W - b.w - 34.0;
	b.y = neY() + 34.0;
	b.label = "START";
	return b;
}

static Button neBackButton()
{
	Button b;
	b.w = 180.0; b.h = 44.0;
	b.x = neX() + 34.0;
	b.y = neY() + 34.0;
	b.label = "BACK";
	return b;
}

// ---------------------------------------------------------------------------
//  Confirming
// ---------------------------------------------------------------------------
static void nameEntryStart()
{
	char clean[HS_NAME_MAX + 1];
	hsTrimCopy(gNameBuf, clean, sizeof(clean));

	if (clean[0] == '\0') {
		gNameError = 2.4;                 // inline, no modal, no beep
		return;
	}

	strcpy_s(gPlayerName, sizeof(gPlayerName), clean);

	int lv = gPendingLevel;
	gPendingLevel = -1;

	// setState() clears held keys, so the ENTER or the click that confirmed
	// this cannot carry through into the first frame of gameplay.
	if (lv == 0)      setState(STATE_LEVEL01);
	else if (lv == 1) setState(STATE_LEVEL02);
	else if (lv == 2) setState(STATE_LEVEL03);
	else              setState(STATE_LEVEL_SELECT);
}

static void nameEntryCancel()
{
	gPendingLevel = -1;                   // nothing started, nothing recorded
	setState(STATE_LEVEL_SELECT);
}

// ---------------------------------------------------------------------------
void nameEntryUpdate()
{
	gNameCaret += LS_TICK_SECONDS;
	if (gNameError > 0.0) {
		gNameError -= LS_TICK_SECONDS;
		if (gNameError < 0.0) gNameError = 0.0;
	}

	if (keyJustPressed(27)) { nameEntryCancel(); return; }
	if (keyJustPressed(13)) { nameEntryStart();  return; }

	if (keyJustPressed(8)) {              // backspace
		if (gNameLen > 0) {
			gNameLen--;
			gNameBuf[gNameLen] = '\0';
			gNameError = 0.0;
		}
		return;
	}

	// Every printable key, once per press. ENTER, ESC and backspace are dealt
	// with above and are outside this range anyway.
	for (int c = 32; c <= 126; c++) {
		if (!keyJustPressed((unsigned char)c)) continue;
		if (!hsCharIsAllowed((char)c)) continue;
		if (gNameLen >= HS_NAME_MAX) continue;

		gNameBuf[gNameLen++] = (char)c;
		gNameBuf[gNameLen] = '\0';
		gNameError = 0.0;
	}
}

void nameEntryDraw()
{
	if (TEX_POSTER != 0) iShowImage(0, 0, WIN_W, WIN_H, TEX_POSTER);
	else                 dFillRectA(0, 0, WIN_W, WIN_H, C_INK, 1.0);
	dDimScreen(0.82);

	const double x = neX(), y = neY();

	dFillRectA(x, y, NE_W, NE_H, C_PANEL, 0.92);
	dRectOutlineA(x, y, NE_W, NE_H, C_ACCENT, 0.75, 2.0);

	dSetColor(C_ACCENT);
	dStrokeText(x + 34, y + NE_H - 58, "WHO IS PLAYING?", 30, 2.2);

	dSetColor(C_TEXT_DIM);
	dText(x + 36, y + NE_H - 84,
	      (gPendingLevel >= 0 && gPendingLevel < 3) ? kLevelTitle[gPendingLevel] : "",
	      GLUT_BITMAP_HELVETICA_12);

	// --- the field ---
	const double fx = x + 34, fy = y + 120, fw = NE_W - 68, fh = 52;

	dFillRectA(fx, fy, fw, fh, C_INK, 0.85);
	dRectOutlineA(fx, fy, fw, fh, C_ACCENT, 0.8, 2.0);

	dSetColor(C_TEXT);
	dText(fx + 14, fy + fh / 2 - 7, gNameBuf, GLUT_BITMAP_HELVETICA_18);

	// caret, blinking, sitting after the last character
	if (((int)(gNameCaret * 2.0) % 2) == 0) {
		int tw = dTextWidth(gNameBuf, GLUT_BITMAP_HELVETICA_18);
		dFillRectA(fx + 16 + tw, fy + 12, 2.0, fh - 24, C_ACCENT, 0.95);
	}

	char count[48];
	sprintf_s(count, sizeof(count), "%d / %d", gNameLen, HS_NAME_MAX);
	dSetColor(C_TEXT_DIM);
	dTextRight(fx + fw - 12, fy - 18, count, GLUT_BITMAP_HELVETICA_12);

	if (gNameError > 0.0) {
		Color red = { 255, 90, 75 };
		dSetColor(red);
		dText(fx + 2, fy - 18, "Enter a name to start.", GLUT_BITMAP_HELVETICA_12);
	} else {
		dSetColor(C_TEXT_DIM);
		dText(fx + 2, fy - 18, "Type your name  -  ENTER to start  -  ESC to go back",
		      GLUT_BITMAP_HELVETICA_12);
	}

	buttonDraw(neBackButton(),  false);
	buttonDraw(neStartButton(), true);
}

void nameEntryMouse(int button, int state, int mx, int my)
{
	if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return;

	if (buttonContains(neStartButton(), mx, my)) { nameEntryStart();  return; }
	if (buttonContains(neBackButton(),  mx, my)) { nameEntryCancel(); return; }
}

#endif // NAMEENTRY_HPP

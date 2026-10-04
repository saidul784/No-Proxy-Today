//
//  MenuScreen.hpp  --  the title screen: game_poster.jpeg plus four buttons.
//
//  Mouse and keyboard both drive the same highlight index, so hovering with the
//  mouse and then pressing ENTER does what you would expect.
//
#ifndef MENUSCREEN_HPP
#define MENUSCREEN_HPP


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
#include "Input.hpp"
#include "Assets.hpp"
#include "GameState.hpp"
#include "Button.hpp"
#endif
enum MenuItem {
	MENU_NEW_GAME = 0,
	MENU_SCORES,
	MENU_KEYS,
	MENU_ABOUT,
	MENU_EXIT
};

static const char *kMenuLabels[MENU_ITEM_COUNT] = {
	"NEW GAME",
	"HIGHEST SCORES",
	"KEYS",
	"ABOUT THE GAME",
	"EXIT"
};

static int gMenuSelected = MENU_NEW_GAME;

// Buttons stack downward from BTN_TOP_Y.
static Button menuButton(int index)
{
	Button b;
	b.x     = BTN_X;
	b.w     = BTN_W;
	b.h     = BTN_H;
	b.y     = BTN_TOP_Y - (index + 1) * BTN_H - index * BTN_GAP;
	b.label = kMenuLabels[index];
	return b;
}

static void menuActivate(int index)
{
	switch (index) {
	case MENU_NEW_GAME: setState(STATE_LEVEL_SELECT); break;   // was STATE_LEVEL01
	case MENU_SCORES:   setState(STATE_SCORES);  break;
	case MENU_KEYS:     setState(STATE_KEYS);    break;
	case MENU_ABOUT:    setState(STATE_ABOUT);   break;
	case MENU_EXIT:     quitGame();              break;
	}
}

// ---------------------------------------------------------------------------
void menuEnter()
{
	gMenuSelected = MENU_NEW_GAME;
}

void menuDraw()
{
	// 1. the poster, filling the window
	if (TEX_POSTER != 0) {
		iShowImage(0, 0, WIN_W, WIN_H, TEX_POSTER);
	} else {
		dFillRectA(0, 0, WIN_W, WIN_H, C_INK, 1.0);
		dSetColor(C_ACCENT);
		dStrokeTextCentered(WIN_W / 2.0, WIN_H / 2.0 + 40, "NO PROXY TODAY", 54, 3.0);
		dSetColor(C_TEXT_DIM);
		dTextCentered(WIN_W / 2.0, WIN_H / 2.0 - 10,
		              "game_poster.jpeg could not be loaded - see the console window",
		              GLUT_BITMAP_HELVETICA_12);
	}

	// 2. a dark plate so the buttons stay readable over the artwork
	dFillRectA(MENU_PANEL_X, MENU_PANEL_Y, MENU_PANEL_W, MENU_PANEL_H, C_PANEL, 0.55);

	// 3. the buttons
	for (int i = 0; i < MENU_ITEM_COUNT; i++) {
		Button b = menuButton(i);
		buttonDraw(b, i == gMenuSelected);
	}

	// 4. footer hints, on their own plate -- the poster art underneath is busy
	//    enough to swallow plain text
	const char *hint   = "Arrow keys / mouse to choose  -  ENTER or click to select";
	const char *credit = "CSE-1200 Software Development I  -  AUST  -  Team A2_06";

	int hintW   = dTextWidth(hint,   GLUT_BITMAP_HELVETICA_12);
	int creditW = dTextWidth(credit, GLUT_BITMAP_HELVETICA_12);
	int plateW  = (hintW > creditW ? hintW : creditW) + 24;

	dFillRectA(WIN_W - 18 - plateW + 12, 8, (double)plateW, 44, C_PANEL, 0.68);

	dSetColor(C_TEXT);
	dTextRight(WIN_W - 18, 34, hint,   GLUT_BITMAP_HELVETICA_12);
	dSetColor(C_TEXT_DIM);
	dTextRight(WIN_W - 18, 16, credit, GLUT_BITMAP_HELVETICA_12);
}

void menuUpdate()
{
	if (specialKeyJustPressed(GLUT_KEY_UP)) {
		gMenuSelected = (gMenuSelected + MENU_ITEM_COUNT - 1) % MENU_ITEM_COUNT;
	}
	if (specialKeyJustPressed(GLUT_KEY_DOWN)) {
		gMenuSelected = (gMenuSelected + 1) % MENU_ITEM_COUNT;
	}
	if (keyJustPressed('w') || keyJustPressed('W')) {
		gMenuSelected = (gMenuSelected + MENU_ITEM_COUNT - 1) % MENU_ITEM_COUNT;
	}
	if (keyJustPressed('s') || keyJustPressed('S')) {
		gMenuSelected = (gMenuSelected + 1) % MENU_ITEM_COUNT;
	}

	if (keyJustPressed(13) || keyJustPressed(' ')) {   // 13 = ENTER
		menuActivate(gMenuSelected);
		return;
	}

	// number shortcuts
	for (int i = 0; i < MENU_ITEM_COUNT; i++) {
		if (keyJustPressed((unsigned char)('1' + i))) {
			gMenuSelected = i;
			menuActivate(i);
			return;
		}
	}

	if (keyJustPressed(27)) {   // ESC on the title screen quits
		quitGame();
	}
}

// Hovering moves the highlight, which keeps mouse and keyboard in sync.
void menuPassiveMouseMove(int mx, int my)
{
	for (int i = 0; i < MENU_ITEM_COUNT; i++) {
		Button b = menuButton(i);
		if (buttonContains(b, mx, my)) {
			gMenuSelected = i;
			return;
		}
	}
}

void menuMouse(int button, int state, int mx, int my)
{
	if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return;

	for (int i = 0; i < MENU_ITEM_COUNT; i++) {
		Button b = menuButton(i);
		if (buttonContains(b, mx, my)) {
			gMenuSelected = i;
			menuActivate(i);
			return;
		}
	}
}

#endif // MENUSCREEN_HPP

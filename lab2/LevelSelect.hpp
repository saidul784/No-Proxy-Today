//
//  LevelSelect.hpp  --  the campaign map that NEW GAME now opens.
//
//  The whole screen is the provided levelmap.jpeg drawn as-is. The artwork
//  already carries its own title board, legend and three numbered pins, so this
//  module adds only what the picture cannot do on its own: a status badge under
//  each pin, a hover highlight, a lock on level three, a back button and a
//  small campaign readout.
//
//  ASPECT RATIO: the map is 1536 x 1024 (3:2) and the window is 1200 x 675
//  (16:9). Stretching it to fill the window would squash it, so it is fitted
//  INSIDE the window at its true ratio and the two narrow side bars are painted
//  dark. Nothing is cropped and nothing is distorted.
//
//  Because of that fit, every position in this file is written in the map
//  image's own pixel coordinates and converted by mapX()/mapY(). Measure a
//  feature once in the .jpeg and it lands in the right place on screen, at any
//  window size.
//
#ifndef LEVELSELECT_HPP
#define LEVELSELECT_HPP

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
#include "LevelState.hpp"
#endif

// ---------------------------------------------------------------------------
//  Campaign progress
//
//  A plain in-memory struct with one instance. It is a file-scope global, so it
//  is initialised once when the program starts and NEVER again -- opening the
//  map, backing out to the menu and opening it again all leave it untouched.
//  Nothing in this file resets it.
// ---------------------------------------------------------------------------
struct CampaignProgress {
	bool level1Unlocked;
	bool level2Unlocked;
	bool level3Unlocked;

	bool level1Completed;
	bool level2Completed;
	bool level3Completed;
};

static CampaignProgress gCampaign = {
	true,   // level1Unlocked   -- Mohanogar Housing Society, open from the start
	true,   // level2Unlocked   -- Hateerjheel, open from the start
	false,  // level3Unlocked   -- Kunipara to AUST, locked until level 2 is done

	false,  // level1Completed
	false,  // level2Completed
	false   // level3Completed
};

// The hook Level 02 and Level 03 will call when they are built. Level 01 needs
// no hook: it already sets gLevel01Cleared, which is picked up in
// levelSelectSyncProgress() below, so Level01.hpp is not touched at all.
void campaignMarkLevelComplete(int level)
{
	if (level == 1) {
		gCampaign.level1Completed = true;
		gCampaign.level2Unlocked  = true;
	} else if (level == 2) {
		gCampaign.level2Completed = true;
		gCampaign.level3Unlocked  = true;   // level 3 opens ONLY from here
	} else if (level == 3) {
		gCampaign.level3Completed = true;
	}
}

static int campaignCompletedCount()
{
	int n = 0;
	if (gCampaign.level1Completed) n++;
	if (gCampaign.level2Completed) n++;
	if (gCampaign.level3Completed) n++;
	return n;
}

// Level 01 already raises gLevel01Cleared when it is beaten. Reading it here
// keeps the unlock rule working without editing a single line of Level01.hpp.
static void levelSelectSyncProgress()
{
	if (gLevel01Cleared)
		campaignMarkLevelComplete(1);
}

// ---------------------------------------------------------------------------
//  Fitting the map into the window without distorting it
// ---------------------------------------------------------------------------
static double gMapScale = 1.0;
static double gMapX = 0.0;
static double gMapY = 0.0;
static double gMapW = 0.0;
static double gMapH = 0.0;

static void levelSelectComputeFit()
{
	double iw = (SZ_LEVELMAP_W > 0) ? (double)SZ_LEVELMAP_W : LEVELMAP_NOMINAL_W;
	double ih = (SZ_LEVELMAP_H > 0) ? (double)SZ_LEVELMAP_H : LEVELMAP_NOMINAL_H;

	double sx = (double)WIN_W / iw;
	double sy = (double)WIN_H / ih;

	gMapScale = (sx < sy) ? sx : sy;      // "contain": the whole map stays visible
	gMapW = iw * gMapScale;
	gMapH = ih * gMapScale;
	gMapX = (WIN_W - gMapW) * 0.5;
	gMapY = (WIN_H - gMapH) * 0.5;
}

// Map-image pixel -> screen pixel. The image counts y downward from its top
// edge; iGraphics counts y upward from the bottom, hence the flip.
static double mapX(double imageX) { return gMapX + imageX * gMapScale; }
static double mapY(double imageY)
{
	double ih = (SZ_LEVELMAP_H > 0) ? (double)SZ_LEVELMAP_H : LEVELMAP_NOMINAL_H;
	return gMapY + (ih - imageY) * gMapScale;
}
static double mapLen(double imagePixels) { return imagePixels * gMapScale; }

// ---------------------------------------------------------------------------
//  The three levels
//
//  PIN_IMG_X / PIN_IMG_Y are the centre and base of each pin as it is painted
//  in levelmap.jpeg, measured off the artwork. The badge hangs directly beneath
//  the pin's pedestal so it never covers the pin itself.
// ---------------------------------------------------------------------------
const int LS_LEVEL_COUNT = 3;

static const double PIN_IMG_X[LS_LEVEL_COUNT] = { 198.0, 528.0, 852.0 };
static const double PIN_IMG_Y[LS_LEVEL_COUNT] = { 648.0, 362.0, 660.0 };

static const char *LS_NAME[LS_LEVEL_COUNT] = {
	"MOHANOGAR HOUSING SOCIETY",
	"HATEERJHEEL",
	"KUNIPARA TO AUST"
};

static const char *LS_NUMBER[LS_LEVEL_COUNT] = { "LEVEL 01", "LEVEL 02", "LEVEL 03" };

static bool levelIsUnlocked(int i)
{
	if (i == 0) return gCampaign.level1Unlocked;
	if (i == 1) return gCampaign.level2Unlocked;
	return gCampaign.level3Unlocked;
}

static bool levelIsCompleted(int i)
{
	if (i == 0) return gCampaign.level1Completed;
	if (i == 1) return gCampaign.level2Completed;
	return gCampaign.level3Completed;
}

// Wide enough for the longest line it has to hold, never narrower than the
// nominal width. "MOHANOGAR HOUSING SOCIETY" is a good deal longer than the
// other two names, so a single fixed width would either overflow on that one or
// leave the other two looking empty.
static double levelBadgeWidth(int i)
{
	int widest = dTextWidth(LS_NAME[i], GLUT_BITMAP_HELVETICA_10);

	int n = dTextWidth(LS_NUMBER[i], GLUT_BITMAP_HELVETICA_12);
	if (n > widest) widest = n;

	double need = widest + 26.0;
	return (need < LS_BADGE_W) ? LS_BADGE_W : need;
}

// The badge under a pin, in screen coordinates.
static Rect levelBadgeRect(int i)
{
	double w  = levelBadgeWidth(i);
	double cx = mapX(PIN_IMG_X[i]);
	double by = mapY(PIN_IMG_Y[i]);          // pedestal base
	return makeRect(cx - w * 0.5, by - LS_BADGE_H - 4.0, w, LS_BADGE_H);
}

// The clickable area: the badge plus the pin standing above it, and nothing
// more. The rest of the map is not clickable.
static Rect levelHitBox(int i)
{
	Rect b = levelBadgeRect(i);
	return makeRect(b.x, b.y, b.w, b.h + mapLen(LS_PIN_REACH_IMG));
}

static Rect levelSelectBackRect()
{
	return makeRect(LS_BACK_X, LS_BACK_Y, LS_BACK_W, LS_BACK_H);
}

// ---------------------------------------------------------------------------
//  Screen state
// ---------------------------------------------------------------------------
static int    gLsHover = -1;          // level index under the cursor, -1 = none
static bool   gLsBackHover = false;
static double gLsToastTimer = 0.0;    // seconds left on the message
static const char *gLsToastLine1 = "";
static const char *gLsToastLine2 = "";

static void levelSelectToast(const char *line1, const char *line2)
{
	gLsToastLine1 = line1;
	gLsToastLine2 = line2;
	gLsToastTimer = LS_TOAST_SECONDS;
}

void levelSelectEnter()
{
	levelSelectComputeFit();
	levelSelectSyncProgress();      // pick up a Level 01 win from this session
	gLsHover = -1;
	gLsBackHover = false;
	gLsToastTimer = 0.0;

	// The marker boxes are derived from the map fit, so print them once. If a
	// marker ever needs nudging, change PIN_IMG_X / PIN_IMG_Y (map-image pixels)
	// and read the new screen rectangle back from here.
	printf("[levelselect] map fit  %.0f x %.0f at (%.0f, %.0f)  scale %.3f\n",
	       gMapW, gMapH, gMapX, gMapY, gMapScale);
	for (int i = 0; i < LS_LEVEL_COUNT; i++) {
		Rect h = levelHitBox(i);
		printf("[levelselect] %-9s x %.0f  y %.0f  w %.0f  h %.0f   %s\n",
		       LS_NUMBER[i], h.x, h.y, h.w, h.h,
		       levelIsUnlocked(i) ? "UNLOCKED" : "LOCKED");
	}
	Rect b = levelSelectBackRect();
	printf("[levelselect] BACK      x %.0f  y %.0f  w %.0f  h %.0f\n",
	       b.x, b.y, b.w, b.h);
}

// ---------------------------------------------------------------------------
//  Drawing
// ---------------------------------------------------------------------------
static void levelSelectDrawBadge(int i)
{
	Rect b = levelBadgeRect(i);

	bool unlocked = levelIsUnlocked(i);
	bool hovered  = (gLsHover == i);
	bool done     = levelIsCompleted(i);

	if (unlocked) {
		// Active: dark plate, gold edge, brighter still under the cursor.
		dFillRectA(b.x, b.y, b.w, b.h, C_INK, hovered ? 0.90 : 0.78);
		dRectOutlineA(b.x, b.y, b.w, b.h, C_ACCENT, hovered ? 1.0 : 0.85,
		              hovered ? 2.5 : 1.5);
		if (hovered) {
			// A soft glow ring on the pin above, so the pin reads as live too.
			double cx = mapX(PIN_IMG_X[i]);
			double cy = mapY(PIN_IMG_Y[i]) + mapLen(28.0);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glColor4f(1.0f, 0.78f, 0.25f, 0.28f);
			iFilledEllipse(cx, cy, mapLen(52.0), mapLen(40.0));
			glDisable(GL_BLEND);
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		}
	} else {
		// Locked: darker, greyed edge, deliberately flat.
		dFillRectA(b.x, b.y, b.w, b.h, C_INK, 0.88);
		dRectOutlineA(b.x, b.y, b.w, b.h, C_TEXT_DIM, 0.55, 1.5);
	}

	double cx = b.x + b.w * 0.5;

	dSetColor(unlocked ? C_TEXT : C_TEXT_DIM);
	dTextCentered(cx, b.y + b.h - 15.0, LS_NAME[i], GLUT_BITMAP_HELVETICA_10);

	dSetColor(unlocked ? C_ACCENT : C_TEXT_DIM);
	dTextCentered(cx, b.y + b.h - 29.0, LS_NUMBER[i], GLUT_BITMAP_HELVETICA_12);

	if (!unlocked) {
		dSetColor(C_TEXT_DIM);
		dTextCentered(cx, b.y + 6.0, "[ LOCKED ]", GLUT_BITMAP_HELVETICA_10);
	} else if (done) {
		Color green = { 90, 220, 120 };
		dSetColor(green);
		dTextCentered(cx, b.y + 6.0, "COMPLETED", GLUT_BITMAP_HELVETICA_10);
	} else {
		Color green = { 120, 210, 130 };
		dSetColor(green);
		dTextCentered(cx, b.y + 6.0, "UNLOCKED", GLUT_BITMAP_HELVETICA_10);
	}
}

void levelSelectDraw()
{
	// 1. the map, fitted and undistorted, with the side bars painted in
	dFillRectA(0, 0, WIN_W, WIN_H, C_INK, 1.0);
	if (TEX_LEVELMAP != 0)
		dImage(gMapX, gMapY, gMapW, gMapH, TEX_LEVELMAP);

	// 2. title, sitting in the clear band between the map's own sign and its
	//    mission panel
	double tx = mapX(LS_TITLE_IMG_X);
	double ty = mapY(LS_TITLE_IMG_Y);
	double tw = mapLen(LS_TITLE_IMG_W);
	double th = mapLen(LS_TITLE_IMG_H);

	dFillRectA(tx, ty, tw, th, C_INK, 0.72);
	dRectOutlineA(tx, ty, tw, th, C_ACCENT, 0.65, 1.5);

	dSetColor(C_ACCENT);
	dTextCentered(tx + tw * 0.5, ty + th - 20.0, "DHAKA CAMPAIGN  -  LEVEL SELECT",
	              GLUT_BITMAP_HELVETICA_18);

	char buf[96];
	sprintf_s(buf, sizeof(buf), "CAMPAIGN STATUS      LEVELS COMPLETED: %d / 3",
	          campaignCompletedCount());
	dSetColor(C_TEXT);
	dTextCentered(tx + tw * 0.5, ty + 9.0, buf, GLUT_BITMAP_HELVETICA_12);

	// 3. the three markers
	for (int i = 0; i < LS_LEVEL_COUNT; i++)
		levelSelectDrawBadge(i);

	// 4. back button
	Rect bk = levelSelectBackRect();
	dFillRectA(bk.x, bk.y, bk.w, bk.h, C_INK, gLsBackHover ? 0.92 : 0.78);
	dRectOutlineA(bk.x, bk.y, bk.w, bk.h, C_ACCENT, gLsBackHover ? 1.0 : 0.7,
	              gLsBackHover ? 2.5 : 1.5);
	dSetColor(gLsBackHover ? C_ACCENT : C_TEXT);
	dTextCentered(bk.x + bk.w * 0.5, bk.y + bk.h * 0.5 - 6.0, "< BACK",
	              GLUT_BITMAP_HELVETICA_18);

	// 5. the locked message, if one is showing
	if (gLsToastTimer > 0.0) {
		double w = 380.0, h = 62.0;
		double x = (WIN_W - w) * 0.5;
		double y = 96.0;

		Color red = { 235, 70, 60 };
		dFillRectA(x, y, w, h, C_INK, 0.92);
		dRectOutlineA(x, y, w, h, red, 0.95, 2.0);

		dSetColor(red);
		dTextCentered(x + w * 0.5, y + h - 24.0, gLsToastLine1, GLUT_BITMAP_HELVETICA_18);
		dSetColor(C_TEXT);
		dTextCentered(x + w * 0.5, y + 14.0, gLsToastLine2, GLUT_BITMAP_HELVETICA_12);
	}
}

// ---------------------------------------------------------------------------
//  Update and input
// ---------------------------------------------------------------------------
void levelSelectUpdate()
{
	// The message fades on its own after a moment.
	if (gLsToastTimer > 0.0) {
		gLsToastTimer -= LS_TICK_SECONDS;
		if (gLsToastTimer < 0.0) gLsToastTimer = 0.0;
	}

	// ESC is the keyboard equivalent of BACK.
	if (keyJustPressed(27))
		setState(STATE_MENU);
}

// Hover tracking. Uses the mouse hooks the project already has.
void levelSelectPassiveMouseMove(int mx, int my)
{
	gLsHover = -1;
	for (int i = 0; i < LS_LEVEL_COUNT; i++) {
		Rect h = levelHitBox(i);
		if (mx >= h.x && mx <= h.x + h.w && my >= h.y && my <= h.y + h.h) {
			gLsHover = i;
			break;
		}
	}

	Rect bk = levelSelectBackRect();
	gLsBackHover = (mx >= bk.x && mx <= bk.x + bk.w &&
	                my >= bk.y && my <= bk.y + bk.h);
}

void levelSelectMouse(int button, int state, int mx, int my)
{
	if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return;

	Rect bk = levelSelectBackRect();
	if (mx >= bk.x && mx <= bk.x + bk.w && my >= bk.y && my <= bk.y + bk.h) {
		setState(STATE_MENU);          // progress stays in gCampaign
		return;
	}

	for (int i = 0; i < LS_LEVEL_COUNT; i++) {
		Rect h = levelHitBox(i);
		if (mx < h.x || mx > h.x + h.w || my < h.y || my > h.y + h.h) continue;

		if (!levelIsUnlocked(i)) {
			levelSelectToast("LEVEL LOCKED", "COMPLETE LEVEL 02 TO UNLOCK");
			return;
		}

		if (i == 0) {
			setState(STATE_LEVEL01);   // the existing Level 01, untouched
		} else if (i == 1) {
			// Part one of Level 02 -- the river crossing. It does NOT call
			// campaignMarkLevelComplete(2) yet, because part one is not the
			// whole level: Level 03 stays locked until the rest of Level 02
			// exists and something marks it beaten.
			setState(STATE_LEVEL02);
		} else {
			// Unlocked Level 03 has no gameplay yet either.
			levelSelectToast("LEVEL 03 NOT BUILT YET", "KUNIPARA TO AUST is coming later");
		}
		return;
	}

	// A click anywhere else dismisses the message.
	gLsToastTimer = 0.0;
}

#endif // LEVELSELECT_HPP

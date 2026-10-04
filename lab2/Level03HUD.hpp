//
//  Level03HUD.hpp  --  the read-outs for the road to AUST.
//
//  Level 03 has its own health, no shield and no lives, and what the player
//  needs instead is what he is carrying and how far along the road he is --
//  including on the jam, which is the last stretch of that same road. So this
//  is its own HUD rather than a rewrite of either of the others, which both
//  stay exactly as they were.
//
//  The score read-out is the SAME gScore the whole game shares.
//
#ifndef LEVEL03HUD_HPP
#define LEVEL03HUD_HPP


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
#include "LevelState.hpp"
#include "Level03State.hpp"
#include "Level03Entities.hpp"
#include "Level03Scenes.hpp"
#endif
// ---------------------------------------------------------------------------
//  The panel plate
//
//  hud_frame.png reassembled to whatever size a panel needs: the two ornate
//  end caps drawn at their own aspect ratio, and the middle stretched between
//  them. Stretching the whole banner instead would smear the gold ends
//  differently on every panel, because the panels are nothing like the same
//  shape as each other -- the score bar is 10:1 and the carry box is under
//  4:1.
//
//  A cap is never allowed to eat more than a third of the width, so a short
//  panel degrades to "mostly caps" rather than to two caps overlapping.
// ---------------------------------------------------------------------------
static void hudPlate(double x, double y, double w, double h)
{
	if (SPR_HUD_M.tex == 0) {          // artwork missing: the old flat plate
		dFillRectA(x, y, w, h, C_PANEL, 0.78);
		return;
	}

	double cap = h * spriteAspect(SPR_HUD_L);
	if (cap > w * 0.33) cap = w * 0.33;

	double mid = w - cap * 2.0;
	if (mid < 1.0) mid = 1.0;

	dImage(x,             y, cap, h, SPR_HUD_L.tex);
	dImage(x + cap,       y, mid, h, SPR_HUD_M.tex);
	dImage(x + cap + mid, y, cap, h, SPR_HUD_R.tex);
}

// What the current set-piece needs from him, in one line, top right.
static void l03HudObjective()
{
	char buf[96];
	const double x = WIN_W - 268.0, y = WIN_H - 78.0;

	hudPlate(x, y, 250, 58);

	const char *title = "OBJECTIVE";
	buf[0] = '\0';

	if (gL03Stage == L03_MICHIL) {
		title = "MICHIL";
		sprintf_s(buf, sizeof(buf), "%d / %d ROCKS", gL03MichilHits, MICHIL_HITS_TO_CLEAR);
	} else if (gL03Stage == L03_RAGGING) {
		title = "RAGGING";
		sprintf_s(buf, sizeof(buf), "LINE %d / %d", gL03TalkLine, TALK_LINE_COUNT);
	} else if (gL03Stage == L03_EVETEASING) {
		title = "EVE TEASERS";
		sprintf_s(buf, sizeof(buf), "%d / %d STICKS", gL03EveHits, EVE_HITS_TO_CLEAR);
	} else if (gL03Stage == L03_CHOR) {
		title = "CHOR  -  PRESS H";
		sprintf_s(buf, sizeof(buf), "%d / %d SLAPS", gL03Slaps, CHOR_SLAPS_TO_CLEAR);
	} else if (gL03Stage == L03_TRAFFIC) {
		// How much of the crossing is behind him. The jam ends where the road
		// does, so this is the same measure as the route bar, rescaled to the
		// stretch he is actually on.
		double from = L03_DIST_TRAFFIC;
		double span = level03RoadEnd() - from;
		double done = (span > 1.0) ? (gL03Progress - from) / span : 1.0;
		if (done < 0.0) done = 0.0;
		if (done > 1.0) done = 1.0;

		title = "CROSS THE JAM";
		sprintf_s(buf, sizeof(buf), "%d%% ACROSS", (int)(done * 100.0));
	} else if (gL03Stage == L03_ARRIVE) {
		title = "AUST";
		sprintf_s(buf, sizeof(buf), "THE GATE IS RIGHT THERE");
	} else if (gL03Stage == L03_LEVEL_WON) {
		title = "AUST";
		sprintf_s(buf, sizeof(buf), "ARRIVED");
	} else {
		title = "KEEP WALKING";
		sprintf_s(buf, sizeof(buf), "%d%% OF THE ROAD",
		          (int)(level03RoadFraction() * 100.0));
	}

	dSetColor(C_TEXT_DIM);
	dText(x + 30, y + 40, title, GLUT_BITMAP_HELVETICA_12);
	dSetColor(C_ACCENT);
	dText(x + 30, y + 12, buf, GLUT_BITMAP_HELVETICA_18);
}

void level03DrawHUD()
{
	char buf[160];

	const double barX = 24, barW = 300, barH = 24;
	const double scoreY = WIN_H - 44;
	const double barY   = WIN_H - 80;

	// ----- SCORE (the shared counter) ---------------------------------------
	sprintf_s(buf, sizeof(buf), "SCORE: %d", gScore);
	hudPlate(barX - 3, scoreY - 6, barW + 6, 34);
	dSetColor(C_ACCENT);
	dText(barX + 18, scoreY + 4, buf, GLUT_BITMAP_HELVETICA_18);

	// The coin count sits beside the score now that this level has coins to
	// count -- the same gCoinsTaken Levels 01 and 02 show.
	sprintf_s(buf, sizeof(buf), "coins %d   hits %d", gCoinsTaken, gL03HitsTaken);
	dSetColor(C_TEXT_DIM);
	dTextRight(barX + barW - 18, scoreY + 6, buf, GLUT_BITMAP_HELVETICA_12);

	// ----- HP out of LEVEL03_INITIAL_HP -------------------------------------
	int shownHP = gL03HP < 0 ? 0 : gL03HP;
	double frac = level03HealthFraction();

	// Wider than the bar so the ornate ends sit OUTSIDE the coloured fill
	// instead of being painted over by it.
	hudPlate(barX - 24, barY - 9, barW + 48, barH + 18);

	Color hpColor;
	if (frac > 0.55)      { hpColor.r = 80;  hpColor.g = 210; hpColor.b = 110; }
	else if (frac > 0.28) { hpColor.r = 245; hpColor.g = 185; hpColor.b = 45;  }
	else                  { hpColor.r = 235; hpColor.g = 70;  hpColor.b = 60;  }

	dFillRectA(barX, barY, barW, barH, C_INK, 0.88);
	dFillRectA(barX, barY, barW * frac, barH, hpColor, 0.95);
	// no outline: the banner's own gold edge is the border now

	sprintf_s(buf, sizeof(buf), "HP: %d / %d", shownHP, LEVEL03_INITIAL_HP);
	dSetColor(C_WHITE);
	dText(barX + 10, barY + 7, buf, GLUT_BITMAP_HELVETICA_12);

	// ----- what is happening right now ---------------------------------------
	hudPlate(barX - 3, barY - 38, 306, 32);
	dSetColor(C_SKY);
	dText(barX + 20, barY - 27, L03_STAGE_CAPTION[gL03Stage], GLUT_BITMAP_HELVETICA_12);

	// ----- what he is carrying -----------------------------------------------
	// He can only throw what he has picked up, so this has to be on screen: at
	// CARRY_NONE the throw key does nothing at all.
	{
		const double bx = barX, by = barY - 100.0;

		hudPlate(bx, by, 216, 58);
		dSetColor(C_TEXT_DIM);
		dText(bx + 30, by + 42, "F  THROW", GLUT_BITMAP_HELVETICA_12);

		if (gL03Carry == CARRY_NONE) {
			dSetColor(C_TEXT_DIM);
			dText(bx + 30, by + 14, "EMPTY HANDED", GLUT_BITMAP_HELVETICA_18);
		} else {
			Sprite held = (gL03Carry == CARRY_STICK) ? SPR_L03_STICK : SPR_L03_ROCK;
			double hh = 32.0;
			double hw = hh * spriteAspect(held);
			dImage(bx + 30, by + 6, hw, hh, held.tex);

			dSetColor(C_ACCENT);
			dText(bx + 38 + hw, by + 15,
			      (gL03Carry == CARRY_STICK) ? "STICK" : "ROCK",
			      GLUT_BITMAP_HELVETICA_18);
		}
	}

	l03HudObjective();

	// ----- the road ----------------------------------------------------------
	const double pX = 24, pY = 18, pW = WIN_W - 48, pH = 12;
	// The whole road, jam included -- the bar used to fill up the moment he
	// reached the jam, which made the last stretch look like an epilogue.
	double done = level03RoadFraction();

	dFillRectA(pX, pY, pW, pH, C_PANEL, 0.78);
	dFillRectA(pX, pY, pW * done, pH, C_ACCENT, 0.9);
	dRectOutlineA(pX, pY, pW, pH, C_WHITE, 0.4, 1.0);

	dSetColor(C_TEXT_DIM);
	dText(pX, pY + 18, "KUNIPARA", GLUT_BITMAP_HELVETICA_12);
	dTextRight(pX + pW, pY + 18, "AUST", GLUT_BITMAP_HELVETICA_12);
}

#endif // LEVEL03HUD_HPP

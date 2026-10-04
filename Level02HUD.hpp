//
//  Level02HUD.hpp  --  the read-outs for the river crossing.
//
//  Level 01's drawHUD() is built around 200 health, one life, a shield and a
//  two-minute clock, none of which apply here: Level 02 has 700 health, no
//  clock and no shield, and what the player needs instead is how many
//  crocodiles are left. So this is a separate HUD rather than a rewrite of the
//  Level 01 one, which stays exactly as it was.
//
//  The score read-out is the SAME gScore the whole game shares.
//
#ifndef LEVEL02HUD_HPP
#define LEVEL02HUD_HPP


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
#include "Level02State.hpp"
#include "Level02Entities.hpp"
#include "Level02Deck.hpp"
#endif
// The deck's read-out. While the dakat are aboard it counts them, six of them,
// three chidori each, and shows how many chidori he is holding; once the
// dakatleader is up it becomes the boss counter, 0 / 6.
static void l02HudDeckTally()
{
	char buf[80];
	const double x = WIN_W - 232.0, y = WIN_H - 78.0;

	dFillRectA(x, y, 208, 58, C_PANEL, 0.78);

	if (gL02Stage == L02_DAKATLEADER_BATTLE ||
	    gL02Stage == L02_HATIRJHEEL_RETURN ||
	    gL02Stage == L02_LEVEL_WON) {
		dSetColor(C_TEXT_DIM);
		dText(x + 12, y + 40, "DAKATLEADER", GLUT_BITMAP_HELVETICA_12);

		// The boss counter, read straight off the screen.
		sprintf_s(buf, sizeof(buf), "%d / %d", gL02LeaderHits, LEADER_HITS_TO_KILL);
		dSetColor(C_ACCENT);
		dText(x + 12, y + 12, buf, GLUT_BITMAP_HELVETICA_18);

		double barX = x + 82, barW = 114;
		double frac = 1.0 - (double)gL02LeaderHits / (double)LEADER_HITS_TO_KILL;
		if (frac < 0.0) frac = 0.0;

		Color red = { 210, 60, 70 };
		dFillRectA(barX, y + 14, barW, 16, C_INK, 0.88);
		dFillRectA(barX, y + 14, barW * frac, 16, red, 0.95);
		dRectOutlineA(barX, y + 14, barW, 16, C_WHITE, 0.5, 1.0);
		return;
	}

	dSetColor(C_TEXT_DIM);
	sprintf_s(buf, sizeof(buf), "DAKAT   %d / %d DOWN", gL02DakatKilled, DAKAT_TOTAL);
	dText(x + 12, y + 40, buf, GLUT_BITMAP_HELVETICA_12);

	// The ones on the deck right now, each with its own remaining hits.
	for (int i = 0; i < MAX_DAKAT; i++) {
		const Dakat &d = gDakat[i];

		double bx = x + 12 + i * 52.0;
		double by = y + 12;

		bool alive = d.active && !d.dead;

		dFillRectA(bx, by, 44, 18, alive ? C_ACCENT : C_INK, alive ? 0.92 : 0.45);
		dRectOutlineA(bx, by, 44, 18, C_WHITE, alive ? 0.55 : 0.20, 1.0);

		if (alive) {
			sprintf_s(buf, sizeof(buf), "%d", d.hp);
			dSetColor(C_INK);
			dTextCentered(bx + 22, by + 5, buf, GLUT_BITMAP_HELVETICA_12);
		}
	}
}

// One pip per crocodile still in the water, so "three hits each, four of them"
// is visible rather than something to be remembered.
static void l02HudCrocodileTally()
{
	const double x = WIN_W - 232.0, y = WIN_H - 78.0;

	dFillRectA(x, y, 208, 58, C_PANEL, 0.78);
	dSetColor(C_TEXT_DIM);
	dText(x + 12, y + 40, "CROCODILES", GLUT_BITMAP_HELVETICA_12);

	for (int i = 0; i < MAX_CROCODILES; i++) {
		const Crocodile &c = gCrocs[i];

		double bx = x + 12 + i * 48.0;
		double by = y + 12;

		bool killed  = c.dead || (!c.active && c.hitCount >= c.maxHp);
		bool inWater = c.active && !c.dead;

		Color box = killed ? C_PANEL : (inWater ? C_ACCENT : C_INK);
		dFillRectA(bx, by, 38, 18, box, killed ? 0.45 : 0.92);
		dRectOutlineA(bx, by, 38, 18, C_WHITE, killed ? 0.20 : 0.55, 1.0);

		if (inWater) {
			char pip[8];
			sprintf_s(pip, sizeof(pip), "%d", c.hp);
			dSetColor(C_INK);
			dTextCentered(bx + 19, by + 5, pip, GLUT_BITMAP_HELVETICA_12);
		}
	}
}

void level02DrawHUD()
{
	char buf[160];

	const double barX = 24, barW = 320, barH = 24;
	const double scoreY = WIN_H - 44;
	const double barY   = WIN_H - 80;

	// ----- SCORE (the shared counter) ---------------------------------------
	sprintf_s(buf, sizeof(buf), "SCORE: %d", gScore);
	dFillRectA(barX - 3, scoreY - 4, barW + 6, 30, C_PANEL, 0.78);
	dSetColor(C_ACCENT);
	dText(barX + 8, scoreY + 4, buf, GLUT_BITMAP_HELVETICA_18);

	sprintf_s(buf, sizeof(buf), "coins  %d", gCoinsTaken);
	dSetColor(C_TEXT_DIM);
	dTextRight(barX + barW - 8, scoreY + 6, buf, GLUT_BITMAP_HELVETICA_12);

	// ----- HP out of LEVEL02_INITIAL_HP -------------------------------------
	int shownHP = gL02HP < 0 ? 0 : gL02HP;
	double frac = level02HealthFraction();

	dFillRectA(barX - 3, barY - 3, barW + 6, barH + 6, C_PANEL, 0.78);

	Color hpColor;
	if (frac > 0.55)      { hpColor.r = 80;  hpColor.g = 210; hpColor.b = 110; }
	else if (frac > 0.28) { hpColor.r = 245; hpColor.g = 185; hpColor.b = 45;  }
	else                  { hpColor.r = 235; hpColor.g = 70;  hpColor.b = 60;  }

	dFillRectA(barX, barY, barW, barH, C_INK, 0.88);
	dFillRectA(barX, barY, barW * frac, barH, hpColor, 0.95);
	dRectOutlineA(barX, barY, barW, barH, C_WHITE, 0.5, 1.0);

	sprintf_s(buf, sizeof(buf), "HP: %d / %d", shownHP, LEVEL02_INITIAL_HP);
	dSetColor(C_WHITE);
	dText(barX + 10, barY + 7, buf, GLUT_BITMAP_HELVETICA_12);

	// ----- what is happening right now ---------------------------------------
	dFillRectA(barX, barY - 34, 320, 26, C_PANEL, 0.78);
	dSetColor(C_SKY);
	dText(barX + 10, barY - 27, L02_STAGE_CAPTION[gL02Stage], GLUT_BITMAP_HELVETICA_12);

	// The boss read-out follows him back to the river, so the 6 / 6 he finished
	// on is still on screen under the win message.
	if (level02OnDeck() || level02Returning())
		l02HudDeckTally();
	else
		l02HudCrocodileTally();

	// What he is holding. Chidori are spent one per throw and only an icon puts
	// more in his hand, so the count has to be on screen -- at zero, pressing
	// SPACE does nothing until he walks into another icon.
	if (level02OnDeck() && gL02Stage != L02_LEVEL_WON) {
		const double bx = barX, by = barY - 106.0;

		dFillRectA(bx, by, 216, 60, C_PANEL, 0.78);
		dSetColor(C_TEXT_DIM);
		dText(bx + 10, by + 44, "SPACE  THROW", GLUT_BITMAP_HELVETICA_12);

		if (gL02HasPowerful) {
			double hh = 34.0;
			double hw = hh * spriteAspect(SPR_L02B_POWERFUL);
			dImageAdditive(bx + 10, by + 6, hw, hh, SPR_L02B_POWERFUL.tex, 1.0);

			dSetColor(C_ACCENT);
			dText(bx + 18 + hw, by + 16, "POWERFUL CHIDORI", GLUT_BITMAP_HELVETICA_12);
		} else {
			double hh = 32.0;
			double hw = hh * spriteAspect(SPR_L02B_ICON);
			dImage(bx + 10, by + 7, hw, hh, SPR_L02B_ICON.tex);

			sprintf_s(buf, sizeof(buf), "CHIDORI  x %d", gL02Chidori);
			dSetColor(gL02Chidori > 0 ? C_ACCENT : C_TEXT_DIM);
			dText(bx + 18 + hw, by + 16, buf, GLUT_BITMAP_HELVETICA_18);
		}
	}
}

#endif // LEVEL02HUD_HPP

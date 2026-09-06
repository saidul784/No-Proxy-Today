//
//  HUD.hpp  --  SCORE, HP, LIVES, TIME and SHIELD, all live.
//
#ifndef HUD_HPP
#define HUD_HPP


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
#endif
static void hudHeart(double cx, double cy, double r, bool filled)
{
	const Color c = filled ? C_ACCENT : C_TEXT_DIM;
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f((float)(c.r / 255.0), (float)(c.g / 255.0), (float)(c.b / 255.0), filled ? 1.0f : 0.30f);

	iFilledCircle(cx - r * 0.42, cy + r * 0.30, r * 0.55);
	iFilledCircle(cx + r * 0.42, cy + r * 0.30, r * 0.55);

	double xs[3] = { cx - r * 0.95, cx + r * 0.95, cx };
	double ys[3] = { cy + r * 0.34, cy + r * 0.34, cy - r * 0.95 };
	iFilledPolygon(xs, ys, 3);

	glDisable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void drawHUD()
{
	char buf[160];

	const double barX = 24, barW = 300, barH = 24;
	const double scoreY = WIN_H - 44;
	const double barY   = WIN_H - 80;

	// ----- SCORE ------------------------------------------------------------
	sprintf_s(buf, sizeof(buf), "SCORE: %d", gScore);
	dFillRectA(barX - 3, scoreY - 4, barW + 6, 30, C_PANEL, 0.78);
	dSetColor(C_ACCENT);
	dText(barX + 8, scoreY + 4, buf, GLUT_BITMAP_HELVETICA_18);

	sprintf_s(buf, sizeof(buf), "coins  %d", gCoinsTaken);
	dSetColor(C_TEXT_DIM);
	dTextRight(barX + barW - 8, scoreY + 6, buf, GLUT_BITMAP_HELVETICA_12);

	// ----- HP ---------------------------------------------------------------
	int shownHP = gHP < 0 ? 0 : gHP;
	double frac = (double)shownHP / (double)PLAYER_MAX_HP;

	dFillRectA(barX - 3, barY - 3, barW + 6, barH + 6, C_PANEL, 0.78);

	Color hpColor;
	if (frac > 0.55)      { hpColor.r = 80;  hpColor.g = 210; hpColor.b = 110; }
	else if (frac > 0.28) { hpColor.r = 245; hpColor.g = 185; hpColor.b = 45;  }
	else                  { hpColor.r = 235; hpColor.g = 70;  hpColor.b = 60;  }

	dFillRectA(barX, barY, barW, barH, C_INK, 0.88);
	dFillRectA(barX, barY, barW * frac, barH, hpColor, 0.95);
	dRectOutlineA(barX, barY, barW, barH, C_WHITE, 0.5, 1.0);

	sprintf_s(buf, sizeof(buf), "HP: %d / %d", shownHP, PLAYER_MAX_HP);
	dSetColor(C_WHITE);
	dText(barX + 10, barY + 7, buf, GLUT_BITMAP_HELVETICA_12);

	// ----- LIVES ------------------------------------------------------------
	sprintf_s(buf, sizeof(buf), "LIVES: %d", gLives);
	dFillRectA(barX, barY - 34, 168, 26, C_PANEL, 0.78);
	dSetColor(C_TEXT);
	dText(barX + 10, barY - 27, buf, GLUT_BITMAP_HELVETICA_12);
	for (int i = 0; i < PLAYER_LIVES; i++)
		hudHeart(barX + 108 + i * 30, barY - 21, 11, i < gLives);

	// ----- SHIELD  (always shown, so 0 reads as clearly as 5) ----------------
	sprintf_s(buf, sizeof(buf), "SHIELD: %d", gShield);
	bool up = gShield > 0;
	dFillRectA(barX, barY - 68, 168, 28, up ? C_INK : C_PANEL, up ? 0.86 : 0.78);
	dRectOutlineA(barX, barY - 68, 168, 28, C_SKY, up ? 0.9 : 0.25, 1.0);
	dSetColor(up ? C_SKY : C_TEXT_DIM);
	dText(barX + 11, barY - 60, buf, GLUT_BITMAP_HELVETICA_18);

	// ----- TIME -------------------------------------------------------------
	int total = (int)(gTimeLeft < 0 ? 0 : gTimeLeft);
	sprintf_s(buf, sizeof(buf), "%02d:%02d", total / 60, total % 60);

	double clockW = dStrokeTextWidth(buf, 40) + 56;
	dFillRectA(WIN_W / 2.0 - clockW / 2.0, WIN_H - 64, clockW, 54, C_PANEL, 0.78);

	dSetColor(C_TEXT_DIM);
	dTextCentered(WIN_W / 2.0, WIN_H - 22, "TIME", GLUT_BITMAP_HELVETICA_12);

	if (gTimeLeft <= 30.0 && ((int)(gTimeLeft * 2.0) % 2) == 0) {
		Color red = { 235, 70, 60 };
		dSetColor(red);
	} else if (gTimeLeft <= 60.0) {
		dSetColor(C_ACCENT);
	} else {
		dSetColor(C_SKY);
	}
	dStrokeTextCentered(WIN_W / 2.0, WIN_H - 52, buf, 40, 3.0);

	// ----- route ------------------------------------------------------------
	const double pX = 24, pY = 18, pW = WIN_W - 48, pH = 12;
	double done = levelFraction();

	dFillRectA(pX, pY, pW, pH, C_PANEL, 0.78);
	dFillRectA(pX, pY, pW * done, pH, C_ACCENT, 0.9);
	dRectOutlineA(pX, pY, pW, pH, C_WHITE, 0.4, 1.0);

	dSetColor(C_TEXT_DIM);
	dText(pX, pY + 18, "MOHANAGAR", GLUT_BITMAP_HELVETICA_12);
	dTextRight(pX + pW, pY + 18, "HATIRJHEEL", GLUT_BITMAP_HELVETICA_12);

	sprintf_s(buf, sizeof(buf), "%d%%", (int)(done * 100.0));
	dSetColor(C_WHITE);
	dTextCentered(pX + pW / 2.0, pY + 18, buf, GLUT_BITMAP_HELVETICA_12);
}

#endif // HUD_HPP

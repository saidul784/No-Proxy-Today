//
//  ScoresScreen.hpp  --  the five best scores on each level, one tab per level.
//
//  Read-only. It never writes, never re-stamps a record and never re-sorts the
//  stored data: opening the leaderboard has no effect on it whatsoever.
//
#ifndef SCORESSCREEN_HPP
#define SCORESSCREEN_HPP


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
#include "Page.hpp"
#include "HighScores.hpp"
#endif

static int gScoresTab = 0;           // 0, 1, 2  ->  level 1, 2, 3

static const char *kScoreTabLabel[HS_LEVELS] = { "LEVEL 1", "LEVEL 2", "LEVEL 3" };
static const char *kScoreTabName[HS_LEVELS]  = {
	"MOHANAGAR TO HATIRJHEEL",
	"THE RIVER AND THE PIRATE SHIP",
	"KUNIPARA TO AUST"
};

void scoresEnter()
{
	gScoresTab = 0;
}

// ---------------------------------------------------------------------------
//  Layout
// ---------------------------------------------------------------------------
static Button scoresTabButton(int i)
{
	Button b;
	b.w = 190.0; b.h = 40.0;
	b.x = PAGE_MARGIN_X + i * (b.w + 14.0);
	b.y = PAGE_BODY_TOP - 46.0;
	b.label = kScoreTabLabel[i];
	return b;
}

static Button scoresBackButton()
{
	Button b;
	b.w = 150.0; b.h = 40.0;
	b.x = PAGE_MARGIN_X;
	b.y = PAGE_BODY_BOT - 12.0;
	b.label = "BACK";
	return b;
}

// ---------------------------------------------------------------------------
void scoresDraw()
{
	pageDrawBackdrop("HIGHEST SCORES", "Top five on each level  -  best score per player");

	for (int i = 0; i < HS_LEVELS; i++)
		buttonDraw(scoresTabButton(i), i == gScoresTab);

	const int level = gScoresTab + 1;
	const int rows  = highScoreCount(level);

	// --- the level this tab is showing ---
	dSetColor(C_ACCENT);
	dText(PAGE_MARGIN_X, PAGE_BODY_TOP - 84.0, kScoreTabName[gScoresTab],
	      GLUT_BITMAP_HELVETICA_18);

	// --- column headings ---
	const double colRank  = PAGE_MARGIN_X + 10.0;
	const double colName  = PAGE_MARGIN_X + 110.0;
	const double colScore = WIN_W - PAGE_MARGIN_X - 30.0;
	const double headY    = PAGE_BODY_TOP - 120.0;

	dSetColor(C_TEXT_DIM);
	dText(colRank,  headY, "RANK",   GLUT_BITMAP_HELVETICA_12);
	dText(colName,  headY, "PLAYER", GLUT_BITMAP_HELVETICA_12);
	dTextRight(colScore, headY, "HIGHEST SCORE", GLUT_BITMAP_HELVETICA_12);

	dFillRectA(PAGE_MARGIN_X, headY - 10.0,
	           WIN_W - 2 * PAGE_MARGIN_X, 1.0, C_ACCENT_DIM, 0.6);

	if (rows <= 0) {
		dSetColor(C_TEXT_DIM);
		dText(colRank, headY - 54.0,
		      "No scores yet. Play this level to set a record.",
		      GLUT_BITMAP_HELVETICA_18);
	}

	// --- the rows ---
	for (int i = 0; i < rows; i++) {
		const ScoreRow *r = highScoreRow(level, i);
		if (r == 0) continue;

		const double y = headY - 42.0 - i * 40.0;

		// a plate behind alternate rows, so long names stay easy to follow
		if ((i % 2) == 0)
			dFillRectA(PAGE_MARGIN_X, y - 10.0,
			           WIN_W - 2 * PAGE_MARGIN_X, 34.0, C_INK, 0.35);

		char rank[16];
		sprintf_s(rank, sizeof(rank), "%d", i + 1);

		// The top row is the one worth looking at, so it gets the accent.
		dSetColor(i == 0 ? C_ACCENT : C_TEXT_DIM);
		dText(colRank, y, rank, GLUT_BITMAP_HELVETICA_18);

		dSetColor(i == 0 ? C_ACCENT : C_TEXT);
		dText(colName, y, r->display, GLUT_BITMAP_HELVETICA_18);

		char score[24];
		sprintf_s(score, sizeof(score), "%d", r->score);
		dSetColor(i == 0 ? C_ACCENT : C_TEXT);
		dTextRight(colScore, y, score, GLUT_BITMAP_HELVETICA_18);
	}

	buttonDraw(scoresBackButton(), false);

	dSetColor(C_TEXT_DIM);
	dTextRight(WIN_W - PAGE_MARGIN_X, PAGE_BODY_BOT - 2.0,
	           "LEFT / RIGHT or 1-3 to switch level  -  ESC to go back",
	           GLUT_BITMAP_HELVETICA_12);
}

void scoresUpdate()
{
	if (specialKeyJustPressed(GLUT_KEY_RIGHT) || keyJustPressed('d') || keyJustPressed('D'))
		gScoresTab = (gScoresTab + 1) % HS_LEVELS;

	if (specialKeyJustPressed(GLUT_KEY_LEFT) || keyJustPressed('a') || keyJustPressed('A'))
		gScoresTab = (gScoresTab + HS_LEVELS - 1) % HS_LEVELS;

	for (int i = 0; i < HS_LEVELS; i++)
		if (keyJustPressed((unsigned char)('1' + i))) gScoresTab = i;

	if (keyJustPressed(27) || keyJustPressed(8))      // ESC or BACKSPACE
		setState(STATE_MENU);
}

void scoresMouse(int button, int state, int mx, int my)
{
	if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return;

	for (int i = 0; i < HS_LEVELS; i++)
		if (buttonContains(scoresTabButton(i), mx, my)) { gScoresTab = i; return; }

	if (buttonContains(scoresBackButton(), mx, my)) setState(STATE_MENU);
}

void scoresPassiveMouseMove(int mx, int my)
{
	// Hovering a tab previews it, the same way the main menu's highlight works.
	for (int i = 0; i < HS_LEVELS; i++)
		if (buttonContains(scoresTabButton(i), mx, my)) { gScoresTab = i; return; }
}

#endif // SCORESSCREEN_HPP

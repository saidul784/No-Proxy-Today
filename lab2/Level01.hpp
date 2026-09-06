//
//  Level01.hpp  --  Mohanagar to Hatirjheel.
//
//  The character runs on one fixed ground line and never travels along the
//  street. The street scrolls past, cycling through every image in the backdrop
//  folder. One obstacle at a time -- bike, car, dog, rickshaw or a flock of
//  birds -- arrives from far enough away that its own sound plays first and
//  there is time to react.
//
//  Jump is the only control that matters. Coins are worth 10 each. 200 health,
//  one life, two minutes to the finish gate.
//
#ifndef LEVEL01_HPP
#define LEVEL01_HPP


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
#include "Collision.hpp"
#include "Draw.hpp"
#include "Input.hpp"
#include "Audio.hpp"
#include "Assets.hpp"
#include "BackgroundScan.hpp"
#include "GameState.hpp"
#include "LevelState.hpp"
#include "Entities.hpp"
#include "Player.hpp"
#include "HUD.hpp"
#endif
// Wall-clock source. fixedUpdate() fires on a Windows timer whose spacing is
// only roughly 16 ms, so the run is paced off real elapsed time.
static DWORD gLastTick = 0;

// A brief "+10" that floats up where a coin was taken.
static double gRushBanner = 0.0;      // seconds left on the "RUSH HOUR" flash
static bool   gRushAnnounced = false;

static double gCoinPopTimer = 0.0;
static double gCoinPopX = 0.0;
static double gCoinPopY = 0.0;

// ---------------------------------------------------------------------------
//  Backdrop  --  a continuous horizontal scroll through the image sequence
// ---------------------------------------------------------------------------
void drawBackground()
{
	if (gBgCount <= 0) {
		dFillRectA(0, 0, WIN_W, WIN_H, C_INK, 1.0);
		dSetColor(C_TEXT_DIM);
		dTextCentered(WIN_W / 2.0, WIN_H / 2.0,
		              "No backdrop images found in the \"backround images\" folder",
		              GLUT_BITMAP_HELVETICA_18);
		return;
	}

	// Two screens each frame: the one scrolling out and the one coming in
	// behind it, so the street never shows a gap.
	double screen = gProgress / (double)WIN_W;
	int    index  = (int)screen % gBgCount;
	double offset = (screen - floor(screen)) * WIN_W;
	int    next   = (index + 1) % gBgCount;

	dImage(-offset,        0, WIN_W, WIN_H, TEX_BACKGROUND[index]);
	dImage(WIN_W - offset, 0, WIN_W, WIN_H, TEX_BACKGROUND[next]);

	// Two different streets butted together leaves a hard vertical line. A soft
	// shadow over the join reads as the gap between buildings instead.
	Color seam = { 18, 22, 32 };
	dSeamShade(WIN_W - offset, 78.0, seam, 0.45);
}

// ---------------------------------------------------------------------------
//  Finish line
// ---------------------------------------------------------------------------
void drawFinishLine()
{
	if (distanceToFinish() > FINISH_APPROACH) return;

	double x = PLAYER_X + (distanceToFinish() / FINISH_APPROACH) * (WIN_W + 300.0 - PLAYER_X);

	const double postW = 20.0, gateH = 300.0, base = 20.0;
	Color dark = { 30, 34, 46 };

	dFillRectA(x - 8,       base, postW, gateH, dark, 0.95);
	dFillRectA(x + 210 - 8, base, postW, gateH, dark, 0.95);

	const double bannerY = base + gateH - 62.0;
	const double cell = 22.0;
	for (int row = 0; row < 2; row++)
		for (int col = 0; col < 10; col++) {
			Color c = (((row + col) % 2) == 0) ? C_WHITE : dark;
			dFillRectA(x + col * cell, bannerY + row * cell, cell, cell, c, 0.97);
		}

	dFillRectA(x - 10, bannerY - 34, 240, 30, C_INK, 0.88);
	dSetColor(C_ACCENT);
	dTextCentered(x + 110, bannerY - 26, "HATIRJHEEL", GLUT_BITMAP_HELVETICA_18);
}

// ---------------------------------------------------------------------------
//  Loading screen, painted straight to the buffer while textures stream in
// ---------------------------------------------------------------------------
static void levelLoadingScreen(const char *stage, int done, int total)
{
	iClear();

	if (TEX_POSTER) {
		iShowImage(0, 0, WIN_W, WIN_H, TEX_POSTER);
		dDimScreen(0.82);
	} else {
		dFillRectA(0, 0, WIN_W, WIN_H, C_INK, 1.0);
	}

	dSetColor(C_ACCENT);
	dStrokeTextCentered(WIN_W / 2.0, WIN_H / 2.0 + 46, "LEVEL 01", 46, 3.0);

	dSetColor(C_TEXT);
	dTextCentered(WIN_W / 2.0, WIN_H / 2.0 + 12, "MOHANAGAR  ->  HATIRJHEEL", GLUT_BITMAP_HELVETICA_18);

	const double bw = 520, bh = 16;
	const double bx = WIN_W / 2.0 - bw / 2.0, by = WIN_H / 2.0 - 42;
	double frac = (total > 0) ? (double)done / (double)total : 0.0;

	dFillRectA(bx, by, bw, bh, C_PANEL, 0.85);
	dFillRectA(bx, by, bw * frac, bh, C_ACCENT, 0.95);
	dRectOutlineA(bx, by, bw, bh, C_WHITE, 0.45, 1.0);

	char buf[128];
	sprintf_s(buf, sizeof(buf), "loading %s   %d / %d", stage, done, total);
	dSetColor(C_TEXT_DIM);
	dTextCentered(WIN_W / 2.0, by - 24, buf, GLUT_BITMAP_HELVETICA_12);

	glutSwapBuffers();
}

static void levelLoadProgress(const char *stage, int done, int total)
{
	if (done % 2 == 0 || done == total)
		levelLoadingScreen(stage, done, total);
}

// ---------------------------------------------------------------------------
//  Collisions
//
//  Everything goes through checkCollision() on bounding boxes, and both boxes
//  carry a real y. That is what makes the jump work: lift the character's box
//  above the car's box and the two simply do not overlap, so no health is lost.
//  X alone is never enough.
// ---------------------------------------------------------------------------
void checkCollisions()
{
	Rect body = characterHitBox();

	// --- obstacles ---
	for (int i = 0; i < MAX_ROAD_OBSTACLES; i++) {
		Obstacle &o = gObstacles[i];
		if (!o.active) continue;

		if (!o.hasHitCharacter && checkCollision(body, obstacleHitBox(o))) {
			o.hasHitCharacter = true;      // spent, whatever happens next
			applyObstacleHit(o.type);
		}

		// A clean pass counts as a dodge.
		if (!o.scored && !o.hasHitCharacter) {
			Rect r = obstacleDrawRect(o);
			bool past = (o.mode == MOVE_COMING) ? (r.x + r.w < body.x)
			                                    : (r.x > body.x + body.w);
			if (past) {
				o.scored = true;
				gDodgeCount++;
			}
		}
	}

	// --- coins: score only, never health, never the shield ---
	for (int i = 0; i < MAX_COINS; i++) {
		Coin &c = gCoins[i];
		if (!c.active || c.collected) continue;
		if (!checkCollision(body, coinHitBox(c))) continue;

		Rect cr = coinDrawRect(c);
		c.collected = true;
		c.active = false;              // gone, so it cannot score twice
		collectCoin();

		gCoinPopX = cr.x + cr.w * 0.5;
		gCoinPopY = cr.y + cr.h;
		gCoinPopTimer = 0.7;
	}

	// --- power-ups ---
	for (int i = 0; i < MAX_PICKUPS; i++) {
		Pickup &p = gPickups[i];
		if (!p.active) continue;
		if (!checkCollision(body, pickupHitBox(p))) continue;

		p.active = false;              // collected, so it disappears

		if (p.type == PU_HEALTH) healCharacter(HEAL_AMOUNT);
		else                     activateShield();
	}
}

// ---------------------------------------------------------------------------
//  Entry and restart
// ---------------------------------------------------------------------------
static void levelStart()
{
	levelReset();
	resetCharacter();
	resetEntities();

	// Long enough for a run of about TARGET_RUN_SECONDS, but never shorter than
	// one pass through every backdrop image, so none goes unseen however many
	// are in the folder.
	double byTime = RUN_SPEED * TARGET_RUN_SECONDS;
	double byImages = (double)gBgCount * WIN_W;
	gLevelLength = (byTime > byImages) ? byTime : byImages;

	gCoinPopTimer = 0.0;
	gRushBanner = 0.0;
	gRushAnnounced = false;
	gLastTick = 0;
	audioPlayLoop("bgsong");
}

void level01Enter()
{
	if (!assetsLevel01Loaded()) {
		levelLoadingScreen("level 01", 0, 1);
		assetsLoadLevel01(levelLoadProgress);
	}
	levelStart();
}

// The win / lose sting is a long clip, and it keeps playing on the result
// screen. Anything that leaves that screen -- R, ESC, ENTER -- has to cut it,
// or it carries on underneath the next run or the menu narration.
static void stopEndingSounds()
{
	audioStop("winsnd");
	audioStop("losesnd");
}

static void endLevel(bool won, const char *reason)
{
	audioStop("bgsong");

	if (won) {
		levelComplete();
		audioPlayOnce("winsnd");
	} else {
		gameOver(reason);
		audioPlayOnce("losesnd");
	}
}

// ---------------------------------------------------------------------------
//  Update  --  the whole game loop, driven off elapsed time, never blocking
// ---------------------------------------------------------------------------
void level01Update()
{
	DWORD now = timeGetTime();
	if (gLastTick == 0) gLastTick = now;
	double dt = (now - gLastTick) / 1000.0;
	gLastTick = now;

	// A long stall -- dragging the window, the loading pause -- must not
	// teleport the world forward.
	if (dt > 0.08) dt = 0.08;
	if (dt < 0.0)  dt = 0.0;

	// --- keys that work in any phase ---
	if (keyJustPressed(27)) {
		audioStop("bgsong");
		stopEndingSounds();
		setState(STATE_MENU);
		return;
	}
	if (keyJustPressed('r') || keyJustPressed('R')) {
		stopEndingSounds();
		levelStart();
		return;
	}
	if (keyJustPressed('p') || keyJustPressed('P')) {
		if (gPhase == LP_PLAYING) {
			gPhase = LP_PAUSED;
			audioPause("bgsong");
		} else if (gRushBanner > 0.0) {
		Color red = { 240, 90, 60 };
		dSetColor(red);
		dStrokeTextCentered(WIN_W / 2.0, WIN_H / 2.0 + 40, "RUSH HOUR", 50, 3.2);
		dSetColor(C_TEXT);
		dTextCentered(WIN_W / 2.0, WIN_H / 2.0 + 4,
		              "One minute left - the traffic does not queue any more.",
		              GLUT_BITMAP_HELVETICA_18);
	}

	if (gPhase == LP_PAUSED) {
			gPhase = LP_PLAYING;
			audioResume("bgsong");
		}
	}

	if (gPhase == LP_PAUSED) return;

	// Level over: obstacle spawning and the clock have both stopped.
	if (gPhase == LP_COMPLETE || gPhase == LP_GAMEOVER) {
		if (keyJustPressed(13)) {
			stopEndingSounds();
			setState(STATE_MENU);
		}
		return;
	}

	// --- the "get ready", where the character is already running on the spot ---
	if (gPhase == LP_READY) {
		gReadyTimer -= dt;
		updateCharacter(dt, false);
		if (gReadyTimer <= 0.0) gPhase = LP_PLAYING;
		return;
	}

	// --- the run ---
	gTimeLeft -= dt;
	if (gTimeLeft <= 0.0) {
		gTimeLeft = 0.0;
		endLevel(false, "The clock beat you. Time ran out before Hatirjheel.");
		return;
	}

	updateCharacter(dt, true);

	// The character stays put; the road moves. This is the forward motion.
	gProgress += RUN_SPEED * dt;

	bool onFinalApproach = distanceToFinish() <= FINISH_APPROACH;

	updateObstacles(dt, !onFinalApproach);
	updateCoins(dt, RUN_SPEED, !onFinalApproach);
	updatePowerUps(dt, RUN_SPEED, !onFinalApproach);

	checkCollisions();

	// The street turns for the last minute -- say so once, clearly.
	if (!gRushAnnounced && inFinalRush()) {
		gRushAnnounced = true;
		gRushBanner = 2.2;
	}
	if (gRushBanner > 0.0) gRushBanner -= dt;

	if (gInvuln       > 0.0) gInvuln       -= dt;
	if (gHitFlash     > 0.0) gHitFlash     -= dt;
	if (gShieldFlash  > 0.0) gShieldFlash  -= dt;
	if (gHealFlash    > 0.0) gHealFlash    -= dt;
	if (gCoinPopTimer > 0.0) gCoinPopTimer -= dt;

	// Health ran out inside checkCollisions().
	if (gPhase == LP_GAMEOVER) {
		endLevel(false, gEndReason);
		return;
	}

	if (gProgress >= gLevelLength) {
		gProgress = gLevelLength;
		endLevel(true, "");
	}
}

// ---------------------------------------------------------------------------
//  Overlays
//
//  Do not call a parameter "small" here. windows.h drags in rpcndr.h, which
//  does "#define small char", and the error points somewhere else entirely.
// ---------------------------------------------------------------------------
static void levelDrawBanner(const char *headline, const Color &color, const char *subtitle)
{
	dDimScreen(0.74);

	dSetColor(color);
	dStrokeTextCentered(WIN_W / 2.0, WIN_H / 2.0 + 30, headline, 56, 3.5);

	if (subtitle) {
		dSetColor(C_TEXT);
		dTextCentered(WIN_W / 2.0, WIN_H / 2.0 - 12, subtitle, GLUT_BITMAP_HELVETICA_18);
	}
}

static void levelDrawEndStats(bool won)
{
	char buf[180];
	int used = (int)(LEVEL_TIME_LIMIT - gTimeLeft);

	dSetColor(C_TEXT_DIM);

	sprintf_s(buf, sizeof(buf), "Score %d      Coins %d      Obstacles dodged %d      Hits taken %d",
	          gScore, gCoinsTaken, gDodgeCount, gHitCount);
	dTextCentered(WIN_W / 2.0, WIN_H / 2.0 - 58, buf, GLUT_BITMAP_HELVETICA_12);

	sprintf_s(buf, sizeof(buf), "Time taken %d:%02d      HP left %d      Route %d%%",
	          used / 60, used % 60, gHP, (int)(levelFraction() * 100.0));
	dTextCentered(WIN_W / 2.0, WIN_H / 2.0 - 80, buf, GLUT_BITMAP_HELVETICA_12);

	if (won) {
		dSetColor(C_ACCENT);
		dTextCentered(WIN_W / 2.0, WIN_H / 2.0 - 112,
		              "Level 02 is not built yet - Hatirjheel to AUST comes next.",
		              GLUT_BITMAP_HELVETICA_12);
	}

	dSetColor(C_TEXT);
	dTextCentered(WIN_W / 2.0, WIN_H / 2.0 - 140,
	              "ENTER or ESC  -  main menu          R  -  run it again",
	              GLUT_BITMAP_HELVETICA_12);
}

static void drawCoinPop()
{
	if (gCoinPopTimer <= 0.0) return;

	double t = 1.0 - (gCoinPopTimer / 0.7);
	dSetColor(C_ACCENT);
	dTextCentered(gCoinPopX, gCoinPopY + t * 46.0, "+10", GLUT_BITMAP_HELVETICA_18);
}

// ---------------------------------------------------------------------------
//  Draw  --  layered back to front
// ---------------------------------------------------------------------------
void level01Draw()
{
	drawBackground();     // 1. the street
	drawFinishLine();     // 2. distant scenery: the gate, once it is in view
	drawBirds();          // 3. flying birds
	drawObstacles();      // 4. road traffic
	drawCoins();          // 5. coins
	drawPowerUps();       //    and power-ups
	drawCharacter();      // 6. the character, in front of the scene
	drawCoinPop();

	if (gHitFlash > 0.0) {
		Color red = { 220, 40, 40 };
		dFillRectA(0, 0, WIN_W, WIN_H, red, gHitFlash * 0.55);
	}
	if (gShieldFlash > 0.0)
		dFillRectA(0, 0, WIN_W, WIN_H, C_SKY, gShieldFlash * 0.40);
	if (gHealFlash > 0.0) {
		Color green = { 90, 230, 120 };
		dFillRectA(0, 0, WIN_W, WIN_H, green, gHealFlash * 0.30);
	}

	drawHUD();            // 7. HUD on top

	if (gPhase == LP_READY) {
		dDimScreen(0.35);
		dSetColor(C_ACCENT);
		dStrokeTextCentered(WIN_W / 2.0, WIN_H / 2.0 + 10, "GET READY", 52, 3.2);
		dSetColor(C_TEXT);
		dTextCentered(WIN_W / 2.0, WIN_H / 2.0 - 28,
		              "SPACE jumps.  Listen for the horn - and mind the birds overhead.",
		              GLUT_BITMAP_HELVETICA_12);
	} else if (gPhase == LP_PAUSED) {
		levelDrawBanner("PAUSED", C_SKY, "P to resume      R to restart      ESC for the menu");
	} else if (gPhase == LP_COMPLETE) {
		Color green = { 90, 220, 120 };
		levelDrawBanner("LEVEL 01 COMPLETE", green, "You made it to Hatirjheel before class.");
		levelDrawEndStats(true);
	} else if (gPhase == LP_GAMEOVER) {
		Color red = { 235, 70, 60 };
		levelDrawBanner("GAME OVER", red, gEndReason);
		levelDrawEndStats(false);
	}
}

void level01Mouse(int button, int state, int mx, int my)
{
}

#endif // LEVEL01_HPP

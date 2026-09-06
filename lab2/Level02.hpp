//
//  Level02.hpp  --  the river crossing, and the pirate ship after it.
//
//  The whole level, in the order it plays:
//
//    the river
//      1. the boat travels down the river, the boy rowing
//      2. a Dragon Power ball comes in from ahead and strikes the boat
//      3. the boat stops and the boy stands up
//      4. two crocodiles attack; each takes exactly three chidori
//      5. two more crocodiles attack; each takes exactly three chidori
//
//    the pirate ship
//      6. he sits back down and rows on
//      7. a pirate ship rams the boat
//      8. aboard, he walks the deck to the first chidori icon
//      9. six dakat, two at a time, three chidori each
//     10. the powerup grants the powerful chidori
//     11. the dakatleader: six powerful chidori, while he throws enemy
//         chidori back at seventy-five health a time
//     12. Level 02 is won, and Level 03 unlocks
//
//  The work is split the way the rest of the project splits it:
//
//      Level02State.hpp     the stage machine, health, the rules, the audio
//      Level02Entities.hpp  the boat, the ball, the crocodiles, the bolts, coins
//      Level02Deck.hpp      the ship, the walker, the icons, the dakat, the boss
//      Level02HUD.hpp       the read-outs
//      this file            entry, input, the stage machine, collision, drawing
//
//  Nothing here is done inside iDraw(): level02Draw() only draws, and every
//  decision is made in level02Update() off real elapsed time.
//
#ifndef LEVEL02_HPP
#define LEVEL02_HPP


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
#include "GameState.hpp"
#include "LevelState.hpp"
#include "Level02State.hpp"
#include "Level02Entities.hpp"
#include "Level02Deck.hpp"
#include "Level02HUD.hpp"
#endif
#include <math.h>

// Wall clock, exactly as Level 01 does it: fixedUpdate() fires on a Windows
// timer whose spacing is only roughly 16 ms, so nothing is paced off frames.
static DWORD gL02LastTick = 0;

// The boat half's throw animation. ThrowPhase itself lives in Level02State.hpp
// because the deck half uses the same three steps.
static ThrowPhase gL02Throw = THROW_IDLE;
static double gL02ThrowTimer = 0.0;
static double gL02ThrowCooldown = 0.0;

// Counts down between the two crocodile groups.
static double gL02GroupGap = 0.0;

// How long the crash has been held on screen before the cut below decks.
static double gL02ShipHold = 0.0;

// A brief "+10" where a coin was taken, same idea as Level 01.
static double gL02CoinPop = 0.0;
static double gL02CoinPopX = 0.0;
static double gL02CoinPopY = 0.0;

// ---------------------------------------------------------------------------
//  Loading screen
// ---------------------------------------------------------------------------
static void level02LoadingScreen(const char *stage, int done, int total)
{
	iClear();

	if (TEX_POSTER) {
		iShowImage(0, 0, WIN_W, WIN_H, TEX_POSTER);
		dDimScreen(0.82);
	} else {
		dFillRectA(0, 0, WIN_W, WIN_H, C_INK, 1.0);
	}

	dSetColor(C_ACCENT);
	dStrokeTextCentered(WIN_W / 2.0, WIN_H / 2.0 + 46, "LEVEL 02", 46, 3.0);

	dSetColor(C_TEXT);
	dTextCentered(WIN_W / 2.0, WIN_H / 2.0 + 12, "ACROSS THE WATER", GLUT_BITMAP_HELVETICA_18);

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

static void level02LoadProgress(const char *stage, int done, int total)
{
	level02LoadingScreen(stage, done, total);
}

// ---------------------------------------------------------------------------
//  Entry and restart
// ---------------------------------------------------------------------------
static void level02Start()
{
	level02ResetState();
	l02ResetEntities();

	gL02Throw = THROW_IDLE;
	gL02ThrowTimer = 0.0;
	gL02ThrowCooldown = 0.0;
	gL02GroupGap = 0.0;
	gL02ShipHold = 0.0;
	gL02CoinPop = 0.0;
	gL02LastTick = 0;

	// The second half starts from scratch too, so restarting from the boss
	// puts him back in the boat with all six dakat still to come.
	l02bResetDeck();
	level02PirateSongStop();      // back to the river, so back to the rain

	// One rain loop for the whole level. It is started here and never stopped
	// between stages -- only leaving the level silences it.
	level02RainStart();
}

void level02Enter()
{
	if (!assetsLevel02Loaded()) {
		level02LoadingScreen("level 02", 0, 1);
		assetsLoadLevel02(level02LoadProgress);
	}
	level02Start();
}

// Everything this level can leave sounding, silenced in one place.
static void level02SilenceAll()
{
	level02RainStop();
	level02PirateSongStop();
	level02AllCrocSoundsStop();
	audioStop("winsnd");
	audioStop("winsnd2");
	audioStop("losesnd");
}

// ---------------------------------------------------------------------------
//  INPUT
// ---------------------------------------------------------------------------

// Raises the hand. The bolt itself is created by the animation, not here.
static void level02BeginThrow()
{
	if (!level02CanThrow()) return;
	if (gL02Phase != L02P_PLAYING) return;
	if (gL02Throw != THROW_IDLE) return;
	if (gL02ThrowCooldown > 0.0) return;

	gL02Throw = THROW_WINDUP;
	gL02ThrowTimer = CHIDORI_WINDUP;
	gL02ThrowCooldown = CHIDORI_COOLDOWN;
}

// Returns true if the update should stop here, because the screen has changed
// or the level is paused.
static bool level02HandleKeys()
{
	// ESC goes back to the campaign map, which is where this level was chosen.
	if (keyJustPressed(27)) {
		level02SilenceAll();
		setState(STATE_LEVEL_SELECT);
		return true;
	}

	if (keyJustPressed('r') || keyJustPressed('R')) {
		audioStop("winsnd");
		audioStop("winsnd2");
		audioStop("losesnd");
		level02AllCrocSoundsStop();
		level02Start();
		return true;
	}

	if (keyJustPressed('p') || keyJustPressed('P')) {
		if (gL02Phase == L02P_PLAYING) {
			gL02Phase = L02P_PAUSED;
			audioPause("rain");
			audioPause("piratesong");
			level02AllCrocSoundsStop();
		} else if (gL02Phase == L02P_PAUSED) {
			gL02Phase = L02P_PLAYING;
			audioResume("rain");
			audioResume("piratesong");
			for (int i = 0; i < MAX_CROCODILES; i++)
				if (gCrocs[i].active && !gCrocs[i].dead)
					level02CrocSoundStart(gCrocs[i].soundSlot);
		}
	}

	if (gL02Phase == L02P_PAUSED) return true;

	// Once the level has ended either way, ENTER leaves for the campaign map.
	if (gL02Phase == L02P_GAMEOVER || gL02Phase == L02P_WON) {
		if (keyJustPressed(13)) {
			level02SilenceAll();
			setState(STATE_LEVEL_SELECT);
		}
		return true;
	}

	// The throw. Same keys as the Level 01 jump, so the controls stay learnable,
	// and the same key does the right thing in both halves: on the river it is
	// the blue chidori at a crocodile; on the deck it is an ordinary chidori at
	// a dakat, or the powerful chidori at the dakatleader.
	// On the river, any of the three throws -- that half has no other control.
	// On the deck the keys split up, because there he has three things to do:
	//     A / D or LEFT / RIGHT   walk
	//     W or UP                 jump
	//     SPACE                   throw a chidori
	// Walking and jumping are read inside l02bUpdateWalker(), where they can be
	// held down; only the throw is edge-triggered here.
	if (level02OnDeck()) {
		if (keyJustPressed(' '))
			l02bBeginThrow();
	} else if (keyJustPressed(' ') ||
	           keyJustPressed('w') || keyJustPressed('W') ||
	           specialKeyJustPressed(GLUT_KEY_UP)) {
		level02BeginThrow();
	}

	return false;
}

// ---------------------------------------------------------------------------
//  The throw animation
//
//      standing  ->  hand raised (throwpic)  ->  chidori created  ->  standing
// ---------------------------------------------------------------------------
static void level02UpdateThrow(double dt)
{
	if (gL02ThrowCooldown > 0.0) gL02ThrowCooldown -= dt;

	if (gL02Throw == THROW_IDLE) return;

	gL02ThrowTimer -= dt;
	if (gL02ThrowTimer > 0.0) return;

	if (gL02Throw == THROW_WINDUP) {
		l02LaunchChidori();                 // it leaves the raised hand
		gL02Throw = THROW_RECOVER;
		gL02ThrowTimer = CHIDORI_RECOVER;
	} else {
		gL02Throw = THROW_IDLE;             // back to standing
	}
}

// The pose is decided entirely by the stage and the throw animation, so it can
// never disagree with them.
static void level02UpdateBoatPose()
{
	if (level02BoatIsMoving()) {
		gBoat.pose = BOAT_POSE_ROW;
		return;
	}

	gBoat.pose = (gL02Throw == THROW_IDLE) ? BOAT_POSE_STAND : BOAT_POSE_THROW;
}

// ---------------------------------------------------------------------------
//  COLLISION
//
//  Every test is a bounding box through checkCollision(), never a distance
//  between sprite positions.
// ---------------------------------------------------------------------------

// A chidori landing on a crocodile. One bolt is one hit: it is deactivated the
// instant it connects, so it cannot carry on and hit a second crocodile.
static void level02CheckChidoriHits()
{
	for (int b = 0; b < MAX_CHIDORI; b++) {
		Chidori &bolt = gChidori[b];
		if (!bolt.active) continue;

		Rect boltBox = l02ChidoriHitBox(bolt);

		for (int i = 0; i < MAX_CROCODILES; i++) {
			Crocodile &c = gCrocs[i];
			if (!c.active || c.dead) continue;
			if (!checkCollision(boltBox, l02CrocodileHitBox(c))) continue;

			bolt.active = false;            // spent
			l02DamageCrocodile(c);
			break;                          // this bolt is done
		}
	}
}

// Coins are score only -- never health, never the shield.
static void level02CheckCoins()
{
	Rect body = l02BoatHitBox();

	for (int i = 0; i < MAX_L02_COINS; i++) {
		RiverCoin &c = gL02Coins[i];
		if (!c.active) continue;
		if (!checkCollision(body, l02CoinHitBox(c))) continue;

		Rect r = l02CoinRect(c);
		c.active = false;                   // gone, so it cannot score twice
		collectCoin();                      // the shared scoring architecture

		gL02CoinPopX = r.x + r.w * 0.5;
		gL02CoinPopY = r.y + r.h;
		gL02CoinPop = 0.7;
	}
}

// The Dragon Power ball reaching the boat. A story trigger: it stops the boat
// and puts the boy on his feet, and it costs no health.
static bool level02CheckDragonImpact()
{
	if (!gDragon.active || gDragon.hasStruck) return false;

	// It is aimed at the boat, but the boat bobs, so treat sailing PAST the
	// boat as a hit too. Without this the stage could stall for ever on a near
	// miss, waiting for a collision that can no longer happen.
	bool overshot = (gDragon.x + DRAGON_SIZE) < l02BoatHitBox().x;

	if (!overshot && !checkCollision(l02BoatHitBox(), l02DragonHitBox()))
		return false;

	gDragon.hasStruck = true;
	gDragon.active = false;
	gL02DragonFlash = DRAGON_FLASH;
	playCollisionSound();                   // it visibly strikes -- but no damage
	printf("[level02] dragon power struck the boat -- the boat stops\n");
	return true;
}

// ---------------------------------------------------------------------------
//  THE STAGE MACHINE
//
//  Five stages, in order, and then it stops. Each stage decides one thing: what
//  ends it.
// ---------------------------------------------------------------------------
// Puts a fresh chidori icon somewhere along the deck. Called whenever he runs
// out, which is what stops the attack being spammable: with nothing left to
// throw, the only way to attack again is to walk into another icon.
static void level02OfferChidoriIcon()
{
	double x = randBetween(ICON_MIN_X, ICON_MAX_X);
	l02bShowPickup(PICK_CHIDORI, x, ICON_RESPAWN_WAIT);
}

static void level02UpdateStage(double dt)
{
	gL02StageTimer += dt;

	switch (gL02Stage) {

	// --- 1. rowing down the river ---
	case L02_BOAT_TRAVEL:
		if (gL02StageTimer >= BOAT_TRAVEL_DURATION) {
			l02SpawnDragon();
			level02SetStage(L02_DRAGON_POWER_EVENT);
		}
		break;

	// --- 2. the ball is in flight; the boat is still under way ---
	case L02_DRAGON_POWER_EVENT:
		l02UpdateDragon(dt);
		if (level02CheckDragonImpact())
			level02SetStage(L02_BOAT_STOPPED);
		break;

	// --- 3. stopped, on his feet, a beat before the water moves ---
	case L02_BOAT_STOPPED:
		if (gL02StageTimer >= L02_BOAT_STOPPED_SECONDS) {
			l02SpawnCrocodileGroup(0);
			level02SetStage(L02_CROCODILE_GROUP_1);
		}
		break;

	// --- 4. the first two ---
	case L02_CROCODILE_GROUP_1:
		l02UpdateCrocodiles(dt);

		// Both of the first pair are dead and gone: count a short breath, then
		// the second pair comes in.
		if (l02CrocodileGroupCleared(0)) {
			gL02GroupGap += dt;
			if (gL02GroupGap >= L02_GROUP_GAP_SECONDS) {
				l02SpawnCrocodileGroup(1);
				level02SetStage(L02_CROCODILE_GROUP_2);
			}
		}
		break;

	// --- 5. the second two ---
	case L02_CROCODILE_GROUP_2:
		l02UpdateCrocodiles(dt);
		if (l02CrocodileGroupCleared(1))
			level02SetStage(L02_BOAT_AFTER_CROCODILE);
		break;

	// =====================================================================
	//  SECOND HALF  --  the pirate ship
	//
	//  Each stage below names the ONE thing that ends it, and there is no
	//  other way out of it. That is what makes the order impossible to skip:
	//  the powerup is never put on the deck until all six dakat are dead, and
	//  the dakatleader is never summoned until the powerup has been taken.
	// =====================================================================

	// --- 6. he survived. He sits back down and takes up the oar again ---
	case L02_BOAT_AFTER_CROCODILE:
		if (gL02StageTimer >= L02_AFTER_CROC_SECONDS) {
			l02bSpawnShip();
			level02SetStage(L02_PIRATE_COLLISION);
		}
		break;

	// --- 7. the ship closes and rams the boat ---
	// Like the Dragon Power ball, the scene changes because two boxes
	// actually touched, not because a timer ran out.
	case L02_PIRATE_COLLISION:
		l02bUpdateShip(dt);

		if (!gShip.hasStruck &&
		    checkCollision(l02BoatHitBox(), l02bShipHitBox())) {
			gShip.hasStruck = true;
			gL02DragonFlash = DRAGON_FLASH;   // the same white jolt
			playCollisionSound();
			printf("[level02] the pirate ship rams the boat\n");
		}

		// A beat on the crash, then the cut to below decks.
		if (gShip.hasStruck) {
			gL02ShipHold += dt;
			if (gL02ShipHold >= PIRATE_HIT_HOLD) {
				l02bResetDeck();

				// He is aboard. The rain stops and the pirate song takes over
				// for the whole of the rest of the level -- it is started here
				// and nowhere else, so moving between dakat and boss never
				// interrupts it.
				level02PirateSongStart();

				level02SetStage(L02_PIRATE_INTERIOR);
			}
		}
		break;

	// --- 8. aboard. He walks the deck to the first chidori icon ---
	// This is where the pirate song starts, and it is not started anywhere
	// else: every stage from here to the end of the level runs under it.
	case L02_PIRATE_INTERIOR:
		if (gPick.kind == PICK_NONE)
			l02bShowPickup(PICK_CHIDORI, L02B_ICON_X_FIRST, 0.0);

		if (l02bTookPickup()) {
			gPick.active = false;
			gPick.kind = PICK_NONE;
			gL02Chidori += CHIDORI_PER_ICON;
			l02bFeedDakat(dt);                 // the first one walks on
			level02SetStage(L02_DAKAT_BATTLE);
		}
		break;

	// --- 9. the six dakat ---
	//
	// One stage, not six: they arrive progressively, two on the deck at a
	// time, and gL02DakatKilled is what the boss waits on. Jumping over one
	// cannot skip it -- it turns round and comes back -- so the count of six
	// is the only way forward.
	case L02_DAKAT_BATTLE:
		l02bFeedDakat(dt);
		l02bUpdateDakat(dt);

		// Out of chidori: another icon appears to be walked into.
		if (gL02Chidori <= 0 && gPick.kind == PICK_NONE)
			level02OfferChidoriIcon();

		if (l02bTookPickup()) {
			gPick.active = false;
			gPick.kind = PICK_NONE;
			gL02Chidori += CHIDORI_PER_ICON;
			printf("[level02] chidori taken  (%d in hand)\n", gL02Chidori);
		}

		// All six down. The powerup appears, and NOT before -- which is what
		// keeps the powerful chidori out of his hands until it is earned.
		if (l02bAllDakatCleared()) {
			l02bShowPickup(PICK_POWERUP, L02B_POWERUP_X, 0.6);
			level02SetStage(L02_POWERUP);
		}
		break;

	// --- 10. the powerup ---
	case L02_POWERUP:
		if (l02bTookPickup()) {
			gPick.active = false;
			gPick.kind = PICK_NONE;
			gL02HasPowerful = true;
			printf("[level02] POWERFUL CHIDORI acquired\n");

			l02bSummonLeader();
			level02SetStage(L02_DAKATLEADER_BATTLE);
		}
		break;

	// --- 11. the dakatleader. Six powerful chidori, and he throws back ---
	case L02_DAKATLEADER_BATTLE:
		l02bUpdateLeader(dt);

		// He is only beaten once he has finished falling.
		if (l02bLeaderFinished()) {
			// The pirate ship is behind him. Its music stops here and the river
			// he was rowing before the ram comes back -- the SAME Hatirjheel
			// scene, drawn by the same level02DrawRiver() and l02DrawBoat(),
			// with the boat put back to its rowing pose.
			level02PirateSongStop();
			level02RainStart();          // the river's own ambience returns

			gBoat.pose = BOAT_POSE_ROW;
			gL02ScrollSpeed = RIVER_SCROLL_SPEED;

			printf("[level02] the pirate ship is behind him -- back to Hatirjheel\n");
			level02SetStage(L02_HATIRJHEEL_RETURN);
		}
		break;

	// --- 12. rowing home. He is on the water again before anything is said ---
	case L02_HATIRJHEEL_RETURN:
		if (gL02StageTimer >= L02_RETURN_ROW_SECONDS) {
			level02Win();                    // plays "loose sound2", once
			level02SetStage(L02_LEVEL_WON);
		}
		break;

	// --- 13. won. He keeps rowing behind the message ---
	case L02_LEVEL_WON:
		break;
	}
}

// ---------------------------------------------------------------------------
//  UPDATE  --  paced off real elapsed time, never frame counts, never Sleep()
// ---------------------------------------------------------------------------
void level02Update()
{
	DWORD now = timeGetTime();
	if (gL02LastTick == 0) gL02LastTick = now;
	double dt = (now - gL02LastTick) / 1000.0;
	gL02LastTick = now;

	// A long stall -- dragging the window, the loading pause -- must not
	// teleport the world forward.
	if (dt > 0.08) dt = 0.08;
	if (dt < 0.0)  dt = 0.0;

	gL02RainPhase += dt;

	if (level02HandleKeys()) return;

	// --- "get ready", with the boat already under way ---
	if (gL02Phase == L02P_READY) {
		gL02ReadyTimer -= dt;
		l02UpdateBoat(dt, true);
		gL02Scroll += gL02ScrollSpeed * dt;
		if (gL02ReadyTimer <= 0.0) gL02Phase = L02P_PLAYING;
		return;
	}

	if (gL02Phase != L02P_PLAYING) return;

	// --- the river ---
	// It slides past while the boat is under way and coasts to a stop once the
	// ball has landed, rather than halting on a single frame.
	double target = level02BoatIsMoving() ? RIVER_SCROLL_SPEED : 0.0;
	if (gL02ScrollSpeed > target) {
		gL02ScrollSpeed -= 260.0 * dt;
		if (gL02ScrollSpeed < target) gL02ScrollSpeed = target;
	} else if (gL02ScrollSpeed < target) {
		gL02ScrollSpeed += 260.0 * dt;
		if (gL02ScrollSpeed > target) gL02ScrollSpeed = target;
	}
	gL02Scroll += gL02ScrollSpeed * dt;

	// --- the boy, in the boat or on the deck ---
	if (level02OnDeck()) {
		// One call a frame, whatever the stage: his walking, his jump, his
		// throw animation, the pickup and the boss's projectiles. Movement is
		// live only while he is actually in control of himself.
		l02bUpdateCommon(dt, level02DeckControlLive());
		level02KeepMusicAlive(dt);
	} else {
		l02UpdateBoat(dt, level02BoatIsMoving());
		level02UpdateThrow(dt);
		level02UpdateBoatPose();
	}

	// --- projectiles and pickups ---
	// The projectile pool is shared: the same array carries the blue chidori on
	// the river and both the ordinary and the powerful chidori on the deck.
	l02UpdateChidori(dt);
	l02UpdateCoins(dt, gL02Stage == L02_BOAT_TRAVEL);

	// --- the stage machine drives the crocodiles, the ball, the ship, the
	//     dakat and the dakatleader ---
	level02UpdateStage(dt);

	// --- collision ---
	if (level02OnDeck()) {
		l02bCheckCombat();             // birds, red chidori, blue flame
	} else {
		level02CheckChidoriHits();
		level02CheckCoins();
	}

	// --- timers ---
	if (gL02HitFlash    > 0.0) gL02HitFlash    -= dt;
	if (gL02DragonFlash > 0.0) gL02DragonFlash -= dt;
	if (gL02BannerTimer > 0.0) gL02BannerTimer -= dt;
	if (gL02CoinPop     > 0.0) gL02CoinPop     -= dt;
}

// ---------------------------------------------------------------------------
//  DRAWING
//
//  Draw only. Every decision was already made in level02Update().
// ---------------------------------------------------------------------------
static void level02DrawRiver()
{
	if (TEX_L02_BG == 0) {
		dFillRectA(0, 0, WIN_W, WIN_H, C_INK, 1.0);
		return;
	}

	// There is only ONE river photograph, so butting copies of it edge to edge
	// puts a hard vertical line across the water every screen width. Every
	// SECOND copy is mirrored instead: a mirrored copy meets its neighbour on
	// identical pixels, so the join disappears and the river reads as
	// continuous however long it scrolls.
	double screen = gL02Scroll / (double)WIN_W;
	double whole  = floor(screen);
	double offset = (screen - whole) * WIN_W;
	int    index  = (int)whole;

	bool flipA = ((index % 2) + 2) % 2 != 0;   // stays right for negatives

	dImageFlipped(-offset,        0, WIN_W, WIN_H, TEX_L02_BG, flipA);
	dImageFlipped(WIN_W - offset, 0, WIN_W, WIN_H, TEX_L02_BG, !flipA);
}

// Cosmetic rain over the whole scene, to match the rain that is playing and the
// storm in the backdrop. Set RAIN_STREAKS to 0 in Config.hpp to remove it.
static void level02DrawRain()
{
	if (RAIN_STREAKS <= 0) return;

	const double fallSpan = WIN_H + 200.0;
	const double slantSpan = WIN_W + 400.0;
	const double dirLen = sqrt(RAIN_SLANT * RAIN_SLANT + RAIN_FALL * RAIN_FALL);
	const double ux = -RAIN_SLANT / dirLen;
	const double uy = -RAIN_FALL / dirLen;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.74f, 0.82f, 0.94f, 0.26f);
	glLineWidth(1.0f);

	glBegin(GL_LINES);
	for (int i = 0; i < RAIN_STREAKS; i++) {
		double speed = 0.75 + ((i * 37) % 50) / 100.0;
		double y = WIN_H + 100.0 - fmod(gL02RainPhase * RAIN_FALL * speed + i * 97.0, fallSpan);
		double x = -200.0 + fmod(i * 137.0 + (WIN_H + 100.0 - y) * (RAIN_SLANT / RAIN_FALL), slantSpan);

		glVertex2d(x, y);
		glVertex2d(x + ux * RAIN_LENGTH, y + uy * RAIN_LENGTH);
	}
	glEnd();

	glDisable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

static void level02DrawCoinPop()
{
	if (gL02CoinPop <= 0.0) return;

	double t = 1.0 - (gL02CoinPop / 0.7);
	dSetColor(C_ACCENT);
	dTextCentered(gL02CoinPopX, gL02CoinPopY + t * 46.0, "+10", GLUT_BITMAP_HELVETICA_18);
}

static void level02DrawBanner(const char *headline, const Color &color, const char *subtitle)
{
	dDimScreen(0.74);

	dSetColor(color);
	dStrokeTextCentered(WIN_W / 2.0, WIN_H / 2.0 + 30, headline, 54, 3.4);

	if (subtitle) {
		dSetColor(C_TEXT);
		dTextCentered(WIN_W / 2.0, WIN_H / 2.0 - 12, subtitle, GLUT_BITMAP_HELVETICA_18);
	}
}

static void level02DrawEndStats()
{
	char buf[200];

	dSetColor(C_TEXT_DIM);
	sprintf_s(buf, sizeof(buf), "Score %d      Coins %d      Crocodiles %d / %d      Dakat %d / %d",
	          gScore, gCoinsTaken, gL02CrocsKilled, MAX_CROCODILES,
	          gL02DakatKilled, DAKAT_TOTAL);
	dTextCentered(WIN_W / 2.0, WIN_H / 2.0 - 58, buf, GLUT_BITMAP_HELVETICA_12);

	sprintf_s(buf, sizeof(buf), "Dakatleader %d / %d      Thrown %d      Hits taken %d      HP left %d",
	          gL02LeaderHits, LEADER_HITS_TO_KILL,
	          gL02ChidoriThrown + gL02BoltsThrown, gL02BitesTaken, gL02HP);
	dTextCentered(WIN_W / 2.0, WIN_H / 2.0 - 80, buf, GLUT_BITMAP_HELVETICA_12);

	dSetColor(C_TEXT);
	dTextCentered(WIN_W / 2.0, WIN_H / 2.0 - 116,
	              "ENTER or ESC  -  campaign map          R  -  run it again",
	              GLUT_BITMAP_HELVETICA_12);
}

// The pirate ship's interior. It is the backdrop for EVERY remaining stage --
// the walk, every dakat, the powerup, the dakatleader and both endings -- and
// it never changes to anything else, so this is a single full-screen draw with
// no scroll and no swap.
static void level02DrawDeckBackdrop()
{
	if (TEX_L02B_DECK == 0) {
		dFillRectA(0, 0, WIN_W, WIN_H, C_INK, 1.0);
		return;
	}

	dImage(0, 0, WIN_W, WIN_H, TEX_L02B_DECK);
}

void level02Draw()
{
	if (level02OnDeck()) {
		// ----- below decks -----
		level02DrawDeckBackdrop();   // 1. boatside view, and nothing else
		l02bDrawDeckScene();         // 2. pickup, dakat, boss, him, projectiles
	} else {
		// ----- on the river -----
		level02DrawRiver();    // 1. the water and the far bank
		l02DrawCoins();        // 2. coins drifting down
		l02DrawCrocodiles();   // 3. the enemies
		l02bDrawShip();        //    the pirate ship, once it is bearing down
		l02DrawBoat();         // 4. the boat, with the boy already in it
		l02DrawChidori();      // 5. bolts in flight, in front of him
		l02DrawDragon();       // 6. the power ball, in front of everything
		level02DrawCoinPop();
		level02DrawRain();     // 7. weather over the whole scene
	}

	if (gL02HitFlash > 0.0) {
		Color red = { 220, 40, 40 };
		dFillRectA(0, 0, WIN_W, WIN_H, red, gL02HitFlash * 0.55);
	}
	if (gL02DragonFlash > 0.0) {
		Color amber = { 255, 190, 90 };
		dFillRectA(0, 0, WIN_W, WIN_H, amber, (gL02DragonFlash / DRAGON_FLASH) * 0.50);
	}

	level02DrawHUD();      // 8. HUD on top

	// --- the stage caption, briefly, as each stage begins ---
	if (gL02BannerTimer > 0.0 && gL02Phase == L02P_PLAYING &&
	    gL02Stage != L02_LEVEL_WON) {
		dSetColor(C_SKY);
		dStrokeTextCentered(WIN_W / 2.0, WIN_H - 150.0,
		                    L02_STAGE_CAPTION[gL02Stage], 34, 2.4);
	}

	// --- overlays ---
	if (gL02Phase == L02P_READY) {
		dDimScreen(0.35);
		dSetColor(C_ACCENT);
		dStrokeTextCentered(WIN_W / 2.0, WIN_H / 2.0 + 10, "GET READY", 52, 3.2);
		dSetColor(C_TEXT);
		dTextCentered(WIN_W / 2.0, WIN_H / 2.0 - 28,
		              "Row for Hatirjheel.  SPACE throws a chidori once you are on your feet.",
		              GLUT_BITMAP_HELVETICA_12);
	} else if (gL02Phase == L02P_PAUSED) {
		level02DrawBanner("PAUSED", C_SKY, "P to resume      R to restart      ESC for the map");
	} else if (gL02Phase == L02P_GAMEOVER) {
		Color red = { 235, 70, 60 };
		level02DrawBanner("GAME OVER", red, gL02EndReason);
		level02DrawEndStats();
	} else if (gL02Phase == L02P_WON) {
		// The dakatleader is down and he is back on the water. The Hatirjheel
		// river is behind this banner, not the pirate deck -- he rows on under
		// it, which is why the boat keeps animating.
		Color green = { 90, 220, 120 };
		level02DrawBanner("LEVEL 02 COMPLETE!", green,
		                  "You escaped the pirate ship. Hatirjheel again, and rowing.");
		level02DrawEndStats();

		dSetColor(C_ACCENT);
		dTextCentered(WIN_W / 2.0, WIN_H / 2.0 - 140,
		              "LEVEL 03 UNLOCKED  -  KUNIPARA TO AUST",
		              GLUT_BITMAP_HELVETICA_12);
	}
}

void level02Mouse(int button, int state, int mx, int my)
{
	// A left click throws, for anyone who reaches for the mouse.
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		level02BeginThrow();
}

#endif // LEVEL02_HPP

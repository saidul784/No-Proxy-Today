//
//  Level03.hpp  --  Kunipara to AUST.
//
//  The level, in the order it plays:
//
//       1. he walks; a rock is lying in the road, and he picks it up
//       2. a michil comes chanting down the road; seven rocks break it up
//       3. he walks on
//       4. he is stopped and ragged: twenty-two lines, speaking alternately
//       5. he walks on
//       6. eve-teasers are blocking the way; five sticks scatter them
//       7. he walks on
//       8. a chor sprints at him, stops, begs; six slaps and he bolts
//       9. the last stretch of open road
//      10. a jam of parked traffic to walk the length of, under falling fire
//      11. WELCOME TO AUST
//
//  Street traffic crosses the road through every one of those except the jam,
//  and has to be jumped, exactly as in Level 01.
//
//  The work is split the way the rest of the project splits it:
//
//      Level03State.hpp     the stage machine, health, the rules, the audio
//      Level03Entities.hpp  the walker, street traffic, pickups, projectiles
//      Level03Scenes.hpp    the five set-pieces
//      Level03HUD.hpp       the read-outs
//      this file            entry, input, the stage machine, collision, drawing
//
//  Nothing here is done inside iDraw(): level03Draw() only draws, and every
//  decision is made in level03Update() off real elapsed time.
//
#ifndef LEVEL03_HPP
#define LEVEL03_HPP


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
#include "Level03State.hpp"
#include "Level03Entities.hpp"
#include "Level03Scenes.hpp"
#include "Level03HUD.hpp"
#endif
#include <math.h>

// Wall clock, exactly as the other levels do it: fixedUpdate() fires on a
// Windows timer whose spacing is only roughly 16 ms, so nothing is paced off
// frame counts and nothing ever calls Sleep().
static DWORD gL03LastTick = 0;

// The "+10" that floats up where a coin was taken, exactly as Level 01 does it.
static double gL03CoinPop = 0.0;
static double gL03CoinPopX = 0.0;
static double gL03CoinPopY = 0.0;

// ---------------------------------------------------------------------------
//  Loading screen
// ---------------------------------------------------------------------------
static void level03LoadingScreen(const char *stage, int done, int total)
{
	iClear();

	if (TEX_POSTER) {
		iShowImage(0, 0, WIN_W, WIN_H, TEX_POSTER);
		dDimScreen(0.82);
	} else {
		dFillRectA(0, 0, WIN_W, WIN_H, C_INK, 1.0);
	}

	dSetColor(C_ACCENT);
	dStrokeTextCentered(WIN_W / 2.0, WIN_H / 2.0 + 46, "LEVEL 03", 46, 3.0);

	dSetColor(C_TEXT);
	dTextCentered(WIN_W / 2.0, WIN_H / 2.0 + 12, "KUNIPARA  ->  AUST",
	              GLUT_BITMAP_HELVETICA_18);

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

static void level03LoadProgress(const char *stage, int done, int total)
{
	if (done % 2 == 0 || done == total)
		level03LoadingScreen(stage, done, total);
}

// ---------------------------------------------------------------------------
//  Entry and restart
// ---------------------------------------------------------------------------
static void level03Start()
{
	// Restarting banks the previous attempt before the score is zeroed below.
	highScoreEndAttempt(gScore);

	level03ResetState();
	l03ResetEntities();
	l03ResetScenes();

	gL03LastTick = 0;

	// The score is the game-wide counter, so this run starts it from zero the
	// same way the other levels do.
	gScore = 0;
	gCoinsTaken = 0;

	gL03CoinPop = 0.0;

	// A fresh attempt, so a fresh result may be banked when it is won.
	saveNoteLevelStarted(3);
	highScoreBeginAttempt(3);
}

void level03Enter()
{
	if (!assetsLevel03Loaded()) {
		level03LoadingScreen("level 03", 0, 1);
		assetsLoadLevel03(level03LoadProgress);
	}
	level03Start();
}

// ---------------------------------------------------------------------------
//  THROWING
//
//  What he is aiming at depends on the set-piece, so the release goes through
//  here rather than living in the entity module.
// ---------------------------------------------------------------------------
static void level03ReleaseThrow()
{
	if (gL03Stage == L03_MICHIL && l03MichilIsAlive()) {
		Rect r = l03MichilRect();
		l03Launch(r.x + r.w * 0.30, r.y + r.h * 0.45, true);
		return;
	}

	// The stick goes at the AGGRESSOR, never at the group -- the girl they are
	// surrounding is not a target. This is scoped to the eve stage, so the
	// michil's aim is untouched.
	if (gL03Stage == L03_EVETEASING && l03EveIsAlive()) {
		Rect r = l03RusherRect();
		l03Launch(r.x + r.w * 0.50, r.y + r.h * 0.55, true);
		return;
	}

	l03Launch(0.0, 0.0, false);      // straight ahead, if there is no target
}

// ---------------------------------------------------------------------------
//  THE THIEF INTERACTION
//
//  Three small functions, and between them they own every one of the rules:
//  the pose he is in, the clip that is sounding, and what H is allowed to do.
//  Nothing else in the level touches gL03Interact.
// ---------------------------------------------------------------------------

// He goes down on his knees. Called once, the frame the thief pulls up.
static void level03BeginBegging()
{
	if (gL03Interact != L03I_NONE) return;

	gL03Interact = L03I_BEGGING;
	gL03InteractTimer = 0.0;
	gW3.pose = L03_POSE_BEG;

	// Not mid-air and not mid-throw: he is on his knees.
	gW3.isJumping = false;
	gW3.velocityY = 0.0;
	gW3.y = gW3.groundY;

	level03BegSoundStart();
	printf("[level03] you are down on your knees  --  H to fight back\n");
}

// One slap, from the key press. The guard is level03CanSlap(), which is false
// while a slap is already running, so hammering H cannot restart the animation
// or land two hits for one press -- and false outside the thief scene, so H is
// dead everywhere else in the level.
static void level03Slap()
{
	if (!level03CanSlap()) return;

	// The plea stops the moment he stops pleading, not when the thief leaves.
	level03BegSoundStop();

	gL03Interact = L03I_SLAP_UP;
	gL03InteractTimer = SLAP_UP_TIME;
	gW3.pose = L03_POSE_SLAP_UP;

	// The swing is now IN THE AIR. Nothing is counted here -- the hit is
	// resolved when the hand lands, at the end of the windup, in
	// level03ResolveSlap() below.
	//
	// Whether this was a fair attempt is decided HERE and never revisited: a
	// swing begun while the opening was up stays valid through its windup even
	// if he starts backing off, and a swing begun at thin air stays invalid
	// even if he walks into it.
	gL03SlapPending = true;
	gL03SlapValid   = l03ChorOpeningUp();
}

// The hand lands. This is the only place a slap is ever counted.
static void level03ResolveSlap()
{
	if (!gL03SlapPending) return;
	gL03SlapPending = false;

	// Valid attempt AND he is still there to be hit. The opening holds itself
	// open for a pending valid swing, so in practice both are true together --
	// the reach test is what makes that a fact rather than a promise.
	if (gL03SlapValid && l03ChorInReach()) {
		l03SlapChor();               // counts, flashes, plays the thud, once
		gL03SlapValid = false;
		return;
	}

	// A swing at nothing. No counter change, no health, no score -- just the
	// animation playing out as a miss, a word about it, and a moment before he
	// can try again.
	gL03SlapValid = false;
	gL03SlapRecover = CHOR_MISS_RECOVER;
	gChor.missCue = CHOR_CUE_TIME;
	printf("[level03] TOO FAR  --  still %d / %d\n", gL03Slaps, CHOR_SLAPS_TO_CLEAR);
}

// Back to normal. Called when the thief has gone, and by the reset.
static void level03EndInteraction()
{
	if (gL03Interact == L03I_NONE) return;

	gL03Interact = L03I_NONE;
	gL03InteractTimer = 0.0;
	gW3.pose = L03_POSE_WALK;

	level03BegSoundStop();
	printf("[level03] back on the road\n");
}

// Runs the two slap frames in order, on their own clock. After the second one
// he stays on his feet -- he does not drop back to his knees between blows.
static void level03UpdateInteraction(double dt)
{
	if (gL03Interact == L03I_NONE) return;

	if (gL03SlapRecover > 0.0) gL03SlapRecover -= dt;

	if (gL03Interact == L03I_SLAP_UP || gL03Interact == L03I_SLAP_HIT) {
		gL03InteractTimer -= dt;
		if (gL03InteractTimer <= 0.0) {
			if (gL03Interact == L03I_SLAP_UP) {
				// End of the windup: this is the impact frame, and the moment
				// the swing finds out whether it hit anything.
				level03ResolveSlap();

				gL03Interact = L03I_SLAP_HIT;
				gL03InteractTimer = SLAP_HIT_TIME;
			} else {
				gL03Interact = L03I_STANDING;
				gL03InteractTimer = 0.0;
			}
		}
	}

	// The pose is decided here, every frame, so it can never drift out of step
	// with the state -- the same rule l03UpdateThrow() follows.
	switch (gL03Interact) {
	case L03I_BEGGING:  gW3.pose = L03_POSE_BEG;      break;
	case L03I_SLAP_UP:  gW3.pose = L03_POSE_SLAP_UP;  break;
	case L03I_SLAP_HIT: gW3.pose = L03_POSE_SLAP_HIT; break;
	default:            gW3.pose = L03_POSE_WALK;     break;
	}
}

// ---------------------------------------------------------------------------
//  INPUT
//
//  Returns true if the update should stop here, because the screen has changed
//  or the level is paused.
// ---------------------------------------------------------------------------
static bool level03HandleKeys()
{
	// ESC goes back to the campaign map, which is where this level was chosen.
	if (keyJustPressed(27)) {
		level03SilenceAll();
		setState(STATE_LEVEL_SELECT);
		return true;
	}

	if (keyJustPressed('r') || keyJustPressed('R')) {
		level03SilenceAll();
		level03Start();
		return true;
	}

	if (keyJustPressed('p') || keyJustPressed('P')) {
		if (gL03Phase == L03P_PLAYING) {
			gL03Phase = L03P_PAUSED;
			audioPause("michil");
		} else if (gL03Phase == L03P_PAUSED) {
			gL03Phase = L03P_PLAYING;
			audioResume("michil");
		}
	}

	if (gL03Phase == L03P_PAUSED) return true;

	// Once the level has ended either way, ENTER leaves for the campaign map.
	if (gL03Phase == L03P_GAMEOVER || gL03Phase == L03P_WON) {
		if (keyJustPressed(13)) {
			level03SilenceAll();
			setState(STATE_LEVEL_SELECT);
		}
		return true;
	}

	// F throws whatever he is holding. SPACE, W and UP jump -- and jumping is
	// read inside l03UpdateWalker, where the key can be held.
	//
	// HELD, not just pressed. keyJustPressed() fires on the up-to-down edge
	// alone, so holding F down threw exactly one rock however long you leaned
	// on it and the michil took seven separate presses to break up -- which
	// reads as a broken throw rather than a deliberate one.
	//
	// isKeyPressed() needs no repeat logic of its own: l03BeginThrow() already
	// refuses while a throw is mid-animation or the cooldown is still running,
	// so a held key settles into one throw every L03_THROW_COOLDOWN and a tap
	// still throws exactly once.
	if (isKeyPressed('f') || isKeyPressed('F'))
		l03BeginThrow();

	// H slaps the thief, and only the thief. level03Slap() is guarded by
	// level03CanSlap(), so outside that scene the key does nothing at all,
	// and inside it a second press during a swing is ignored rather than
	// restarting the swing.
	if (keyJustPressed('h') || keyJustPressed('H'))
		level03Slap();

	return false;
}

// ---------------------------------------------------------------------------
//  COLLISION
//
//  Every test is a bounding box through checkCollision(), never a distance
//  between sprite positions. A projectile is deactivated the instant it
//  connects, so one throw can never count twice.
// ---------------------------------------------------------------------------
static void level03CheckShotHits()
{
	for (int i = 0; i < MAX_L03_SHOTS; i++) {
		L03Shot &s = gL03Shots[i];
		if (!s.active) continue;

		Rect box = l03ShotHitBox(s);

		// The michil guards. A rock that reaches it is spent either way -- but
		// it only COUNTS if it arrives while the shield is down. That is the
		// whole encounter: pressing F is not a hit, a rock landing in the open
		// window is. Note the test is on arrival, here, not on the key press.
		if (gL03Stage == L03_MICHIL && l03MichilIsAlive() &&
		    checkCollision(box, l03MichilHitBox())) {
			s.active = false;

			if (l03MichilIsVulnerable())
				l03HitMichil();
			else
				l03MichilBlock();

			continue;
		}

		// Same rule as the michil: the stick is spent either way, but it only
		// COUNTS if it lands while he is winded and open. A stick that arrives
		// during guard, warning, rush or retreat is blocked.
		if (gL03Stage == L03_EVETEASING && l03EveIsAlive() &&
		    checkCollision(box, l03RusherHitBox())) {
			s.active = false;

			if (l03EveIsOpen())
				l03HitEve();
			else
				l03EveBlock();
		}
	}
}

// ---------------------------------------------------------------------------
//  THE STAGE MACHINE
//
//  Eleven stages, in order, and then it stops. Each one names the single
//  condition that ends it, which is what makes the road impossible to run out
//  of sequence: the michil is never spawned before the rock has been picked
//  up, and the jam is never reached before the chor has gone.
// ---------------------------------------------------------------------------
static void level03UpdateStage(double dt)
{
	gL03StageTimer += dt;

	switch (gL03Stage) {

	// --- 1. walking, with a rock lying in the road ahead ---
	case L03_WALK_TO_ROCK:
		// The rock is laid down once, a little ahead of him, and then drifts
		// back with the road until he walks into it.
		if (!gL03Pick.active && gL03Carry == CARRY_NONE &&
		    gL03Progress > L03_DIST_ROCK * 0.45)
			l03ShowPickup(CARRY_ROCK, WIN_W - 240.0);

		if (l03TookPickup()) {
			gL03Pick.active = false;
			gL03Carry = CARRY_ROCK;
			printf("[level03] picked up the rock\n");
		}

		if (gL03Progress >= L03_DIST_ROCK && gL03Carry == CARRY_ROCK) {
			l03SpawnMichil();
			level03SetStage(L03_MICHIL);
		}
		break;

	// --- 2. the michil: seven rocks ---
	case L03_MICHIL:
		l03UpdateMichil(dt);
		if (l03MichilCleared()) {
			gL03Carry = CARRY_NONE;           // the rock is spent with them
			l03ResetCocktail();               // nothing of theirs leaves with him
			level03SetStage(L03_WALK_TO_RAGGING);
		}
		break;

	// --- 3. walking on ---
	case L03_WALK_TO_RAGGING:
		if (gL03Progress >= L03_DIST_RAGGING) {
			l03StartRagging();
			level03SetStage(L03_RAGGING);
		}
		break;

	// --- 4. the conversation: he stands still and they take turns ---
	case L03_RAGGING:
		if (l03UpdateRagging(dt))
			level03SetStage(L03_WALK_TO_EVE);
		break;

	// --- 5. walking on, and a stick in the road ---
	case L03_WALK_TO_EVE:
		if (!gL03Pick.active && gL03Carry == CARRY_NONE &&
		    gL03Progress > L03_DIST_RAGGING + 420.0)
			l03ShowPickup(CARRY_STICK, WIN_W - 240.0);

		if (l03TookPickup()) {
			gL03Pick.active = false;
			gL03Carry = CARRY_STICK;
			printf("[level03] picked up the stick\n");
		}

		if (gL03Progress >= L03_DIST_EVE && gL03Carry == CARRY_STICK) {
			l03SpawnEve();
			level03SetStage(L03_EVETEASING);
		}
		break;

	// --- 6. the eve-teasers: five sticks ---
	case L03_EVETEASING:
		l03UpdateEve(dt);
		if (l03EveCleared()) {
			gL03Carry = CARRY_NONE;
			level03SetStage(L03_WALK_TO_CHOR);
		}
		break;

	// --- 7. walking on ---
	case L03_WALK_TO_CHOR:
		if (gL03Progress >= L03_DIST_CHOR) {
			l03SpawnChor();
			level03SetStage(L03_CHOR);
		}
		break;

	// --- 8. the chor: he runs in, and the character goes down and fights ---
	case L03_CHOR:
		l03UpdateChor(dt);

		// The one place begging ever begins: the frame the thief pulls up in
		// front of him. level03BeginBegging() is idempotent, so testing it
		// every frame of the scene costs nothing and cannot double-fire.
		if (l03ChorIsBegging())
			level03BeginBegging();

		level03UpdateInteraction(dt);

		if (l03ChorCleared()) {
			level03EndInteraction();
			level03SetStage(L03_WALK_TO_TRAFFIC);
		}
		break;

	// --- 9. the last stretch of open road ---
	case L03_WALK_TO_TRAFFIC:
		if (gL03Progress >= L03_DIST_TRAFFIC) {
			// Up onto the roofs. His ground line changes; his jump does not.
			gW3.groundY = JAM_ROOF_Y;
			gW3.y = JAM_ROOF_Y;
			gW3.isJumping = false;
			gW3.velocityY = 0.0;
			gW3.x = L03_PLAYER_X;

			// He is off the road now, so the road goes with him. Anything
			// still crossing would otherwise be drawn over the parked line
			// he is standing on.
			l03ClearTraffic();

			// Coins laid on the road would be left hanging below the roofs he
			// is now standing on, so the road's are cleared and the jam lays
			// down its own at roof height.
			l03ResetCoins();

			level03SetStage(L03_TRAFFIC);
		}
		break;

	// --- 10. across the jam, under fire, to the end of the road ---
	//
	// This is the last stretch, and it is walked like every other stretch:
	// what he pushes forward is what the world moves, and the backdrop, the
	// parked line and the fire all take the same step. Reaching the far end
	// of the street is AUST.
	case L03_TRAFFIC: {
		double push = l03JamPushThisFrame();

		gL03Progress += push;      // the backdrop reads straight off this
		l03UpdateJam(push);
		l03UpdateFlames(dt, push, true);

		// The end of the road is not the end of the level: it is where he
		// gets down off the jam and runs the last stretch to the gate.
		if (gL03Progress >= level03RoadEnd()) {
			gL03Progress = level03RoadEnd();

			// Back onto the pavement, in front of the university, entering
			// from the left. The fire does not follow him here.
			gW3.groundY = L03_GROUND_Y;
			gW3.y = L03_GROUND_Y;
			gW3.isJumping = false;
			gW3.velocityY = 0.0;
			gW3.x = L03_GATE_START_X;
			l03ResetFlames();
			l03ResetCoins();

			level03SetStage(L03_ARRIVE);
		}
		break;
	}

	// --- 11. the last few steps, in through the gate ---
	//
	// Scripted on purpose: the level is decided by now and this is the pay-off
	// for it, so he runs it himself rather than waiting on the player. The
	// walk cycle is already a run -- it is the same six frames Level 01
	// sprints the whole way on.
	case L03_ARRIVE:
		gW3.x += L03_GATE_RUN_SPEED * dt;

		if (gW3.x >= L03_GATE_STOP_X) {
			gW3.x = L03_GATE_STOP_X;

			// He is there. THIS is the win -- the sound, the campaign flag
			// and the save all fire here rather than out on the jam.
			level03Win();
			level03SetStage(L03_LEVEL_WON);
		}
		break;

	// --- 12. AUST ---
	case L03_LEVEL_WON:
		break;                             // he has arrived; nothing left to move
	}
}

// ---------------------------------------------------------------------------
//  UPDATE  --  paced off real elapsed time, never frame counts
// ---------------------------------------------------------------------------
void level03Update()
{
	DWORD now = timeGetTime();
	if (gL03LastTick == 0) gL03LastTick = now;
	double dt = (now - gL03LastTick) / 1000.0;
	gL03LastTick = now;

	// A long stall -- dragging the window, the loading pause -- must not
	// teleport the world forward.
	if (dt > 0.08) dt = 0.08;
	if (dt < 0.0)  dt = 0.0;

	if (level03HandleKeys()) return;

	// --- "get ready", with him already walking ---
	if (gL03Phase == L03P_READY) {
		gL03ReadyTimer -= dt;
		l03UpdateWalker(dt, false, false, true);
		if (gL03ReadyTimer <= 0.0) gL03Phase = L03P_PLAYING;
		return;
	}

	if (gL03Phase != L03P_PLAYING) return;

	// --- the road ---
	// He never travels along it: this is how far it has moved past him.
	bool walking = level03IsWalking();
	if (walking) gL03Progress += L03_WALK_SPEED * dt;

	// --- him ---
	// He drives his own x only on the jam. Everywhere else the road moves and
	// he stays put, exactly as in Level 01.
	// Kneeling in front of the thief he is not free to jump or run: the
	// interaction has the controls until it is over.
	// On the jam the cycle is driven by gW3.moving instead, so he stands still
	// when he stands still rather than jogging on the spot.
	//
	// The arrival takes the controls off him altogether: the run to the gate
	// is scripted, and his legs have to keep turning through it.
	bool onJam = level03OnTheJam();
	bool arriving = (gL03Stage == L03_ARRIVE);
	bool controlsLive = !level03InteractionActive() && !level03AtTheGate();

	l03UpdateWalker(dt, controlsLive, onJam, walking || arriving);
	l03UpdateThrow(dt, level03ReleaseThrow);

	// --- the road furniture ---
	l03UpdatePickup(dt, walking ? L03_WALK_SPEED : 0.0);
	l03UpdateShots(dt);

	// How far the street moved this frame: the walk on the open stretches,
	// nothing while a set-piece has him stopped, and his own push across the
	// jam. Everything that lies ON the road moves by this one number.
	double worldStep = (walking ? L03_WALK_SPEED * dt : 0.0)
	                 + l03JamPushThisFrame();

	// Coins are laid down wherever he is actually travelling -- the open road
	// and the jam -- and never during a set-piece or the run to the gate.
	bool coinsSpawning = walking || (gL03Stage == L03_TRAFFIC);
	l03UpdateCoins(dt, worldStep, coinsSpawning);

	if (l03TakeCoin(&gL03CoinPopX, &gL03CoinPopY))
		gL03CoinPop = L03_COIN_POP;

	// Street traffic runs the whole way up to the jam, through the michil and
	// the eve-teasers too, so standing still is not in itself safe.
	//
	// The two exceptions are the ragging and the thief: both take the controls
	// away from him completely, and being run over while pinned in a scripted
	// scene is not a difficulty, it is a bug. level03SafeFromTraffic() holds
	// the road for exactly those two stages and hands it straight back.
	l03UpdateTraffic(dt, level03StreetTrafficLive(), level03SafeFromTraffic());

	// --- the set-pieces ---
	level03UpdateStage(dt);

	// --- collision ---
	level03CheckShotHits();

	// --- timers ---
	if (gL03Invuln      > 0.0) gL03Invuln      -= dt;
	if (gL03HitFlash    > 0.0) gL03HitFlash    -= dt;
	if (gL03BannerTimer > 0.0) gL03BannerTimer -= dt;
	if (gL03CoinPop     > 0.0) gL03CoinPop     -= dt;
}

// ---------------------------------------------------------------------------
//  DRAWING
//
//  Draw only. Every decision was already made in level03Update().
// ---------------------------------------------------------------------------
// The AUST gate, drawn to COVER the window at its true aspect instead of being
// stretched to fit it. bg_last.jpeg is 4:3 against a 16:9 window, and a third
// of a stretch across a photograph of a real building is impossible to miss.
//
// Scaling by whichever axis needs the most and letting the rest run off the
// edges is the standard "cover" fit; OpenGL clips whatever leaves the window,
// so no texture-coordinate work is needed. L03_GATE_BG_BIAS chooses which end
// of the overflow survives -- high keeps the sign, low keeps the steps.
// The "+10" rising off a coin he just took. Same shape as Level 01's, drawn
// after him so it is never hidden behind his head.
static void level03DrawCoinPop()
{
	if (gL03CoinPop <= 0.0) return;

	double t = 1.0 - (gL03CoinPop / L03_COIN_POP);
	dSetColor(C_ACCENT);
	dTextCentered(gL03CoinPopX, gL03CoinPopY + t * 46.0, "+10",
	              GLUT_BITMAP_HELVETICA_18);
}

static void level03DrawGate()
{
	if (TEX_L03_BG_LAST == 0) {
		dFillRectA(0, 0, WIN_W, WIN_H, C_INK, 1.0);
		return;
	}

	double iw = (SZ_L03_BG_LAST_W > 0) ? (double)SZ_L03_BG_LAST_W : (double)WIN_W;
	double ih = (SZ_L03_BG_LAST_H > 0) ? (double)SZ_L03_BG_LAST_H : (double)WIN_H;

	double sx = (double)WIN_W / iw;
	double sy = (double)WIN_H / ih;
	double scale = (sx > sy) ? sx : sy;      // "cover": no gaps at any edge

	double w = iw * scale;
	double h = ih * scale;
	double x = -(w - (double)WIN_W) * 0.5;               // centred horizontally
	double y = -(h - (double)WIN_H) * L03_GATE_BG_BIAS;

	dImage(x, y, w, h, TEX_L03_BG_LAST);
}

static void level03DrawRoad()
{
	if (gBg3Count <= 0) {
		dFillRectA(0, 0, WIN_W, WIN_H, C_INK, 1.0);
		dSetColor(C_TEXT_DIM);
		dTextCentered(WIN_W / 2.0, WIN_H / 2.0,
		              "No backdrops found in level03/backround",
		              GLUT_BITMAP_HELVETICA_18);
		return;
	}

	// Two screens each frame: the one scrolling out and the one coming in
	// behind it, so the road never shows a gap. Fifteen backdrops in order,
	// wrapping, exactly the way Level 01 walks its street.
	double screen = gL03Progress / (double)WIN_W;
	int    index  = (int)screen % gBg3Count;
	double offset = (screen - floor(screen)) * WIN_W;
	int    next   = (index + 1) % gBg3Count;

	dImage(-offset,        0, WIN_W, WIN_H, TEX_L03_BG[index]);
	dImage(WIN_W - offset, 0, WIN_W, WIN_H, TEX_L03_BG[next]);

	// Two different streets butted together leaves a hard vertical line. A
	// soft shadow over the join reads as the gap between buildings instead.
	Color seam = { 18, 22, 32 };
	dSeamShade(WIN_W - offset, 78.0, seam, 0.45);
}

static void level03DrawBanner(const char *headline, const Color &color,
                              const char *subtitle)
{
	dDimScreen(0.74);

	dSetColor(color);
	dStrokeTextCentered(WIN_W / 2.0, WIN_H / 2.0 + 30, headline, 54, 3.4);

	if (subtitle) {
		dSetColor(C_TEXT);
		dTextCentered(WIN_W / 2.0, WIN_H / 2.0 - 12, subtitle,
		              GLUT_BITMAP_HELVETICA_18);
	}
}

static void level03DrawEndStats()
{
	char buf[200];

	dSetColor(C_TEXT_DIM);
	sprintf_s(buf, sizeof(buf), "Michil %d / %d      Eve teasers %d / %d      Slaps %d / %d",
	          gL03MichilHits, MICHIL_HITS_TO_CLEAR,
	          gL03EveHits, EVE_HITS_TO_CLEAR,
	          gL03Slaps, CHOR_SLAPS_TO_CLEAR);
	dTextCentered(WIN_W / 2.0, WIN_H / 2.0 - 58, buf, GLUT_BITMAP_HELVETICA_12);

	sprintf_s(buf, sizeof(buf), "Thrown %d      Hits taken %d      HP left %d",
	          gL03Thrown, gL03HitsTaken, gL03HP);
	dTextCentered(WIN_W / 2.0, WIN_H / 2.0 - 80, buf, GLUT_BITMAP_HELVETICA_12);

	dSetColor(C_TEXT);
	dTextCentered(WIN_W / 2.0, WIN_H / 2.0 - 116,
	              "ENTER or ESC  -  campaign map          R  -  run it again",
	              GLUT_BITMAP_HELVETICA_12);
}

// The layer order, and it is an order rather than a list: everything with a
// vehicle in it is drawn BEFORE the character, and everything that belongs to
// the character is drawn after him. Nothing below may be reshuffled without
// reading this comment first.
//
//      1  backdrop
//      2  the set-piece he is standing in front of
//      3  the parked jam -- the floor he walks along, so it is under him
//      4  road furniture
//      5  street traffic crossing
//      -- everything above this line is the world --
//      6  his contact shadow, then HIM
//      -- everything below this line is his, or is in front of everyone --
//      7  what he has thrown
//      8  fire from above
//      9  markers, then the HUD
void level03Draw()
{
	// 1. Once the jam is behind him the street is finished with, and the gate
	//    takes its place for the rest of the level.
	if (level03AtTheGate())
		level03DrawGate();
	else
		level03DrawRoad();

	l03DrawMichil();         // 2.
	l03DrawRagging();
	l03DrawEve();
	l03DrawChor();

	if (level03OnTheJam())   // 3.
		l03DrawJam();

	l03DrawPickup();         // 4.
	l03DrawCoins();
	l03DrawCocktail();       //    what the michil threw, at his shin height
	l03DrawRusher();         //    the eve-teaser who breaks off to charge
	l03DrawTraffic();        // 5.

	if (level03OnTheJam())   // 6. the shadow that ties his feet to the roof
		l03DrawWalkerShadow();
	l03DrawWalker();         //    ...and him, in front of every vehicle above

	l03DrawMichilCues();     //    JUMP! / THROW NOW! / BLOCKED!, over him
	l03DrawEveCues();        //    WATCH OUT! / JUMP! / HIT NOW! / DODGED!
	l03DrawChorCues();       //    WAIT... / SLAP NOW! / TOO FAR! / HIT!
	level03DrawCoinPop();    //    the "+10", over him
	l03DrawShots();          // 7.
	l03DrawFlames();         // 8.
	l03DrawTalkMarker();     // 9.

	if (gL03HitFlash > 0.0) {
		Color red = { 220, 40, 40 };
		dFillRectA(0, 0, WIN_W, WIN_H, red, gL03HitFlash * 0.55);
	}

	level03DrawHUD();        // 9. HUD on top

	// --- the stage caption, briefly, as each stage begins ---
	if (gL03BannerTimer > 0.0 && gL03Phase == L03P_PLAYING &&
	    gL03Stage != L03_LEVEL_WON) {
		dSetColor(C_SKY);
		dStrokeTextCentered(WIN_W / 2.0, WIN_H - 150.0,
		                    L03_STAGE_CAPTION[gL03Stage], 34, 2.4);
	}

	// --- overlays ---
	if (gL03Phase == L03P_READY) {
		dDimScreen(0.35);
		dSetColor(C_ACCENT);
		dStrokeTextCentered(WIN_W / 2.0, WIN_H / 2.0 + 10, "GET READY", 52, 3.2);
		dSetColor(C_TEXT);
		dTextCentered(WIN_W / 2.0, WIN_H / 2.0 - 28,
		              "SPACE jumps the traffic.  F throws.  H slaps.",
		              GLUT_BITMAP_HELVETICA_12);
	} else if (gL03Phase == L03P_PAUSED) {
		level03DrawBanner("PAUSED", C_SKY,
		                  "P to resume      R to restart      ESC for the map");
	} else if (gL03Phase == L03P_GAMEOVER) {
		Color red = { 235, 70, 60 };
		level03DrawBanner("GAME OVER", red, gL03EndReason);
		level03DrawEndStats();
	} else if (gL03Phase == L03P_WON) {
		Color green = { 90, 220, 120 };
		level03DrawBanner("WELCOME TO AUST", green,
		                  "You made it across the jam. Level 03 complete.");
		level03DrawEndStats();
	}
}

void level03Mouse(int button, int state, int mx, int my)
{
	// A left click throws, for anyone who reaches for the mouse.
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		l03BeginThrow();
}

#endif // LEVEL03_HPP

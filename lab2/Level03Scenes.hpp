//
//  Level03Scenes.hpp  --  the five set-pieces along the road to AUST.
//
//      the michil       a protest march, seven rocks to break it up
//      the ragging      twenty-two spoken lines, alternating speakers
//      the eve-teasers  five sticks
//      the chor         he runs in, stops, begs, and is slapped six times
//      the jam          a line of parked traffic to cross under falling fire
//
//  Each is a plain struct with its own reset, update and draw, the same shape
//  the rest of the project uses. None of them decides what stage the level is
//  in and none of them calls setState(): the stage machine in Level03.hpp
//  drives them all.
//
#ifndef LEVEL03SCENES_HPP
#define LEVEL03SCENES_HPP


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
#include "Assets.hpp"
#include "Entities.hpp"
#include "LevelState.hpp"
#include "Level03State.hpp"
#include "Level03Entities.hpp"
#endif
#include <math.h>

// ===========================================================================
//  1.  THE MICHIL
//
//  A wide crowd that comes down the road towards him chanting, presses in to a
//  set distance and holds there. Seven rocks break it up -- each one shoves it
//  back a little, so the fight has some give to it.
//
//  protest.png is drawn deliberately large: it is a march, not a person.
// ===========================================================================
//  It does not simply stand there and take it. It guards behind a frame of
//  portraits, warns, lobs a petrol bomb at his shins, and is open only for a
//  moment afterwards:
//
//      GUARD -> WARNING -> ATTACK -> VULNERABLE -> GUARD
//
//  Rocks that arrive while the shield is up are blocked and spent. Only a rock
//  that lands inside the vulnerable window counts, and seven counted hits
//  still break the march up. All the timings are in Config.hpp, where the
//  arithmetic behind them is written out.
enum MichilPhase {
	MICHIL_GUARD = 0,     // shield up, nothing counts
	MICHIL_WARNING,       // red "JUMP!", shield still up
	MICHIL_ATTACK,        // the cocktail is away, shield still up
	MICHIL_VULNERABLE     // shield down: the only window a rock counts in
};

struct Michil {
	bool        active;
	bool        dead;
	double      x, y;
	double      w, h;
	double      hitFlash;
	double      deathTimer;
	double      swayPhase;

	bool        engaged;       // has it closed to its holding distance yet
	MichilPhase phase;
	double      phaseTimer;    // counts DOWN through the current phase
	double      warnTime;      // this cycle's warning length
	bool        warnSounded;   // the horn goes once per warning, not per frame
	double      blockFlash;    // how long "BLOCKED!" stays up
	int         windowHits;    // rocks banked in the CURRENT open window
};

static Michil gMichil;

// ---------------------------------------------------------------------------
//  The petrol bomb
//
//  One at a time, launched at ground level, travelling in a straight line at a
//  fixed speed. It does NOT steer towards him: where it is going is settled
//  the moment it leaves, which is what makes the jump a real answer.
// ---------------------------------------------------------------------------
struct Cocktail {
	bool   active;
	bool   spent;          // has already been resolved against him
	double x, y;
	double w, h;
	double vx;
	double spin;
};

static Cocktail gCocktail;

void l03ResetCocktail()
{
	gCocktail.active = false;
	gCocktail.spent = false;
	gCocktail.x = 0.0;
	gCocktail.y = MICHIL_COCKTAIL_Y;
	gCocktail.h = MICHIL_COCKTAIL_H;
	gCocktail.w = MICHIL_COCKTAIL_H * spriteAspect(SPR_L03_COCKTAIL);
	gCocktail.vx = 0.0;
	gCocktail.spin = 0.0;
}

void l03ResetMichil()
{
	gMichil.active = false;
	gMichil.dead = false;
	gMichil.h = MICHIL_H;
	gMichil.w = MICHIL_H * spriteAspect(SPR_L03_PROTEST);
	gMichil.x = WIN_W + 80.0;
	gMichil.y = MICHIL_Y;
	gMichil.hitFlash = 0.0;
	gMichil.deathTimer = 0.0;
	gMichil.swayPhase = 0.0;

	gMichil.engaged = false;
	gMichil.phase = MICHIL_GUARD;
	gMichil.phaseTimer = MICHIL_GUARD_TIME;
	gMichil.warnTime = MICHIL_WARN_TIME;
	gMichil.warnSounded = false;
	gMichil.blockFlash = 0.0;
	gMichil.windowHits = 0;

	l03ResetCocktail();
}

void l03SpawnMichil()
{
	l03ResetMichil();
	gMichil.active = true;
	gL03MichilHits = 0;
	level03MichilSoundStart();
	printf("[level03] a michil is coming down the road  (0 / %d)\n",
	       MICHIL_HITS_TO_CLEAR);
}

Rect l03MichilRect()
{
	return makeRect(gMichil.x,
	                gMichil.y + sin(gMichil.swayPhase) * 4.0,
	                gMichil.w, gMichil.h);
}

// The body of the crowd, not the placards waving above it.
Rect l03MichilHitBox()
{
	Rect r = l03MichilRect();
	return makeRect(r.x + r.w * 0.10,
	                r.y + r.h * 0.06,
	                r.w * 0.80,
	                r.h * 0.70);
}

bool l03MichilIsAlive()
{
	return gMichil.active && !gMichil.dead;
}

// ---------------------------------------------------------------------------
//  Guarding, and the one window that is not
// ---------------------------------------------------------------------------

// The only time a rock counts.
bool l03MichilIsVulnerable()
{
	return l03MichilIsAlive() && gMichil.phase == MICHIL_VULNERABLE;
}

// The shield is up for everything except that window -- and it comes down for
// good once the crowd is beaten.
bool l03MichilShieldUp()
{
	return l03MichilIsAlive() && gMichil.phase != MICHIL_VULNERABLE;
}

// A rock that arrived while the shield was up. It is spent either way; this
// only raises the feedback, and deliberately does NOT touch the hit counter.
void l03MichilBlock()
{
	if (!l03MichilIsAlive()) return;
	gMichil.blockFlash = MICHIL_BLOCK_FLASH;
	printf("[level03] BLOCKED -- still %d / %d\n",
	       gL03MichilHits, MICHIL_HITS_TO_CLEAR);
}

// ---------------------------------------------------------------------------
//  The petrol bomb
// ---------------------------------------------------------------------------

// Launched from the front of the crowd at the height of his shins.
//
// The SPEED is worked out from the distance so that the flight always lasts
// MICHIL_COCKTAIL_FLIGHT seconds. Every hit shoves the crowd back a little, so
// a fixed speed would quietly stretch the warning-to-impact gap as the fight
// went on and the dodge that worked at the start would stop working. A fixed
// flight time keeps that gap identical every single cycle.
static void l03LaunchCocktail()
{
	Rect m = l03MichilRect();

	l03ResetCocktail();

	gCocktail.x = m.x;                       // the face of the march
	gCocktail.y = MICHIL_COCKTAIL_Y;
	gCocktail.active = true;
	gCocktail.spent = false;

	Rect body = l03WalkerHitBox();
	double travel = gCocktail.x - (body.x + body.w);
	if (travel < 120.0) travel = 120.0;      // never absurdly fast up close

	gCocktail.vx = -(travel / MICHIL_COCKTAIL_FLIGHT);

	printf("[level03] the michil throws a cocktail  (%.0f px in %.2f s)\n",
	       travel, MICHIL_COCKTAIL_FLIGHT);
}

Rect l03CocktailRect()
{
	return makeRect(gCocktail.x, gCocktail.y, gCocktail.w, gCocktail.h);
}

// The bottle, not the empty margin the photograph leaves around it.
Rect l03CocktailHitBox()
{
	return shrinkRect(l03CocktailRect(), 0.74, 0.80);
}

void l03UpdateCocktail(double dt)
{
	if (!gCocktail.active) return;

	gCocktail.x += gCocktail.vx * dt;
	gCocktail.spin += dt * 7.0;

	// One bottle can only ever cost him once, however long it stays beside him.
	if (!gCocktail.spent &&
	    checkCollision(l03WalkerHitBox(), l03CocktailHitBox())) {
		gCocktail.spent = true;
		gCocktail.active = false;            // and it is gone on contact
		level03Damage(MICHIL_COCKTAIL_DAMAGE);
		return;
	}

	if (gCocktail.x + gCocktail.w < -80.0) gCocktail.active = false;
}

void l03DrawCocktail()
{
	if (!gCocktail.active) return;

	Rect r = l03CocktailRect();
	dImage(r.x, r.y, r.w, r.h, SPR_L03_COCKTAIL.tex);
}

static void l03MichilEnterPhase(MichilPhase next);   // defined below

// One rock has landed. Returns true on the seventh.
bool l03HitMichil()
{
	if (!l03MichilIsAlive()) return false;

	gL03MichilHits++;
	gMichil.hitFlash = MICHIL_HIT_FLASH;
	gMichil.x += MICHIL_KNOCKBACK;          // shoved back down the road

	playCollisionSound();

	printf("[level03] michil hit  %d / %d\n", gL03MichilHits, MICHIL_HITS_TO_CLEAR);

	if (gL03MichilHits >= MICHIL_HITS_TO_CLEAR) {
		gMichil.dead = true;
		gMichil.deathTimer = MICHIL_DEATH_TIME;
		level03MichilSoundStop();            // the chanting stops with it

		// Beaten means beaten: the bottle already in the air is put out with
		// them, so nothing can land a hit after the fight is over.
		l03ResetCocktail();

		printf("[level03] the michil breaks up\n");
		return true;
	}

	// That opening has cost it enough: the guard slams straight back up, which
	// the player SEES as the shield returning rather than as a rock silently
	// not counting. The rest of the seven have to be earned in later windows.
	gMichil.windowHits++;
	if (gMichil.windowHits >= MICHIL_HITS_PER_WINDOW)
		l03MichilEnterPhase(MICHIL_GUARD);

	return false;
}

// ---------------------------------------------------------------------------
//  The attack cycle
//
//  One phase, one timer, counted down on dt. Nothing here runs on frames and
//  nothing blocks, so pausing -- which simply stops calling this -- freezes
//  the cycle mid-phase and resumes it exactly where it stopped.
// ---------------------------------------------------------------------------
static void l03MichilEnterPhase(MichilPhase next)
{
	gMichil.phase = next;

	switch (next) {
	case MICHIL_GUARD:
		gMichil.phaseTimer = MICHIL_GUARD_TIME;
		break;

	case MICHIL_WARNING:
		// Once it has been hurt enough it sometimes hurries the warning. Still
		// one cocktail, still dodgeable -- it just asks for a quicker read.
		gMichil.warnTime =
			(gL03MichilHits >= MICHIL_RAGE_AFTER &&
			 randRange(1, 100) <= MICHIL_RAGE_PERCENT)
			? MICHIL_WARN_TIME_FAST
			: MICHIL_WARN_TIME;

		gMichil.phaseTimer = gMichil.warnTime;
		gMichil.warnSounded = false;
		break;

	case MICHIL_ATTACK:
		gMichil.phaseTimer = MICHIL_ATTACK_TIME;
		l03LaunchCocktail();
		break;

	case MICHIL_VULNERABLE:
		gMichil.phaseTimer = MICHIL_VULN_TIME;
		gMichil.windowHits = 0;        // a fresh opening is worth the full cap
		break;
	}
}

static void l03UpdateMichilPhases(double dt)
{
	if (!l03MichilIsAlive()) return;

	// It only starts fighting back once it has closed to its holding distance;
	// while it is still walking in, there is nothing to dodge.
	//
	// ONCE, though. This used to be tested every frame, and every landed rock
	// shoves the crowd MICHIL_KNOCKBACK back -- straight back outside the test
	// -- which froze the phase timer mid-VULNERABLE. The window then never
	// closed, and a player leaning on the throw key emptied all seven hits
	// into a single opening. Latching it keeps the cycle running through the
	// knockback, which is what makes each window worth only what fits in it.
	if (!gMichil.engaged) {
		if (gMichil.x > MICHIL_ENGAGE_X) return;
		gMichil.engaged = true;
	}

	if (gMichil.phase == MICHIL_WARNING && !gMichil.warnSounded) {
		gMichil.warnSounded = true;          // once per warning, never per frame
		audioPlayOnce("sfxcar");             // an existing horn, no new asset
	}

	gMichil.phaseTimer -= dt;
	if (gMichil.phaseTimer > 0.0) return;

	switch (gMichil.phase) {
	case MICHIL_GUARD:      l03MichilEnterPhase(MICHIL_WARNING);    break;
	case MICHIL_WARNING:    l03MichilEnterPhase(MICHIL_ATTACK);     break;
	case MICHIL_ATTACK:     l03MichilEnterPhase(MICHIL_VULNERABLE); break;
	case MICHIL_VULNERABLE: l03MichilEnterPhase(MICHIL_GUARD);      break;
	}
}

void l03UpdateMichil(double dt)
{
	if (!gMichil.active) return;

	gMichil.swayPhase += dt * 2.2;
	if (gMichil.hitFlash   > 0.0) gMichil.hitFlash   -= dt;
	if (gMichil.blockFlash > 0.0) gMichil.blockFlash -= dt;

	// A bottle still in the air keeps travelling while the crowd scatters, so
	// this sits above the dead check -- but l03HitMichil() has already cleared
	// it on the seventh hit, so after a win there is nothing left to update.
	l03UpdateCocktail(dt);

	if (gMichil.dead) {
		gMichil.deathTimer -= dt;
		gMichil.x += 170.0 * dt;             // scatters back up the road
		if (gMichil.deathTimer <= 0.0) gMichil.active = false;
		return;
	}

	l03UpdateMichilPhases(dt);

	// Presses in, then holds at arm's length and chants.
	if (gMichil.x > MICHIL_STOP_X)
		gMichil.x -= MICHIL_SPEED * dt;
}

bool l03MichilCleared()
{
	return gL03MichilHits >= MICHIL_HITS_TO_CLEAR && !gMichil.active;
}

void l03DrawMichil()
{
	if (!gMichil.active) return;

	Rect r = l03MichilRect();

	if (gMichil.dead) {
		double fade = gMichil.deathTimer / MICHIL_DEATH_TIME;
		if (fade < 0.0) fade = 0.0;
		dImageEx(r.x, r.y, r.w, r.h, SPR_L03_PROTEST.tex, false, 1.0, 1.0, 1.0, fade);
		return;
	}

	dImage(r.x, r.y, r.w, r.h, SPR_L03_PROTEST.tex);

	if (gMichil.hitFlash > 0.0)
		dImageEx(r.x, r.y, r.w, r.h, SPR_L03_PROTEST.tex, false,
		         1.0, 0.65, 0.55, gMichil.hitFlash / MICHIL_HIT_FLASH);

	// --- the shield ---------------------------------------------------------
	//
	// Sized off the crowd, but by HEIGHT only: the width follows the frame's
	// own aspect ratio so the portraits keep their proportions instead of
	// being smeared across a box that is nothing like their shape. The march
	// is wider than the window, so the frame sits over its LEFT end -- the
	// face he is actually throwing at -- rather than over the middle of it.
	if (l03MichilShieldUp()) {
		double sh = r.h * MICHIL_SHIELD_H_SCALE;
		double sw = sh * spriteAspect(SPR_L03_SHIELD);
		double cx = r.x + r.w * MICHIL_SHIELD_CX;
		double cy = r.y + r.h * 0.5;

		dImage(cx - sw * 0.5, cy - sh * 0.5, sw, sh, SPR_L03_SHIELD.tex);
	}

	// Seven pips over the crowd, so the count is on screen rather than in the
	// player's head.
	const double pipW = 17.0, pipH = 8.0, gap = 5.0;
	int max = MICHIL_HITS_TO_CLEAR;
	double totalW = max * pipW + (max - 1) * gap;
	double px = r.x + r.w * 0.5 - totalW * 0.5;
	double py = r.y + r.h + 10.0;

	int left = max - gL03MichilHits;
	for (int i = 0; i < max; i++) {
		Color pip = (i < left) ? C_ACCENT : C_PANEL;
		dFillRectA(px + i * (pipW + gap), py, pipW, pipH, pip, 0.93);
	}
}

// ---------------------------------------------------------------------------
//  What the encounter is asking of him, in words, where he is looking
//
//  Drawn separately from the crowd so it lands in FRONT of the character and
//  the shield rather than behind them.
// ---------------------------------------------------------------------------
void l03DrawMichilCues()
{
	if (!gMichil.active) return;

	const double cx = WIN_W * 0.42;

	// "JUMP!" -- red, and it pulses, because this is the one cue that costs
	// health to miss. The old blue guard rectangle is gone, so the red has to
	// carry the warning on its own.
	if (l03MichilIsAlive() && gMichil.phase == MICHIL_WARNING) {
		double t = gMichil.phaseTimer / gMichil.warnTime;   // 1 -> 0
		double pulse = 0.72 + 0.28 * sin(gMichil.phaseTimer * 26.0);

		// Dark plate under BRIGHT red letters. Red-on-red was unreadable
		// against a sunlit street, which is no use at all for the one cue
		// that costs health to miss.
		Color plate = { 40, 6, 6 };
		Color red   = { 255, 70, 55 };

		dFillRectA(cx - 132.0, WIN_H - 232.0, 264.0, 52.0, plate, 0.55 + 0.30 * (1.0 - t));
		dRectOutlineA(cx - 132.0, WIN_H - 232.0, 264.0, 52.0, red, 0.55 + 0.45 * pulse, 2.0);

		dSetColor(red);
		dStrokeTextCentered(cx, WIN_H - 220.0, "JUMP!", 40, 3.0 * pulse);
	}

	// "THROW NOW!" -- the shield is down and only now does a rock count.
	if (l03MichilIsVulnerable()) {
		Color green = { 90, 225, 120 };
		dSetColor(green);
		dStrokeTextCentered(cx, WIN_H - 220.0, "THROW NOW!", 34, 2.4);
	}

	// "BLOCKED!" -- a rock that arrived while the shield was up.
	if (gMichil.blockFlash > 0.0) {
		double fade = gMichil.blockFlash / MICHIL_BLOCK_FLASH;
		Rect r = l03MichilRect();

		dSetColor(C_TEXT_DIM);
		dStrokeTextCentered(r.x + r.w * MICHIL_SHIELD_CX,
		                    r.y + r.h * 0.5 + 40.0 * (1.0 - fade),
		                    "BLOCKED!", 26, 2.0);
	}
}

// ===========================================================================
//  2.  THE RAGGING
//
//  He is stopped in the road and spoken at. Twenty-two clips play strictly in
//  order, and the NEXT one only begins once the last has actually finished --
//  the device is asked, not guessed at with a timer, so the exchange never
//  talks over itself however long or short a line happens to be.
//
//  Odd lines are the raggers, even lines are him, so they alternate by
//  counting rather than by a table.
// ===========================================================================
struct Ragging {
	bool   active;
	double x, y;
	double w, h;
	double gap;          // breath between lines
	bool   finished;
};

static Ragging gRag;

// Whether the current line is still sounding, and when to ask again.
//
// This is CACHED deliberately. Asking MCI "status mode" is a string command
// that has to be parsed and dispatched to the device, and the first version
// asked once per frame -- sixty round trips a second, for the length of a
// twenty-two line conversation. That dragged the whole level down to a crawl
// and the process did not survive it. Polling seven times a second is far more
// than enough to catch the end of a clip and costs almost nothing.
static bool   gRagSpeaking = false;
static double gRagPoll = 0.0;

const double RAG_POLL_EVERY = 0.15;

void l03ResetRagging()
{
	gRag.active = false;
	gRag.h = RAGGING_H;
	gRag.w = RAGGING_H * spriteAspect(SPR_L03_RAGGING);
	gRag.x = RAGGING_X;
	gRag.y = RAGGING_Y;
	gRag.gap = 0.0;
	gRag.finished = false;

	gRagSpeaking = false;
	gRagPoll = 0.0;
}

void l03StartRagging()
{
	l03ResetRagging();
	gRag.active = true;
	gL03TalkLine = 0;
	gRag.gap = 0.6;            // a beat before the first word
	printf("[level03] ragging: %d lines to get through\n", TALK_LINE_COUNT);
}

Rect l03RaggingRect()
{
	return makeRect(gRag.x, gRag.y, gRag.w, gRag.h);
}

// Runs the conversation. Returns true once the last line has been spoken.
bool l03UpdateRagging(double dt)
{
	if (!gRag.active) return false;
	if (gRag.finished) return true;

	// Ask the device only every so often, and believe the answer in between.
	gRagPoll -= dt;
	if (gRagPoll <= 0.0) {
		gRagPoll = RAG_POLL_EVERY;
		gRagSpeaking = level03LineIsSpeaking();
	}

	// Still talking: leave it alone.
	if (gL03TalkLine >= 1 && gRagSpeaking) return false;

	// Between lines.
	if (gRag.gap > 0.0) {
		gRag.gap -= dt;
		return false;
	}

	if (gL03TalkLine >= TALK_LINE_COUNT) {
		gRag.finished = true;

		// They are done with him, so they clear off. The michil and the
		// eve-teasers take themselves off the road when they die; this group
		// never dies, so without this it stood there for the rest of the level
		// and the chor ran in from behind it.
		gRag.active = false;

		printf("[level03] the conversation is over\n");
		return true;
	}

	gL03TalkLine++;
	level03SpeakLine(gL03TalkLine);
	gRag.gap = TALK_GAP;

	// It has only just been told to play, so do not let the next poll catch it
	// before the device has actually started and skip the line.
	gRagSpeaking = true;
	gRagPoll = RAG_POLL_EVERY;
	return false;
}

bool l03RaggerSpeaking()
{
	return gL03TalkLine >= 1 && (gL03TalkLine % 2) == 1;
}

void l03DrawRagging()
{
	if (!gRag.active) return;

	Rect r = l03RaggingRect();
	dImage(r.x, r.y, r.w, r.h, SPR_L03_RAGGING.tex);
}

// A marker over whoever is talking, so it is obvious the two are taking turns.
void l03DrawTalkMarker()
{
	if (!gRag.active || gL03TalkLine < 1) return;
	if (!gRagSpeaking) return;           // the cached answer, not a fresh query

	Rect r = l03RaggerSpeaking() ? l03RaggingRect() : l03WalkerRect();

	double cx = r.x + r.w * 0.5;
	double cy = r.y + r.h + 24.0;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1.0f, 0.78f, 0.25f, 0.85f);
	iFilledCircle(cx, cy, 9.0);
	glDisable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	dSetColor(C_TEXT);
	dTextCentered(cx, cy + 16.0,
	              l03RaggerSpeaking() ? "RAGGER" : "YOU",
	              GLUT_BITMAP_HELVETICA_12);
}

// ===========================================================================
//  3.  THE EVE-TEASERS
//
//  A group blocking the road, harassing. Five sticks clears them. It is the
//  same shape as the michil, with its own counter and its own artwork.
// ===========================================================================
//  The group itself does not move and is not the target. One of them breaks
//  off as the RUSHER and does all the fighting; the girl they surround stays
//  in the artwork where she is.
enum EvePhase {
	EVE_GUARD = 0,      // open stance, sticks bounce off him
	EVE_WARNING,        // "WATCH OUT!", then "JUMP!" near the end
	EVE_RUSH,           // crouched, charging along the ground
	EVE_RETURN,         // backing off, harmless, cannot touch him
	EVE_RECOVERY        // winded at his spot: the only window a stick counts
};

struct EveScene {
	bool     active;
	bool     dead;
	double   x, y;              // the GROUP, which never moves off its mark
	double   w, h;
	double   hitFlash;
	double   deathTimer;
	double   swayPhase;

	// --- the aggressor ---
	EvePhase phase;
	double   phaseTimer;        // counts DOWN through the current phase
	double   warnTime;          // this cycle's warning length
	bool     warnSounded;       // the horn goes once per warning, never per frame
	bool     jumpCued;          // "JUMP!" has been raised this warning
	double   rusherX;           // his left edge; the group's x is separate
	bool     rushSpent;         // this rush has already cost the player health
	bool     cycleHit;          // this cycle has already been counted
	double   blockFlash;        // "BLOCKED!"
	double   dodgeFlash;        // "DODGED!"
	double   scoreFlash;        // "+1 HIT"
	double   stepPhase;         // his legs, while he moves
};

static EveScene gEve;

void l03ResetEve()
{
	gEve.active = false;
	gEve.dead = false;
	gEve.h = EVE_H;
	gEve.w = EVE_H * spriteAspect(SPR_L03_EVETEASE);
	gEve.x = EVE_X;
	gEve.y = EVE_Y;
	gEve.hitFlash = 0.0;
	gEve.deathTimer = 0.0;
	gEve.swayPhase = 0.0;

	gEve.phase = EVE_GUARD;
	gEve.phaseTimer = EVE_GUARD_TIME;
	gEve.warnTime = EVE_WARN_TIME;
	gEve.warnSounded = false;
	gEve.jumpCued = false;
	gEve.rusherX = EVE_RUSHER_HOME_X;
	gEve.rushSpent = false;
	gEve.cycleHit = false;
	gEve.blockFlash = 0.0;
	gEve.dodgeFlash = 0.0;
	gEve.scoreFlash = 0.0;
	gEve.stepPhase = 0.0;
}

void l03SpawnEve()
{
	l03ResetEve();
	gEve.active = true;
	gL03EveHits = 0;
	printf("[level03] eve-teasers blocking the road  (0 / %d)\n", EVE_HITS_TO_CLEAR);
}

Rect l03EveRect()
{
	return makeRect(gEve.x, gEve.y + sin(gEve.swayPhase) * 3.0, gEve.w, gEve.h);
}

Rect l03EveHitBox()
{
	Rect r = l03EveRect();
	return makeRect(r.x + r.w * 0.12,
	                r.y + r.h * 0.08,
	                r.w * 0.76,
	                r.h * 0.76);
}

bool l03EveIsAlive()
{
	return gEve.active && !gEve.dead;
}

// ---------------------------------------------------------------------------
//  The aggressor
// ---------------------------------------------------------------------------

// Low and long while charging, upright the rest of the time.
static bool l03RusherIsLow()
{
	return gEve.phase == EVE_RUSH;
}

// Constant width, taken from the artwork's own proportions; only the height
// drops for the charge. Same rect the sprite is drawn into, so the box can
// never claim reach he does not visibly have.
Rect l03RusherRect()
{
	double w = EVE_RUSHER_H * spriteAspect(SPR_L03_EVE_RUSHER);
	double h = l03RusherIsLow() ? EVE_RUSHER_LOW_H : EVE_RUSHER_H;

	return makeRect(gEve.rusherX, L03_GROUND_Y, w, h);
}

// Pulled well inside the drawing, so there is no reach he does not visibly
// have. This is the box the jump has to clear, and the arithmetic that says
// it can is written out in Config.hpp.
Rect l03RusherHitBox()
{
	return shrinkRect(l03RusherRect(), EVE_RUSH_BOX_W, EVE_RUSH_BOX_H);
}

// The one moment a stick counts -- and only the first stick of the cycle.
bool l03EveIsOpen()
{
	return l03EveIsAlive() && gEve.phase == EVE_RECOVERY && !gEve.cycleHit;
}

// A stick reached him while he was not open. Spent, but never counted.
void l03EveBlock()
{
	if (!l03EveIsAlive()) return;
	gEve.blockFlash = EVE_FEEDBACK_TIME;
	printf("[level03] BLOCKED -- still %d / %d\n", gL03EveHits, EVE_HITS_TO_CLEAR);
}

static void l03EveEnterPhase(EvePhase next);   // defined below

bool l03HitEve()
{
	// Only while he is open, and only once per cycle. Pressing F is not a hit;
	// a stick ARRIVING here is, which is why this is called from the collision
	// pass and not from the key handler.
	if (!l03EveIsOpen()) return false;

	gEve.cycleHit = true;
	gEve.scoreFlash = EVE_FEEDBACK_TIME;

	gL03EveHits++;
	gEve.hitFlash = EVE_HIT_FLASH;

	playCollisionSound();

	printf("[level03] eve-teaser hit  %d / %d\n", gL03EveHits, EVE_HITS_TO_CLEAR);

	if (gL03EveHits >= EVE_HITS_TO_CLEAR) {
		gEve.dead = true;
		gEve.deathTimer = EVE_DEATH_TIME;

		// Beaten means beaten: the rush stops dead and cannot cost the player
		// anything on the way out.
		gEve.phase = EVE_GUARD;
		gEve.rusherX = EVE_RUSHER_HOME_X;
		gEve.rushSpent = true;

		printf("[level03] the eve-teasers scatter\n");
		return true;
	}

	// Taking one sends him straight back to his guard, which the player SEES,
	// rather than later sticks silently not counting.
	l03EveEnterPhase(EVE_GUARD);
	return false;
}

// ---------------------------------------------------------------------------
//  The attack cycle
//
//  One phase, one dt-driven timer. Nothing blocks and nothing sleeps, so a
//  pause -- which simply stops calling this -- freezes him mid-phase and
//  resumes exactly where he stopped.
// ---------------------------------------------------------------------------
static void l03EveEnterPhase(EvePhase next)
{
	gEve.phase = next;

	switch (next) {
	case EVE_GUARD:
		gEve.phaseTimer = EVE_GUARD_TIME;
		gEve.rusherX = EVE_RUSHER_HOME_X;
		break;

	case EVE_WARNING:
		// Once he has been hurt enough, some cycles hurry the warning. Still
		// one rush, still the same dodge -- it just asks for a quicker read.
		gEve.warnTime =
			(gL03EveHits >= EVE_RAGE_AFTER &&
			 randRange(1, 100) <= EVE_RAGE_PERCENT)
			? EVE_WARN_TIME_FAST
			: EVE_WARN_TIME;

		gEve.phaseTimer = gEve.warnTime;
		gEve.warnSounded = false;
		gEve.jumpCued = false;
		break;

	case EVE_RUSH:
		gEve.phaseTimer = 0.0;      // driven by distance, not by a clock
		gEve.rushSpent = false;     // one rush, one chance to cost health
		break;

	case EVE_RETURN:
		gEve.phaseTimer = 0.0;
		// He cannot touch the player on the way back, whatever he passes
		// through. The player is landing about now and must not be punished
		// for a dodge that already worked.
		gEve.rushSpent = true;
		break;

	case EVE_RECOVERY:
		gEve.phaseTimer = EVE_RECOVER_TIME;
		gEve.rusherX = EVE_RUSHER_HOME_X;
		gEve.cycleHit = false;      // a fresh opening is worth one hit
		break;
	}
}

// The rush, the retreat, and the one collision that can cost health.
static void l03UpdateRusher(double dt)
{
	if (!l03EveIsAlive()) return;

	if (gEve.phase == EVE_RUSH || gEve.phase == EVE_RETURN)
		gEve.stepPhase += dt * 14.0;

	switch (gEve.phase) {

	case EVE_GUARD:
		gEve.phaseTimer -= dt;
		if (gEve.phaseTimer <= 0.0) l03EveEnterPhase(EVE_WARNING);
		break;

	case EVE_WARNING:
		if (!gEve.warnSounded) {
			gEve.warnSounded = true;         // once per warning, never per frame
			audioPlayOnce("sfxbike");        // an existing horn, no new asset
		}
		// "JUMP!" replaces "WATCH OUT!" near the end, close enough to the
		// launch that reacting to it -- even instantly -- still clears him.
		if (gEve.phaseTimer <= EVE_JUMP_CUE_AT) gEve.jumpCued = true;

		gEve.phaseTimer -= dt;
		if (gEve.phaseTimer <= 0.0) l03EveEnterPhase(EVE_RUSH);
		break;

	case EVE_RUSH:
		// A fixed horizontal path. It does not follow his height and it does
		// not steer towards him: where it is going was settled when it started.
		gEve.rusherX -= EVE_RUSH_SPEED * dt;

		if (!gEve.rushSpent &&
		    checkCollision(l03WalkerHitBox(), l03RusherHitBox())) {
			gEve.rushSpent = true;           // one rush, one hit, explicitly
			level03Damage(EVE_RUSH_DAMAGE);
		}

		if (gEve.rusherX <= EVE_RUSHER_TURN_X) {
			// He got past. If he never touched the player, that was a dodge.
			if (!gEve.rushSpent) gEve.dodgeFlash = EVE_FEEDBACK_TIME;
			l03EveEnterPhase(EVE_RETURN);
		}
		break;

	case EVE_RETURN:
		// Backing off, and harmless: rushSpent is already true, so the code
		// above cannot fire and nothing here tests collision at all.
		gEve.rusherX += EVE_RETURN_SPEED * dt;
		if (gEve.rusherX >= EVE_RUSHER_HOME_X) {
			gEve.rusherX = EVE_RUSHER_HOME_X;
			l03EveEnterPhase(EVE_RECOVERY);
		}
		break;

	case EVE_RECOVERY:
		gEve.phaseTimer -= dt;
		if (gEve.phaseTimer <= 0.0) l03EveEnterPhase(EVE_GUARD);
		break;
	}
}

void l03UpdateEve(double dt)
{
	if (!gEve.active) return;

	gEve.swayPhase += dt * 2.6;
	if (gEve.hitFlash   > 0.0) gEve.hitFlash   -= dt;
	if (gEve.blockFlash > 0.0) gEve.blockFlash -= dt;
	if (gEve.dodgeFlash > 0.0) gEve.dodgeFlash -= dt;
	if (gEve.scoreFlash > 0.0) gEve.scoreFlash -= dt;

	if (gEve.dead) {
		gEve.deathTimer -= dt;
		gEve.x += 230.0 * dt;
		if (gEve.deathTimer <= 0.0) gEve.active = false;
		return;
	}

	l03UpdateRusher(dt);
}

bool l03EveCleared()
{
	return gL03EveHits >= EVE_HITS_TO_CLEAR && !gEve.active;
}

void l03DrawEve()
{
	if (!gEve.active) return;

	Rect r = l03EveRect();

	if (gEve.dead) {
		double fade = gEve.deathTimer / EVE_DEATH_TIME;
		if (fade < 0.0) fade = 0.0;
		dImageEx(r.x, r.y, r.w, r.h, SPR_L03_EVETEASE.tex, false, 1.0, 1.0, 1.0, fade);
		return;
	}

	dImage(r.x, r.y, r.w, r.h, SPR_L03_EVETEASE.tex);

	if (gEve.hitFlash > 0.0)
		dImageEx(r.x, r.y, r.w, r.h, SPR_L03_EVETEASE.tex, false,
		         1.0, 0.65, 0.55, gEve.hitFlash / EVE_HIT_FLASH);

	const double pipW = 18.0, pipH = 8.0, gap = 6.0;
	int max = EVE_HITS_TO_CLEAR;
	double totalW = max * pipW + (max - 1) * gap;
	double px = r.x + r.w * 0.5 - totalW * 0.5;
	double py = r.y + r.h + 10.0;

	int left = max - gL03EveHits;
	for (int i = 0; i < max; i++) {
		Color pip = (i < left) ? C_ACCENT : C_PANEL;
		dFillRectA(px + i * (pipW + gap), py, pipW, pipH, pip, 0.93);
	}
}

// ---------------------------------------------------------------------------
//  The aggressor, drawn from primitives
//
//  A red-shirt figure, as the concept preview used. He is NOT cut out of
//  eveteasing.jpg: the boys in that picture overlap one another, so every
//  rectangle that takes one takes slices of his neighbours with it -- I tried
//  it. Drawing him keeps the group artwork whole and untouched, which is the
//  point: the girl they are surrounding stays in the picture and never
//  becomes the attacker or the target.
// ---------------------------------------------------------------------------
void l03DrawRusher()
{
	if (!l03EveIsAlive()) return;

	Rect r = l03RusherRect();

	// He blinks when a stick lands, so the hit reads off the figure as well as
	// off the caption.
	double a = 1.0;
	if (gEve.scoreFlash > 0.0) {
		double f = gEve.scoreFlash / EVE_FEEDBACK_TIME;
		if (((int)(gEve.scoreFlash * 22.0) % 2) == 0) a = 0.40 + 0.60 * (1.0 - f);
	}

	if (l03RusherIsLow()) {
		// Speed lines trailing behind the charge, so the direction is obvious
		// even at a glance.
		Color trail = { 70, 90, 170 };
		for (int i = 0; i < 3; i++)
			dFillRectA(r.x + r.w + 6.0 + i * 6.0,
			           r.y + r.h * (0.30 + 0.22 * i),
			           24.0 - i * 6.0, 3.0, trail, 0.34 * a);
	}

	if (a < 1.0)
		dImageEx(r.x, r.y, r.w, r.h, SPR_L03_EVE_RUSHER.tex, false, 1.0, 1.0, 1.0, a);
	else
		dImage(r.x, r.y, r.w, r.h, SPR_L03_EVE_RUSHER.tex);

	// The "!" over his head while he winds up, so the attack is telegraphed
	// from the figure itself and not only from the caption.
	if (gEve.phase == EVE_WARNING) {
		Color red = { 255, 70, 55 };
		dSetColor(red);
		dStrokeTextCentered(r.x + r.w * 0.5, r.y + r.h + 14.0, "!", 30, 3.0);
	}
}

// ---------------------------------------------------------------------------
//  What the encounter is asking of him, in words
//
//  Drawn after the character so nothing hides behind him. Same shape as the
//  michil's cues -- this is the game's existing HUD language, not the concept
//  video's tutorial chrome.
// ---------------------------------------------------------------------------
void l03DrawEveCues()
{
	if (!gEve.active) return;

	const double cx = WIN_W * 0.42;

	if (l03EveIsAlive() && gEve.phase == EVE_WARNING) {
		double pulse = 0.72 + 0.28 * sin(gEve.phaseTimer * 26.0);
		Color plate = { 40, 6, 6 };
		Color red   = { 255, 70, 55 };

		dFillRectA(cx - 150.0, WIN_H - 232.0, 300.0, 52.0, plate, 0.72);
		dRectOutlineA(cx - 150.0, WIN_H - 232.0, 300.0, 52.0, red,
		              0.55 + 0.45 * pulse, 2.0);

		dSetColor(red);
		dStrokeTextCentered(cx, WIN_H - 220.0,
		                    gEve.jumpCued ? "JUMP!" : "WATCH OUT!",
		                    gEve.jumpCued ? 40 : 30,
		                    (gEve.jumpCued ? 3.0 : 2.2) * pulse);
	}

	if (l03EveIsAlive() && gEve.phase == EVE_RECOVERY) {
		Color green = { 90, 225, 120 };
		dSetColor(green);
		dStrokeTextCentered(cx, WIN_H - 220.0, "HIT NOW!", 34, 2.4);
	}

	if (gEve.dodgeFlash > 0.0) {
		double f = gEve.dodgeFlash / EVE_FEEDBACK_TIME;
		Rect b = l03WalkerRect();
		Color green = { 120, 230, 150 };
		dSetColor(green);
		dStrokeTextCentered(b.x + b.w * 0.5, b.y + b.h + 16.0 + 30.0 * (1.0 - f),
		                    "DODGED!", 22, 1.8);
	}

	if (gEve.blockFlash > 0.0) {
		double f = gEve.blockFlash / EVE_FEEDBACK_TIME;
		Rect r = l03RusherRect();
		dSetColor(C_TEXT_DIM);
		dStrokeTextCentered(r.x + r.w * 0.5, r.y + r.h + 34.0 * (1.0 - f),
		                    "BLOCKED!", 24, 1.9);
	}

	if (gEve.scoreFlash > 0.0) {
		double f = gEve.scoreFlash / EVE_FEEDBACK_TIME;
		Rect r = l03RusherRect();
		Color green = { 110, 240, 140 };
		dSetColor(green);
		dStrokeTextCentered(r.x + r.w * 0.5, r.y + r.h + 20.0 + 34.0 * (1.0 - f),
		                    "+1 HIT", 24, 2.0);
	}
}

// ===========================================================================
//  4.  THE CHOR
//
//  He sprints in on the twelve run_frames and stops dead in front.
//
//  What happens next belongs to the CHARACTER, not to him: beg.png, and then
//  the two slap frames, are all the character's poses and are driven by the
//  interaction state machine in Level03State.hpp. Everything this struct does
//  is be a thief -- run in, stand there, flinch when hit, and bolt on the
//  sixth. It counts the slaps but it does not draw them and it does not
//  decide when one happens.
// ===========================================================================
enum ChorPhase {
	CHOR_RUNNING_IN = 0,
	CHOR_BEGGING,
	CHOR_FLEEING,
	CHOR_GONE
};

// What he is doing inside CHOR_BEGGING: the repeating cycle the player has to
// read. PLEADING is the opening -- he has just pulled up, the player is still
// on his knees, and the first opening does not run out. Everything after that
// is timed.
enum ChorStance {
	CHOR_PLEADING = 0,   // the opening beg, in reach, no clock on it
	CHOR_FAR,            // out of reach: "WAIT..."
	CHOR_APPROACH,       // darting in
	CHOR_OPEN,           // in reach: "SLAP NOW! - H"
	CHOR_RETREAT         // backing off again
};

struct Chor {
	bool       active;
	ChorPhase  phase;
	double     x, y;
	double     w, h;
	int        frame;
	double     frameTimer;
	double     slapShow;     // how long he is still reeling from the last one
	double     begBob;

	ChorStance stance;
	double     stanceTimer;  // counts DOWN through the current stance
	bool       openHit;      // this opening has already been paid for
	double     hitCue;       // "HIT!"
	double     missCue;      // "TOO FAR!"
};

static Chor gChor;

void l03ResetChor()
{
	gChor.active = false;
	gChor.phase = CHOR_RUNNING_IN;
	gChor.h = CHOR_H;
	// He is only ever drawn as a run frame now, so he is framed as one. This
	// used to borrow the slap sprite's aspect, which stretched him a little.
	gChor.w = CHOR_H * L03_RUN_ASPECT;
	gChor.x = CHOR_START_X;
	gChor.y = L03_GROUND_Y;
	gChor.frame = 0;
	gChor.frameTimer = 0.0;
	gChor.slapShow = 0.0;
	gChor.begBob = 0.0;

	gChor.stance = CHOR_PLEADING;
	gChor.stanceTimer = 0.0;
	gChor.openHit = false;
	gChor.hitCue = 0.0;
	gChor.missCue = 0.0;
}

void l03SpawnChor()
{
	l03ResetChor();
	gChor.active = true;
	gL03Slaps = 0;
	printf("[level03] a chor is running at you  (H to slap, %d needed)\n",
	       CHOR_SLAPS_TO_CLEAR);
}

Rect l03ChorRect()
{
	return makeRect(gChor.x, gChor.y, gChor.w, gChor.h);
}

Rect l03ChorHitBox()
{
	Rect r = l03ChorRect();
	return makeRect(r.x + r.w * 0.22, r.y + r.h * 0.06, r.w * 0.56, r.h * 0.88);
}

bool l03ChorIsBegging()
{
	return gChor.active && gChor.phase == CHOR_BEGGING;
}

// ---------------------------------------------------------------------------
//  Reach, and the opening
// ---------------------------------------------------------------------------

// How far the player's hand covers: his own body box plus CHOR_SLAP_REACH.
// Kept as a real rectangle so the test below is the same checkCollision() call
// everything else in the level uses.
Rect l03SlapReachBox()
{
	Rect b = l03WalkerHitBox();
	return makeRect(b.x, b.y, b.w + CHOR_SLAP_REACH, b.h);
}

// Is he actually close enough to be hit RIGHT NOW? This is geometry, not
// stance: it is what decides whether a landing hand connects.
bool l03ChorInReach()
{
	if (!l03ChorIsBegging()) return false;
	return checkCollision(l03SlapReachBox(), l03ChorHitBox());
}

// Is the opening being advertised? This is what makes an attempt VALID, and it
// is read the moment H goes down -- never again -- so a swing begun out of
// range cannot turn into a hit just because he closes in afterwards.
bool l03ChorOpeningUp()
{
	if (!l03ChorIsBegging()) return false;
	if (gChor.openHit) return false;
	return gChor.stance == CHOR_OPEN || gChor.stance == CHOR_PLEADING;
}

// The thief's half of one slap: he takes it and reels. The character's half
// -- coming up off his knees and swinging -- is the interaction state machine,
// which is what calls this. Returns true on the sixth, when he gives up.
bool l03SlapChor()
{
	if (!l03ChorIsBegging()) return false;

	// One opening is worth one slap, whatever the player does inside it.
	if (gChor.openHit) return false;

	gChor.openHit = true;
	gChor.hitCue = CHOR_CUE_TIME;

	// He reels, then backs off. The pause is what makes the recoil readable
	// instead of him snapping straight back out of range.
	gChor.stance = CHOR_RETREAT;
	gChor.stanceTimer = CHOR_HIT_PAUSE;

	gL03Slaps++;
	gChor.slapShow = SLAP_SHOW_TIME;

	playCollisionSound();            // once per landed slap, and only here

	printf("[level03] slap  %d / %d\n", gL03Slaps, CHOR_SLAPS_TO_CLEAR);

	if (gL03Slaps >= CHOR_SLAPS_TO_CLEAR) {
		gChor.phase = CHOR_FLEEING;

		// Beaten: no more openings, nothing left pending, and no further
		// counting. The existing flee animation takes it from here and
		// l03ChorCleared() makes the stage transition exactly once.
		gChor.stance = CHOR_RETREAT;
		gChor.stanceTimer = 0.0;
		gL03SlapPending = false;
		gL03SlapValid = false;
		gL03SlapRecover = 0.0;

		printf("[level03] the chor gives up and bolts\n");
		return true;
	}
	return false;
}

// ---------------------------------------------------------------------------
//  The cycle
//
//  Distances drive the movement and dt drives the clocks, so a pause -- which
//  simply stops calling this -- freezes him exactly where he is, mid-stride
//  included.
//
//  The one subtlety is the GRACE at the end of an opening. A slap takes
//  SLAP_UP_TIME to land, so an H pressed on the last frame of the opening
//  would otherwise swing into empty air the moment he steps back. While a
//  valid attempt is still in the air the opening simply does not expire: he
//  waits to be hit. That grace is only ever given to an attempt that BEGAN
//  inside the opening -- see l03ChorOpeningUp().
// ---------------------------------------------------------------------------
static void l03UpdateChorStance(double dt)
{
	if (gChor.hitCue  > 0.0) gChor.hitCue  -= dt;
	if (gChor.missCue > 0.0) gChor.missCue -= dt;

	// The run cycle turns over only while he is actually moving.
	if (gChor.stance == CHOR_APPROACH || gChor.stance == CHOR_RETREAT) {
		gChor.frameTimer += dt;
		while (gChor.frameTimer >= CHOR_FRAME_TIME) {
			gChor.frameTimer -= CHOR_FRAME_TIME;
			gChor.frame = (gChor.frame + 1) % L03_RUN_FRAME_COUNT;
		}
	}

	switch (gChor.stance) {

	case CHOR_PLEADING:
		// The opening beg. It does not run out -- the player is still on his
		// knees and has not been shown the rhythm yet.
		break;

	case CHOR_FAR:
		gChor.stanceTimer -= dt;
		if (gChor.stanceTimer <= 0.0) {
			gChor.stance = CHOR_APPROACH;
			gChor.stanceTimer = 0.0;
		}
		break;

	case CHOR_APPROACH:
		gChor.x -= CHOR_APPROACH_SPEED * dt;
		if (gChor.x <= CHOR_STOP_X) {
			gChor.x = CHOR_STOP_X;
			gChor.stance = CHOR_OPEN;
			gChor.stanceTimer = CHOR_OPEN_TIME;
			gChor.openHit = false;       // a fresh opening, worth one slap
		}
		break;

	case CHOR_OPEN:
		// Hold the opening open while a valid swing is still travelling.
		if (level03SlapPendingValid()) break;

		gChor.stanceTimer -= dt;
		if (gChor.stanceTimer <= 0.0) {
			gChor.stance = CHOR_RETREAT;
			gChor.stanceTimer = 0.0;
		}
		break;

	case CHOR_RETREAT:
		// A landed slap leaves him reeling for a moment before he moves.
		if (gChor.stanceTimer > 0.0) { gChor.stanceTimer -= dt; break; }

		gChor.x += CHOR_RETREAT_SPEED * dt;
		if (gChor.x >= CHOR_FAR_X) {
			gChor.x = CHOR_FAR_X;
			gChor.stance = CHOR_FAR;
			gChor.stanceTimer = CHOR_WAIT_TIME;
		}
		break;
	}
}

void l03UpdateChor(double dt)
{
	if (!gChor.active) return;

	if (gChor.slapShow > 0.0) gChor.slapShow -= dt;

	if (gChor.phase == CHOR_RUNNING_IN) {
		gChor.x -= CHOR_SPEED * dt;

		// The run cycle only turns over while he is actually running.
		gChor.frameTimer += dt;
		while (gChor.frameTimer >= CHOR_FRAME_TIME) {
			gChor.frameTimer -= CHOR_FRAME_TIME;
			gChor.frame = (gChor.frame + 1) % L03_RUN_FRAME_COUNT;
		}

		if (gChor.x <= CHOR_STOP_X) {
			gChor.x = CHOR_STOP_X;
			gChor.phase = CHOR_BEGGING;
			// The plea and the pose are the CHARACTER'S, so the stage machine
			// starts them off the back of this. Playing the clip from in here
			// would put the audio in one file and the pose in another.
			printf("[level03] the chor pulls up right in front of you\n");
		}
		return;
	}

	if (gChor.phase == CHOR_BEGGING) {
		gChor.begBob += dt * 3.0;
		l03UpdateChorStance(dt);
		return;
	}

	if (gChor.phase == CHOR_FLEEING) {
		gChor.x += CHOR_FLEE_SPEED * dt;

		gChor.frameTimer += dt;
		while (gChor.frameTimer >= CHOR_FRAME_TIME) {
			gChor.frameTimer -= CHOR_FRAME_TIME;
			gChor.frame = (gChor.frame + 1) % L03_RUN_FRAME_COUNT;
		}

		if (gChor.x > WIN_W + 120.0) {
			gChor.phase = CHOR_GONE;
			gChor.active = false;
		}
	}
}

bool l03ChorCleared()
{
	return gL03Slaps >= CHOR_SLAPS_TO_CLEAR && !gChor.active;
}

void l03DrawChor()
{
	if (!gChor.active) return;

	Rect r = l03ChorRect();

	// He is always a run frame: charging in facing LEFT, running away facing
	// RIGHT, and standing on frame nought in between. A slap knocks him back a
	// few pixels and reddens him for a moment -- that is his whole reaction.
	// The slap artwork itself is the character's, and is drawn with him.
	bool   flee = (gChor.phase == CHOR_FLEEING);
	double reel = (gChor.slapShow > 0.0)
	            ? (gChor.slapShow / SLAP_SHOW_TIME) * 14.0
	            : 0.0;

	if (gChor.slapShow > 0.0)
		dImageEx(r.x + reel, r.y, r.w, r.h, TEX_L03_RUN[gChor.frame],
		         flee, 1.0, 0.45, 0.45, 1.0);
	else
		dImageFlipped(r.x, r.y, r.w, r.h, TEX_L03_RUN[gChor.frame], flee);

	// How many he has left in him.
	if (gChor.phase == CHOR_BEGGING) {
		const double pipW = 16.0, pipH = 8.0, gap = 5.0;
		int max = CHOR_SLAPS_TO_CLEAR;
		double totalW = max * pipW + (max - 1) * gap;
		double px = r.x + r.w * 0.5 - totalW * 0.5;
		double py = r.y + r.h + 2.0;

		int left = max - gL03Slaps;
		for (int i = 0; i < max; i++) {
			Color pip = (i < left) ? C_ACCENT : C_PANEL;
			dFillRectA(px + i * (pipW + gap), py, pipW, pipH, pip, 0.93);
		}
	}
}

// ---------------------------------------------------------------------------
//  What the thief is asking of the player, in words
//
//  Drawn after the character so nothing hides behind him, in the same shape as
//  the michil's and the eve-teasers' cues.
// ---------------------------------------------------------------------------
void l03DrawChorCues()
{
	if (!gChor.active) return;

	const double cx = WIN_W * 0.42;

	if (l03ChorIsBegging()) {
		if (l03ChorOpeningUp()) {
			Color green = { 90, 225, 120 };
			dSetColor(green);
			dStrokeTextCentered(cx, WIN_H - 220.0, "SLAP NOW!  -  H", 30, 2.3);
		} else if (!gChor.openHit) {
			dSetColor(C_TEXT_DIM);
			dStrokeTextCentered(cx, WIN_H - 220.0, "WAIT...", 26, 2.0);
		}
	}

	// "HIT!" over the thief, and the running count beside it.
	if (gChor.hitCue > 0.0) {
		double f = gChor.hitCue / CHOR_CUE_TIME;
		Rect r = l03ChorRect();
		Color green = { 110, 240, 140 };

		char buf[48];
		sprintf_s(buf, sizeof(buf), "HIT!   %d / %d", gL03Slaps, CHOR_SLAPS_TO_CLEAR);

		dSetColor(green);
		dStrokeTextCentered(r.x + r.w * 0.5, r.y + r.h + 22.0 + 30.0 * (1.0 - f),
		                    buf, 24, 2.0);
	}

	// "TOO FAR!" over the player, where the swing happened.
	if (gChor.missCue > 0.0) {
		double f = gChor.missCue / CHOR_CUE_TIME;
		Rect b = l03WalkerRect();
		Color red = { 255, 90, 75 };

		dSetColor(red);
		dStrokeTextCentered(b.x + b.w * 0.5, b.y + b.h + 16.0 + 30.0 * (1.0 - f),
		                    "TOO FAR!", 24, 2.0);
	}
}

// ===========================================================================
//  5.  THE JAM, AND FIRE FROM ABOVE
//
//  A long line of parked vehicles laid end to end, in a fixed order that is
//  laid out once and never reshuffled. He jumps up onto the roofs and WALKS
//  the length of them while flames drop out of the sky, pushing forward and
//  giving ground to get out from under each one.
//
//  The line only moves when he does: what he walks forward is what slides
//  left, so the cars travel backwards past him and the street behind them
//  changes with it. Standing still, nothing moves but the fire.
//
//  The roofs are one flat line -- JAM_ROOF_Y -- so his ground changes but his
//  jump is the same jump, and his box tracks his real y exactly as it does on
//  the road.
// ===========================================================================
struct JamVehicle {
	int    art;            // which of the six
	double x;
	double w, h;
};

static JamVehicle gJam[JAM_VEHICLE_COUNT];
static double gJamScroll = 0.0;

void l03ResetJam()
{
	double x = 0.0;
	for (int i = 0; i < JAM_VEHICLE_COUNT; i++) {
		JamVehicle &v = gJam[i];
		v.art = i % 6;                      // bus, car01..03, cng, rickshaw
		v.h = JAM_VEHICLE_H;
		v.w = JAM_VEHICLE_H * spriteAspect(SPR_L03_JAM[v.art]);
		v.x = x;
		x += v.w - 6.0;                     // nose to tail, slightly overlapping
	}
	gJamScroll = 0.0;
}

// The line slides left by exactly what he walked forward, and by nothing at
// all when he stands still. That is what makes it a crossing he makes rather
// than a conveyor he rides.
//
// push is in pixels, already worked out by the walker.
void l03UpdateJam(double push)
{
	gJamScroll += push;
}

void l03DrawJam()
{
	// The line is longer than the window and it wraps, so there is always
	// traffic under him however far he walks.
	//
	// The ORDER never changes. gJam[] is laid out once in l03ResetJam -- bus,
	// car01, car02, car03, cng, rickshaw, repeating -- and nothing here or
	// anywhere else reshuffles it or picks a fresh vehicle. Wrapping repeats
	// the same sequence, so the jam he walks back over is the jam he walked.
	double span = 0.0;
	for (int i = 0; i < JAM_VEHICLE_COUNT; i++) span += gJam[i].w - 6.0;
	if (span < 1.0) span = 1.0;

	double shift = fmod(gJamScroll, span);

	for (int pass = 0; pass < 2; pass++) {
		double base = -shift + pass * span;
		for (int i = 0; i < JAM_VEHICLE_COUNT; i++) {
			const JamVehicle &v = gJam[i];
			double x = base + v.x;
			if (x > WIN_W + 60.0 || x + v.w < -60.0) continue;
			dImage(x, JAM_BASE_Y, v.w, v.h, SPR_L03_JAM[v.art].tex);
		}
	}
}

// --- the fire ---------------------------------------------------------------
struct Flame {
	bool   active;
	bool   spent;
	double x, y;
	double w, h;
	double speed;
	double flicker;
};

static Flame gFlames[MAX_FLAMES];
static double gFlameTimer = 0.0;

void l03ResetFlames()
{
	for (int i = 0; i < MAX_FLAMES; i++) gFlames[i].active = false;
	gFlameTimer = FLAME_MIN_GAP;
}

Rect l03FlameRect(const Flame &f)
{
	return makeRect(f.x, f.y, f.w, f.h);
}

// The burning core, not the whole sprite: most of a flame picture is smoke and
// being burned by the smoke would feel wrong.
Rect l03FlameHitBox(const Flame &f)
{
	return shrinkRect(l03FlameRect(f), 0.46, 0.58);
}

static void l03DropFlame()
{
	int slot = -1;
	for (int i = 0; i < MAX_FLAMES; i++)
		if (!gFlames[i].active) { slot = i; break; }
	if (slot < 0) return;

	Flame &f = gFlames[slot];
	f.h = FLAME_H;
	f.w = FLAME_H * spriteAspect(SPR_L03_FLAME);

	// Anywhere across the street, NOT centred on where he is standing. Homing
	// the spread on him meant moving barely changed the odds; spreading it
	// wide is what makes stepping aside work -- and the ones that land ahead
	// of him are what make pushing forward a decision rather than a free win.
	f.x = randBetween(FLAME_SPAWN_MIN_X, FLAME_SPAWN_MAX_X);
	if (f.x < 20.0) f.x = 20.0;
	if (f.x > WIN_W - 80.0) f.x = WIN_W - 80.0;
	f.y = WIN_H + 40.0;
	f.speed = FLAME_FALL_SPEED * randBetween(0.88, 1.18);
	f.flicker = randBetween(0.0, 6.28);
	f.spent = false;
	f.active = true;
}

//  push      how far the world moved forward this frame
//  dropping  whether new flames may still be lit
//
//  Flames travel with the street. If they did not, walking forward would
//  carry him straight into the one he was trying to leave, and the only dodge
//  left would be backwards.
void l03UpdateFlames(double dt, double push, bool dropping)
{
	Rect body = l03WalkerHitBox();

	for (int i = 0; i < MAX_FLAMES; i++) {
		Flame &f = gFlames[i];
		if (!f.active) continue;

		f.x -= push;
		f.y -= f.speed * dt;
		f.flicker += dt * 10.0;

		if (f.x + f.w < -120.0) { f.active = false; continue; }

		// One flame can only ever cost him once, however long it falls past.
		if (!f.spent && checkCollision(body, l03FlameHitBox(f))) {
			f.spent = true;
			level03Damage(L03_HIT_DAMAGE);
		}

		if (f.y + f.h < -40.0) f.active = false;
	}

	if (!dropping) return;

	gFlameTimer -= dt;
	if (gFlameTimer <= 0.0) {
		gFlameTimer = randBetween(FLAME_MIN_GAP, FLAME_MAX_GAP);
		l03DropFlame();
	}
}

void l03DrawFlames()
{
	for (int i = 0; i < MAX_FLAMES; i++) {
		const Flame &f = gFlames[i];
		if (!f.active) continue;

		Rect r = l03FlameRect(f);
		double pulse = 0.88 + 0.12 * sin(f.flicker);
		dImageEx(r.x, r.y, r.w, r.h, SPR_L03_FLAME.tex, false, 1.0, pulse, pulse, 1.0);
	}
}

// ---------------------------------------------------------------------------
//  Everything at once, for a fresh run
// ---------------------------------------------------------------------------
void l03ResetScenes()
{
	l03ResetMichil();
	l03ResetRagging();
	l03ResetEve();
	l03ResetChor();
	l03ResetJam();
	l03ResetFlames();
}

#endif // LEVEL03SCENES_HPP

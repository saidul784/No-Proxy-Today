//
//  Entities.hpp  --  road traffic, birds, coins and power-ups.
//
//  For most of the run exactly ONE obstacle is on the road at a time -- it
//  appears far off, closes, passes or hits, leaves, and only then does the gap
//  timer start counting towards the next. Birds share the same pool, so the
//  game can never demand a jump and a no-jump in the same moment.
//
//  Inside the final minute that opens up: the gap collapses, traffic speeds up,
//  and up to RUSH_MAX_ACTIVE obstacles run at once. Even then nothing is
//  released until the previous one has cleared RUSH_MIN_SEPARATION, so there is
//  always room to land a jump and take off again.
//
//  Each obstacle carries warningSoundPlayed. Its own sound fires once, when it
//  first comes inside WARNING_DISTANCE, and never again. A fresh obstacle in a
//  reused slot gets a fresh flag.
//
#ifndef ENTITIES_HPP
#define ENTITIES_HPP


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
#include "Assets.hpp"
#include "LevelState.hpp"
#endif
#include <stdlib.h>

enum MoveMode {
	MOVE_COMING = 0,   // head-on from the right, travelling left
	MOVE_GOING         // overtaking from the left, travelling right
};

// ---------------------------------------------------------------------------
struct Obstacle {
	bool   active;
	bool   warningSoundPlayed;   // its sound has already been heard
	bool   hasHitCharacter;      // it has already landed its one hit
	bool   scored;
	int    type;
	int    mode;
	double x;                    // left edge, screen space
	double y;                    // bottom edge, screen space
	double speed;                // pixels per second along x
};

struct Coin {
	bool   active;
	bool   collected;
	double x;
};

enum PickupType { PU_HEALTH = 0, PU_SHIELD };

struct Pickup {
	bool   active;
	int    type;
	double x;
	double bob;
};

static Obstacle gObstacles[MAX_ROAD_OBSTACLES];
static Coin     gCoins[MAX_COINS];
static Pickup   gPickups[MAX_PICKUPS];

static double gObstacleTimer = 0.0;
static double gCoinTimer     = 0.0;
static double gPickupTimer   = 0.0;

// ---------------------------------------------------------------------------
//  Helpers
// ---------------------------------------------------------------------------
// MSVC's rand() is a 15-bit linear congruential generator, and consecutive
// draws taken modulo a small number correlate badly. In the final minute, where
// spawns are only a fraction of a second apart and barely any other randomness
// is consumed between them, that showed up as long runs of the same obstacle --
// five dogs in a row, and not a single bird. This is a xorshift32 instead: the
// same handful of instructions, without the patterning.
static unsigned int gRngState = 2463534242u;

void seedGameRandom(unsigned int seed)
{
	gRngState = (seed == 0) ? 2463534242u : seed;
}

static unsigned int nextRandom()
{
	gRngState ^= gRngState << 13;
	gRngState ^= gRngState >> 17;
	gRngState ^= gRngState << 5;
	return gRngState;
}

static int randRange(int lo, int hi)
{
	return lo + (int)(nextRandom() % (unsigned int)(hi - lo + 1));
}

static double randUnit()
{
	return (double)(nextRandom() & 0xFFFFFFu) / (double)0xFFFFFFu;
}

static double randBetween(double lo, double hi) { return lo + randUnit() * (hi - lo); }

// The street only turns frantic once the clock is inside the final minute.
bool inFinalRush()
{
	return gPhase == LP_PLAYING && gTimeLeft <= FINAL_RUSH_SECONDS;
}

// Which artwork to use. Every road type has a left-facing "coming" sprite and a
// right-facing "going" one, so nothing normally needs mirroring.
static Sprite obstacleSprite(int type, int mode)
{
	bool coming = (mode == MOVE_COMING);
	switch (type) {
	case OB_BIKE:     return coming ? SPR_BIKE_COMING : SPR_BIKE_GOING;
	case OB_CAR:      return coming ? SPR_CAR_COMING  : SPR_CAR_GOING;
	case OB_DOG:      return coming ? SPR_DOG_COMING  : SPR_DOG_GOING;
	case OB_RICKSHAW:
		if (!coming) return SPR_RICK_GOING;
		return RICKSHAW_COMING_NEEDS_FALLBACK ? SPR_RICK_GOING : SPR_RICK_COMING;
	case OB_BIRD:     return SPR_BIRDS;
	}
	return SPR_BIRDS;
}

// True only when a right-facing sprite is standing in for a left-facing one.
static bool obstacleNeedsFlip(const Obstacle &o)
{
	return o.type == OB_RICKSHAW && o.mode == MOVE_COMING && RICKSHAW_COMING_NEEDS_FALLBACK;
}

Rect obstacleDrawRect(const Obstacle &o)
{
	Sprite s = obstacleSprite(o.type, o.mode);
	double h = OB_HEIGHT[o.type];
	double w = h * spriteAspect(s);
	return makeRect(o.x, o.y, w, h);
}

// Each type gets its own box. A car is close to a solid block; a dog is mostly
// legs and tail; the bird sprite is a spread-out flock with sky between the
// birds. The road vehicles are trimmed hard vertically so that clearing them
// in a jump is comfortable rather than frame-perfect.
Rect obstacleHitBox(const Obstacle &o)
{
	Rect r = obstacleDrawRect(o);
	switch (o.type) {
	case OB_CAR:      return shrinkRect(r, 0.66, 0.74);
	case OB_BIKE:     return shrinkRect(r, 0.80, 0.74);
	case OB_RICKSHAW: return shrinkRect(r, 0.82, 0.74);
	case OB_DOG:      return shrinkRect(r, 0.78, 0.70);
	case OB_BIRD:     return shrinkRect(r, 0.56, 0.60);
	}
	return shrinkRect(r, 0.80, 0.74);
}

// How far the obstacle still is from the character, along x.
double obstacleDistanceToCharacter(const Obstacle &o)
{
	Rect r = obstacleDrawRect(o);
	if (o.mode == MOVE_COMING) return r.x - PLAYER_X;          // still to the right
	return PLAYER_X - (r.x + r.w);                             // still to the left
}

Rect coinDrawRect(const Coin &c)
{
	return makeRect(c.x, GROUND_Y + COIN_HEIGHT, COIN_SIZE, COIN_SIZE);
}

Rect coinHitBox(const Coin &c)
{
	return shrinkRect(coinDrawRect(c), 0.86, 0.86);
}

Rect pickupDrawRect(const Pickup &p)
{
	double h = PICKUP_SIZE;
	double w = h * spriteAspect(p.type == PU_HEALTH ? SPR_PU_HEALTH : SPR_PU_SHIELD);
	return makeRect(p.x, GROUND_Y + PICKUP_HEIGHT + sin(p.bob) * 9.0, w, h);
}

Rect pickupHitBox(const Pickup &p)
{
	// The artwork is mostly glow, so the collectable part is the middle.
	return shrinkRect(pickupDrawRect(p), 0.55, 0.55);
}

static int activeObstacleCount()
{
	int n = 0;
	for (int i = 0; i < MAX_ROAD_OBSTACLES; i++)
		if (gObstacles[i].active) n++;
	return n;
}

// True while any obstacle is near the stretch of road the character is on.
// Coins and power-ups hold off while this is the case, so nothing is ever
// dropped on top of an obstacle.
bool obstacleIsNear()
{
	for (int i = 0; i < MAX_ROAD_OBSTACLES; i++) {
		if (!gObstacles[i].active) continue;
		Rect r = obstacleDrawRect(gObstacles[i]);
		if (r.x + r.w > PLAYER_X - 700.0 && r.x < PLAYER_X + 1000.0) return true;
	}
	return false;
}

// ---------------------------------------------------------------------------
//  Spawning
// ---------------------------------------------------------------------------
static void scheduleNextObstacle()
{
	if (inFinalRush()) gObstacleTimer = randBetween(RUSH_MIN_INTERVAL, RUSH_MAX_INTERVAL);
	else               gObstacleTimer = randBetween(MIN_OBSTACLE_INTERVAL, MAX_OBSTACLE_INTERVAL);
}

// Has everything already on the road put enough distance behind it for another
// one to be fair? Measured from the edge each new obstacle enters at.
static bool roomForAnotherObstacle()
{
	for (int i = 0; i < MAX_ROAD_OBSTACLES; i++) {
		Obstacle &o = gObstacles[i];
		if (!o.active) continue;

		Rect r = obstacleDrawRect(o);
		double travelled = (o.mode == MOVE_COMING) ? (WIN_W - r.x) : (r.x + r.w);
		if (travelled < RUSH_MIN_SEPARATION) return false;
	}
	return true;
}

static void spawnObstacle()
{
	int slot = -1;
	for (int i = 0; i < MAX_ROAD_OBSTACLES; i++)
		if (!gObstacles[i].active) { slot = i; break; }
	if (slot < 0) return;

	Obstacle &o = gObstacles[slot];

	o.active = true;
	o.warningSoundPlayed = false;   // a fresh obstacle gets a fresh sound state
	o.hasHitCharacter = false;
	o.scored = false;
	o.type = randRange(0, OB_TYPE_COUNT - 1);

	double rush = inFinalRush() ? RUSH_SPEED_SCALE : 1.0;

	if (o.type == OB_BIRD) {
		// Birds always fly in from ahead, in one of two bands. The low one has
		// to be jumped; the high one is safe unless the character jumps into it.
		o.mode = MOVE_COMING;
		o.speed = OB_SPEED_BIRD * rush;
		o.y = GROUND_Y + ((randRange(1, 100) <= 50) ? BIRD_LOW_Y : BIRD_HIGH_Y);
	} else {
		o.mode = (randRange(1, 100) <= 60) ? MOVE_COMING : MOVE_GOING;
		o.speed = ((o.mode == MOVE_COMING) ? OB_SPEED_COMING : OB_SPEED_GOING) * rush;
		o.y = GROUND_Y;
	}

	// Spawn well off the edge, so it is seen and heard long before it arrives.
	Rect r = obstacleDrawRect(o);
	if (o.mode == MOVE_COMING) o.x = (double)WIN_W + 80.0;
	else                       o.x = -r.w - 80.0;

	printf("[level] %-8s %s%s\n", OB_NAME[o.type],
	       (o.mode == MOVE_COMING) ? "coming" : "going",
	       inFinalRush() ? "   (rush)" : "");
}

static void spawnCoinRow()
{
	int count = randRange(COINS_PER_ROW_MIN, COINS_PER_ROW_MAX);
	double x = (double)WIN_W + 90.0;

	for (int n = 0; n < count; n++)
		for (int i = 0; i < MAX_COINS; i++) {
			if (gCoins[i].active) continue;
			gCoins[i].active = true;
			gCoins[i].collected = false;
			gCoins[i].x = x + n * COIN_SPACING;
			break;
		}
}

static void spawnPickup()
{
	for (int i = 0; i < MAX_PICKUPS; i++) {
		if (gPickups[i].active) continue;

		gPickups[i].active = true;
		gPickups[i].x = (double)WIN_W + 90.0;
		gPickups[i].bob = randUnit() * 6.28;

		// Offer health when hurt, the shield when healthy.
		if (gHP < PLAYER_MAX_HP * 3 / 5)
			gPickups[i].type = (randRange(1, 100) <= 75) ? PU_HEALTH : PU_SHIELD;
		else
			gPickups[i].type = (randRange(1, 100) <= 45) ? PU_HEALTH : PU_SHIELD;
		return;
	}
}

void resetEntities()
{
	for (int i = 0; i < MAX_ROAD_OBSTACLES; i++) gObstacles[i].active = false;
	for (int i = 0; i < MAX_COINS; i++)          gCoins[i].active = false;
	for (int i = 0; i < MAX_PICKUPS; i++)        gPickups[i].active = false;

	gObstacleTimer = 2.6;
	gCoinTimer     = 1.6;
	gPickupTimer   = randBetween(MIN_PICKUP_INTERVAL, MAX_PICKUP_INTERVAL);
}

// ---------------------------------------------------------------------------
//  Update
// ---------------------------------------------------------------------------
void updateObstacles(double dt, bool allowSpawning)
{
	for (int i = 0; i < MAX_ROAD_OBSTACLES; i++) {
		Obstacle &o = gObstacles[i];
		if (!o.active) continue;

		o.x += (o.mode == MOVE_COMING ? -o.speed : o.speed) * dt;

		// The warning: once, on first entering range, then never again.
		if (!o.warningSoundPlayed && obstacleDistanceToCharacter(o) <= WARNING_DISTANCE) {
			playWarningSound(o.type);
			o.warningSoundPlayed = true;
		}

		Rect r = obstacleDrawRect(o);
		if (o.x + r.w < -80.0 || o.x > WIN_W + 140.0)
			o.active = false;
	}

	if (!allowSpawning) return;

	// One at a time for most of the run; up to RUSH_MAX_ACTIVE in the last
	// minute. Either way the separation rule has to be satisfied first.
	int limit = inFinalRush() ? RUSH_MAX_ACTIVE : 1;
	if (activeObstacleCount() >= limit) return;

	gObstacleTimer -= dt;
	if (gObstacleTimer <= 0.0) {
		if (roomForAnotherObstacle()) {
			spawnObstacle();
			scheduleNextObstacle();
		} else {
			gObstacleTimer = 0.15;   // try again shortly
		}
	}
}

void updateCoins(double dt, double scrollSpeed, bool allowSpawning)
{
	if (allowSpawning) {
		gCoinTimer -= dt;
		// Held back while an obstacle is about, so a coin is never sitting
		// inside one and the player is never forced to choose between
		// collecting and surviving.
		if (gCoinTimer <= 0.0 && !obstacleIsNear()) {
			spawnCoinRow();
			gCoinTimer = randBetween(MIN_COIN_INTERVAL, MAX_COIN_INTERVAL);
		}
	}

	for (int i = 0; i < MAX_COINS; i++) {
		Coin &c = gCoins[i];
		if (!c.active) continue;

		c.x -= scrollSpeed * dt;
		if (c.x < -COIN_SIZE - 60.0) c.active = false;   // passed by, uncollected
	}
}

void updatePowerUps(double dt, double scrollSpeed, bool allowSpawning)
{
	if (allowSpawning) {
		gPickupTimer -= dt;
		if (gPickupTimer <= 0.0 && !obstacleIsNear()) {
			spawnPickup();
			gPickupTimer = randBetween(MIN_PICKUP_INTERVAL, MAX_PICKUP_INTERVAL);
		}
	}

	for (int i = 0; i < MAX_PICKUPS; i++) {
		Pickup &p = gPickups[i];
		if (!p.active) continue;

		p.x -= scrollSpeed * dt;
		p.bob += dt * 3.4;

		Rect r = pickupDrawRect(p);
		if (p.x < -r.w - 80.0) p.active = false;
	}
}

// ---------------------------------------------------------------------------
//  Draw
// ---------------------------------------------------------------------------
void drawBirds()
{
	for (int i = 0; i < MAX_ROAD_OBSTACLES; i++) {
		Obstacle &o = gObstacles[i];
		if (!o.active || o.type != OB_BIRD) continue;
		Rect r = obstacleDrawRect(o);
		dImage(r.x, r.y, r.w, r.h, SPR_BIRDS.tex);
	}
}

void drawObstacles()
{
	for (int i = 0; i < MAX_ROAD_OBSTACLES; i++) {
		Obstacle &o = gObstacles[i];
		if (!o.active || o.type == OB_BIRD) continue;

		Rect r = obstacleDrawRect(o);
		Sprite s = obstacleSprite(o.type, o.mode);
		dImageFlipped(r.x, r.y, r.w, r.h, s.tex, obstacleNeedsFlip(o));
	}
}

void drawCoins()
{
	for (int i = 0; i < MAX_COINS; i++) {
		Coin &c = gCoins[i];
		if (!c.active) continue;
		Rect r = coinDrawRect(c);
		dImage(r.x, r.y, r.w, r.h, SPR_COIN.tex);
	}
}

void drawPowerUps()
{
	for (int i = 0; i < MAX_PICKUPS; i++) {
		Pickup &p = gPickups[i];
		if (!p.active) continue;

		Rect r = pickupDrawRect(p);
		unsigned int tex = (p.type == PU_HEALTH) ? SPR_PU_HEALTH.tex : SPR_PU_SHIELD.tex;
		dImageAlpha(r.x, r.y, r.w, r.h, tex, 0.96);
	}
}

#endif // ENTITIES_HPP

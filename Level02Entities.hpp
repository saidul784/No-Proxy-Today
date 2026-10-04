//
//  Level02Entities.hpp  --  everything on the river that has a position.
//
//  The boat (which carries the boy), the Dragon Power ball, the four
//  crocodiles, the chidori the boy throws, and the coins on the water.
//
//  Each kind is a plain struct with its own update and its own draw, and the
//  file is divided by kind:
//
//      the boat        -- pose, oar animation, bob, where his hand is
//      dragon power    -- the story projectile
//      crocodiles      -- the enemies
//      chidori         -- the player's projectile
//      coins           -- score pickups
//
//  Nothing in here decides what stage the level is in and nothing in here calls
//  setState(): the stage machine lives in Level02.hpp and drives these. Nothing
//  in here draws from inside an update either.
//
#ifndef LEVEL02ENTITIES_HPP
#define LEVEL02ENTITIES_HPP


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
#include "Entities.hpp"
#include "Level02State.hpp"
#endif
#include <math.h>

// ===========================================================================
//  THE BOAT
//
//  boatPic01/02, boatstand and throwpic all have the boy drawn INSIDE them, so
//  the boat rect IS the character rect -- there is nothing to composite.
//
//  All four are anchored by their bottom edge and share BOAT_W, so the hull
//  keeps its place on the water while he sits, stands and throws.
// ===========================================================================
enum BoatPose {
	BOAT_POSE_ROW = 0,     // boatPic01 / boatPic02, alternating
	BOAT_POSE_STAND,       // boatstand.png
	BOAT_POSE_THROW        // throwpic.png, hand raised
};

struct Boat {
	BoatPose pose;
	int      frame;         // 0 or 1, which oar stroke
	double   frameTimer;
	double   bobPhase;
};

static Boat gBoat;

void l02ResetBoat()
{
	gBoat.pose = BOAT_POSE_ROW;
	gBoat.frame = 0;
	gBoat.frameTimer = 0.0;
	gBoat.bobPhase = 0.0;
}

static Sprite l02BoatSprite()
{
	if (gBoat.pose == BOAT_POSE_THROW) return SPR_L02_BOAT_THROW;
	if (gBoat.pose == BOAT_POSE_STAND) return SPR_L02_BOAT_STAND;
	return (gBoat.frame == 0) ? SPR_L02_BOAT_1 : SPR_L02_BOAT_2;
}

// The rect the boat is drawn into. Height comes from the sprite's own aspect,
// so swapping the artwork cannot squash it.
Rect l02BoatRect()
{
	Sprite s = l02BoatSprite();
	double h = BOAT_W / spriteAspect(s);
	double y = BOAT_Y + sin(gBoat.bobPhase) * BOAT_BOB;
	return makeRect(BOAT_X, y, BOAT_W, h);
}

// A box around the hull and the boy, not the whole canvas: the oar sticks a
// long way out to the left and being bitten by the tip of an oar is nonsense.
Rect l02BoatHitBox()
{
	Rect r = l02BoatRect();
	return makeRect(r.x + r.w * 0.30,
	                r.y + r.h * 0.10,
	                r.w * 0.58,
	                r.h * 0.78);
}

// Where the boy's raised hand is, as a fraction of the boat rect, so the bolt
// leaves the hand at any size the boat is drawn.
void l02HandPoint(double *hx, double *hy)
{
	Rect r = l02BoatRect();
	*hx = r.x + r.w * HAND_FX;
	*hy = r.y + r.h * HAND_FY;
}

// rowing decides whether the oar animation turns over; the pose itself is set
// by the stage machine.
void l02UpdateBoat(double dt, bool rowing)
{
	gBoat.bobPhase += dt * BOAT_BOB_SPEED;
	if (gBoat.bobPhase > 6.2831853) gBoat.bobPhase -= 6.2831853;

	if (!rowing) return;

	gBoat.frameTimer += dt;
	while (gBoat.frameTimer >= BOAT_ROW_FRAME_TIME) {
		gBoat.frameTimer -= BOAT_ROW_FRAME_TIME;
		gBoat.frame = 1 - gBoat.frame;
	}
}

void l02DrawBoat()
{
	Rect r = l02BoatRect();
	Sprite s = l02BoatSprite();

	if (gL02HitFlash > 0.0)
		dImageEx(r.x, r.y, r.w, r.h, s.tex, false, 1.0, 0.42, 0.42, 1.0);
	else
		dImage(r.x, r.y, r.w, r.h, s.tex);
}

// ===========================================================================
//  DRAGON POWER
//
//  A real projectile: spawned off the right edge, moving left, and the stage
//  only advances when its box actually reaches the boat. It is a story beat, so
//  it takes no health -- it stops the boat, nothing more.
// ===========================================================================
struct DragonBall {
	bool   active;
	bool   hasStruck;
	double x, y;
	double vx;
	double pulse;
};

static DragonBall gDragon;

void l02ResetDragon()
{
	gDragon.active = false;
	gDragon.hasStruck = false;
	gDragon.x = 0.0;
	gDragon.y = DRAGON_Y;
	gDragon.vx = 0.0;
	gDragon.pulse = 0.0;
}

void l02SpawnDragon()
{
	gDragon.active = true;
	gDragon.hasStruck = false;
	gDragon.x = WIN_W + 120.0;
	gDragon.y = DRAGON_Y;
	gDragon.vx = -DRAGON_SPEED;
	gDragon.pulse = 0.0;
}

Rect l02DragonRect()
{
	return makeRect(gDragon.x, gDragon.y, DRAGON_SIZE, DRAGON_SIZE);
}

Rect l02DragonHitBox()
{
	return shrinkRect(l02DragonRect(), 0.74, 0.74);
}

void l02UpdateDragon(double dt)
{
	if (!gDragon.active) return;

	gDragon.x += gDragon.vx * dt;
	gDragon.pulse += dt * DRAGON_PULSE;

	// If it somehow misses, it must not sail on for ever.
	if (gDragon.x + DRAGON_SIZE < -200.0)
		gDragon.active = false;
}

void l02DrawDragon()
{
	if (!gDragon.active) return;

	Rect r = l02DragonRect();

	// A soft halo, breathing, so it reads as power rather than as a sticker.
	double glow = 0.28 + 0.12 * sin(gDragon.pulse);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1.0f, 0.55f, 0.15f, (float)glow);
	iFilledCircle(r.x + r.w * 0.5, r.y + r.h * 0.5, r.w * 0.72);
	glDisable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	dImage(r.x, r.y, r.w, r.h, SPR_L02_DRAGON.tex);
}

// ===========================================================================
//  CROCODILES
//
//  Four independent objects, brought on in two groups of two. Each carries its
//  own position, size, speed, health, hit count and voice, exactly as the
//  scenario asks, and each takes exactly CROCODILE_HITS_TO_KILL chidori.
//
//  crocodile_image.jpg already faces LEFT, which is the way they swim, so it is
//  drawn unflipped.
// ===========================================================================
struct Crocodile {
	bool   active;        // on the water and being updated
	bool   dead;          // killed; sinking out of the scene
	bool   arrived;       // has reached the boat and is biting
	double x, y;
	double w, h;
	double speed;
	int    hp;
	int    maxHp;
	int    hitCount;      // chidori that have landed
	double biteCooldown;  // seconds until it may bite again
	double hitFlash;
	double deathTimer;
	double bobPhase;
	int    soundSlot;     // which MCI voice is this one's
};

static Crocodile gCrocs[MAX_CROCODILES];

void l02ResetCrocodiles()
{
	for (int i = 0; i < MAX_CROCODILES; i++) {
		Crocodile &c = gCrocs[i];
		c.active = false;
		c.dead = false;
		c.arrived = false;
		c.x = 0.0; c.y = 0.0;
		c.w = CROC_W;
		c.h = CROC_W / spriteAspect(SPR_L02_CROCODILE);
		c.speed = 0.0;
		c.hp = CROCODILE_HITS_TO_KILL;
		c.maxHp = CROCODILE_HITS_TO_KILL;
		c.hitCount = 0;
		c.biteCooldown = 0.0;
		c.hitFlash = 0.0;
		c.deathTimer = 0.0;
		c.bobPhase = 0.0;
		c.soundSlot = i;
	}
}

Rect l02CrocodileRect(const Crocodile &c)
{
	return makeRect(c.x, c.y + sin(c.bobPhase) * 5.0, c.w, c.h);
}

// The snout and the body, not the tail tip and not the empty canvas above it.
Rect l02CrocodileHitBox(const Crocodile &c)
{
	Rect r = l02CrocodileRect(c);
	return makeRect(r.x + r.w * 0.04,
	                r.y + r.h * 0.16,
	                r.w * 0.76,
	                r.h * 0.66);
}

// group is 0 for the first pair and 1 for the second. Both crocodiles of a
// group enter from off the right edge, one behind the other and at different
// depths, so neither is hidden behind the other.
void l02SpawnCrocodileGroup(int group)
{
	int first = group * CROC_GROUP_SIZE;

	for (int k = 0; k < CROC_GROUP_SIZE; k++) {
		int i = first + k;
		if (i >= MAX_CROCODILES) break;

		Crocodile &c = gCrocs[i];

		c.active = true;
		c.dead = false;
		c.arrived = false;
		c.w = CROC_W;
		c.h = CROC_W / spriteAspect(SPR_L02_CROCODILE);
		c.x = WIN_W + 60.0 + k * CROC_SPAWN_GAP;
		c.y = (k == 0) ? CROC_Y_LOW : CROC_Y_HIGH;
		c.speed = randBetween(CROC_SPEED_MIN, CROC_SPEED_MAX);
		c.hp = CROCODILE_HITS_TO_KILL;
		c.maxHp = CROCODILE_HITS_TO_KILL;
		c.hitCount = 0;
		c.biteCooldown = 0.0;
		c.hitFlash = 0.0;
		c.deathTimer = 0.0;
		c.bobPhase = randBetween(0.0, 6.28);
		c.soundSlot = i;

		// Its voice starts the moment it becomes active, and only stops when it
		// dies -- one loop per crocodile, so two can be heard at once.
		level02CrocSoundStart(c.soundSlot);

		printf("[level02] crocodile %d in the water  (hp %d)\n", i + 1, c.hp);
	}
}

// One chidori has landed on this crocodile. Returns true if that killed it.
bool l02DamageCrocodile(Crocodile &c)
{
	if (!c.active || c.dead) return false;

	c.hitCount++;
	c.hp--;
	c.hitFlash = CROC_HIT_FLASH;
	c.x += CROC_KNOCKBACK;          // shoved back down the river
	c.arrived = false;

	// One landed bolt, one thud. The caller deactivates the bolt before calling
	// this, so it can only run once per projectile. The crocodile's own bite
	// already had a sound; the hit going the OTHER way did not.
	playCollisionSound();

	if (c.hp <= 0) {
		c.hp = 0;
		c.dead = true;
		c.deathTimer = CROC_DEATH_TIME;
		level02CrocSoundStop(c.soundSlot);   // its growl dies with it
		gL02CrocsKilled++;
		printf("[level02] crocodile down  (%d chidori)  total killed %d\n",
		       c.hitCount, gL02CrocsKilled);
		return true;
	}

	printf("[level02] crocodile hit  %d / %d\n", c.hitCount, c.maxHp);
	return false;
}

// Swims in, stops at the boat, then bites on a cooldown so the bar drains one
// bite at a time instead of every frame it is touching.
void l02UpdateCrocodiles(double dt)
{
	Rect body = l02BoatHitBox();

	for (int i = 0; i < MAX_CROCODILES; i++) {
		Crocodile &c = gCrocs[i];
		if (!c.active) continue;

		c.bobPhase += dt * 2.4;
		if (c.hitFlash > 0.0) c.hitFlash -= dt;

		// --- dying: sinks for a moment, then it is gone ---
		if (c.dead) {
			c.deathTimer -= dt;
			c.y -= 40.0 * dt;
			c.x += 30.0 * dt;
			if (c.deathTimer <= 0.0) c.active = false;
			continue;
		}

		if (c.biteCooldown > 0.0) c.biteCooldown -= dt;

		// --- closing in ---
		if (!c.arrived)
			c.x -= c.speed * dt;

		// --- at the boat ---
		if (checkCollision(body, l02CrocodileHitBox(c))) {
			c.arrived = true;                 // stop, do not swim through it

			if (c.biteCooldown <= 0.0) {
				c.biteCooldown = CROCODILE_DAMAGE_COOLDOWN;
				level02CrocodileBite();        // one thud, one 20 off the bar
			}
		} else {
			c.arrived = false;                 // knocked clear, so it closes again
		}

		// It never leaves on its own: the only way past a crocodile is to kill
		// it. This is just a guard against one being shoved off the map.
		if (c.x > WIN_W + 400.0) c.x = WIN_W + 400.0;
	}
}

// True once every crocodile of the group has been killed AND has finished
// sinking, so the next group never lands on top of the last one's corpse.
bool l02CrocodileGroupCleared(int group)
{
	int first = group * CROC_GROUP_SIZE;

	for (int k = 0; k < CROC_GROUP_SIZE; k++) {
		int i = first + k;
		if (i >= MAX_CROCODILES) break;
		if (gCrocs[i].active) return false;
		if (!gCrocs[i].dead)  return false;    // never spawned yet
	}
	return true;
}

static void l02DrawCrocodileHealth(const Crocodile &c)
{
	Rect r = l02CrocodileRect(c);

	const double pipW = 16.0, pipH = 7.0, gap = 5.0;
	double totalW = c.maxHp * pipW + (c.maxHp - 1) * gap;
	double x = r.x + r.w * 0.42 - totalW * 0.5;
	double y = r.y + r.h + 8.0;

	for (int i = 0; i < c.maxHp; i++) {
		Color pip = (i < c.hp) ? C_ACCENT : C_PANEL;
		dFillRectA(x + i * (pipW + gap), y, pipW, pipH, pip, 0.92);
	}
}

void l02DrawCrocodiles()
{
	for (int i = 0; i < MAX_CROCODILES; i++) {
		const Crocodile &c = gCrocs[i];
		if (!c.active) continue;

		Rect r = l02CrocodileRect(c);

		if (c.dead) {
			double fade = c.deathTimer / CROC_DEATH_TIME;
			if (fade < 0.0) fade = 0.0;
			dImageEx(r.x, r.y, r.w, r.h, SPR_L02_CROCODILE.tex, false,
			         0.55, 0.55, 0.6, fade);
			continue;
		}

		dImage(r.x, r.y, r.w, r.h, SPR_L02_CROCODILE.tex);

		// A pale blue wash over the top of it for the moment a bolt lands, so
		// a hit that did not kill still reads as a hit.
		if (c.hitFlash > 0.0)
			dImageEx(r.x, r.y, r.w, r.h, SPR_L02_CROCODILE.tex, false,
			         0.60, 0.85, 1.0, c.hitFlash / CROC_HIT_FLASH);

		l02DrawCrocodileHealth(c);
	}
}

// ===========================================================================
//  CHIDORI
//
//  The player's projectile. One bolt is one hit: it deactivates the instant it
//  connects, so a single throw can never kill two crocodiles or count twice.
//
//  It is aimed: on release it takes the direction to the nearest live
//  crocodile, so it travels at an angle rather than only straight ahead.
// ===========================================================================
struct Chidori {
	bool   active;
	double x, y;
	double w, h;
	double vx, vy;
	double life;

	// Which artwork this bolt wears. The river half always fires the blue
	// chidori; the pirate ship reuses this same array and the same update and
	// the same hit box for the angry birds and the red chidori, and this is the
	// only thing that differs between them.
	unsigned int tex;
};

static Chidori gChidori[MAX_CHIDORI];

void l02ResetChidori()
{
	for (int i = 0; i < MAX_CHIDORI; i++)
		gChidori[i].active = false;
}

// The nearest crocodile that is alive and on screen, or -1.
static int l02NearestCrocodile(double fromX, double fromY)
{
	int best = -1;
	double bestD = 0.0;

	for (int i = 0; i < MAX_CROCODILES; i++) {
		const Crocodile &c = gCrocs[i];
		if (!c.active || c.dead) continue;

		Rect r = l02CrocodileRect(c);
		double dx = (r.x + r.w * 0.35) - fromX;
		double dy = (r.y + r.h * 0.5) - fromY;
		double d = dx * dx + dy * dy;

		if (best < 0 || d < bestD) { best = i; bestD = d; }
	}
	return best;
}

// Launches one bolt from the boy's hand. Called by the throw animation the
// moment the wind-up finishes -- never straight from the key press.
void l02LaunchChidori()
{
	int slot = -1;
	for (int i = 0; i < MAX_CHIDORI; i++)
		if (!gChidori[i].active) { slot = i; break; }
	if (slot < 0) return;

	double hx, hy;
	l02HandPoint(&hx, &hy);

	Chidori &b = gChidori[slot];
	b.w = CHIDORI_W;
	b.h = CHIDORI_W / spriteAspect(SPR_L02_CHIDORI);
	b.x = hx;
	b.y = hy - b.h * 0.5;
	b.life = CHIDORI_LIFE;
	b.tex = SPR_L02_CHIDORI.tex;
	b.active = true;

	// Aim at whatever is closest; with nothing to aim at, straight ahead.
	int target = l02NearestCrocodile(hx, hy);
	if (target < 0) {
		b.vx = CHIDORI_SPEED;
		b.vy = 0.0;
	} else {
		Rect r = l02CrocodileRect(gCrocs[target]);
		double dx = (r.x + r.w * 0.30) - hx;
		double dy = (r.y + r.h * 0.55) - hy;
		double len = sqrt(dx * dx + dy * dy);
		if (len < 1.0) len = 1.0;
		b.vx = dx / len * CHIDORI_SPEED;
		b.vy = dy / len * CHIDORI_SPEED;
	}

	gL02ChidoriThrown++;
}

Rect l02ChidoriRect(const Chidori &b)
{
	return makeRect(b.x, b.y, b.w, b.h);
}

Rect l02ChidoriHitBox(const Chidori &b)
{
	return shrinkRect(l02ChidoriRect(b), 0.60, 0.52);
}

void l02UpdateChidori(double dt)
{
	for (int i = 0; i < MAX_CHIDORI; i++) {
		Chidori &b = gChidori[i];
		if (!b.active) continue;

		b.x += b.vx * dt;
		b.y += b.vy * dt;
		b.life -= dt;

		if (b.life <= 0.0 || b.x > WIN_W + 200.0 || b.x + b.w < -200.0 ||
		    b.y > WIN_H + 200.0 || b.y + b.h < -200.0)
			b.active = false;
	}
}

void l02DrawChidori()
{
	for (int i = 0; i < MAX_CHIDORI; i++) {
		const Chidori &b = gChidori[i];
		if (!b.active) continue;

		Rect r = l02ChidoriRect(b);

		// A tight core of light around the bolt. It was a wide, pale disc at
		// first, which over dark water read as a bubble sitting on the boat
		// rather than as lightning, so it is kept small and faint.
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(0.45f, 0.75f, 1.0f, 0.14f);
		iFilledCircle(r.x + r.w * 0.5, r.y + r.h * 0.5, r.w * 0.28);
		glDisable(GL_BLEND);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

		dImage(r.x, r.y, r.w, r.h, b.tex);
	}
}

// ===========================================================================
//  COINS
//
//  Score only. They never touch health and never touch the shield, and they go
//  through the same collectCoin() Level 01 uses, so one score architecture
//  serves both levels.
// ===========================================================================
struct RiverCoin {
	bool   active;
	double x, y;
	double size;
	double bobPhase;
};

static RiverCoin gL02Coins[MAX_L02_COINS];
static double gL02CoinTimer = 0.0;

void l02ResetCoins()
{
	for (int i = 0; i < MAX_L02_COINS; i++)
		gL02Coins[i].active = false;
	gL02CoinTimer = randBetween(L02_COIN_MIN_INTERVAL, L02_COIN_MAX_INTERVAL);
}

static void l02SpawnCoin()
{
	for (int i = 0; i < MAX_L02_COINS; i++) {
		RiverCoin &c = gL02Coins[i];
		if (c.active) continue;

		c.active = true;
		c.size = L02_COIN_SIZE;
		c.x = WIN_W + 40.0;
		c.y = randBetween(L02_COIN_Y_LOW, L02_COIN_Y_HIGH);
		c.bobPhase = randBetween(0.0, 6.28);
		return;
	}
}

Rect l02CoinRect(const RiverCoin &c)
{
	return makeRect(c.x, c.y + sin(c.bobPhase) * 6.0, c.size, c.size);
}

Rect l02CoinHitBox(const RiverCoin &c)
{
	return shrinkRect(l02CoinRect(c), 0.80, 0.80);
}

// spawning is false once the boat has stopped: nothing new drifts in, but what
// is already on the water keeps moving and can still be picked up.
void l02UpdateCoins(double dt, bool spawning)
{
	for (int i = 0; i < MAX_L02_COINS; i++) {
		RiverCoin &c = gL02Coins[i];
		if (!c.active) continue;

		c.x -= L02_COIN_DRIFT * dt;
		c.bobPhase += dt * 3.0;

		if (c.x + c.size < -60.0) c.active = false;
	}

	if (!spawning) return;

	gL02CoinTimer -= dt;
	if (gL02CoinTimer <= 0.0) {
		gL02CoinTimer = randBetween(L02_COIN_MIN_INTERVAL, L02_COIN_MAX_INTERVAL);
		l02SpawnCoin();
	}
}

void l02DrawCoins()
{
	for (int i = 0; i < MAX_L02_COINS; i++) {
		const RiverCoin &c = gL02Coins[i];
		if (!c.active) continue;

		Rect r = l02CoinRect(c);
		dImage(r.x, r.y, r.w, r.h, SPR_L02_COIN.tex);
	}
}

// ---------------------------------------------------------------------------
//  Everything at once, for a fresh run
// ---------------------------------------------------------------------------
void l02ResetEntities()
{
	l02ResetBoat();
	l02ResetDragon();
	l02ResetCrocodiles();
	l02ResetChidori();
	l02ResetCoins();
}

#endif // LEVEL02ENTITIES_HPP

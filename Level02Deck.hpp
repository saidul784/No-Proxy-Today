//
//  Level02Deck.hpp  --  the second half of Level 02: aboard the pirate ship.
//
//  Everything from the moment the fourth crocodile dies:
//
//      the pirate ship   that rams the boat and ends the river section
//      the walker        the character on the deck: walks, jumps, throws
//      the chidori icon  what buys him the right to attack at all
//      the dakat         six of them, three chidori each
//      the powerup       granted only once all six are dead
//      the dakatleader   the boss: six powerful chidori, and he throws back
//
//  It follows the same shape as Level02Entities.hpp -- plain structs, an update
//  per kind, a draw per kind -- and it borrows rather than repeats:
//
//      TEX_CHARACTER[]        Level 01's walk cycle, not a second one
//      JUMP physics           the same velocity-against-gravity integration
//                             Player.hpp uses, with its own constants
//      gChidori[]             the same projectile pool the river half throws
//                             from, the same update, the same hit box
//      checkCollision()       every hit test on this deck
//      level02Damage()        the one place Level 02 health ever changes
//      playCollisionSound()   the same thud, on every damage event
//
//  The stage machine in Level02.hpp drives this; nothing here calls setState().
//
#ifndef LEVEL02DECK_HPP
#define LEVEL02DECK_HPP


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
#include "Level02State.hpp"
#include "Level02Entities.hpp"
#endif
#include <math.h>

// ===========================================================================
//  THE PIRATE SHIP
//
//  It closes on the boat from ahead and the crash is what ends the river. Like
//  the Dragon Power ball it is a real object with a real box, not a timer: the
//  scene changes because two rectangles touched.
//
//  pirates ship.png already faces LEFT -- bowsprit first -- which is the way it
//  travels, so it is drawn unflipped.
// ===========================================================================
struct PirateShip {
	bool   active;
	bool   hasStruck;
	double x, y;
	double w, h;
	double bobPhase;
};

static PirateShip gShip;

void l02bResetShip()
{
	gShip.active = false;
	gShip.hasStruck = false;
	gShip.x = SHIP_START_X;
	gShip.y = SHIP_Y;
	gShip.h = SHIP_H;
	gShip.w = SHIP_H * spriteAspect(SPR_L02B_SHIP);
	gShip.bobPhase = 0.0;
}

void l02bSpawnShip()
{
	l02bResetShip();
	gShip.active = true;
	printf("[level02] a pirate ship is bearing down on the boat\n");
}

Rect l02bShipRect()
{
	return makeRect(gShip.x, gShip.y + sin(gShip.bobPhase) * 6.0, gShip.w, gShip.h);
}

// The hull only. The masts and rigging reach most of the way up the sprite and
// a boat is not rammed by a mast.
Rect l02bShipHitBox()
{
	Rect r = l02bShipRect();
	return makeRect(r.x + r.w * 0.06, r.y, r.w * 0.80, r.h * 0.42);
}

void l02bUpdateShip(double dt)
{
	if (!gShip.active) return;

	gShip.bobPhase += dt * 1.7;
	if (!gShip.hasStruck)
		gShip.x -= SHIP_SPEED * dt;
}

void l02bDrawShip()
{
	if (!gShip.active) return;

	Rect r = l02bShipRect();
	dImage(r.x, r.y, r.w, r.h, SPR_L02B_SHIP.tex);
}

// ===========================================================================
//  THE WALKER
//
//  The character on the deck. He runs on the SAME six frames as Level 01 and
//  the same frame timer -- there is one walk cycle in this project, not two.
//
//  Unlike Level 01 he is NOT locked to one spot: he is driven left and right
//  between two walls and he can jump. Both are needed -- the pickups have to be
//  walked into, and the boss throws something that has to be dodged.
//
//  The jump is the same physics Player.hpp uses: an upward velocity fought by
//  gravity, integrated every frame, with its own constants for this deck. It
//  cannot be started in mid-air, so there is no double jump, and the y it
//  produces is what collision actually tests against -- which is what makes
//  jumping over a dakat genuinely work rather than merely look like it does.
// ===========================================================================
enum WalkPose {
	WALK_POSE_WALK = 0,     // character_1..6
	WALK_POSE_THROW         // throw.png, arm out
};

struct Walker {
	double   x, y;          // y is the DRAWN bottom edge: ground + jump
	double   groundY;       // where his feet are when he is standing
	double   velocityY;
	bool     isJumping;
	WalkPose pose;
	int      frame;
	double   frameTimer;
	bool     facingLeft;    // which way he last moved
	bool     moving;
};

static Walker gWalker;

// The throw animation, the same three steps the boat half uses: press, hand
// out, the bolt leaves the hand, back to walking.
static ThrowPhase gWalkThrow = THROW_IDLE;
static double gWalkThrowTimer = 0.0;
static double gWalkThrowCooldown = 0.0;

void l02bResetWalker()
{
	gWalker.x = L02B_START_X;
	gWalker.groundY = L02B_GROUND_Y;
	gWalker.y = gWalker.groundY;
	gWalker.velocityY = 0.0;
	gWalker.isJumping = false;
	gWalker.pose = WALK_POSE_WALK;
	gWalker.frame = 0;
	gWalker.frameTimer = 0.0;
	gWalker.facingLeft = false;
	gWalker.moving = false;

	gWalkThrow = THROW_IDLE;
	gWalkThrowTimer = 0.0;
	gWalkThrowCooldown = 0.0;
}

bool l02bWalkerAirborne() { return gWalker.isJumping; }

Rect l02bWalkerRect()
{
	if (gWalker.pose == WALK_POSE_THROW) {
		// throw.png was cut down to its content while the six walk frames keep
		// their whole canvas, so the two are not framed alike. Matching the
		// visible height and anchoring both at the feet keeps him the same size
		// through the swap instead of growing every time he throws.
		double h = PLAYER_BASE_H * L02B_THROW_H_SCALE;
		double w = h * spriteAspect(SPR_L02B_THROW);
		return makeRect(gWalker.x, gWalker.y, w, h);
	}

	double h = PLAYER_BASE_H;
	double w = h * PLAYER_ASPECT;
	return makeRect(gWalker.x, gWalker.y, w, h);
}

// The body, not the swinging arms and not the empty canvas around them -- the
// same proportions Level 01 uses on the same artwork. It tracks his real y, so
// a jump lifts it clear of whatever is passing underneath.
Rect l02bWalkerHitBox()
{
	Rect r = l02bWalkerRect();
	return makeRect(r.x + r.w * 0.24,
	                r.y + r.h * 0.04,
	                r.w * 0.50,
	                r.h * 0.88);
}

// Where the bolt leaves his hand, as a fraction of the drawn rect, so it starts
// at the end of his arm whatever size he is drawn at.
void l02bWalkerHand(double *hx, double *hy)
{
	Rect r = l02bWalkerRect();
	*hx = r.x + r.w * L02B_HAND_FX;
	*hy = r.y + r.h * L02B_HAND_FY;
}

// controlsLive is false during the scripted beats, so he stands still while a
// banner is up instead of being steerable through it.
void l02bUpdateWalker(double dt, bool controlsLive)
{
	gWalker.moving = false;

	// --- left and right ---
	if (controlsLive) {
		bool left  = isKeyPressed('a') != 0 || isKeyPressed('A') != 0 ||
		             isSpecialKeyPressed(GLUT_KEY_LEFT) != 0;
		bool right = isKeyPressed('d') != 0 || isKeyPressed('D') != 0 ||
		             isSpecialKeyPressed(GLUT_KEY_RIGHT) != 0;

		if (left && !right) {
			gWalker.x -= L02B_WALK_SPEED * dt;
			gWalker.facingLeft = true;
			gWalker.moving = true;
		} else if (right && !left) {
			gWalker.x += L02B_WALK_SPEED * dt;
			gWalker.facingLeft = false;
			gWalker.moving = true;
		}

		if (gWalker.x < L02B_MIN_X) gWalker.x = L02B_MIN_X;
		if (gWalker.x > L02B_MAX_X) gWalker.x = L02B_MAX_X;
	}

	// --- jump ---
	// Cannot be started while already in the air, so there is no double jump.
	if (controlsLive && !gWalker.isJumping) {
		if (keyJustPressed('w') || keyJustPressed('W') ||
		    specialKeyJustPressed(GLUT_KEY_UP)) {
			gWalker.velocityY = L02B_JUMP_VELOCITY;
			gWalker.isJumping = true;
		}
	}

	if (gWalker.isJumping) {
		gWalker.velocityY -= L02B_JUMP_GRAVITY * dt;
		gWalker.y += gWalker.velocityY * dt;

		if (gWalker.y <= gWalker.groundY) {        // landed
			gWalker.y = gWalker.groundY;
			gWalker.velocityY = 0.0;
			gWalker.isJumping = false;
		}
	}

	// --- the walk cycle, only while he is actually going somewhere ---
	if (gWalker.moving) {
		gWalker.frameTimer += dt;
		while (gWalker.frameTimer >= PLAYER_FRAME_TIME) {
			gWalker.frameTimer -= PLAYER_FRAME_TIME;
			gWalker.frame = (gWalker.frame + 1) % CHARACTER_FRAME_COUNT;
		}
	}
}

void l02bDrawWalker()
{
	Rect r = l02bWalkerRect();

	unsigned int tex = (gWalker.pose == WALK_POSE_THROW)
	                 ? SPR_L02B_THROW.tex
	                 : TEX_CHARACTER[gWalker.frame];

	// He throws to the right, so the throw pose is never mirrored -- only the
	// walk cycle turns round.
	bool flip = gWalker.facingLeft && gWalker.pose == WALK_POSE_WALK;

	if (gL02HitFlash > 0.0)
		dImageEx(r.x, r.y, r.w, r.h, tex, flip, 1.0, 0.40, 0.40, 1.0);
	else
		dImageFlipped(r.x, r.y, r.w, r.h, tex, flip);
}

// ===========================================================================
//  PICKUPS
//
//  Two kinds, and only one is ever on the deck: the chidori icon, which buys
//  him attacks, and the powerup, which grants the powerful chidori. Both are
//  taken by walking into them.
// ===========================================================================
enum PickupKind {
	PICK_NONE = 0,
	PICK_CHIDORI,      // icon.jpg
	PICK_POWERUP       // powerup.png
};

struct DeckPickup {
	bool       active;
	PickupKind kind;
	double     x, y;
	double     w, h;
	double     bobPhase;
	double     wait;        // counts down before it appears
};

static DeckPickup gPick;

void l02bResetPickup()
{
	gPick.active = false;
	gPick.kind = PICK_NONE;
	gPick.x = 0.0; gPick.y = L02B_ICON_Y;
	gPick.w = 0.0; gPick.h = L02B_ICON_H;
	gPick.bobPhase = 0.0;
	gPick.wait = 0.0;
}

static Sprite l02bPickupSprite()
{
	return (gPick.kind == PICK_POWERUP) ? SPR_L02B_POWERUP : SPR_L02B_ICON;
}

void l02bShowPickup(PickupKind kind, double x, double delay)
{
	gPick.kind = kind;
	gPick.h = (kind == PICK_POWERUP) ? L02B_POWERUP_H : L02B_ICON_H;
	gPick.w = gPick.h * spriteAspect(l02bPickupSprite());
	gPick.x = x;
	gPick.y = (kind == PICK_POWERUP) ? L02B_POWERUP_Y : L02B_ICON_Y;
	gPick.bobPhase = 0.0;
	gPick.wait = delay;
	gPick.active = (delay <= 0.0);
}

Rect l02bPickupRect()
{
	return makeRect(gPick.x, gPick.y + sin(gPick.bobPhase) * 7.0, gPick.w, gPick.h);
}

Rect l02bPickupHitBox()
{
	return shrinkRect(l02bPickupRect(), 0.82, 0.82);
}

void l02bUpdatePickup(double dt)
{
	if (gPick.kind == PICK_NONE) return;

	if (!gPick.active) {
		if (gPick.wait > 0.0) {
			gPick.wait -= dt;
			if (gPick.wait <= 0.0) gPick.active = true;
		}
		return;
	}

	gPick.bobPhase += dt * 2.6;
}

// True the moment he walks into whatever is on the deck.
bool l02bTookPickup()
{
	if (!gPick.active) return false;
	return checkCollision(l02bWalkerHitBox(), l02bPickupHitBox());
}

void l02bDrawPickup()
{
	if (!gPick.active) return;

	Rect r = l02bPickupRect();

	// A ring under it so it reads as something to collect rather than scenery.
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.45f, 0.80f, 1.0f, 0.20f);
	iFilledCircle(r.x + r.w * 0.5, r.y + r.h * 0.5, r.w * 0.70);
	glDisable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	dImage(r.x, r.y, r.w, r.h, l02bPickupSprite().tex);
}

// ===========================================================================
//  THE DAKAT
//
//  Six in all, on the deck two at a time, each drawn from fighter_01..03 in
//  rotation so the six are not all the same man.
//
//  Each carries its OWN hit counter, reset when it is spawned, so every one of
//  the six needs three chidori of its own -- killing the first does nothing to
//  shorten the second.
//
//  They do NOT stop at him and they are never removed for walking past: one
//  that gets behind him turns round at the wall and comes back. Jumping over a
//  dakat is a dodge, never a way to skip it, so the count of six cannot be
//  cheated by simply running the deck.
// ===========================================================================
struct Dakat {
	bool   active;
	bool   dead;
	int    art;            // which of the three fighter sprites
	double x, y;           // y is the bottom edge, and it never leaves the floor
	double groundY;        // its depth: the floor line it walks along
	double w, h;
	double speed;
	bool   movingLeft;     // false once it has turned round
	int    hp;
	int    maxHp;
	int    hitCount;       // chidori that have landed on THIS one
	double hurtCooldown;   // seconds until it may cut him again
	double hitFlash;
	double deathTimer;
	double bobPhase;
};

static Dakat gDakat[MAX_DAKAT];
static double gDakatNextTimer = 0.0;

void l02bResetDakat()
{
	for (int i = 0; i < MAX_DAKAT; i++) {
		Dakat &d = gDakat[i];
		d.active = false;
		d.dead = false;
		d.art = 0;
		d.x = 0.0;
		d.groundY = L02B_GROUND_Y;
		d.y = d.groundY;
		d.h = DAKAT_H;
		d.w = DAKAT_H * spriteAspect(SPR_L02B_FIGHTER[0]);
		d.speed = 0.0;
		d.movingLeft = true;
		d.hp = DAKAT_HITS_TO_KILL;
		d.maxHp = DAKAT_HITS_TO_KILL;
		d.hitCount = 0;
		d.hurtCooldown = 0.0;
		d.hitFlash = 0.0;
		d.deathTimer = 0.0;
		d.bobPhase = 0.0;
	}
	gDakatNextTimer = 0.0;
}

Rect l02bDakatRect(const Dakat &d)
{
	return makeRect(d.x, d.y + sin(d.bobPhase) * 3.0, d.w, d.h);
}

// Its body and the ground it stands on -- NOT the whole sprite.
//
// HEIGHT is what decides whether the jump can clear it. At 0.88 it reached
// 187 px up while the character's box only reached 180 at the top of his jump,
// so the jump could not be made at all. At 0.72 the box tops out at 117 px
// above the floor, which the jump clears for 0.63 s -- see L02B_JUMP_VELOCITY.
//
// WIDTH had to move with the new artwork. Measuring how tall each column of
// the cut-out is puts the man himself at 0.45-0.89, 0.49-0.82 and 0.46-0.82 of
// the sprite width for the three fighters: the whole left of the picture is his
// outstretched sword arm. The old 0.20-0.78 box was mostly sword and stopped
// short of his back, so he could be cut by a blade that was nowhere near him
// and walk through his own body. 0.44-0.86 is the body on all three.
Rect l02bDakatHitBox(const Dakat &d)
{
	Rect r = l02bDakatRect(d);
	return makeRect(r.x + r.w * 0.44,
	                r.y + r.h * 0.06,
	                r.w * 0.42,
	                r.h * 0.72);
}

// Brings the next dakat on, if there is a free slot and any of the six are
// still to come. The art cycles 1, 2, 3, 1, 2, 3 across the six.
static bool l02bSpawnNextDakat()
{
	if (gL02DakatSpawned >= DAKAT_TOTAL) return false;

	int slot = -1;
	for (int i = 0; i < MAX_DAKAT; i++)
		if (!gDakat[i].active) { slot = i; break; }
	if (slot < 0) return false;

	Dakat &d = gDakat[slot];
	int index = gL02DakatSpawned;          // 0-based, so 0,1,2,3,4,5

	d.art = index % 3;                     // fighter_01, _02, _03, _01, ...
	d.active = true;
	d.dead = false;
	d.h = DAKAT_H;
	d.w = DAKAT_H * spriteAspect(SPR_L02B_FIGHTER[d.art]);
	d.x = WIN_W + 30.0 + slot * DAKAT_SPAWN_GAP;

	// Alternating sides of the character's own floor line, so one of any pair
	// walks in front of him and the other behind. Its feet are ON that line --
	// the sprite bottom, the hit box and the draw order all come from it.
	d.groundY = ((index % 2) == 0) ? DAKAT_Y_NEAR : DAKAT_Y_FAR;
	d.y = d.groundY;
	d.speed = randBetween(DAKAT_SPEED_MIN, DAKAT_SPEED_MAX);
	d.movingLeft = true;
	d.hp = DAKAT_HITS_TO_KILL;
	d.maxHp = DAKAT_HITS_TO_KILL;
	d.hitCount = 0;                        // a NEW dakat starts at zero
	d.hurtCooldown = 0.0;
	d.hitFlash = 0.0;
	d.deathTimer = 0.0;
	d.bobPhase = randBetween(0.0, 6.28);

	gL02DakatSpawned++;
	printf("[level02] dakat %d of %d boards  (fighter_0%d, needs %d hits)\n",
	       gL02DakatSpawned, DAKAT_TOTAL, d.art + 1, d.hp);
	return true;
}

// One chidori has landed on this dakat. Returns true if that killed it.
bool l02bDamageDakat(Dakat &d)
{
	if (!d.active || d.dead) return false;

	d.hitCount++;
	d.hp--;
	d.hitFlash = DAKAT_HIT_FLASH;
	d.x += DAKAT_KNOCKBACK;                // shoved back down the deck

	// One landed bolt, one thud. The caller has already deactivated the bolt,
	// so this can only run once per projectile however long the sprites sit on
	// top of each other.
	playCollisionSound();

	if (d.hp <= 0) {
		d.hp = 0;
		d.dead = true;
		d.deathTimer = DAKAT_DEATH_TIME;
		gL02DakatKilled++;
		printf("[level02] dakat down  (%d hits)  killed %d / %d\n",
		       d.hitCount, gL02DakatKilled, DAKAT_TOTAL);
		return true;
	}

	printf("[level02] dakat hit  %d / %d\n", d.hitCount, d.maxHp);
	return false;
}

void l02bUpdateDakat(double dt)
{
	Rect body = l02bWalkerHitBox();

	for (int i = 0; i < MAX_DAKAT; i++) {
		Dakat &d = gDakat[i];
		if (!d.active) continue;

		d.bobPhase += dt * 3.0;
		if (d.hitFlash > 0.0) d.hitFlash -= dt;

		// --- dying: sinks away, then it is gone ---
		if (d.dead) {
			d.deathTimer -= dt;
			d.y -= 46.0 * dt;          // sinks out of the scene as it fades
			if (d.deathTimer <= 0.0) d.active = false;
			continue;
		}

		if (d.hurtCooldown > 0.0) d.hurtCooldown -= dt;

		// --- advancing, and turning round ---
		//
		// It never leaves and it is never removed for getting past him: once it
		// is clear behind him it turns and comes back. That is what stops the
		// six being skipped by simply jumping over all of them.
		//
		// It turns as soon as it is a body's width clear of HIM rather than at
		// the edge of the screen. Turning at the wall meant a dakat he had run
		// past spent fifteen seconds walking to the far side and back, and the
		// fight kept emptying out.
		d.x += (d.movingLeft ? -d.speed : d.speed) * dt;

		double turnBack = body.x - DAKAT_TURN_CLEARANCE;

		if (d.movingLeft && (d.x + d.w) < turnBack)
			d.movingLeft = false;
		else if (!d.movingLeft && d.x > body.x + body.w + DAKAT_TURN_CLEARANCE)
			d.movingLeft = true;

		// A hard stop at the screen edges, in case he is backed into a corner.
		if (d.x < DAKAT_TURN_X)     d.movingLeft = false;
		if (d.x > WIN_W + 60.0)     d.movingLeft = true;

		// --- the sword ---
		// Airborne he is above it, so a jump over a dakat costs nothing. That
		// is the whole point of the jump, and it falls out of the boxes rather
		// than being a special case: his box simply is not there any more.
		//
		// The cooldown is what stops one dakat draining the bar frame by frame
		// while the two sprites happen to overlap, and what stops the collision
		// sound retriggering every frame with it.
		if (checkCollision(body, l02bDakatHitBox(d)) && d.hurtCooldown <= 0.0) {
			d.hurtCooldown = DAKAT_DAMAGE_COOLDOWN;
			playCollisionSound();
			level02Damage(DAKAT_COLLISION_DAMAGE);
		}
	}
}

// True when nothing is left on the deck AND all six have been and gone.
bool l02bAllDakatCleared()
{
	if (gL02DakatKilled < DAKAT_TOTAL) return false;

	for (int i = 0; i < MAX_DAKAT; i++)
		if (gDakat[i].active) return false;

	return true;
}

// Keeps the deck stocked: a new one walks on a beat after a slot frees up,
// until all six have been sent.
void l02bFeedDakat(double dt)
{
	if (gL02DakatSpawned >= DAKAT_TOTAL) return;

	bool freeSlot = false;
	for (int i = 0; i < MAX_DAKAT; i++)
		if (!gDakat[i].active) { freeSlot = true; break; }

	if (!freeSlot) { gDakatNextTimer = DAKAT_NEXT_WAIT; return; }

	gDakatNextTimer -= dt;
	if (gDakatNextTimer <= 0.0) {
		l02bSpawnNextDakat();
		gDakatNextTimer = DAKAT_NEXT_WAIT;
	}
}

static void l02bDrawDakatPips(const Dakat &d)
{
	Rect r = l02bDakatRect(d);

	const double pipW = 15.0, pipH = 7.0, gap = 5.0;
	double totalW = d.maxHp * pipW + (d.maxHp - 1) * gap;

	// Over the man, not over the middle of the picture -- the sword arm pushes
	// the sprite's centre well to the left of where he actually stands.
	double x = r.x + r.w * 0.65 - totalW * 0.5;
	double y = r.y + r.h + 8.0;

	for (int i = 0; i < d.maxHp; i++) {
		Color pip = (i < d.hp) ? C_ACCENT : C_PANEL;
		dFillRectA(x + i * (pipW + gap), y, pipW, pipH, pip, 0.92);
	}
}

// One dakat, on its own, so the scene can draw them in depth order rather than
// all together in array order.
void l02bDrawOneDakat(const Dakat &d)
{
	if (!d.active) return;

	Rect r = l02bDakatRect(d);
	unsigned int tex = SPR_L02B_FIGHTER[d.art].tex;

	// The artwork faces left, which is the way it advances; mirrored only when
	// it has turned round and is coming back.
	bool flip = !d.movingLeft;

	if (d.dead) {
		double fade = d.deathTimer / DAKAT_DEATH_TIME;
		if (fade < 0.0) fade = 0.0;
		dImageEx(r.x, r.y, r.w, r.h, tex, flip, 0.7, 0.5, 0.5, fade);
		return;
	}

	dImageFlipped(r.x, r.y, r.w, r.h, tex, flip);

	if (d.hitFlash > 0.0)
		dImageEx(r.x, r.y, r.w, r.h, tex, flip,
		         0.6, 0.85, 1.0, d.hitFlash / DAKAT_HIT_FLASH);

	l02bDrawDakatPips(d);
}

// ===========================================================================
//  THE DAKATLEADER
//
//  One object, six powerful chidori, and he answers with enemy chidori. He is
//  drawn well over twice a dakat's height so the last fight reads as one.
// ===========================================================================
struct DakatLeader {
	bool   active;
	bool   dead;
	double x, y;
	double w, h;
	double bobPhase;
	double entry;          // counts down while he walks into the scene
	double shotTimer;      // until the next enemy chidori
	double hitFlash;
	double deathTimer;
};

static DakatLeader gLeader;

void l02bResetLeader()
{
	gLeader.active = false;
	gLeader.dead = false;
	gLeader.x = LEADER_X;
	gLeader.y = LEADER_Y;      // the same floor line as everyone else
	gLeader.h = LEADER_H;
	gLeader.w = LEADER_H * spriteAspect(SPR_L02B_LEADER);
	gLeader.bobPhase = 0.0;
	gLeader.entry = 0.0;
	gLeader.shotTimer = 0.0;
	gLeader.hitFlash = 0.0;
	gLeader.deathTimer = 0.0;
}

void l02bSummonLeader()
{
	l02bResetLeader();
	gLeader.active = true;
	gLeader.entry = LEADER_ENTRY_SECONDS;
	gLeader.shotTimer = ENEMY_CHI_INTERVAL_MIN;
	gL02LeaderHits = 0;
	printf("[level02] THE DAKATLEADER APPEARS  (0 / %d)\n", LEADER_HITS_TO_KILL);
}

Rect l02bLeaderRect()
{
	// He walks in from off the right rather than appearing all at once.
	double slide = 0.0;
	if (gLeader.entry > 0.0)
		slide = (gLeader.entry / LEADER_ENTRY_SECONDS) * (WIN_W + 60.0 - LEADER_X);

	return makeRect(gLeader.x + slide,
	                gLeader.y + sin(gLeader.bobPhase) * LEADER_BOB,
	                gLeader.w, gLeader.h);
}

Rect l02bLeaderHitBox()
{
	Rect r = l02bLeaderRect();
	return makeRect(r.x + r.w * 0.22,
	                r.y + r.h * 0.10,
	                r.w * 0.56,
	                r.h * 0.82);
}

bool l02bLeaderIsFighting()
{
	return gLeader.active && !gLeader.dead && gLeader.entry <= 0.0;
}

// One powerful chidori has landed. Returns true on the sixth.
bool l02bDamageLeader()
{
	if (!l02bLeaderIsFighting()) return false;

	gL02LeaderHits++;
	gLeader.hitFlash = 0.26;

	playCollisionSound();                  // once per landed powerful chidori

	printf("[level02] DAKATLEADER hit  %d / %d\n", gL02LeaderHits, LEADER_HITS_TO_KILL);

	if (gL02LeaderHits >= LEADER_HITS_TO_KILL) {
		gLeader.dead = true;
		gLeader.deathTimer = LEADER_DEATH_TIME;
		printf("[level02] THE DAKATLEADER IS DEFEATED\n");
		return true;
	}
	return false;
}

// ===========================================================================
//  ENEMY CHIDORI  --  the boss's attack
//
//  Its own pool, because it travels the other way and hurts the other side. It
//  is spent the instant it lands, which is what stops one projectile taking
//  seventy-five health more than once.
//
//  enemy chidori.jpg is a red burst on SOLID BLACK with no alpha, so it is
//  drawn additively -- the black adds nothing and vanishes on its own.
// ===========================================================================
struct EnemyShot {
	bool   active;
	double x, y;
	double w, h;
	double vx, vy;
	double life;
	double spin;
};

static EnemyShot gEnemyShots[MAX_ENEMY_SHOTS];

void l02bResetEnemyShots()
{
	for (int i = 0; i < MAX_ENEMY_SHOTS; i++)
		gEnemyShots[i].active = false;
}

static void l02bFireEnemyChidori()
{
	int slot = -1;
	for (int i = 0; i < MAX_ENEMY_SHOTS; i++)
		if (!gEnemyShots[i].active) { slot = i; break; }
	if (slot < 0) return;

	// Thrown from his BODY, not from the left edge of his picture. The
	// replacement artwork has a wide empty margin down its left side, so
	// anchoring to the sprite rect put the flash a long way out in front of him
	// -- it appeared to come out of bare deck. His hit box is where he actually
	// is, in either version of the art.
	Rect boss = l02bLeaderHitBox();
	Rect body = l02bWalkerHitBox();

	EnemyShot &s = gEnemyShots[slot];
	s.h = ENEMY_CHI_H;
	s.w = ENEMY_CHI_H * spriteAspect(SPR_L02B_ENEMY_CHI);
	s.x = boss.x - s.w * 0.5;
	s.y = boss.y + boss.h * 0.30;
	s.life = 8.0;
	s.spin = 0.0;
	s.active = true;

	// Aimed where he is at the moment it is thrown, NOT tracking him after --
	// that is what makes stepping aside or jumping a real dodge.
	double dx = (body.x + body.w * 0.5) - (s.x + s.w * 0.5);
	double dy = (body.y + body.h * 0.50) - (s.y + s.h * 0.5);
	double len = sqrt(dx * dx + dy * dy);
	if (len < 1.0) len = 1.0;

	s.vx = dx / len * ENEMY_CHI_SPEED;
	s.vy = dy / len * ENEMY_CHI_SPEED;
}

Rect l02bEnemyShotRect(const EnemyShot &s)
{
	return makeRect(s.x, s.y, s.w, s.h);
}

// The burning core, not the whole square of artwork -- most of that image is
// dark space that draws as nothing, and being hurt by it would feel wrong.
Rect l02bEnemyShotHitBox(const EnemyShot &s)
{
	return shrinkRect(l02bEnemyShotRect(s), 0.42, 0.42);
}

void l02bUpdateEnemyShots(double dt)
{
	for (int i = 0; i < MAX_ENEMY_SHOTS; i++) {
		EnemyShot &s = gEnemyShots[i];
		if (!s.active) continue;

		s.x += s.vx * dt;
		s.y += s.vy * dt;
		s.life -= dt;
		s.spin += dt * 3.0;

		// Gone when it leaves the playable area, as well as when it lands.
		if (s.life <= 0.0 ||
		    s.x + s.w < -220.0 || s.x > WIN_W + 220.0 ||
		    s.y + s.h < -220.0 || s.y > WIN_H + 220.0)
			s.active = false;
	}
}

void l02bDrawEnemyShots()
{
	for (int i = 0; i < MAX_ENEMY_SHOTS; i++) {
		const EnemyShot &s = gEnemyShots[i];
		if (!s.active) continue;

		Rect r = l02bEnemyShotRect(s);

		// Held below full. The file's "black" is not quite black -- there is a
		// dark haze over the whole square -- and added at full strength that
		// haze shows as a faintly lighter rectangle around the burst. At this
		// level the haze adds almost nothing while the core still glows.
		double pulse = 0.72 + 0.16 * sin(s.spin);
		dImageAdditive(r.x, r.y, r.w, r.h, SPR_L02B_ENEMY_CHI.tex, pulse);
	}
}

void l02bUpdateLeader(double dt)
{
	if (!gLeader.active) return;

	gLeader.bobPhase += dt * LEADER_BOB_SPEED;
	if (gLeader.hitFlash > 0.0) gLeader.hitFlash -= dt;

	if (gLeader.dead) {
		gLeader.deathTimer -= dt;
		return;
	}

	if (gLeader.entry > 0.0) {
		gLeader.entry -= dt;
		return;                       // he does not throw while walking in
	}

	gLeader.shotTimer -= dt;
	if (gLeader.shotTimer <= 0.0) {
		gLeader.shotTimer = randBetween(ENEMY_CHI_INTERVAL_MIN, ENEMY_CHI_INTERVAL_MAX);
		l02bFireEnemyChidori();
	}
}

// True once he has finished falling, which is when the level is won.
bool l02bLeaderFinished()
{
	return gLeader.dead && gLeader.deathTimer <= 0.0;
}

void l02bDrawLeader()
{
	if (!gLeader.active) return;

	Rect r = l02bLeaderRect();

	if (gLeader.dead) {
		double fade = gLeader.deathTimer / LEADER_DEATH_TIME;
		if (fade < 0.0) fade = 0.0;
		dImageEx(r.x, r.y, r.w, r.h, SPR_L02B_LEADER.tex, false,
		         0.8, 0.4, 0.4, fade);
		return;
	}

	dImage(r.x, r.y, r.w, r.h, SPR_L02B_LEADER.tex);

	if (gLeader.hitFlash > 0.0)
		dImageEx(r.x, r.y, r.w, r.h, SPR_L02B_LEADER.tex, false,
		         0.55, 0.80, 1.0, gLeader.hitFlash / 0.26);

	// His six-hit counter, over his head, so the boss fight can be read off the
	// screen rather than remembered.
	if (gLeader.entry <= 0.0) {
		const double pipW = 20.0, pipH = 9.0, gap = 6.0;
		int max = LEADER_HITS_TO_KILL;
		double totalW = max * pipW + (max - 1) * gap;
		double x = r.x + r.w * 0.5 - totalW * 0.5;
		double y = r.y + r.h + 10.0;

		int left = max - gL02LeaderHits;
		for (int i = 0; i < max; i++) {
			Color pip = (i < left) ? C_ACCENT : C_PANEL;
			dFillRectA(x + i * (pipW + gap), y, pipW, pipH, pip, 0.94);
		}
	}
}

// ===========================================================================
//  THROWING
//
//  Both of his attacks go through the projectile pool the river half already
//  uses -- gChidori[], l02UpdateChidori(), l02ChidoriHitBox(). Only the
//  artwork, the size and what it is aimed at differ.
//
//  He cannot throw at will. Against the dakat every throw costs one of the
//  chidori the icon bought him, and at zero he has to walk into another icon
//  before he can attack again.
// ===========================================================================

// Whether this bolt should be drawn additively -- true for the powerful
// chidori, which is a glow on black. Kept parallel to gChidori[] rather than as
// another field on it, so the river half's struct is left exactly as it was.
static bool gBoltAdditive[MAX_CHIDORI] = { false };

// The nearest live dakat, or -1 if there is none.
static int l02bNearestDakat(double fromX, double fromY)
{
	int best = -1;
	double bestD = 0.0;

	for (int i = 0; i < MAX_DAKAT; i++) {
		const Dakat &d = gDakat[i];
		if (!d.active || d.dead) continue;

		Rect r = l02bDakatRect(d);
		double dx = (r.x + r.w * 0.5) - fromX;
		double dy = (r.y + r.h * 0.5) - fromY;
		double dist = dx * dx + dy * dy;

		if (best < 0 || dist < bestD) { best = i; bestD = dist; }
	}
	return best;
}

// Puts one projectile in the pool, aimed at a point. Shared by the ordinary
// chidori and the powerful one so the two cannot drift apart.
static void l02bLaunch(const Sprite &art, double width, double speed,
                       double targetX, double targetY, bool haveTarget,
                       bool additive)
{
	int slot = -1;
	for (int i = 0; i < MAX_CHIDORI; i++)
		if (!gChidori[i].active) { slot = i; break; }
	if (slot < 0) return;

	double hx, hy;
	l02bWalkerHand(&hx, &hy);

	Chidori &b = gChidori[slot];
	b.w = width;
	b.h = width / spriteAspect(art);     // the artwork's own shape, never forced
	b.x = hx;
	b.y = hy - b.h * 0.5;
	b.life = CHIDORI_LIFE;
	b.tex = art.tex;
	b.active = true;

	gBoltAdditive[slot] = additive;

	if (!haveTarget) {
		b.vx = speed;
		b.vy = 0.0;
		return;
	}

	double dx = targetX - hx;
	double dy = targetY - hy;
	double len = sqrt(dx * dx + dy * dy);
	if (len < 1.0) len = 1.0;

	b.vx = dx / len * speed;
	b.vy = dy / len * speed;
}

// Called by the throw animation when the wind-up finishes -- never straight
// from the key press, so the bolt always leaves an outstretched hand.
static void l02bReleaseThrow()
{
	double hx, hy;
	l02bWalkerHand(&hx, &hy);

	// Against the boss he throws the powerful chidori, and only if the powerup
	// actually granted it.
	if (gL02Stage == L02_DAKATLEADER_BATTLE) {
		if (!gL02HasPowerful) return;

		Rect r = l02bLeaderRect();
		l02bLaunch(SPR_L02B_POWERFUL, POWERFUL_W, POWERFUL_SPEED,
		           r.x + r.w * 0.5, r.y + r.h * 0.5, l02bLeaderIsFighting(),
		           true);
		gL02BoltsThrown++;
		return;
	}

	// Otherwise an ordinary chidori, and it costs one of the ones he holds.
	if (gL02Chidori <= 0) return;
	gL02Chidori--;

	int target = l02bNearestDakat(hx, hy);
	double tx = 0.0, ty = 0.0;
	if (target >= 0) {
		Rect r = l02bDakatRect(gDakat[target]);
		tx = r.x + r.w * 0.5;
		ty = r.y + r.h * 0.5;
	}

	l02bLaunch(SPR_L02B_CHIDORI, L02B_CHIDORI_W, L02B_CHIDORI_SPEED,
	           tx, ty, target >= 0, false);
	gL02BoltsThrown++;
}

// He can only throw when he is holding something and there is something to
// throw it at. This is what the icon is for: with no chidori left, nothing
// happens until he walks into another one.
bool l02bCanThrow()
{
	if (gL02Stage == L02_DAKAT_BATTLE)       return gL02Chidori > 0;
	if (gL02Stage == L02_DAKATLEADER_BATTLE) return gL02HasPowerful && l02bLeaderIsFighting();
	return false;
}

void l02bBeginThrow()
{
	if (!l02bCanThrow()) return;
	if (gL02Phase != L02P_PLAYING) return;
	if (gWalkThrow != THROW_IDLE) return;
	if (gWalkThrowCooldown > 0.0) return;

	gWalkThrow = THROW_WINDUP;
	gWalkThrowTimer = L02B_THROW_WINDUP;
	gWalkThrowCooldown = (gL02Stage == L02_DAKATLEADER_BATTLE)
	                   ? POWERFUL_COOLDOWN : L02B_THROW_COOLDOWN;
}

static void l02bUpdateThrow(double dt)
{
	// The pose is decided here, every frame, so it can never disagree with the
	// animation. It cannot live in l02bUpdateWalker(): during a battle he may
	// be standing still, and he still has to be able to raise his arm.
	gWalker.pose = (gWalkThrow == THROW_IDLE) ? WALK_POSE_WALK : WALK_POSE_THROW;

	if (gWalkThrowCooldown > 0.0) gWalkThrowCooldown -= dt;

	if (gWalkThrow == THROW_IDLE) return;

	gWalkThrowTimer -= dt;
	if (gWalkThrowTimer > 0.0) return;

	if (gWalkThrow == THROW_WINDUP) {
		l02bReleaseThrow();
		gWalkThrow = THROW_RECOVER;
		gWalkThrowTimer = L02B_THROW_RECOVER;
	} else {
		gWalkThrow = THROW_IDLE;
	}
}

// The bolts he has in the air. The pool and its update belong to the river
// half; only the drawing differs, because the powerful one is additive.
void l02bDrawBolts()
{
	for (int i = 0; i < MAX_CHIDORI; i++) {
		const Chidori &b = gChidori[i];
		if (!b.active) continue;

		Rect r = makeRect(b.x, b.y, b.w, b.h);

		if (gBoltAdditive[i]) {
			dImageAdditive(r.x, r.y, r.w, r.h, b.tex, 1.0);
			continue;
		}

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
//  COLLISION
//
//  Everything here goes through checkCollision() on two boxes, the same way the
//  rest of the game does it.
// ===========================================================================

// A chidori landing on a dakat. One projectile is one hit: it is spent the
// moment it connects, so it can never carry on into the second dakat.
static void l02bCheckChidoriHits()
{
	for (int b = 0; b < MAX_CHIDORI; b++) {
		Chidori &bolt = gChidori[b];
		if (!bolt.active) continue;

		Rect box = l02ChidoriHitBox(bolt);

		for (int i = 0; i < MAX_DAKAT; i++) {
			Dakat &d = gDakat[i];
			if (!d.active || d.dead) continue;
			if (!checkCollision(box, l02bDakatHitBox(d))) continue;

			bolt.active = false;
			l02bDamageDakat(d);
			break;
		}
	}
}

// A powerful chidori landing on the dakatleader. Same rule: one bolt, one hit.
static void l02bCheckLeaderHits()
{
	if (!l02bLeaderIsFighting()) return;

	Rect boss = l02bLeaderHitBox();

	for (int b = 0; b < MAX_CHIDORI; b++) {
		Chidori &bolt = gChidori[b];
		if (!bolt.active) continue;
		if (!checkCollision(l02ChidoriHitBox(bolt), boss)) continue;

		bolt.active = false;
		l02bDamageLeader();
	}
}

// An enemy chidori landing on him. The projectile is deactivated FIRST, so the
// seventy-five comes off exactly once however long the sprites overlap.
static void l02bCheckEnemyShotHits()
{
	Rect body = l02bWalkerHitBox();

	for (int i = 0; i < MAX_ENEMY_SHOTS; i++) {
		EnemyShot &s = gEnemyShots[i];
		if (!s.active) continue;
		if (!checkCollision(body, l02bEnemyShotHitBox(s))) continue;

		s.active = false;                 // spent before anything else happens
		playCollisionSound();
		level02Damage(ENEMY_CHI_DAMAGE);  // exactly -75, once
	}
}

void l02bCheckCombat()
{
	l02bCheckChidoriHits();
	l02bCheckLeaderHits();
	l02bCheckEnemyShotHits();
}

// ===========================================================================
//  Everything at once, for a fresh run
// ===========================================================================
void l02bResetDeck()
{
	// The projectile pool is shared with the river half, so it is emptied on
	// the way in. Nothing in flight at the crash should arrive below decks.
	l02ResetChidori();
	for (int i = 0; i < MAX_CHIDORI; i++) gBoltAdditive[i] = false;

	l02bResetShip();
	l02bResetWalker();
	l02bResetPickup();
	l02bResetDakat();
	l02bResetLeader();
	l02bResetEnemyShots();
}

// The parts of the deck that tick every frame whatever the stage.
void l02bUpdateCommon(double dt, bool controlsLive)
{
	l02bUpdateWalker(dt, controlsLive);
	l02bUpdateThrow(dt);
	l02bUpdatePickup(dt);
	l02bUpdateEnemyShots(dt);
}

// ===========================================================================
//  DRAWING, BACK TO FRONT
//
//  This used to be a fixed list -- dakat, then leader, then the character --
//  which meant the character was painted over BOTH of them every frame, in
//  every situation. A dakat walking into him disappeared behind him and stayed
//  there, and so did the boss. Nothing about their positions was consulted.
//
//  Now the actors that share the deck floor are sorted by DEPTH and drawn back
//  to front, so a dakat on the near line passes in front of him and one on the
//  far line passes behind him -- which of the two it is comes from where it
//  actually is, not from the order the draw calls happen to be written in.
//
//  Depth is the actor's GROUND line, never its drawn y. That distinction is the
//  whole reason jumping does not break the layering: leaving the floor moves
//  where he is drawn without changing what he is drawn in front of. Sorting on
//  the drawn y would send him behind everything the instant he jumped.
// ===========================================================================
enum DeckActorKind {
	ACTOR_WALKER = 0,
	ACTOR_DAKAT,
	ACTOR_LEADER
};

struct DeckActor {
	DeckActorKind kind;
	int    index;      // which dakat, when kind is ACTOR_DAKAT
	double depth;      // its floor line: larger is further back
};

static void l02bDrawOneActor(const DeckActor &a)
{
	if (a.kind == ACTOR_WALKER)      l02bDrawWalker();
	else if (a.kind == ACTOR_LEADER) l02bDrawLeader();
	else                             l02bDrawOneDakat(gDakat[a.index]);
}

void l02bDrawDeckScene()
{
	// 1. the pickup, which lies on the deck behind everyone
	l02bDrawPickup();

	// 2. gather whoever is standing on the floor right now
	DeckActor actors[MAX_DAKAT + 2];
	int count = 0;

	actors[count].kind = ACTOR_WALKER;
	actors[count].index = 0;
	actors[count].depth = gWalker.groundY;      // his floor line, not his jump
	count++;

	for (int i = 0; i < MAX_DAKAT; i++) {
		if (!gDakat[i].active) continue;
		actors[count].kind = ACTOR_DAKAT;
		actors[count].index = i;
		actors[count].depth = gDakat[i].groundY;
		count++;
	}

	if (gLeader.active) {
		actors[count].kind = ACTOR_LEADER;
		actors[count].index = 0;
		actors[count].depth = gLeader.y;
		count++;
	}

	// 3. sort furthest-back first. An insertion sort, because there are at most
	//    four of them and it keeps equal depths in a stable order.
	for (int i = 1; i < count; i++) {
		DeckActor key = actors[i];
		int j = i - 1;
		while (j >= 0 && actors[j].depth < key.depth) {
			actors[j + 1] = actors[j];
			j--;
		}
		actors[j + 1] = key;
	}

	for (int i = 0; i < count; i++)
		l02bDrawOneActor(actors[i]);

	// 4. projectiles and effects sit in front of everyone
	l02bDrawBolts();         // his chidori
	l02bDrawEnemyShots();    // and the boss's, in front of everything
}

#endif // LEVEL02DECK_HPP

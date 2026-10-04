//
//  Level03Entities.hpp  --  the things on the road that have a position.
//
//  The walking character, the street traffic he has to jump, the pickups he
//  walks into, and the projectiles he throws.
//
//  It borrows rather than repeats:
//
//      TEX_L03_CHAR[]        Level 03's own copy of the six walk frames
//      JUMP physics          the same velocity-against-gravity integration
//                            Player.hpp uses, with Level 03's constants
//      checkCollision()      every hit test here
//      level03Damage()       the one place Level 03 health ever changes
//      randBetween/randRange the xorshift generator from Entities.hpp
//
//  The set-pieces live next door in Level03Scenes.hpp. Nothing here decides
//  what stage the level is in; the stage machine in Level03.hpp drives it.
//
#ifndef LEVEL03ENTITIES_HPP
#define LEVEL03ENTITIES_HPP


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
#endif
#include <math.h>

// ===========================================================================
//  THE WALKER
//
//  Six frames, the same cycle the rest of the game uses. For most of the level
//  he runs on the spot at a fixed x and the road moves past him, exactly as in
//  Level 01. On the jam he takes over his own x so he can dodge forward and
//  back, which is the only place his position is his to choose.
//
//  The jump is real physics: an upward velocity fought by gravity, integrated
//  every frame. It cannot start in mid-air, so there is no double jump, and
//  the y it produces is what collision actually tests -- which is what makes
//  jumping over a bike genuinely work rather than merely look like it does.
// ===========================================================================
//  The poses below WALK are all cut down to their own content, so they are
//  sized by an explicit height and lifted by L03_POSE_FOOT_LIFT to stand on
//  the same line the walk frames stand on.
enum L03Pose {
	L03_POSE_WALK = 0,     // character_1..6
	L03_POSE_THROW,        // throw.png, arm out
	L03_POSE_BEG,          // beg.png, on his knees in front of the thief
	L03_POSE_SLAP_UP,      // boy_01_with_fx.png, up off his knees, hand landing
	L03_POSE_SLAP_HIT      // boy_02.png, the follow-through
};

struct L03Walker {
	double  x, y;          // y is the DRAWN bottom edge: ground + jump
	double  groundY;       // the line his feet return to
	double  velocityY;
	bool    isJumping;
	L03Pose pose;
	int     frame;
	double  frameTimer;
	bool    moving;
};

static L03Walker gW3;

// The throw: press, hand out, the projectile leaves that hand, back to walking.
static ThrowPhase gL03Throw = THROW_IDLE;
static double gL03ThrowTimer = 0.0;
static double gL03ThrowCooldown = 0.0;

void l03ResetWalker()
{
	gW3.x = L03_PLAYER_X;
	gW3.groundY = L03_GROUND_Y;
	gW3.y = gW3.groundY;
	gW3.velocityY = 0.0;
	gW3.isJumping = false;
	gW3.pose = L03_POSE_WALK;
	gW3.frame = 0;
	gW3.frameTimer = 0.0;
	gW3.moving = false;

	gL03Throw = THROW_IDLE;
	gL03ThrowTimer = 0.0;
	gL03ThrowCooldown = 0.0;
}

bool l03WalkerAirborne() { return gW3.isJumping; }

// The walk frames' own framing: the whole canvas, as the artwork ships. This
// is what his BODY is, whatever he happens to be doing with his arms, so it is
// what the hit box is built from.
Rect l03WalkerBodyRect()
{
	double h = PLAYER_BASE_H;
	double w = h * PLAYER_ASPECT;
	return makeRect(gW3.x, gW3.y, w, h);
}

// What is actually drawn. throw.png and the three interaction poses are cut
// down to their content while the six walk frames keep their whole canvas, so
// the two are not framed alike. Matching the visible height and anchoring both
// at the feet keeps him the same size through the swap instead of growing
// every time he throws.
Rect l03WalkerRect()
{
	double h, w;

	switch (gW3.pose) {
	case L03_POSE_THROW:
		h = PLAYER_BASE_H * L02B_THROW_H_SCALE;
		w = h * spriteAspect(SPR_L03_THROW);
		return makeRect(gW3.x, gW3.y, w, h);

	case L03_POSE_BEG:
		h = BEG_H;
		w = h * spriteAspect(SPR_L03_BEG);
		return makeRect(gW3.x, gW3.y + L03_POSE_FOOT_LIFT, w, h);

	case L03_POSE_SLAP_UP:
		h = SLAP_H;
		w = h * spriteAspect(SPR_L03_SLAP1);
		return makeRect(gW3.x, gW3.y + L03_POSE_FOOT_LIFT, w, h);

	case L03_POSE_SLAP_HIT:
		h = SLAP_H;
		w = h * spriteAspect(SPR_L03_SLAP2);
		return makeRect(gW3.x, gW3.y + L03_POSE_FOOT_LIFT, w, h);

	default:
		return l03WalkerBodyRect();
	}
}

// The body, not the swinging arms and not the empty canvas around them. It
// tracks his real y, so a jump lifts it clear of whatever is passing under.
//
// The interaction poses deliberately do NOT resize it. Kneeling down must not
// quietly shrink what can hit him and swinging must not grow it: the fix for
// the thief scene is a pose and a pause on the traffic, not a change to the
// numbers collision runs on.
Rect l03WalkerHitBox()
{
	Rect r = (gW3.pose == L03_POSE_BEG ||
	          gW3.pose == L03_POSE_SLAP_UP ||
	          gW3.pose == L03_POSE_SLAP_HIT)
	       ? l03WalkerBodyRect()
	       : l03WalkerRect();

	return makeRect(r.x + r.w * 0.27,
	                r.y + r.h * 0.04,
	                r.w * 0.46,
	                r.h * 0.88);
}

void l03WalkerHand(double *hx, double *hy)
{
	Rect r = l03WalkerRect();
	*hx = r.x + r.w * L03_HAND_FX;
	*hy = r.y + r.h * L03_HAND_FY;
}

// How far the world was pushed forward this frame by him walking on the jam:
// the amount the backdrop, the parked line and the falling fire all have to
// slide left. Zero everywhere else, and never negative -- the road only ever
// runs one way.
static double gL03JamPush = 0.0;

double l03JamPushThisFrame() { return gL03JamPush; }

// freeMove is true only on the jam, where he drives the crossing himself.
// Everywhere else the road moves on its own and he stays put.
//
// On the jam he walks right across the screen until he reaches
// L03_JAM_MAX_X, and from there pressing forward pushes the WORLD instead of
// him -- the same trick every side-scroller uses, and the reason the parked
// cars travel backwards past him rather than him sliding along a still
// street. Pressing back just gives ground inside the dodge window.
void l03UpdateWalker(double dt, bool controlsLive, bool freeMove, bool cycleTurning)
{
	gW3.moving = false;
	gL03JamPush = 0.0;

	if (controlsLive && freeMove) {
		bool left  = isKeyPressed('a') != 0 || isKeyPressed('A') != 0 ||
		             isSpecialKeyPressed(GLUT_KEY_LEFT) != 0;
		bool right = isKeyPressed('d') != 0 || isKeyPressed('D') != 0 ||
		             isSpecialKeyPressed(GLUT_KEY_RIGHT) != 0;

		double step = L03_JAM_MOVE_SPEED * dt;

		if (left && !right) {
			gW3.x -= step;
			gW3.moving = true;
		} else if (right && !left) {
			gW3.moving = true;
			gW3.x += step;

			// Past the push line the rest of the step goes into the world, so
			// his pace over the ground is the same whichever side of the line
			// he is on.
			if (gW3.x > L03_JAM_MAX_X) {
				gL03JamPush = gW3.x - L03_JAM_MAX_X;
				gW3.x = L03_JAM_MAX_X;
			}
		}

		if (gW3.x < L03_JAM_MIN_X) gW3.x = L03_JAM_MIN_X;
		if (gW3.x > L03_JAM_MAX_X) gW3.x = L03_JAM_MAX_X;
	}

	// --- jump ---
	if (controlsLive && !gW3.isJumping) {
		if (keyJustPressed(' ') ||
		    keyJustPressed('w') || keyJustPressed('W') ||
		    specialKeyJustPressed(GLUT_KEY_UP)) {
			gW3.velocityY = L03_JUMP_VELOCITY;
			gW3.isJumping = true;
		}
	}

	if (gW3.isJumping) {
		gW3.velocityY -= L03_JUMP_GRAVITY * dt;
		gW3.y += gW3.velocityY * dt;

		if (gW3.y <= gW3.groundY) {            // landed
			gW3.y = gW3.groundY;
			gW3.velocityY = 0.0;
			gW3.isJumping = false;
		}
	}

	// The cycle turns over while the world is moving past him, or while he is
	// moving himself. Standing still in a conversation, he stands still.
	if (cycleTurning || gW3.moving) {
		gW3.frameTimer += dt;
		while (gW3.frameTimer >= PLAYER_FRAME_TIME) {
			gW3.frameTimer -= PLAYER_FRAME_TIME;
			gW3.frame = (gW3.frame + 1) % CHARACTER_FRAME_COUNT;
		}
	}
}

// A soft patch under his feet, drawn just before he is. Standing on a roof is
// read from the contact, not from the height: without it the eye has nothing
// tying him to the vehicle and he looks like he is hanging behind the row
// rather than walking along the top of it.
void l03DrawWalkerShadow()
{
	if (gW3.isJumping) return;                 // nothing to stand on mid-air

	Rect r = l03WalkerBodyRect();
	double cx = r.x + r.w * 0.5;
	double cy = gW3.y + 6.0;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.0f, 0.0f, 0.0f, 0.24f);

	// An ellipse, drawn as a flattened circle: iGraphics has no ellipse call.
	glPushMatrix();
	glTranslated(cx, cy, 0.0);
	glScaled(1.0, 0.30, 1.0);
	iFilledCircle(0.0, 0.0, r.w * 0.40);
	glPopMatrix();

	glDisable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void l03DrawWalker()
{
	Rect r = l03WalkerRect();

	// Blink through the immunity window so it is obvious the hit registered.
	if (gL03Invuln > 0.0 && ((int)(gL03Invuln * 12.0) % 2) == 0)
		return;

	unsigned int tex;
	switch (gW3.pose) {
	case L03_POSE_THROW:    tex = SPR_L03_THROW.tex; break;
	case L03_POSE_BEG:      tex = SPR_L03_BEG.tex;   break;
	case L03_POSE_SLAP_UP:  tex = SPR_L03_SLAP1.tex; break;
	case L03_POSE_SLAP_HIT: tex = SPR_L03_SLAP2.tex; break;
	default:                tex = TEX_L03_CHAR[gW3.frame]; break;
	}

	if (gL03HitFlash > 0.0)
		dImageEx(r.x, r.y, r.w, r.h, tex, false, 1.0, 0.38, 0.38, 1.0);
	else
		dImage(r.x, r.y, r.w, r.h, tex);
}

// ===========================================================================
//  STREET TRAFFIC  --  jump it or take fifty
//
//  Exactly the Level 01 arrangement: one vehicle at a time, coming from the
//  left or overtaking from the right, arriving far enough away to be seen. The
//  drawn heights are all well under the jump apex of 206 px, so every one of
//  them is clearable -- that is a property of the numbers, not a hope.
// ===========================================================================
enum L03TrafficKind {
	L03T_BIKE = 0,
	L03T_CAR,
	L03T_RICK,
	L03T_KIND_COUNT
};

struct L03Traffic {
	bool   active;
	bool   spent;          // has already been resolved against him
	int    kind;
	bool   coming;         // true = from the left facing left, false = overtaking
	double x, y;
	double w, h;
	double speed;
};

static L03Traffic gL03Traffic[MAX_L03_TRAFFIC];
static double gL03TrafficTimer = 0.0;
static bool   gL03TrafficWasSafe = false;   // was a set-piece holding the road last frame

static Sprite l03TrafficSprite(const L03Traffic &t)
{
	if (t.kind == L03T_BIKE) return t.coming ? SPR_L03_BIKE_COMING : SPR_L03_BIKE_GOING;
	if (t.kind == L03T_CAR)  return t.coming ? SPR_L03_CAR_COMING  : SPR_L03_CAR_GOING;
	return t.coming ? SPR_L03_RICK_COMING : SPR_L03_RICK_GOING;
}

void l03ResetTraffic()
{
	for (int i = 0; i < MAX_L03_TRAFFIC; i++) gL03Traffic[i].active = false;
	gL03TrafficTimer = randBetween(L03_TRAFFIC_MIN_GAP, L03_TRAFFIC_MAX_GAP);
	gL03TrafficWasSafe = false;
}

Rect l03TrafficRect(const L03Traffic &t)
{
	return makeRect(t.x, t.y, t.w, t.h);
}

// Pulled well in from the artwork's edges. A wing mirror should not cost fifty,
// and at fifty a hit the box wants to be the vehicle a player would say hit
// them rather than every pixel of its picture.
Rect l03TrafficHitBox(const L03Traffic &t)
{
	return shrinkRect(l03TrafficRect(t), 0.62, 0.68);
}

static void l03SpawnTraffic()
{
	int slot = -1;
	for (int i = 0; i < MAX_L03_TRAFFIC; i++)
		if (!gL03Traffic[i].active) { slot = i; break; }
	if (slot < 0) return;

	L03Traffic &t = gL03Traffic[slot];

	t.kind = randRange(0, L03T_KIND_COUNT - 1);
	t.coming = randRange(1, 100) <= 60;
	t.active = true;
	t.spent = false;

	if (t.kind == L03T_BIKE)      t.h = L03_H_BIKE;
	else if (t.kind == L03T_CAR)  t.h = L03_H_CAR;
	else                          t.h = L03_H_RICK;

	t.w = t.h * spriteAspect(l03TrafficSprite(t));
	t.y = L03_GROUND_Y;
	t.speed = L03_TRAFFIC_SPEED;

	// Coming traffic enters from the right and crosses leftwards; overtaking
	// traffic comes up from behind him.
	t.x = t.coming ? (WIN_W + 60.0) : (-t.w - 60.0);
}

// Takes the road off him at once. Used when he climbs up onto the jam, where
// a stray bike still driving along the road behind the parked line would be
// drawn over the top of it.
void l03ClearTraffic()
{
	for (int i = 0; i < MAX_L03_TRAFFIC; i++) gL03Traffic[i].active = false;
}

//  spawning  whether new vehicles may be put on the road
//  safe      a set-piece has hold of him and he cannot dodge
//
//  While safe, nothing new is spawned, the spawn timer is held where it is
//  rather than counting down behind his back, and whatever was already on the
//  road is defused and allowed to drive off the way it was already going.
//  Freezing those in place would leave a bus parked across the scene; letting
//  them finish their crossing clears the road in a second or two and looks
//  like nothing at all happened.
//
//  None of this is permanent. The moment the set-piece ends the caller passes
//  safe = false again and the road picks up exactly where it left off.
void l03UpdateTraffic(double dt, bool spawning, bool safe)
{
	Rect body = l03WalkerHitBox();

	// On the way IN to a protected set-piece, put a full fresh gap on the
	// clock. It is not counted down while safe, so this is what he gets back
	// when the set-piece ends -- rather than a vehicle arriving on the frame
	// the thief runs off because the timer had already expired underneath it.
	if (safe && !gL03TrafficWasSafe)
		gL03TrafficTimer = randBetween(L03_TRAFFIC_MIN_GAP, L03_TRAFFIC_MAX_GAP);
	gL03TrafficWasSafe = safe;

	for (int i = 0; i < MAX_L03_TRAFFIC; i++) {
		L03Traffic &t = gL03Traffic[i];
		if (!t.active) continue;

		t.x += (t.coming ? -t.speed : t.speed) * dt;

		if (safe) {
			// Harmless for the rest of its run, so it cannot come back to
			// life when the set-piece ends and it is still on screen.
			t.spent = true;
		} else if (!t.spent && checkCollision(body, l03TrafficHitBox(t))) {
			// One vehicle can only ever cost him once, whatever happens next.
			t.spent = true;
			level03Damage(L03_HIT_DAMAGE);
		}

		if (t.x + t.w < -200.0 || t.x > WIN_W + 200.0) t.active = false;
	}

	if (safe || !spawning) return;

	gL03TrafficTimer -= dt;
	if (gL03TrafficTimer <= 0.0) {
		gL03TrafficTimer = randBetween(L03_TRAFFIC_MIN_GAP, L03_TRAFFIC_MAX_GAP);
		l03SpawnTraffic();
	}
}

void l03DrawTraffic()
{
	for (int i = 0; i < MAX_L03_TRAFFIC; i++) {
		const L03Traffic &t = gL03Traffic[i];
		if (!t.active) continue;

		Rect r = l03TrafficRect(t);

		// rickshaw_is_coming is a watermarked stock preview, so the "going"
		// artwork is mirrored for both directions, exactly as in Level 01.
		if (t.kind == L03T_RICK && t.coming && RICKSHAW_COMING_NEEDS_FALLBACK)
			dImageFlipped(r.x, r.y, r.w, r.h, SPR_L03_RICK_GOING.tex, true);
		else
			dImage(r.x, r.y, r.w, r.h, l03TrafficSprite(t).tex);
	}
}

// ===========================================================================
//  PICKUPS  --  the rock, then the stick
//
//  Only one is ever on the road. Walking into it is what arms the throw: with
//  nothing in hand the throw key does nothing at all.
// ===========================================================================
struct L03Pickup {
	bool     active;
	L03Carry kind;
	double   x, y;
	double   w, h;
	double   bobPhase;
};

static L03Pickup gL03Pick;

void l03ResetPickup()
{
	gL03Pick.active = false;
	gL03Pick.kind = CARRY_NONE;
	gL03Pick.x = 0.0;
	gL03Pick.y = L03_PICKUP_Y;
	gL03Pick.w = 0.0;
	gL03Pick.h = L03_PICKUP_H;
	gL03Pick.bobPhase = 0.0;
}

static Sprite l03PickupSprite(L03Carry kind)
{
	return (kind == CARRY_STICK) ? SPR_L03_STICK : SPR_L03_ROCK;
}

void l03ShowPickup(L03Carry kind, double x)
{
	gL03Pick.active = true;
	gL03Pick.kind = kind;
	gL03Pick.h = L03_PICKUP_H;
	gL03Pick.w = L03_PICKUP_H * spriteAspect(l03PickupSprite(kind));
	gL03Pick.x = x;
	gL03Pick.y = L03_PICKUP_Y;
	gL03Pick.bobPhase = 0.0;
}

Rect l03PickupRect()
{
	return makeRect(gL03Pick.x,
	                gL03Pick.y + sin(gL03Pick.bobPhase) * 7.0,
	                gL03Pick.w, gL03Pick.h);
}

Rect l03PickupHitBox()
{
	return shrinkRect(l03PickupRect(), 0.85, 0.85);
}

// The pickup sits on the road, so it drifts past with the road.
void l03UpdatePickup(double dt, double scrollSpeed)
{
	if (!gL03Pick.active) return;

	gL03Pick.x -= scrollSpeed * dt;
	gL03Pick.bobPhase += dt * 2.6;
}

bool l03TookPickup()
{
	if (!gL03Pick.active) return false;
	return checkCollision(l03WalkerHitBox(), l03PickupHitBox());
}

void l03DrawPickup()
{
	if (!gL03Pick.active) return;

	Rect r = l03PickupRect();

	// A ring under it so it reads as something to collect, not as litter.
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1.0f, 0.82f, 0.35f, 0.22f);
	iFilledCircle(r.x + r.w * 0.5, r.y + r.h * 0.5, r.w * 0.72);
	glDisable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	dImage(r.x, r.y, r.w, r.h, l03PickupSprite(gL03Pick.kind).tex);
}

// ===========================================================================
//  COINS
//
//  Score only. They never touch health, never block him and never end a
//  set-piece, and they are collected through the SAME collectCoin() Level 01
//  and Level 02 call -- so gScore and gCoinsTaken stay the one scoring system
//  the whole game shares, and nothing here keeps a private tally.
//
//  They ride the world rather than the clock: the caller passes how far the
//  street moved this frame, which is the walk on the open stretches, zero
//  while a set-piece has him stopped, and his own push while he crosses the
//  jam. One number, so a coin can never drift out of step with the road it is
//  lying on.
// ===========================================================================
struct L03Coin {
	bool   active;
	double x, y;
	double size;
	double bobPhase;
};

static L03Coin gL03Coins[MAX_L03_COINS];
static double gL03CoinTimer = 0.0;

void l03ResetCoins()
{
	for (int i = 0; i < MAX_L03_COINS; i++) gL03Coins[i].active = false;
	gL03CoinTimer = randBetween(L03_COIN_MIN_GAP, L03_COIN_MAX_GAP);
}

// groundY is whatever he is standing on right now -- the road, or the roofs of
// the jam -- so the coins are always within reach of the surface he is on.
static void l03SpawnCoin(double groundY)
{
	for (int i = 0; i < MAX_L03_COINS; i++) {
		L03Coin &c = gL03Coins[i];
		if (c.active) continue;

		c.active = true;
		c.size = L03_COIN_SIZE;
		c.x = WIN_W + 40.0;
		c.y = groundY + randBetween(L03_COIN_LOW, L03_COIN_HIGH);
		c.bobPhase = randBetween(0.0, 6.28);
		return;
	}
}

Rect l03CoinRect(const L03Coin &c)
{
	return makeRect(c.x, c.y + sin(c.bobPhase) * 5.0, c.size, c.size);
}

Rect l03CoinHitBox(const L03Coin &c)
{
	return shrinkRect(l03CoinRect(c), 0.82, 0.82);
}

//  worldStep  how far the street moved this frame, in pixels
//  spawning   whether new coins may be laid down ahead of him
void l03UpdateCoins(double dt, double worldStep, bool spawning)
{
	for (int i = 0; i < MAX_L03_COINS; i++) {
		L03Coin &c = gL03Coins[i];
		if (!c.active) continue;

		c.x -= worldStep;
		c.bobPhase += dt * 3.2;

		if (c.x + c.size < -60.0) c.active = false;
	}

	if (!spawning) return;

	// The timer only runs while the world is actually moving. Standing still
	// through a conversation must not quietly stack up a queue of coins that
	// all arrive at once the moment he walks on.
	if (worldStep <= 0.0) return;

	gL03CoinTimer -= dt;
	if (gL03CoinTimer <= 0.0) {
		gL03CoinTimer = randBetween(L03_COIN_MIN_GAP, L03_COIN_MAX_GAP);
		l03SpawnCoin(gW3.groundY);
	}
}

// Returns the centre of whichever coin was taken, so the caller can float a
// "+10" there. Only one is collected per frame, which is all that can
// physically overlap him anyway.
bool l03TakeCoin(double *popX, double *popY)
{
	Rect body = l03WalkerHitBox();

	for (int i = 0; i < MAX_L03_COINS; i++) {
		L03Coin &c = gL03Coins[i];
		if (!c.active) continue;
		if (!checkCollision(body, l03CoinHitBox(c))) continue;

		Rect r = l03CoinRect(c);
		c.active = false;              // gone, so it cannot score twice

		collectCoin();                 // the shared counter, not a local one

		if (popX) *popX = r.x + r.w * 0.5;
		if (popY) *popY = r.y + r.h;
		return true;
	}
	return false;
}

void l03DrawCoins()
{
	for (int i = 0; i < MAX_L03_COINS; i++) {
		const L03Coin &c = gL03Coins[i];
		if (!c.active) continue;

		Rect r = l03CoinRect(c);
		dImage(r.x, r.y, r.w, r.h, SPR_L03_COIN.tex);
	}
}

// ===========================================================================
//  WHAT HE THROWS
//
//  One pool for both the rock and the stick: they differ only in artwork and
//  in what they are allowed to hit. A projectile is spent the instant it
//  connects, which is what stops one throw counting twice.
// ===========================================================================
struct L03Shot {
	bool   active;
	double x, y;
	double w, h;
	double vx, vy;
	double life;
	double spin;
	unsigned int tex;
};

static L03Shot gL03Shots[MAX_L03_SHOTS];

void l03ResetShots()
{
	for (int i = 0; i < MAX_L03_SHOTS; i++) gL03Shots[i].active = false;
}

// Launches whatever he is carrying at a point. Called by the throw animation
// when the wind-up finishes -- never straight from the key press, so it always
// leaves an outstretched hand.
void l03Launch(double targetX, double targetY, bool haveTarget)
{
	if (gL03Carry == CARRY_NONE) return;

	int slot = -1;
	for (int i = 0; i < MAX_L03_SHOTS; i++)
		if (!gL03Shots[i].active) { slot = i; break; }
	if (slot < 0) return;

	Sprite art = l03PickupSprite(gL03Carry);

	double hx, hy;
	l03WalkerHand(&hx, &hy);

	L03Shot &s = gL03Shots[slot];
	s.w = L03_SHOT_W;
	s.h = L03_SHOT_W / spriteAspect(art);
	s.x = hx;
	s.y = hy - s.h * 0.5;
	s.life = 2.2;
	s.spin = 0.0;
	s.tex = art.tex;
	s.active = true;

	if (!haveTarget) {
		s.vx = L03_SHOT_SPEED;
		s.vy = 0.0;
	} else {
		double dx = targetX - hx;
		double dy = targetY - hy;
		double len = sqrt(dx * dx + dy * dy);
		if (len < 1.0) len = 1.0;
		s.vx = dx / len * L03_SHOT_SPEED;
		s.vy = dy / len * L03_SHOT_SPEED;
	}

	gL03Thrown++;
}

Rect l03ShotRect(const L03Shot &s)
{
	return makeRect(s.x, s.y, s.w, s.h);
}

Rect l03ShotHitBox(const L03Shot &s)
{
	return shrinkRect(l03ShotRect(s), 0.72, 0.72);
}

void l03UpdateShots(double dt)
{
	for (int i = 0; i < MAX_L03_SHOTS; i++) {
		L03Shot &s = gL03Shots[i];
		if (!s.active) continue;

		s.x += s.vx * dt;
		s.y += s.vy * dt;
		s.life -= dt;
		s.spin += dt * 9.0;

		if (s.life <= 0.0 || s.x > WIN_W + 200.0 || s.x + s.w < -200.0 ||
		    s.y > WIN_H + 200.0 || s.y + s.h < -200.0)
			s.active = false;
	}
}

void l03DrawShots()
{
	for (int i = 0; i < MAX_L03_SHOTS; i++) {
		const L03Shot &s = gL03Shots[i];
		if (!s.active) continue;

		Rect r = l03ShotRect(s);
		dImage(r.x, r.y, r.w, r.h, s.tex);
	}
}

// ---------------------------------------------------------------------------
//  The throw animation
// ---------------------------------------------------------------------------
void l03BeginThrow()
{
	if (!level03CanThrow()) return;
	if (gL03Phase != L03P_PLAYING) return;
	if (gL03Throw != THROW_IDLE) return;
	if (gL03ThrowCooldown > 0.0) return;

	gL03Throw = THROW_WINDUP;
	gL03ThrowTimer = L03_THROW_WINDUP;
	gL03ThrowCooldown = L03_THROW_COOLDOWN;
}

// releaseFn is what actually creates the projectile, because only the stage
// machine knows what he is aiming at.
void l03UpdateThrow(double dt, void (*releaseFn)())
{
	// The pose is decided here, every frame, so it can never disagree with the
	// animation -- including while he is standing still in a set-piece.
	//
	// Except during the thief interaction, which owns the pose for as long as
	// it lasts. He cannot throw there anyway (level03CanThrow is false in the
	// chor stage), so this only stops the two writing over each other.
	if (!level03InteractionActive())
		gW3.pose = (gL03Throw == THROW_IDLE) ? L03_POSE_WALK : L03_POSE_THROW;

	if (gL03ThrowCooldown > 0.0) gL03ThrowCooldown -= dt;

	if (gL03Throw == THROW_IDLE) return;

	gL03ThrowTimer -= dt;
	if (gL03ThrowTimer > 0.0) return;

	if (gL03Throw == THROW_WINDUP) {
		if (releaseFn) releaseFn();
		gL03Throw = THROW_RECOVER;
		gL03ThrowTimer = L03_THROW_RECOVER;
	} else {
		gL03Throw = THROW_IDLE;
	}
}

void l03ResetEntities()
{
	l03ResetWalker();
	l03ResetTraffic();
	l03ResetPickup();
	l03ResetShots();
	l03ResetCoins();
}

#endif // LEVEL03ENTITIES_HPP

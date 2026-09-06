//
//  Player.hpp  --  the running character, locked to one ground line.
//
//  The character does not travel along the street. It runs on the spot at a
//  fixed x; the street and the traffic move past it. There is no left, right,
//  forward or back. The one control is the jump.
//
//  The jump is real physics: an upward velocity fought by gravity, integrated
//  every frame. It cannot be started while already airborne, so there is no
//  double jump, and the y it produces is what collision actually tests against
//  -- which is what makes jumping over a car genuinely work.
//
//  The six frames in characters_by_sequence share one canvas and one foot
//  baseline (every frame's lowest opaque pixel is row 541 of 554), so they all
//  draw into the same rectangle and the run lands cleanly.
//
#ifndef PLAYER_HPP
#define PLAYER_HPP


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
#include "LevelState.hpp"
#endif
struct PlayerState {
	double y;             // bottom edge of the sprite
	double velocityY;
	bool   isJumping;
	int    frame;
	double frameTimer;
	bool   isRunning;     // the run key is held
};

static PlayerState gPlayer;

void resetCharacter()
{
	gPlayer.y = GROUND_Y;
	gPlayer.velocityY = 0.0;
	gPlayer.isJumping = false;
	gPlayer.frame = 0;
	gPlayer.frameTimer = 0.0;
	gPlayer.isRunning = true;
}

bool playerIsAirborne() { return gPlayer.isJumping; }
double playerJumpOffset() { return gPlayer.y - GROUND_Y; }

Rect characterDrawRect()
{
	double h = PLAYER_BASE_H;
	double w = h * PLAYER_ASPECT;
	return makeRect(PLAYER_X, gPlayer.y, w, h);
}

// A smaller box around the body, not the whole sprite. The run cycle throws
// arms and legs well outside the torso, and being clipped by an outstretched
// hand feels wrong. Crucially it tracks the character's actual y, so a jump
// lifts the box clear of whatever is passing underneath.
Rect characterHitBox()
{
	Rect r = characterDrawRect();
	return makeRect(r.x + r.w * 0.27,   // trim the arm swing
	                r.y + r.h * 0.04,   // trim below the feet
	                r.w * 0.46,
	                r.h * 0.88);
}

void updateCharacter(double dt, bool controlsLive)
{
	// --- run ---
	// Holding the run key drives the character forward; releasing it slows the
	// street to a walk. The animation keeps turning over either way.
	gPlayer.isRunning = true;
	if (controlsLive) {
		if (isKeyPressed('d') || isKeyPressed('D') || isSpecialKeyPressed(GLUT_KEY_RIGHT))
			gPlayer.isRunning = true;
	}

	// --- jump ---
	if (controlsLive && !gPlayer.isJumping) {
		if (keyJustPressed(' ') ||
		    keyJustPressed('w') || keyJustPressed('W') ||
		    specialKeyJustPressed(GLUT_KEY_UP)) {
			gPlayer.velocityY = JUMP_VELOCITY;
			gPlayer.isJumping = true;
		}
	}

	if (gPlayer.isJumping) {
		gPlayer.velocityY -= GRAVITY * dt;
		gPlayer.y += gPlayer.velocityY * dt;

		if (gPlayer.y <= GROUND_Y) {     // landed
			gPlayer.y = GROUND_Y;
			gPlayer.velocityY = 0.0;
			gPlayer.isJumping = false;
		}
	}

	// --- the run cycle, always turning over ---
	gPlayer.frameTimer += dt;
	while (gPlayer.frameTimer >= PLAYER_FRAME_TIME) {
		gPlayer.frameTimer -= PLAYER_FRAME_TIME;
		gPlayer.frame = (gPlayer.frame + 1) % CHARACTER_FRAME_COUNT;
	}
}

void drawCharacter()
{
	Rect r = characterDrawRect();

	// Blink through immunity so it is obvious the hit registered.
	if (gInvuln > 0.0 && ((int)(gInvuln * 12.0) % 2) == 0)
		return;

	if (gShield > 0) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(0.55f, 0.78f, 1.0f, 0.28f);
		iFilledEllipse(r.x + r.w / 2.0, r.y + r.h / 2.0, r.w * 0.85, r.h * 0.62);
		glDisable(GL_BLEND);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}

	unsigned int tex = TEX_CHARACTER[gPlayer.frame];

	if (gHitFlash > 0.0)
		dImageEx(r.x, r.y, r.w, r.h, tex, false, 1.0, 0.35, 0.35, 1.0);
	else if (gHealFlash > 0.0)
		dImageEx(r.x, r.y, r.w, r.h, tex, false, 0.6, 1.0, 0.6, 1.0);
	else
		dImage(r.x, r.y, r.w, r.h, tex);
}

#endif // PLAYER_HPP

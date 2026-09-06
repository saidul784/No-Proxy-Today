//
//  LevelState.hpp  --  the state of a run and the rules that act on it.
//
//  Health, the single life, the shield, the coin score, the clock, and how far
//  along the road we are.
//
//  One distinction runs through this file: a WARNING sound is what an obstacle
//  plays as it approaches, once, so the player knows what is coming. A
//  COLLISION sound is what plays when it actually connects. They are separate
//  things, and the project supplies both, so both are used.
//
#ifndef LEVELSTATE_HPP
#define LEVELSTATE_HPP


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
#include "Audio.hpp"
#endif
#include <math.h>

enum LevelPhase {
	LP_READY = 0,       // short "get ready" before traffic starts
	LP_PLAYING,
	LP_PAUSED,
	LP_COMPLETE,        // reached Hatirjheel
	LP_GAMEOVER
};

enum ObstacleType {
	OB_BIKE = 0,
	OB_CAR,
	OB_DOG,
	OB_RICKSHAW,
	OB_BIRD,
	OB_TYPE_COUNT
};

// MCI aliases for the warning sounds, one per obstacle.
static const char *SFX_ALIAS[OB_TYPE_COUNT] = {
	"sfxbike",   // motorcycle horn
	"sfxcar",    // car horn
	"sfxdog",    // dog barking
	"sfxrick",   // rickshaw horn
	"sfxbird"    // birds chirping
};

static const int OB_DAMAGE[OB_TYPE_COUNT] = {
	DMG_BIKE, DMG_CAR, DMG_DOG, DMG_RICKSHAW, DMG_BIRD
};

static const double OB_HEIGHT[OB_TYPE_COUNT] = {
	OB_H_BIKE, OB_H_CAR, OB_H_DOG, OB_H_RICKSHAW, OB_H_BIRD
};

static const char *OB_NAME[OB_TYPE_COUNT] = {
	"bike", "car", "dog", "rickshaw", "birds"
};

// ---------------------------------------------------------------------------
//  Run state
// ---------------------------------------------------------------------------
LevelPhase gPhase       = LP_READY;
int        gHP          = PLAYER_START_HP;
int        gLives       = PLAYER_LIVES;
int        gShield      = 0;          // obstacles left to shrug off
int        gScore       = 0;          // coins only, 10 each
int        gCoinsTaken  = 0;
double     gTimeLeft    = LEVEL_TIME_LIMIT;
double     gProgress    = 0.0;        // pixels of road covered
double     gLevelLength = 21000.0;    // set on entry
double     gInvuln      = 0.0;
double     gHitFlash    = 0.0;
double     gShieldFlash = 0.0;
double     gHealFlash   = 0.0;
double     gReadyTimer  = 2.2;
int        gHitCount    = 0;
int        gDodgeCount  = 0;
const char *gEndReason  = "";

// Set once Level 01 is beaten. Level 02 does not exist yet; this is the flag
// it will hang off when it does.
bool gLevel01Cleared = false;

// ---------------------------------------------------------------------------
//  Rules
// ---------------------------------------------------------------------------

// The warning sound for an approaching obstacle. Called exactly once per
// obstacle, guarded by that obstacle's own flag -- never per frame.
void playWarningSound(int obstacleType)
{
	if (obstacleType < 0 || obstacleType >= OB_TYPE_COUNT) return;
	audioPlayOnce(SFX_ALIAS[obstacleType]);
}

// Every collision, whatever hit and whether or not the shield ate the damage.
// It sits at the top of the mix so it cuts through a horn still sounding.
void playCollisionSound()
{
	audioPlayOnce("sfxhit");
}

void collectCoin()
{
	gCoinsTaken++;
	gScore += COIN_VALUE;
}

// Power-up #1. Never takes health past the maximum.
void healCharacter(int amount)
{
	gHP += amount;
	if (gHP > PLAYER_MAX_HP) gHP = PLAYER_MAX_HP;
	gHealFlash = 0.5;
}

// Power-up #2. Arms protection for the next two obstacle collisions.
void activateShield()
{
	gShield = SHIELD_OBSTACLES;
	gShieldFlash = 0.5;
}

void gameOver(const char *reason)
{
	gPhase = LP_GAMEOVER;
	gEndReason = reason;
}

void levelComplete()
{
	gPhase = LP_COMPLETE;
	gLevel01Cleared = true;
}

// One collision, resolved once by the caller's hit flag.
//
// Immunity first, so a single object cannot drain the bar over consecutive
// frames; then the shield, which absorbs the hit without costing health; then
// real damage. There is only one life, so health reaching zero ends the run.
bool applyObstacleHit(int obstacleType)
{
	if (gInvuln > 0.0) return false;

	playCollisionSound();              // a collision happened either way

	if (gShield > 0) {
		gShield--;                     // counts as one protected obstacle
		gShieldFlash = 0.45;
		gInvuln = 0.6;
		return false;                  // no health lost
	}

	gHP -= OB_DAMAGE[obstacleType];
	if (gHP < 0) gHP = 0;
	if (gHP > PLAYER_MAX_HP) gHP = PLAYER_MAX_HP;

	gHitCount++;
	gInvuln = INVULN_TIME;
	gHitFlash = 0.35;

	if (gHP <= 0) {
		gLives = 0;
		gameOver("Your health ran out before Hatirjheel.");
	}

	return true;
}

void levelReset()
{
	gPhase       = LP_READY;
	gHP          = PLAYER_START_HP;
	gLives       = PLAYER_LIVES;
	gShield      = 0;
	gScore       = 0;
	gCoinsTaken  = 0;
	gTimeLeft    = LEVEL_TIME_LIMIT;
	gProgress    = 0.0;
	gInvuln      = 0.0;
	gHitFlash    = 0.0;
	gShieldFlash = 0.0;
	gHealFlash   = 0.0;
	gReadyTimer  = 2.2;
	gHitCount    = 0;
	gDodgeCount  = 0;
	gEndReason   = "";
}

double levelFraction()
{
	double f = gProgress / gLevelLength;
	if (f < 0.0) f = 0.0;
	if (f > 1.0) f = 1.0;
	return f;
}

double distanceToFinish()
{
	double d = gLevelLength - gProgress;
	return d < 0.0 ? 0.0 : d;
}

#endif // LEVELSTATE_HPP

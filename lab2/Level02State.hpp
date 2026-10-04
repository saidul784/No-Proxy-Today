//
//  Level02State.hpp  --  the state of a Level 02 run, and the rules that act
//                        on it.
//
//  This is Level 02's equivalent of LevelState.hpp, and it is deliberately
//  SEPARATE from it. Level 01 owns gPhase, gHP and gTimeLeft; Level 02 owns
//  gL02Phase, gL02HP and its own stage machine. Nothing here writes a Level 01
//  variable, so the two levels cannot disturb each other however they are
//  entered or restarted.
//
//  The one thing that IS shared is the score: coins on the river go through the
//  same collectCoin() and land in the same gScore, because that is the existing
//  scoring architecture and there is no reason for a second one.
//
//  The stage machine covers BOTH halves of the level: the river crossing
//  first, then the pirate ship. Its stages are listed in order below.
//
#ifndef LEVEL02STATE_HPP
#define LEVEL02STATE_HPP


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
#include "LevelState.hpp"
#endif
#include <math.h>

// ---------------------------------------------------------------------------
//  The stage machine
//
//  Every stage of Level 02, in the order it plays. The first five are the
//  river crossing and are unchanged; the rest are the pirate ship. Progress is
//  strictly forward and strictly in this order, which is what makes the battle
//  sequence impossible to skip: each stage names the one condition that ends
//  it, and nothing else can advance it.
// ---------------------------------------------------------------------------
enum Level02Stage {
	// ---- first half: the river ----
	L02_BOAT_TRAVEL = 0,       // rowing down the river
	L02_DRAGON_POWER_EVENT,    // the ball is in flight
	L02_BOAT_STOPPED,          // hit, boat halted, on his feet
	L02_CROCODILE_GROUP_1,     // two crocodiles
	L02_CROCODILE_GROUP_2,     // two more

	// ---- second half: the pirate ship ----
	// It used to stop dead after the fourth crocodile. It now rows on from
	// there. The five stages above are untouched.
	L02_BOAT_AFTER_CROCODILE,  // he sits back down and takes up the oar
	L02_PIRATE_COLLISION,      // the pirate ship rams the boat
	L02_PIRATE_INTERIOR,       // aboard, walking to the first chidori icon
	L02_DAKAT_BATTLE,          // six dakat, brought on two at a time
	L02_POWERUP,               // walking to the powerup after the sixth dies
	L02_DAKATLEADER_BATTLE,    // the boss: six powerful chidori
	L02_HATIRJHEEL_RETURN,     // back on the river, rowing, before the message
	L02_LEVEL_WON,             // the dakatleader is down

	L02_STAGE_COUNT
};

// Level 02's own phase, so restarting it never touches Level 01's gPhase.
enum Level02Phase {
	L02P_READY = 0,
	L02P_PLAYING,
	L02P_PAUSED,
	L02P_GAMEOVER,
	L02P_WON
};

// The shape of a throw, used by both halves: pressing the key raises the hand,
// the projectile leaves that hand when the wind-up finishes, then the hand
// comes back down. The boat half and the deck half each keep their own timers,
// but the three steps are the same, so the enum is shared.
enum ThrowPhase {
	THROW_IDLE = 0,
	THROW_WINDUP,     // hand raised, projectile not yet released
	THROW_RECOVER     // projectile gone, hand still out for a moment
};


// One MCI voice per crocodile, so two can growl at once without one cutting the
// other off. They are opened in main() alongside every other clip.
static const char *L02_CROC_ALIAS[MAX_CROCODILES] = {
	"sfxcroc0", "sfxcroc1", "sfxcroc2", "sfxcroc3"
};

// Sized by the enum, so adding a stage without adding its caption will not
// compile rather than reading off the end of the array at run time.
static const char *L02_STAGE_CAPTION[L02_STAGE_COUNT] = {
	"ROW FOR HATIRJHEEL",
	"SOMETHING IS COMING",
	"THE RIVER HAS STOPPED",
	"CROCODILES  -  GROUP 1",
	"CROCODILES  -  GROUP 2",

	"THE WATER IS CLEAR  -  ROW ON",
	"SHIP OFF THE BOW",
	"ABOARD  -  TAKE THE CHIDORI",
	"DAKAT",
	"TAKE THE POWERUP",
	"THE DAKATLEADER",
	"BACK TO HATIRJHEEL",
	"THE DAKATLEADER IS DOWN"
};

// ---------------------------------------------------------------------------
//  Run state
// ---------------------------------------------------------------------------
Level02Stage gL02Stage = L02_BOAT_TRAVEL;
Level02Phase gL02Phase = L02P_READY;

int    gL02HP         = LEVEL02_INITIAL_HP;
double gL02ReadyTimer = L02_READY_SECONDS;
double gL02StageTimer = 0.0;      // counts up inside the current stage
double gL02HitFlash   = 0.0;
double gL02DragonFlash = 0.0;     // the white flash when the ball lands
double gL02Scroll     = 0.0;      // how far the river has slid past
double gL02ScrollSpeed = RIVER_SCROLL_SPEED;
double gL02BannerTimer = 0.0;     // the stage caption, on screen briefly
double gL02RainPhase  = 0.0;

int  gL02CrocsKilled   = 0;
int  gL02ChidoriThrown = 0;
int  gL02BitesTaken    = 0;

// --- second half: the pirate ship ---
int  gL02DakatKilled  = 0;      // out of DAKAT_TOTAL
int  gL02DakatSpawned = 0;      // how many have been put on the deck so far
int  gL02LeaderHits   = 0;      // the boss counter, 0 / 6
int  gL02BoltsThrown  = 0;

// He cannot throw at will. This is how many chidori he is currently holding,
// and it only ever goes up by walking into an icon.
int  gL02Chidori      = 0;

// Set by the powerup, and only by the powerup. Until it is true the powerful
// chidori does not exist and the boss cannot be hurt.
bool gL02HasPowerful  = false;

const char *gL02EndReason = "";

// ---------------------------------------------------------------------------
//  Audio
//
//  The rain is started once, on entry, and is NOT stopped between stages -- it
//  runs under the rowing, the ball and both crocodile groups alike. Only
//  leaving the level silences it.
// ---------------------------------------------------------------------------
static bool gL02RainOn = false;

void level02RainStart()
{
	if (gL02RainOn) return;
	audioPlayLoop("rain");
	gL02RainOn = true;
}

void level02RainStop()
{
	if (!gL02RainOn) return;
	audioStop("rain");
	gL02RainOn = false;
}

// A crocodile's growl starts when it becomes active and stops when it dies.
void level02CrocSoundStart(int slot)
{
	if (slot < 0 || slot >= MAX_CROCODILES) return;
	audioPlayLoop(L02_CROC_ALIAS[slot]);
}

void level02CrocSoundStop(int slot)
{
	if (slot < 0 || slot >= MAX_CROCODILES) return;
	audioStop(L02_CROC_ALIAS[slot]);
}

void level02AllCrocSoundsStop()
{
	for (int i = 0; i < MAX_CROCODILES; i++)
		audioStop(L02_CROC_ALIAS[i]);
}

// ---------------------------------------------------------------------------
//  The pirate song
//
//  It starts once, when he boards the ship, and carries the whole second half:
//  the walk, all six dakat, the powerup and the boss, right through to the win
//  or the loss. It is never restarted between those -- level02PirateSongStart()
//  returns immediately if it is already running -- and the rain it replaces is
//  stopped at the same moment so the two do not play over each other.
//
//  Keeping it going is level02KeepMusicAlive()'s job. MCI's "repeat" flag is
//  advisory and some MPEGVideo builds simply stop at the end of the file, so
//  rather than trust it the device is asked what it is doing and restarted only
//  if it has actually stopped. That is the "if finished then play again" the
//  design asks for, without sending a command every frame.
// ---------------------------------------------------------------------------
static bool gL02PirateOn = false;
static double gL02MusicWatch = 0.0;

void level02PirateSongStart()
{
	if (gL02PirateOn) return;

	level02RainStop();               // one ambience at a time
	audioPlayLoop("piratesong");
	gL02PirateOn = true;
	gL02MusicWatch = 0.0;
}

void level02PirateSongStop()
{
	if (!gL02PirateOn) return;
	audioStop("piratesong");
	gL02PirateOn = false;
}

// Called every frame; acts about twice a second.
void level02KeepMusicAlive(double dt)
{
	if (!gL02PirateOn) return;

	gL02MusicWatch += dt;
	if (gL02MusicWatch < 0.5) return;
	gL02MusicWatch = 0.0;

	audioEnsureLooping("piratesong");
}

// ---------------------------------------------------------------------------
//  Rules
// ---------------------------------------------------------------------------
void level02GameOver(const char *reason)
{
	if (gL02Phase == L02P_GAMEOVER || gL02Phase == L02P_WON) return;

	gL02Phase = L02P_GAMEOVER;
	gL02EndReason = reason;

	level02AllCrocSoundsStop();
	highScoreEndAttempt(gScore);
	audioPlayOnce("losesnd");
}

// The other way the level can end. It is the mirror of level02GameOver(): the
// same shape, the other sting, and it is the ONLY place Level 02 is reported
// beaten -- which is what opens Level 03 on the campaign map.
void level02Win()
{
	if (gL02Phase == L02P_WON || gL02Phase == L02P_GAMEOVER) return;

	gL02Phase = L02P_WON;

	level02AllCrocSoundsStop();

	// Level 02 has its own ending clip -- "loose sound2" -- rather than the
	// winsnd Level 01 uses. It is fired exactly once, here, because this
	// function returns immediately if the phase has already been set.
	audioPlayOnce("winsnd2");

	// Marks the level through campaignMarkLevelComplete(), banks the score
	// and coins, and writes info.txt -- once. This function already returns
	// early if the phase is set, so it cannot be reached twice anyway.
	saveLevelComplete(2);
	highScoreEndAttempt(gScore);

	printf("[level02] LEVEL 02 COMPLETE -- level 03 unlocked\n");
}

// Health only ever leaves this level through here, so the clamp and the
// game-over check can never be forgotten at a call site.
void level02Damage(int amount)
{
	if (gL02Phase != L02P_PLAYING) return;

	gL02HP -= amount;
	if (gL02HP < 0) gL02HP = 0;
	if (gL02HP > LEVEL02_INITIAL_HP) gL02HP = LEVEL02_INITIAL_HP;

	gL02HitFlash = 0.35;

	// Every hit anywhere in the level goes through here, so this is also where
	// they are counted for the end screen.
	gL02BitesTaken++;

	if (gL02HP <= 0) {
		// The same lose system either half, but it should say what actually
		// killed him rather than always blaming the crocodiles.
		if (gL02Stage == L02_DAKATLEADER_BATTLE)
			level02GameOver("The dakatleader cut you down on his own deck.");
		else if (gL02Stage == L02_DAKAT_BATTLE)
			level02GameOver("The dakat took you before you reached their leader.");
		else
			level02GameOver("A crocodile pulled you under before Hatirjheel.");
	}
}

// One crocodile bite. The collision sound is the SAME thud Level 01 uses, at
// the same place in the mix, and it plays once per bite -- never per frame,
// which is what the caller's cooldown guarantees.
void level02CrocodileBite()
{
	playCollisionSound();
	level02Damage(CROCODILE_COLLISION_DAMAGE);   // which counts the hit too
}

// Moving to the next stage always resets the stage clock and re-arms the
// caption, so no caller has to remember to.
void level02SetStage(Level02Stage next)
{
	gL02Stage = next;
	gL02StageTimer = 0.0;
	gL02BannerTimer = 2.2;
	printf("[level02] stage -> %s\n", L02_STAGE_CAPTION[next]);
}

void level02ResetState()
{
	gL02Stage       = L02_BOAT_TRAVEL;
	gL02Phase       = L02P_READY;
	gL02HP          = LEVEL02_INITIAL_HP;
	gL02ReadyTimer  = L02_READY_SECONDS;
	gL02StageTimer  = 0.0;
	gL02HitFlash    = 0.0;
	gL02DragonFlash = 0.0;
	gL02Scroll      = 0.0;
	gL02ScrollSpeed = RIVER_SCROLL_SPEED;
	gL02BannerTimer = 2.2;
	gL02RainPhase   = 0.0;

	gL02CrocsKilled   = 0;
	gL02ChidoriThrown = 0;
	gL02BitesTaken    = 0;

	gL02DakatKilled   = 0;
	gL02DakatSpawned  = 0;
	gL02LeaderHits    = 0;
	gL02BoltsThrown   = 0;
	gL02Chidori       = 0;
	gL02HasPowerful   = false;

	gL02EndReason     = "";

	// Coins are scored through the shared architecture, so the shared counters
	// start this run from zero too.
	gScore      = 0;
	gCoinsTaken = 0;

	level02AllCrocSoundsStop();
}

double level02HealthFraction()
{
	double f = (double)gL02HP / (double)LEVEL02_INITIAL_HP;
	if (f < 0.0) f = 0.0;
	if (f > 1.0) f = 1.0;
	return f;
}

// True while the boat is still under way, which is the only time the river
// scrolls. He rows again after the crocodiles, right up until the pirate ship
// hits him, so those two stages count as well.
bool level02BoatIsMoving()
{
	return gL02Stage == L02_BOAT_TRAVEL ||
	       gL02Stage == L02_DRAGON_POWER_EVENT ||
	       gL02Stage == L02_BOAT_AFTER_CROCODILE ||
	       gL02Stage == L02_PIRATE_COLLISION ||
	       // and again at the end, rowing away from the pirate ship
	       gL02Stage == L02_HATIRJHEEL_RETURN ||
	       gL02Stage == L02_LEVEL_WON;
}

// True once the scene has cut to the inside of the pirate ship. From here to
// the end of the level the backdrop is boatside view.jpg and nothing else --
// walking, all three bird battles, the Demon Lord, the win and the loss.
bool level02OnDeck()
{
	// A RANGE, not a floor. The stages after the boss return to the river, so
	// ">= L02_PIRATE_INTERIOR" would have kept drawing the deck over them.
	return gL02Stage >= L02_PIRATE_INTERIOR &&
	       gL02Stage <= L02_DAKATLEADER_BATTLE;
}

// True while dakat are on the deck: the only time a thrown chidori can hit one
// and the only time one can cut him.
bool level02InDakatBattle()
{
	return gL02Stage == L02_DAKAT_BATTLE;
}

// True once he is aboard and under his own control -- walking, jumping and
// throwing all become available together.
bool level02DeckControlLive()
{
	return gL02Stage == L02_PIRATE_INTERIOR ||
	       gL02Stage == L02_DAKAT_BATTLE ||
	       gL02Stage == L02_POWERUP ||
	       gL02Stage == L02_DAKATLEADER_BATTLE;
}

// True on the way home: the pirate ship is behind him and the river is back.
bool level02Returning()
{
	return gL02Stage == L02_HATIRJHEEL_RETURN || gL02Stage == L02_LEVEL_WON;
}

// True once he is on his feet and able to throw.
bool level02CanThrow()
{
	return gL02Stage == L02_CROCODILE_GROUP_1 || gL02Stage == L02_CROCODILE_GROUP_2;
}

#endif // LEVEL02STATE_HPP

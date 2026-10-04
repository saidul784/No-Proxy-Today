//
//  Level03State.hpp  --  the state of a Level 03 run, and the rules on it.
//
//  Level 03's own equivalent of LevelState.hpp and Level02State.hpp, and like
//  them it is deliberately SEPARATE: Level 01 owns gPhase and gHP, Level 02
//  owns gL02Phase and gL02HP, and this owns gL03Phase and gL03HP. No level can
//  disturb another however they are entered or restarted.
//
//  The score is the one thing shared, through the game-wide gScore.
//
#ifndef LEVEL03STATE_HPP
#define LEVEL03STATE_HPP


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
#include "Paths.hpp"
#include "Audio.hpp"
#include "BackgroundScan.hpp"
#include "LevelState.hpp"
#endif
#include <math.h>

// ---------------------------------------------------------------------------
//  The stage machine
//
//  One walk with five set-pieces hung off it. Progress is strictly forward:
//  each stage names the single condition that ends it, so nothing can be
//  skipped and nothing can run out of order.
//
//  The L03_WALK_* stages are the road between set-pieces. Street traffic has
//  to be jumped during all of them, and during the set-pieces too, right up
//  until the jam.
// ---------------------------------------------------------------------------
enum Level03Stage {
	L03_WALK_TO_ROCK = 0,   // walking; the rock is waiting ahead
	L03_MICHIL,             // the protest march, seven rocks
	L03_WALK_TO_RAGGING,    // walking on
	L03_RAGGING,            // the conversation, twenty-two lines
	L03_WALK_TO_EVE,        // walking on
	L03_EVETEASING,         // five sticks
	L03_WALK_TO_CHOR,       // walking on
	L03_CHOR,               // he runs in, begs, and is slapped six times
	L03_WALK_TO_TRAFFIC,    // the last stretch of open road
	L03_TRAFFIC,            // up on the jam, under falling fire, to the far end
	L03_ARRIVE,             // down off the jam, running the last stretch to the gate
	L03_LEVEL_WON,          // WELCOME TO AUST

	L03_STAGE_COUNT
};

enum Level03Phase {
	L03P_READY = 0,
	L03P_PLAYING,
	L03P_PAUSED,
	L03P_GAMEOVER,
	L03P_WON
};

// What he is carrying. He can only throw what he has picked up, and the rock
// and the stick are picked up at different points in the level.
enum L03Carry {
	CARRY_NONE = 0,
	CARRY_ROCK,
	CARRY_STICK
};

// ---------------------------------------------------------------------------
//  The thief interaction
//
//  A small state machine of its own, hung inside the L03_CHOR stage rather
//  than bolted on beside it. It exists because for those few seconds the
//  character is not walking, not standing and not throwing: he is kneeling,
//  and then he is hitting. Driving that off the walk cycle would have him
//  stroll on the spot through the whole thing.
//
//      NONE      normal gameplay; nothing here applies
//      BEGGING   beg.png, beg.mpeg sounding, waiting for H
//      SLAP_UP   boy_01_with_fx.png -- he comes up and the hand lands
//      SLAP_HIT  boy_02.png         -- the follow-through
//      STANDING  on his feet between slaps, and until the thief has gone
//
//  H is read in exactly one place and only advances BEGGING or STANDING into
//  SLAP_UP, so it cannot restart a slap already running and does nothing at
//  all when there is no thief.
// ---------------------------------------------------------------------------
enum L03Interact {
	L03I_NONE = 0,
	L03I_BEGGING,
	L03I_SLAP_UP,
	L03I_SLAP_HIT,
	L03I_STANDING
};

static const char *L03_STAGE_CAPTION[L03_STAGE_COUNT] = {
	"KUNIPARA  -  WALK ON",
	"MICHIL  -  THROW THE ROCK",
	"THE ROAD IS CLEAR",
	"RAGGING",
	"KEEP WALKING",
	"EVE TEASING  -  THROW THE STICK",
	"KEEP WALKING",
	"CHOR  -  PRESS H",
	"TRAFFIC AHEAD",
	"ON THE JAM  -  D TO PUSH ON, DODGE THE FIRE",
	"AUST AHEAD  -  RUN FOR IT",
	"WELCOME TO AUST"
};

// ---------------------------------------------------------------------------
//  Run state
// ---------------------------------------------------------------------------
Level03Stage gL03Stage = L03_WALK_TO_ROCK;
Level03Phase gL03Phase = L03P_READY;

int    gL03HP         = LEVEL03_INITIAL_HP;
double gL03ReadyTimer = L03_READY_SECONDS;
double gL03StageTimer = 0.0;     // counts up inside the current stage
double gL03Progress   = 0.0;     // pixels of road covered
double gL03HitFlash   = 0.0;
double gL03Invuln     = 0.0;     // one hit at a time, never a drain
double gL03BannerTimer = 0.0;

L03Carry gL03Carry = CARRY_NONE;

L03Interact gL03Interact      = L03I_NONE;
double      gL03InteractTimer = 0.0;     // how long the current pose has left

// A swing in the air. The hit is NOT counted when H goes down -- it is counted
// when the hand lands, at the end of the windup -- so a swing has to remember
// whether it was a legitimate attempt while it travels.
//
//   gL03SlapPending  a swing is between the key press and the impact frame
//   gL03SlapValid    ...and it BEGAN while the opening was being advertised
//   gL03SlapRecover  the pause after swinging at nothing
bool   gL03SlapPending = false;
bool   gL03SlapValid   = false;
double gL03SlapRecover = 0.0;

int gL03MichilHits = 0;          // out of MICHIL_HITS_TO_CLEAR
int gL03EveHits    = 0;          // out of EVE_HITS_TO_CLEAR
int gL03Slaps      = 0;          // out of CHOR_SLAPS_TO_CLEAR
int gL03TalkLine   = 0;          // 1 .. TALK_LINE_COUNT, 0 before it starts
int gL03Thrown     = 0;
int gL03HitsTaken  = 0;

const char *gL03EndReason = "";

// ---------------------------------------------------------------------------
//  Audio
//
//  Three sources, all through the existing MCI wrapper:
//
//    "michil"  loops while the march is on screen and stops when it breaks up
//    "begsfx"  plays once when the thief starts pleading
//    "l03talk" is a SINGLE alias the whole twenty-two-line conversation shares.
//              Opening twenty-two permanent aliases would exhaust the table,
//              so audioPlayFileOnce swaps the file behind this one alias.
// ---------------------------------------------------------------------------
static bool gL03MichilOn = false;
static bool gL03BegSfxOn = false;

void level03MichilSoundStart()
{
	if (gL03MichilOn) return;
	audioPlayLoop("michil");
	gL03MichilOn = true;
}

void level03MichilSoundStop()
{
	if (!gL03MichilOn) return;
	audioStop("michil");
	gL03MichilOn = false;
}

// The plea. Once per thief, guarded by its own flag rather than by where it
// happens to be called from -- the begging state is entered once, but a state
// that is entered once today is a state that is entered twice tomorrow, and a
// clip restarted every frame is a horrible noise.
void level03BegSoundStart()
{
	if (gL03BegSfxOn) return;
	audioPlayOnce("begsfx");
	gL03BegSfxOn = true;
}

void level03BegSoundStop()
{
	if (!gL03BegSfxOn) return;
	audioStop("begsfx");
	gL03BegSfxOn = false;
}

// Speaks one line of the ragging conversation. Odd lines are the raggers, even
// lines are the character, so the two alternate simply by counting.
void level03SpeakLine(int line)
{
	if (line < 1 || line > TALK_LINE_COUNT) return;

	char rel[160], full[520];
	sprintf_s(rel, sizeof(rel), PATH_L03_TALK_FMT, line);
	assetPath(rel, full, sizeof(full));

	audioPlayFileOnce("l03talk", full, MIX_TALK);

	printf("[level03] line %2d / %d  (%s)\n", line, TALK_LINE_COUNT,
	       (line % 2) == 1 ? "ragger" : "character");
}

bool level03LineIsSpeaking()
{
	return audioIsPlaying("l03talk");
}

void level03SilenceAll()
{
	level03MichilSoundStop();
	level03BegSoundStop();
	audioStop("l03talk");
	audioStop("winsnd");
	audioStop("losesnd");
}

// ---------------------------------------------------------------------------
//  Rules
// ---------------------------------------------------------------------------
void level03GameOver(const char *reason)
{
	if (gL03Phase == L03P_GAMEOVER || gL03Phase == L03P_WON) return;

	gL03Phase = L03P_GAMEOVER;
	gL03EndReason = reason;

	level03MichilSoundStop();
	audioStop("l03talk");
	highScoreEndAttempt(gScore);
	audioPlayOnce("losesnd");
}

void level03Win()
{
	if (gL03Phase == L03P_WON || gL03Phase == L03P_GAMEOVER) return;

	gL03Phase = L03P_WON;

	level03MichilSoundStop();
	audioPlayOnce("winsnd");

	// Marks the level through campaignMarkLevelComplete(), banks the score
	// and coins, and writes info.txt -- once.
	saveLevelComplete(3);
	highScoreEndAttempt(gScore);

	printf("[level03] WELCOME TO AUST  --  level 03 complete\n");
}

// The single gate every Level 03 health change passes through, so the clamp,
// the immunity window and the game-over check can never be forgotten.
//
// The immunity matters: without it a vehicle he is standing inside would bill
// him every frame it overlapped, and the collision sound would machine-gun.
void level03Damage(int amount)
{
	if (gL03Phase != L03P_PLAYING) return;
	if (gL03Invuln > 0.0) return;

	playCollisionSound();

	gL03HP -= amount;
	if (gL03HP < 0) gL03HP = 0;

	gL03HitsTaken++;
	gL03Invuln = L03_HIT_COOLDOWN;
	gL03HitFlash = 0.35;

	if (gL03HP <= 0) {
		if (gL03Stage == L03_TRAFFIC)
			level03GameOver("The fire caught you on the roof of the jam.");
		else if (gL03Stage == L03_MICHIL)
			// Traffic is held off during the michil, so it was never the road.
			level03GameOver("The michil's petrol bomb caught you.");
		else if (gL03Stage == L03_EVETEASING)
			level03GameOver("The eve-teasers ran you down.");
		else
			level03GameOver("The traffic caught you before AUST.");
	}
}

void level03SetStage(Level03Stage next)
{
	gL03Stage = next;
	gL03StageTimer = 0.0;
	gL03BannerTimer = 2.2;
	printf("[level03] stage -> %s\n", L03_STAGE_CAPTION[next]);
}

void level03ResetState()
{
	gL03Stage       = L03_WALK_TO_ROCK;
	gL03Phase       = L03P_READY;
	gL03HP          = LEVEL03_INITIAL_HP;
	gL03ReadyTimer  = L03_READY_SECONDS;
	gL03StageTimer  = 0.0;
	gL03Progress    = 0.0;
	gL03HitFlash    = 0.0;
	gL03Invuln      = 0.0;
	gL03BannerTimer = 2.2;

	gL03Carry         = CARRY_NONE;
	gL03Interact      = L03I_NONE;
	gL03InteractTimer = 0.0;
	gL03SlapPending   = false;
	gL03SlapValid     = false;
	gL03SlapRecover   = 0.0;

	gL03MichilHits = 0;
	gL03EveHits    = 0;
	gL03Slaps      = 0;
	gL03TalkLine   = 0;
	gL03Thrown     = 0;
	gL03HitsTaken  = 0;
	gL03EndReason  = "";

	level03SilenceAll();
}

double level03HealthFraction()
{
	double f = (double)gL03HP / (double)LEVEL03_INITIAL_HP;
	if (f < 0.0) f = 0.0;
	if (f > 1.0) f = 1.0;
	return f;
}

// True on the stretches of open road between set-pieces: the only time the
// street actually scrolls past him.
bool level03IsWalking()
{
	return gL03Stage == L03_WALK_TO_ROCK ||
	       gL03Stage == L03_WALK_TO_RAGGING ||
	       gL03Stage == L03_WALK_TO_EVE ||
	       gL03Stage == L03_WALK_TO_CHOR ||
	       gL03Stage == L03_WALK_TO_TRAFFIC;
}

// Street traffic runs the whole level EXCEPT on the jam, where he is up off
// the road and the danger comes from above instead.
bool level03StreetTrafficLive()
{
	return gL03Stage < L03_TRAFFIC;
}

// ---------------------------------------------------------------------------
//  Where the road ends
//
//  The backdrops in level03/backround are the road: level03DrawRoad() walks
//  them one screen at a time off gL03Progress, so the last one coming up is
//  literally arriving at the far end of the street. That is AUST, and it is
//  what wins Level 03 -- a place reached, not a clock run down.
//
//  Reading it off gBg3Count rather than writing a number down means dropping
//  another backdrop into the folder lengthens the road by itself.
// ---------------------------------------------------------------------------
double level03RoadEnd()
{
	int screens = (gBg3Count > 1) ? gBg3Count : 2;
	return (double)(screens - 1) * (double)WIN_W;
}

// 0..1 along the whole road, for the KUNIPARA -> AUST bar.
double level03RoadFraction()
{
	double f = gL03Progress / level03RoadEnd();
	if (f < 0.0) f = 0.0;
	if (f > 1.0) f = 1.0;
	return f;
}

// ---------------------------------------------------------------------------
//  The thief interaction, and what it protects
// ---------------------------------------------------------------------------

// Anything other than normal gameplay: kneeling, or mid-slap, or on his feet
// waiting to swing again.
bool level03InteractionActive()
{
	return gL03Interact != L03I_NONE;
}

// A legitimate swing is still travelling towards him. The thief holds his
// opening open while this is true, so an attempt made on the last frame of the
// window is not cheated out of its own windup.
bool level03SlapPendingValid()
{
	return gL03SlapPending && gL03SlapValid;
}

// A slap is already playing. H is ignored while this is true, which is what
// stops a held or hammered key restarting the animation or counting twice.
bool level03SlapBusy()
{
	return gL03Interact == L03I_SLAP_UP || gL03Interact == L03I_SLAP_HIT;
}

// H is only ever a slap, and only in the one place a slap makes sense.
bool level03CanSlap()
{
	if (gL03Phase != L03P_PLAYING) return false;
	if (gL03Stage != L03_CHOR)     return false;
	if (level03SlapBusy())         return false;
	if (gL03SlapRecover > 0.0)     return false;   // still recovering from a miss

	return gL03Interact == L03I_BEGGING || gL03Interact == L03I_STANDING;
}

// The stages where the road must not be allowed to hit him:
//
//   RAGGING  he is pinned in a conversation with no controls at all
//   CHOR     the same, on his knees
//   MICHIL   he has a cocktail to jump and only one jump to do it with. A
//            bike arriving mid-dodge is not difficulty, it is a coin toss --
//            the jump that clears the bottle is the jump that cannot also
//            clear a rickshaw.
//   EVE      the same problem: the jump that clears the low rush cannot also
//            clear a car, and the rush is on a fixed cue he has to commit to.
//
// Held off, not switched off: l03UpdateTraffic() stops spawning, defuses
// whatever is already on the road so it drives away harmlessly, and parks the
// spawn timer on a fresh gap for when the set-piece ends. Everywhere else the
// traffic runs exactly as it always did, so standing still is still not safe.
bool level03SafeFromTraffic()
{
	return gL03Stage == L03_RAGGING ||
	       gL03Stage == L03_CHOR ||
	       gL03Stage == L03_MICHIL ||
	       gL03Stage == L03_EVETEASING;
}

// He is up on the roofs of the parked vehicles, driving his own x.
bool level03OnTheJam()
{
	return gL03Stage == L03_TRAFFIC;
}

// The last two stages, in front of bg_last.jpeg rather than the street: the
// run up to the gate, and standing under the sign with the banner up. The
// scrolling road and the parked jam are both finished by this point.
bool level03AtTheGate()
{
	return gL03Stage == L03_ARRIVE || gL03Stage == L03_LEVEL_WON;
}

// He can throw only what he is holding, and only at something worth hitting.
bool level03CanThrow()
{
	if (gL03Stage == L03_MICHIL)     return gL03Carry == CARRY_ROCK;
	if (gL03Stage == L03_EVETEASING) return gL03Carry == CARRY_STICK;
	return false;
}

#endif // LEVEL03STATE_HPP

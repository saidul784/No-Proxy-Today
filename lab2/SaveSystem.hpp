//
//  SaveSystem.hpp  --  the player's progress, kept in info.txt between runs.
//
//  A plain text file in KEY=VALUE lines, written with <fstream>, read back at
//  startup. Nothing binary, nothing versioned, no dependencies: the file can
//  be opened in Notepad and edited by hand, which is exactly what you want
//  while marking a project.
//
//  WHAT IT DOES NOT DO
//
//  It does not own any gameplay. Every number it writes is read out of a
//  variable the game already had:
//
//      gScore, gCoinsTaken   the CURRENT level's score. The existing design
//                            zeroes both at the start of every level -- see
//                            levelReset(), level02ResetState(), level03Start()
//                            -- so they are a per-level score, not a running
//                            total. This module therefore BANKS them into
//                            gTotalScore / gTotalCoins when a level is
//                            completed. Those two are new, because a career
//                            total genuinely did not exist before; they are
//                            not a second scoring system and nothing in the
//                            levels reads them.
//
//      gLives                Level 01's lives, and only Level 01's -- Levels
//                            02 and 03 do not use it. It is written to
//                            LIFEBOX as a RECORD of what was left when Level
//                            01 was beaten, and is never fed back into play:
//                            each level sets its own starting lives, and
//                            turning a temporary gameplay value into
//                            permanent state would change the difficulty.
//
//      gCampaign             the existing campaign struct in LevelSelect.hpp.
//                            Completion and unlock flags are saved from it and
//                            restored into it, and the only thing that ever
//                            SETS them is the existing
//                            campaignMarkLevelComplete() hook. There is no
//                            second unlock rule anywhere in this file.
//
//  WHERE THE FILE GOES
//
//  "info.txt", relative, so it lands in the process working directory. No
//  absolute path is hard-coded and the save travels with wherever the game is
//  run from. See the note on saveGameInfo() for what that means under VS2013.
//
#ifndef SAVESYSTEM_HPP
#define SAVESYSTEM_HPP


// ---------------------------------------------------------------------------
//  Visual Studio IntelliSense only -- this block is invisible to the compiler.
//
//  Every module is included from iMain.cpp in dependency order and the compiler
//  sees each one exactly once, so these includes are NOT needed to build. The
//  editor, however, parses this header on its own, and without them it marks
//  perfectly valid symbols in red. __INTELLISENSE__ is defined by the editor's
//  parser and never by cl.exe, so this costs the build nothing.
// ---------------------------------------------------------------------------
#ifdef __INTELLISENSE__
#ifndef IG_INTELLISENSE_BASE
#define IG_INTELLISENSE_BASE
#include "iGraphics.h"
#endif
#include "Config.hpp"
#include "LevelState.hpp"
#include "LevelSelect.hpp"
#endif

#include <fstream>
#include <string>
#include <sstream>

// Deliberately NOT "using namespace std". This project is a single translation
// unit that has already included <windows.h> through iGraphics.h, and pulling
// the whole of std into that scope is how you get ambiguous-symbol errors on
// names like min, max and byte. Everything below is qualified.

// Relative on purpose. See the comment on saveGameInfo().
const char SAVE_FILE_NAME[] = "info.txt";

// ---------------------------------------------------------------------------
//  The saved values
//
//  These four are the only state this module owns. Everything else it writes
//  is read straight out of gCampaign.
// ---------------------------------------------------------------------------
int gCurrentLevel  = 0;   // highest level completed so far; 0 = none yet
int gTotalScore    = 0;   // career score,  banked from gScore at each win
int gTotalCoins    = 0;   // career coins,  banked from gCoinsTaken at each win
int gSavedLifeBox  = 0;   // lives left when Level 01 was last beaten (a record)

// One save per completion, per attempt. A level's win condition can stay true
// for many frames -- gProgress >= gLevelLength does, right up until the player
// presses a key -- so without this the file would be rewritten sixty times a
// second for as long as the win screen is up. Cleared again by
// saveNoteLevelStarted() when that level is (re)started.
static bool gProgressSaved[4] = { false, false, false, false };

// ---------------------------------------------------------------------------
//  SAVING
//
//  Called on level completion and nowhere else. Never from iDraw(), never from
//  a mouse handler, never per frame.
//
//  WHERE VISUAL STUDIO 2013 PUTS THE FILE
//
//  std::ofstream with a relative name resolves against the process working
//  directory, which is not the same thing in every way of launching the game:
//
//      F5 / Ctrl-F5 from the IDE   the project directory -- the folder holding
//                                  lab2.vcxproj, i.e. ...\lab2\lab2\lab2\.
//                                  That is VS2013's default $(ProjectDir)
//                                  working directory, so this is where to look
//                                  for info.txt while testing.
//
//      double-clicking the exe     the folder the exe sits in, i.e.
//                                  ...\lab2\lab2\Debug\.
//
//  Both are correct: the save simply belongs to wherever the game was run
//  from. If you want one shared save for both, set the project's Debugging ->
//  Working Directory to the same folder in each case.
// ---------------------------------------------------------------------------
void saveGameInfo()
{
	std::ofstream file(SAVE_FILE_NAME);

	if (!file.is_open()) {
		printf("[save] could not write %s -- progress not saved\n", SAVE_FILE_NAME);
		return;
	}

	file << "# No Proxy Today -- saved progress. Plain text, safe to edit.\n";
	file << "CURRENT_LEVEL=" << gCurrentLevel << "\n";
	file << "TOTAL_SCORE=" << gTotalScore << "\n";
	file << "TOTAL_COINS=" << gTotalCoins << "\n";
	file << "LIFEBOX=" << gSavedLifeBox << "\n";
	file << "LEVEL01_COMPLETED=" << (gCampaign.level1Completed ? 1 : 0) << "\n";
	file << "LEVEL02_COMPLETED=" << (gCampaign.level2Completed ? 1 : 0) << "\n";
	file << "LEVEL03_COMPLETED=" << (gCampaign.level3Completed ? 1 : 0) << "\n";
	file << "LEVEL01_UNLOCKED=" << (gCampaign.level1Unlocked ? 1 : 0) << "\n";
	file << "LEVEL02_UNLOCKED=" << (gCampaign.level2Unlocked ? 1 : 0) << "\n";
	file << "LEVEL03_UNLOCKED=" << (gCampaign.level3Unlocked ? 1 : 0) << "\n";
	file << "GAME_COMPLETED="
	     << ((gCampaign.level1Completed &&
	          gCampaign.level2Completed &&
	          gCampaign.level3Completed) ? 1 : 0) << "\n";

	file.close();

	printf("[save] %s written  (level %d, score %d, coins %d)\n",
	       SAVE_FILE_NAME, gCurrentLevel, gTotalScore, gTotalCoins);
}

// ---------------------------------------------------------------------------
//  LOADING
// ---------------------------------------------------------------------------

// Strips spaces and stray carriage returns off both ends. A file edited by
// hand, or moved between Windows and anything else, picks up both.
static std::string saveTrim(const std::string &s)
{
	std::string::size_type a = 0;
	std::string::size_type b = s.size();

	while (a < b && (s[a] == ' ' || s[a] == '\t')) a++;
	while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t' ||
	                 s[b - 1] == '\r' || s[b - 1] == '\n')) b--;

	return s.substr(a, b - a);
}

// Everything back to a fresh career. Used when there is no file to read, and
// when the file is there but a key is missing from it.
static void saveApplyDefaults()
{
	gCurrentLevel = 0;
	gTotalScore   = 0;
	gTotalCoins   = 0;
	gSavedLifeBox = 0;

	gCampaign.level1Completed = false;
	gCampaign.level2Completed = false;
	gCampaign.level3Completed = false;

	// The UNLOCK defaults are deliberately left exactly as LevelSelect.hpp
	// declares them. That file is the one place that decides which levels are
	// open on a fresh save, and this one does not get a vote.
}

// Reads info.txt if it is there, and quietly starts a fresh career if it is
// not. Missing file, unreadable file, empty file, a line with no '=' on it, a
// key it has never heard of: none of those is an error and none of them stops
// the game starting.
void loadGameInfo()
{
	saveApplyDefaults();

	std::ifstream file(SAVE_FILE_NAME);

	if (!file.is_open()) {
		printf("[save] no %s yet -- starting a fresh career\n", SAVE_FILE_NAME);
		return;
	}

	std::string line;
	int read = 0;

	while (std::getline(file, line)) {
		line = saveTrim(line);

		if (line.empty()) continue;
		if (line[0] == '#') continue;            // a comment

		std::string::size_type eq = line.find('=');
		if (eq == std::string::npos) continue;   // not a KEY=VALUE line

		std::string key   = saveTrim(line.substr(0, eq));
		std::string value = saveTrim(line.substr(eq + 1));
		if (key.empty()) continue;

		int n = atoi(value.c_str());             // a bad value reads as 0
		read++;

		if      (key == "CURRENT_LEVEL")     gCurrentLevel = n;
		else if (key == "TOTAL_SCORE")       gTotalScore   = n;
		else if (key == "TOTAL_COINS")       gTotalCoins   = n;
		else if (key == "LIFEBOX")           gSavedLifeBox = n;
		else if (key == "LEVEL01_COMPLETED") gCampaign.level1Completed = (n != 0);
		else if (key == "LEVEL02_COMPLETED") gCampaign.level2Completed = (n != 0);
		else if (key == "LEVEL03_COMPLETED") gCampaign.level3Completed = (n != 0);
		else if (key == "LEVEL01_UNLOCKED")  gCampaign.level1Unlocked  = (n != 0);
		else if (key == "LEVEL02_UNLOCKED")  gCampaign.level2Unlocked  = (n != 0);
		else if (key == "LEVEL03_UNLOCKED")  gCampaign.level3Unlocked  = (n != 0);
		// GAME_COMPLETED is derived from the three completion flags when it is
		// written, so it is read past rather than stored.
	}

	file.close();

	// Level 01 raises gLevel01Cleared when it is beaten, and
	// levelSelectSyncProgress() reads that when the map opens. Putting the
	// loaded flag back means a Level 01 finished in an earlier session still
	// counts, through the same path a Level 01 finished just now does.
	if (gCampaign.level1Completed) gLevel01Cleared = true;

	// A save written before this rule existed, or edited by hand, could say a
	// level is complete but its successor is locked. Running the completions
	// back through the EXISTING hook rebuilds the unlocks from the one place
	// that is allowed to decide them.
	if (gCampaign.level1Completed) campaignMarkLevelComplete(1);
	if (gCampaign.level2Completed) campaignMarkLevelComplete(2);
	if (gCampaign.level3Completed) campaignMarkLevelComplete(3);

	printf("[save] %s loaded (%d keys): level %d, score %d, coins %d, "
	       "completed %d%d%d\n",
	       SAVE_FILE_NAME, read, gCurrentLevel, gTotalScore, gTotalCoins,
	       gCampaign.level1Completed ? 1 : 0,
	       gCampaign.level2Completed ? 1 : 0,
	       gCampaign.level3Completed ? 1 : 0);
}

// ---------------------------------------------------------------------------
//  THE TWO HOOKS THE LEVELS CALL
// ---------------------------------------------------------------------------

// A level is starting or being restarted: it may bank a result again.
void saveNoteLevelStarted(int level)
{
	if (level < 1 || level > 3) return;
	gProgressSaved[level] = false;
}

// A level has been completed. One call does the lot: marks it through the
// existing campaign hook, banks the level's score and coins into the career
// totals, and writes the file -- once.
void saveLevelComplete(int level)
{
	if (level < 1 || level > 3) return;
	if (gProgressSaved[level]) return;      // already banked this attempt

	gProgressSaved[level] = true;

	// The existing unlock rule, called the existing way. This module never
	// sets an unlock flag itself.
	campaignMarkLevelComplete(level);

	// gScore and gCoinsTaken are this level's, and the next level zeroes them,
	// so this is the moment they are worth anything.
	gTotalScore += gScore;
	gTotalCoins += gCoinsTaken;

	// Lives belong to Level 01 and are meaningless in the other two, so only
	// Level 01 writes the record. It is never read back into play.
	if (level == 1) gSavedLifeBox = gLives;

	if (level > gCurrentLevel) gCurrentLevel = level;

	printf("[save] LEVEL %02d COMPLETE -- banking %d score, %d coins\n",
	       level, gScore, gCoinsTaken);

	saveGameInfo();
}

#endif // SAVESYSTEM_HPP

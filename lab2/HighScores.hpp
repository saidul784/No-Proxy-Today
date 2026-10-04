//
//  HighScores.hpp  --  five best scores per level, kept between runs.
//
//  A SEPARATE store from the campaign save. info.txt holds progress -- which
//  levels are done, the career totals, the unlocks -- and this holds a
//  leaderboard. Keeping them apart means a corrupt or hand-edited leaderboard
//  can never cost anyone their unlocks, and the two can be deleted
//  independently.
//
//  WHAT A RECORD IS
//
//  One row per PLAYER per LEVEL: a personal best, not an attempt log. A
//  player's row is updated when they beat it, and left alone when they do
//  worse. Names are matched case-insensitively, so "noyon" and "Noyon" are the
//  same person, while the name is displayed exactly as it was typed.
//
//  ORDERING
//
//  Score descending; on a tie the more recent record ranks first. Recency is
//  the `stamp` field, which is a real time(NULL) but is forced upward if the
//  clock would not produce a strictly newer value -- so two records made in the
//  same second still order correctly, and a wound-back clock cannot make a new
//  record look old.
//
#ifndef HIGHSCORES_HPP
#define HIGHSCORES_HPP


// ---------------------------------------------------------------------------
//  Visual Studio IntelliSense only -- this block is invisible to the compiler.
// ---------------------------------------------------------------------------
#ifdef __INTELLISENSE__
#ifndef IG_INTELLISENSE_BASE
#define IG_INTELLISENSE_BASE
#include "iGraphics.h"
#endif
#include "Config.hpp"
#include "LevelState.hpp"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

const int  HS_LEVELS      = 3;
const int  HS_MAX_ROWS    = 5;     // five per level, and not six
const int  HS_NAME_MAX    = 20;    // characters the player may type
const char HS_FILE_NAME[] = "highscores.txt";
const char HS_TEMP_NAME[] = "highscores.tmp";

struct ScoreRow {
	char          display[HS_NAME_MAX + 1];   // exactly as typed
	char          key[HS_NAME_MAX + 1];       // lower-cased, for matching
	int           score;
	unsigned long stamp;                      // bigger = more recent
};

static ScoreRow gHsRows[HS_LEVELS][HS_MAX_ROWS];
static int      gHsCount[HS_LEVELS] = { 0, 0, 0 };
static bool     gHsDirty = false;

// The name the player confirmed for this run. Remembered for the session so
// the entry screen can prefill it.
char gPlayerName[HS_NAME_MAX + 1] = "";

// ---------------------------------------------------------------------------
//  Names
// ---------------------------------------------------------------------------

// Trims both ends and copies at most HS_NAME_MAX characters.
void hsTrimCopy(const char *in, char *out, int outSize)
{
	if (out == 0 || outSize <= 0) return;
	out[0] = '\0';
	if (in == 0) return;

	int a = 0;
	while (in[a] == ' ' || in[a] == '\t') a++;

	int b = (int)strlen(in);
	while (b > a && (in[b - 1] == ' ' || in[b - 1] == '\t' ||
	                 in[b - 1] == '\r' || in[b - 1] == '\n')) b--;

	int n = b - a;
	if (n > outSize - 1) n = outSize - 1;
	if (n < 0) n = 0;

	memcpy(out, in + a, n);
	out[n] = '\0';
}

static void hsMakeKey(const char *display, char *key, int keySize)
{
	int i = 0;
	for (; display[i] != '\0' && i < keySize - 1; i++) {
		char c = display[i];
		if (c >= 'A' && c <= 'Z') c = (char)(c - 'A' + 'a');
		key[i] = c;
	}
	key[i] = '\0';
}

// Anything that would confuse the file or cannot be drawn is refused at entry
// time rather than mangled later.
bool hsCharIsAllowed(char c)
{
	return c >= 32 && c <= 126;
}

bool hsNameIsUsable(const char *raw)
{
	char t[HS_NAME_MAX + 1];
	hsTrimCopy(raw, t, sizeof(t));
	return t[0] != '\0';
}

// ---------------------------------------------------------------------------
//  Ordering
// ---------------------------------------------------------------------------

// Higher score first; on a tie the more recent one first.
static bool hsBeats(const ScoreRow &a, const ScoreRow &b)
{
	if (a.score != b.score) return a.score > b.score;
	return a.stamp > b.stamp;
}

static void hsSortLevel(int lv)
{
	// Insertion sort: five rows, and it is stable enough to reason about.
	for (int i = 1; i < gHsCount[lv]; i++) {
		ScoreRow tmp = gHsRows[lv][i];
		int j = i - 1;
		while (j >= 0 && hsBeats(tmp, gHsRows[lv][j])) {
			gHsRows[lv][j + 1] = gHsRows[lv][j];
			j--;
		}
		gHsRows[lv][j + 1] = tmp;
	}
}

// A stamp strictly newer than everything already stored, so ties order by
// when they happened even if the clock stands still or moves backwards.
static unsigned long hsNextStamp()
{
	unsigned long now = (unsigned long)time(NULL);
	unsigned long high = 0;

	for (int lv = 0; lv < HS_LEVELS; lv++)
		for (int i = 0; i < gHsCount[lv]; i++)
			if (gHsRows[lv][i].stamp > high) high = gHsRows[lv][i].stamp;

	if (now <= high) now = high + 1;
	return now;
}

// ---------------------------------------------------------------------------
//  Saving  --  written to a temporary file and renamed over the real one, so
//  an interrupted write cannot leave a half-file where the records were.
// ---------------------------------------------------------------------------
void highScoresSave()
{
	if (!gHsDirty) return;

	FILE *f = fopen(HS_TEMP_NAME, "w");
	if (f == 0) {
		printf("[scores] could not open %s -- leaderboard not saved\n", HS_TEMP_NAME);
		return;
	}

	fprintf(f, "# No Proxy Today -- high scores. Plain text, safe to edit.\n");
	fprintf(f, "# SCORE=<level 1-3> <stamp> <points> <name to end of line>\n");

	for (int lv = 0; lv < HS_LEVELS; lv++)
		for (int i = 0; i < gHsCount[lv]; i++) {
			const ScoreRow &r = gHsRows[lv][i];
			fprintf(f, "SCORE=%d %lu %d %s\n", lv + 1, r.stamp, r.score, r.display);
		}

	if (fclose(f) != 0) {
		printf("[scores] write failed -- leaderboard not saved\n");
		return;
	}

	// rename() will not replace an existing file on Windows, so the old one
	// goes first. The temp file is already complete and closed by this point.
	remove(HS_FILE_NAME);
	if (rename(HS_TEMP_NAME, HS_FILE_NAME) != 0) {
		printf("[scores] could not replace %s\n", HS_FILE_NAME);
		return;
	}

	gHsDirty = false;
	printf("[scores] %s written\n", HS_FILE_NAME);
}

// ---------------------------------------------------------------------------
//  Loading
//
//  A missing file is not an error: it means nobody has played yet. Anything
//  malformed is skipped rather than guessed at, and one bad line never stops
//  the rest from loading.
// ---------------------------------------------------------------------------
void highScoresLoad()
{
	for (int lv = 0; lv < HS_LEVELS; lv++) gHsCount[lv] = 0;
	gHsDirty = false;

	FILE *f = fopen(HS_FILE_NAME, "r");
	if (f == 0) {
		printf("[scores] no %s yet -- leaderboards start empty\n", HS_FILE_NAME);
		return;
	}

	char line[256];
	int kept = 0, skipped = 0;

	while (fgets(line, sizeof(line), f) != 0) {
		if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
		if (strncmp(line, "SCORE=", 6) != 0) { skipped++; continue; }

		int lvl = 0, score = 0, used = 0;
		unsigned long stamp = 0;

		if (sscanf(line + 6, "%d %lu %d %n", &lvl, &stamp, &score, &used) < 3) {
			skipped++;
			continue;
		}

		if (lvl < 1 || lvl > HS_LEVELS || score < 0) { skipped++; continue; }

		char name[HS_NAME_MAX + 1];
		hsTrimCopy(line + 6 + used, name, sizeof(name));
		if (name[0] == '\0') { skipped++; continue; }

		int lv = lvl - 1;
		if (gHsCount[lv] >= HS_MAX_ROWS) { skipped++; continue; }

		ScoreRow &r = gHsRows[lv][gHsCount[lv]];
		strcpy_s(r.display, sizeof(r.display), name);
		hsMakeKey(r.display, r.key, sizeof(r.key));
		r.score = score;
		r.stamp = stamp;
		gHsCount[lv]++;
		kept++;
	}

	fclose(f);

	for (int lv = 0; lv < HS_LEVELS; lv++) hsSortLevel(lv);

	printf("[scores] %s loaded: %d records (%d skipped)\n",
	       HS_FILE_NAME, kept, skipped);
}

// ---------------------------------------------------------------------------
//  Submitting
//
//  level is 1..3. Returns true if the board changed.
//
//  One row per player per level. An existing row is raised, never added to,
//  and a worse attempt leaves it completely alone -- including its stamp, so
//  losing does not make an old record look freshly set.
// ---------------------------------------------------------------------------
bool highScoreSubmit(int level, const char *rawName, int score)
{
	if (level < 1 || level > HS_LEVELS) return false;
	if (score <= 0) return false;              // opening a level is not a score

	char name[HS_NAME_MAX + 1];
	hsTrimCopy(rawName, name, sizeof(name));
	if (name[0] == '\0') return false;

	char key[HS_NAME_MAX + 1];
	hsMakeKey(name, key, sizeof(key));

	const int lv = level - 1;

	// --- already on the board? ---
	for (int i = 0; i < gHsCount[lv]; i++) {
		if (strcmp(gHsRows[lv][i].key, key) != 0) continue;

		if (score > gHsRows[lv][i].score) {
			gHsRows[lv][i].score = score;
			gHsRows[lv][i].stamp = hsNextStamp();
			strcpy_s(gHsRows[lv][i].display, sizeof(gHsRows[lv][i].display), name);
			hsSortLevel(lv);
			gHsDirty = true;
			printf("[scores] LEVEL %d: %s improved to %d\n", level, name, score);
			return true;
		}

		if (score == gHsRows[lv][i].score) {
			// Matching your own best counts as achieving it again, so it moves
			// ahead of equal scores set earlier.
			gHsRows[lv][i].stamp = hsNextStamp();
			hsSortLevel(lv);
			gHsDirty = true;
			return true;
		}

		return false;                          // worse: record untouched
	}

	// --- a new player ---
	ScoreRow fresh;
	strcpy_s(fresh.display, sizeof(fresh.display), name);
	hsMakeKey(fresh.display, fresh.key, sizeof(fresh.key));
	fresh.score = score;
	fresh.stamp = hsNextStamp();

	if (gHsCount[lv] < HS_MAX_ROWS) {
		gHsRows[lv][gHsCount[lv]] = fresh;
		gHsCount[lv]++;
		hsSortLevel(lv);
		gHsDirty = true;
		printf("[scores] LEVEL %d: %s enters with %d\n", level, name, score);
		return true;
	}

	// Board full: only a score that beats the current last place gets on, and
	// it takes that place rather than being appended.
	ScoreRow &last = gHsRows[lv][HS_MAX_ROWS - 1];
	if (!hsBeats(fresh, last)) return false;

	printf("[scores] LEVEL %d: %s (%d) knocks out %s (%d)\n",
	       level, fresh.display, fresh.score, last.display, last.score);

	last = fresh;
	hsSortLevel(lv);
	gHsDirty = true;
	return true;
}

// ---------------------------------------------------------------------------
//  One attempt, recorded once
//
//  An attempt opens when a level starts and closes on the first thing that
//  ends it -- winning, dying, restarting, or walking out through ESC. Closing
//  is what submits the score, and it can only happen once, so the several
//  end-of-run paths that can all fire for a single run cannot produce several
//  records or refresh a stamp twice.
// ---------------------------------------------------------------------------
static int  gHsAttemptLevel = 0;      // 1..3, or 0 when nothing is running
static bool gHsAttemptOpen  = false;

void highScoreBeginAttempt(int level)
{
	gHsAttemptLevel = level;
	gHsAttemptOpen = (level >= 1 && level <= HS_LEVELS);
}

// Submits the score this attempt earned, if any, and saves if the board moved.
void highScoreEndAttempt(int score)
{
	if (!gHsAttemptOpen) return;

	int level = gHsAttemptLevel;
	gHsAttemptOpen = false;
	gHsAttemptLevel = 0;

	if (highScoreSubmit(level, gPlayerName, score))
		highScoresSave();              // a save point, not a per-frame write
}

// ---------------------------------------------------------------------------
//  Reading the board, for the leaderboard screen
// ---------------------------------------------------------------------------
int highScoreCount(int level)
{
	if (level < 1 || level > HS_LEVELS) return 0;
	return gHsCount[level - 1];
}

const ScoreRow *highScoreRow(int level, int rank)
{
	if (level < 1 || level > HS_LEVELS) return 0;
	if (rank < 0 || rank >= gHsCount[level - 1]) return 0;
	return &gHsRows[level - 1][rank];
}

#endif // HIGHSCORES_HPP

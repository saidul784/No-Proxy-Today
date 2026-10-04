//
//  Audio.hpp  --  a thin, safe wrapper over the MCI string interface.
//
//  iGraphics ships no audio API, so everything goes through mciSendString from
//  <windows.h> (already pulled in by iGraphics.h).
//
//  Why "type mpegvideo": MCI picks its device from the file EXTENSION. Four of
//  this project's clips are named *.mpeg but are really plain MP3 files, and
//  MCI will not open them by extension alone. Naming the MPEGVideo (DirectShow)
//  device explicitly makes both .mp3 and .mpeg open correctly. We still fall
//  back to a plain open if that fails.
//
//  Each clip also has a place in the mix (audioSetMix), because MCI tops out at
//  volume 1000: the only way to make the collision thud stand out is to hold
//  the horns and the music below it. Muting and unmuting restores that balance
//  rather than flattening everything back to full.
//
#ifndef AUDIO_HPP
#define AUDIO_HPP


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
#endif
#include <string.h>

// glut32.lib usually drags winmm in already, but MCI is what actually needs it
// -- ask for it explicitly so the link never depends on that side effect.
#pragma comment(lib, "winmm.lib")

// Level 01 opens ten of these; Level 02 adds the rain and one crocodile voice
// per crocodile, which is fifteen. The headroom is for the levels after it.
const int AUDIO_MAX_ALIASES = 24;

static char gAudioAliases[AUDIO_MAX_ALIASES][32];
static int  gAudioMix[AUDIO_MAX_ALIASES];   // each clip's place in the mix, 0-1000
static int  gAudioAliasCount = 0;
static bool gAudioMuted = false;

static void audioRemember(const char *alias)
{
	if (gAudioAliasCount >= AUDIO_MAX_ALIASES) return;
	strcpy_s(gAudioAliases[gAudioAliasCount], sizeof(gAudioAliases[0]), alias);
	gAudioMix[gAudioAliasCount] = 1000;      // full, until the mix says otherwise
	gAudioAliasCount++;
}

static int audioFindAlias(const char *alias)
{
	for (int i = 0; i < gAudioAliasCount; i++)
		if (strcmp(gAudioAliases[i], alias) == 0) return i;
	return -1;
}

// Opens fullPath under the given alias. Returns false (and logs) on failure so
// a missing sound never takes the game down with it.
bool audioOpen(const char *alias, const char *fullPath)
{
	char cmd[600];
	MCIERROR err;

	sprintf_s(cmd, sizeof(cmd), "open \"%s\" type mpegvideo alias %s", fullPath, alias);
	err = mciSendStringA(cmd, NULL, 0, NULL);

	if (err != 0) {
		// Second chance: let MCI guess the device from the extension.
		sprintf_s(cmd, sizeof(cmd), "open \"%s\" alias %s", fullPath, alias);
		err = mciSendStringA(cmd, NULL, 0, NULL);
	}

	if (err != 0) {
		char msg[256] = "";
		mciGetErrorStringA(err, msg, sizeof(msg));
		printf("[audio] FAILED to open %s  (alias %s)\n", fullPath, alias);
		printf("[audio]   %s\n", msg);
		return false;
	}

	audioRemember(alias);
	printf("[audio] opened %-10s -> %s\n", alias, fullPath);
	return true;
}

// Restarts the clip from the beginning. Correct for one-shot sound effects.
void audioPlayOnce(const char *alias)
{
	char cmd[128];
	sprintf_s(cmd, sizeof(cmd), "play %s from 0", alias);
	mciSendStringA(cmd, NULL, 0, NULL);
}

// Loops the clip. A silent loop is hard to notice and easy to misdiagnose, so
// this one reports failure and falls back to the plainer command form.
void audioPlayLoop(const char *alias)
{
	char cmd[128];

	sprintf_s(cmd, sizeof(cmd), "play %s from 0 repeat", alias);
	MCIERROR err = mciSendStringA(cmd, NULL, 0, NULL);

	if (err != 0) {
		sprintf_s(cmd, sizeof(cmd), "play %s repeat", alias);
		err = mciSendStringA(cmd, NULL, 0, NULL);
	}

	if (err != 0) {
		char msg[256] = "";
		mciGetErrorStringA(err, msg, sizeof(msg));
		printf("[audio] could not loop %s: %s\n", alias, msg);
	} else {
		printf("[audio] looping %s\n", alias);
	}
}

// ---------------------------------------------------------------------------
//  Loop watchdog
//
//  "play ... repeat" is what SHOULD keep a clip going round, but MCI's repeat
//  flag is advisory: some MPEGVideo builds honour it and some quietly stop at
//  the end of the file, which leaves a level that should have music playing in
//  silence from then on.
//
//  This asks the device what it is actually doing and starts it again if it has
//  stopped. It is safe to call often -- it only sends a play command when the
//  clip is genuinely no longer playing, so it never restarts a song mid-way and
//  never issues a command every frame.
// ---------------------------------------------------------------------------
void audioEnsureLooping(const char *alias)
{
	char cmd[128];
	char mode[64] = "";

	sprintf_s(cmd, sizeof(cmd), "status %s mode", alias);
	if (mciSendStringA(cmd, mode, sizeof(mode), NULL) != 0)
		return;                        // never opened; nothing to keep alive

	if (strcmp(mode, "playing") == 0)
		return;                        // still going, leave it alone

	if (strcmp(mode, "paused") == 0)
		return;                        // deliberately paused, leave it alone

	sprintf_s(cmd, sizeof(cmd), "play %s from 0 repeat", alias);
	if (mciSendStringA(cmd, NULL, 0, NULL) != 0) {
		sprintf_s(cmd, sizeof(cmd), "play %s from 0", alias);
		mciSendStringA(cmd, NULL, 0, NULL);
	}
}

// True while the clip is actually sounding. Used to drive a conversation: the
// next line starts when the previous one has finished, not on a guessed timer.
bool audioIsPlaying(const char *alias)
{
	char cmd[128];
	char mode[64] = "";

	sprintf_s(cmd, sizeof(cmd), "status %s mode", alias);
	if (mciSendStringA(cmd, mode, sizeof(mode), NULL) != 0) return false;

	return strcmp(mode, "playing") == 0;
}

// Plays ONE file through a reusable alias: closes whatever that alias held,
// opens this file, plays it once.
//
// Level 03's conversation is twenty-two separate clips. Opening twenty-two
// permanent MCI aliases would blow past AUDIO_MAX_ALIASES and leave nothing for
// the rest of the game, so the whole conversation shares a single alias and
// swaps the file behind it line by line.
void audioPlayFileOnce(const char *alias, const char *fullPath, int mix)
{
	char cmd[600];

	MCIERROR err;

	sprintf_s(cmd, sizeof(cmd), "close %s", alias);
	err = mciSendStringA(cmd, NULL, 0, NULL);
	if (err != 0) {
		char msg[256] = "";
		mciGetErrorStringA(err, msg, sizeof(msg));
		printf("[audio] close %s -> %s\n", alias, msg);
	}

	sprintf_s(cmd, sizeof(cmd), "open \"%s\" type mpegvideo alias %s", fullPath, alias);
	err = mciSendStringA(cmd, NULL, 0, NULL);
	if (err != 0) {
		char msg[256] = "";
		mciGetErrorStringA(err, msg, sizeof(msg));
		printf("[audio] open(mpegvideo) failed: %s\n", msg);

		sprintf_s(cmd, sizeof(cmd), "open \"%s\" alias %s", fullPath, alias);
		err = mciSendStringA(cmd, NULL, 0, NULL);
		if (err != 0) {
			mciGetErrorStringA(err, msg, sizeof(msg));
			printf("[audio] could not open %s: %s\n", fullPath, msg);
			return;
		}
	}

	if (mix > 0) {
		sprintf_s(cmd, sizeof(cmd), "setaudio %s volume to %d", alias, mix);
		mciSendStringA(cmd, NULL, 0, NULL);
	}

	sprintf_s(cmd, sizeof(cmd), "play %s from 0", alias);
	mciSendStringA(cmd, NULL, 0, NULL);
}

void audioStop(const char *alias)
{
	char cmd[128];
	sprintf_s(cmd, sizeof(cmd), "stop %s", alias);
	mciSendStringA(cmd, NULL, 0, NULL);
}

void audioPause(const char *alias)
{
	char cmd[128];
	sprintf_s(cmd, sizeof(cmd), "pause %s", alias);
	mciSendStringA(cmd, NULL, 0, NULL);
}

void audioResume(const char *alias)
{
	char cmd[128];
	sprintf_s(cmd, sizeof(cmd), "resume %s", alias);
	mciSendStringA(cmd, NULL, 0, NULL);
}

// volume is 0..1000. This sets the device level right now and does not touch
// the remembered mix, so muting and unmuting still restores the balance.
void audioSetVolume(const char *alias, int volume)
{
	char cmd[128];
	sprintf_s(cmd, sizeof(cmd), "setaudio %s volume to %d", alias, volume);
	mciSendStringA(cmd, NULL, 0, NULL);
}

// Fixes where a clip sits in the mix, and applies it. 1000 is as loud as MCI
// goes, so making one sound stand out means holding the others down rather
// than pushing that one up.
void audioSetMix(const char *alias, int volume)
{
	int i = audioFindAlias(alias);
	if (i < 0) return;                 // never opened; nothing to balance

	gAudioMix[i] = volume;
	if (gAudioMuted) return;

	char cmd[128];
	sprintf_s(cmd, sizeof(cmd), "setaudio %s volume to %d", alias, volume);
	MCIERROR err = mciSendStringA(cmd, NULL, 0, NULL);

	// Not every MCI device implements a volume control. If one refuses, say so
	// rather than leaving the balance silently wrong.
	if (err != 0) {
		char msg[256] = "";
		mciGetErrorStringA(err, msg, sizeof(msg));
		printf("[audio] %s will not take a volume (%s)\n", alias, msg);
	} else {
		printf("[audio] mix %-10s %d\n", alias, volume);
	}
}

bool audioIsMuted()
{
	return gAudioMuted;
}

void audioToggleMute()
{
	gAudioMuted = !gAudioMuted;
	for (int i = 0; i < gAudioAliasCount; i++)
		audioSetVolume(gAudioAliases[i], gAudioMuted ? 0 : gAudioMix[i]);
}

// Frees every MCI device we opened. Call before exit().
void audioCloseAll()
{
	for (int i = 0; i < gAudioAliasCount; i++) {
		char cmd[128];
		sprintf_s(cmd, sizeof(cmd), "close %s", gAudioAliases[i]);
		mciSendStringA(cmd, NULL, 0, NULL);
	}
	gAudioAliasCount = 0;
}

#endif // AUDIO_HPP

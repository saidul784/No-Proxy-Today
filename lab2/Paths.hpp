//
//  Paths.hpp  --  finds the asset folders no matter where the .exe is launched
//                 from.
//
//  The folder layout of this project is:
//
//      lab2\                  <-- solution root: lab2.sln + ALL new assets
//        |  game_poster.jpeg
//        |  backround images\ , characters_by_sequence\ , game_assetes\
//        +- Debug\            <-- lab2.exe lands here
//        +- lab2\             <-- project dir: iMain.cpp + Audios\
//
//  So the images live one level ABOVE the project dir, while Audios\ lives
//  INSIDE it. Visual Studio runs the exe with cwd = project dir, but a
//  double-clicked exe runs with cwd = Debug. Rather than hard-coding "..\",
//  we probe for a sentinel file at start-up and remember what worked.
//
#ifndef PATHS_HPP
#define PATHS_HPP


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
#include <stdio.h>
#include <string.h>

static char gAssetRoot[260] = "";   // folder holding game_poster.jpeg
static char gAudioRoot[260] = "";   // folder holding Audios\

static bool pathFileExists(const char *p)
{
	FILE *f = fopen(p, "rb");
	if (f) { fclose(f); return true; }
	return false;
}

// Tries each candidate prefix until sentinel is found underneath it.
static void pathFindRoot(const char *sentinel, char *out, int outSize, const char *label)
{
	static const char *candidates[] = {
		"",                 // cwd is already the right folder
		"../",              // cwd = project dir, or cwd = Debug
		"lab2/",            // cwd = solution root, looking for Audios
		"../lab2/",         // cwd = Debug, looking for Audios
		"../../",
		"lab2/lab2/",
		"../lab2/lab2/"
	};
	const int n = sizeof(candidates) / sizeof(candidates[0]);
	char probe[520];

	for (int i = 0; i < n; i++) {
		sprintf_s(probe, sizeof(probe), "%s%s", candidates[i], sentinel);
		if (pathFileExists(probe)) {
			strcpy_s(out, outSize, candidates[i]);
			printf("[paths] %-6s root = \"%s\"  (found %s)\n", label, candidates[i], sentinel);
			return;
		}
	}

	out[0] = '\0';
	printf("[paths] WARNING: could not locate \"%s\" from the working directory.\n", sentinel);
	printf("[paths]          %s assets will fail to load.\n", label);
}

void pathsInit()
{
	pathFindRoot(PATH_POSTER,   gAssetRoot, sizeof(gAssetRoot), "image");
	pathFindRoot(PATH_BG_MUSIC, gAudioRoot, sizeof(gAudioRoot), "audio");
}

// Builds a full path to an image/sprite asset. Caller supplies the buffer so
// two calls can safely appear in the same expression.
void assetPath(const char *relative, char *out, int outSize)
{
	sprintf_s(out, outSize, "%s%s", gAssetRoot, relative);
}

// Builds a full path to an audio asset.
void audioPath(const char *relative, char *out, int outSize)
{
	sprintf_s(out, outSize, "%s%s", gAudioRoot, relative);
}

#endif // PATHS_HPP

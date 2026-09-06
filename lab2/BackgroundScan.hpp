//
//  BackgroundScan.hpp  --  finds the route backdrops at run time.
//
//  This used to be a hard-coded list of filenames. It is a directory scan now
//  because the contents of the backdrop folder get swapped out: drop new images
//  in, delete old ones, and the level picks them up with no code change.
//
//  ORDER: files are sorted by filename, but NATURALLY -- runs of digits compare
//  by value, not character by character. That matters because the folder mixes
//  widths: plain text sorting puts bg_10 before bg_6, which would shuffle the
//  journey. Name them however you like; bg_6 and bg_06 both land in the right
//  place, and a new bg_14 slots in after bg_13.
//
#ifndef BACKGROUNDSCAN_HPP
#define BACKGROUNDSCAN_HPP


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
#endif
#include <string.h>
#include <ctype.h>

const int MAX_BACKGROUNDS = 64;

char gBgFiles[MAX_BACKGROUNDS][200];   // paths relative to the asset root
int  gBgCount = 0;

// Compares two names the way a person reads them: a run of digits is compared
// by its value, so bg_9 comes before bg_10 rather than after it.
static int naturalCompare(const char *a, const char *b)
{
	while (*a && *b) {
		if (isdigit((unsigned char)*a) && isdigit((unsigned char)*b)) {
			while (*a == '0') a++;          // leading zeros carry no value
			while (*b == '0') b++;

			const char *startA = a, *startB = b;
			while (isdigit((unsigned char)*a)) a++;
			while (isdigit((unsigned char)*b)) b++;

			int lenA = (int)(a - startA);
			int lenB = (int)(b - startB);
			if (lenA != lenB) return lenA - lenB;   // more digits, bigger number

			int cmp = strncmp(startA, startB, lenA);
			if (cmp != 0) return cmp;
		} else {
			int ca = tolower((unsigned char)*a);
			int cb = tolower((unsigned char)*b);
			if (ca != cb) return ca - cb;
			a++;
			b++;
		}
	}
	return (*a ? 1 : 0) - (*b ? 1 : 0);
}

static bool bgHasImageExtension(const char *name)
{
	const char *dot = strrchr(name, '.');
	if (dot == 0) return false;

	return _stricmp(dot, ".jpg")  == 0 ||
	       _stricmp(dot, ".jpeg") == 0 ||
	       _stricmp(dot, ".png")  == 0 ||
	       _stricmp(dot, ".bmp")  == 0;
}

void scanBackgrounds()
{
	char pattern[520];
	assetPath(PATH_BG_FOLDER, pattern, sizeof(pattern));
	strcat_s(pattern, sizeof(pattern), "*.*");

	gBgCount = 0;

	WIN32_FIND_DATAA find;
	HANDLE handle = FindFirstFileA(pattern, &find);

	if (handle == INVALID_HANDLE_VALUE) {
		printf("[bg] could not open \"%s\"\n", pattern);
		return;
	}

	do {
		if (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
		if (!bgHasImageExtension(find.cFileName)) continue;
		if (gBgCount >= MAX_BACKGROUNDS) break;

		sprintf_s(gBgFiles[gBgCount], sizeof(gBgFiles[0]), "%s%s",
		          PATH_BG_FOLDER, find.cFileName);
		gBgCount++;
	} while (FindNextFileA(handle, &find));

	FindClose(handle);

	// Insertion sort by name. The list is tiny, and this keeps the ordering
	// rule obvious rather than hidden behind qsort and a comparator.
	for (int i = 1; i < gBgCount; i++) {
		char key[200];
		strcpy_s(key, sizeof(key), gBgFiles[i]);
		int j = i - 1;
		while (j >= 0 && naturalCompare(gBgFiles[j], key) > 0) {
			strcpy_s(gBgFiles[j + 1], sizeof(gBgFiles[0]), gBgFiles[j]);
			j--;
		}
		strcpy_s(gBgFiles[j + 1], sizeof(gBgFiles[0]), key);
	}

	printf("[bg] found %d backdrop image%s:\n", gBgCount, gBgCount == 1 ? "" : "s");
	for (int i = 0; i < gBgCount; i++)
		printf("[bg]   %2d  %s\n", i + 1, gBgFiles[i]);
}

#endif // BACKGROUNDSCAN_HPP

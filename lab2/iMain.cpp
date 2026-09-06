 //
//  NO PROXY TODAY  -  A Dhaka Commute Adventure
//
//  CSE-1200 Software Development I  -  Department of CSE, AUST
//  Team A2_06: MD Shahidul Islam Noyon, Ali Muntakim, Abu Nahian Rifat
//
//  iMain.cpp is the only translation unit. Everything else is a header, which
//  is why the .vcxproj never needs touching when a module is added. Include
//  order matters: each header may use anything declared above it, and nothing
//  below it.
//
//  iGraphics.h must come first and must be included exactly once -- it has no
//  include guard and it defines globals plus STB_IMAGE_IMPLEMENTATION.
//

#include "iGraphics.h"

#include "Config.hpp"        // window size, colours, layout, asset paths
#include "Paths.hpp"         // finds the asset folders at run time
#include "Draw.hpp"          // translucent shapes, measured and scalable text
#include "Input.hpp"         // edge-triggered keyboard
#include "Audio.hpp"         // MCI wrapper
#include "Collision.hpp"     // Rect and checkCollision()
#include "BackgroundScan.hpp" // scans the backdrop folder at run time
#include "Assets.hpp"        // texture loading
#include "GameState.hpp"     // which screen is showing
#include "Button.hpp"        // the clickable widget
#include "Page.hpp"          // shared look of the text screens
#include "MenuScreen.hpp"    // title screen
#include "KeysScreen.hpp"    // controls
#include "AboutScreen.hpp"   // proposal summary
#include "LevelState.hpp"    // health, lives, shield, clock, progress, rules
#include "LevelSelect.hpp"   // campaign map (needs gLevel01Cleared)
#include "Entities.hpp"      // the one obstacle, coins and power-ups
#include "Player.hpp"        // the running character
#include "HUD.hpp"           // score, health, lives, shield, clock, route
#include "Level01.hpp"       // Mohanagar -> Hatirjheel
#include "Level02State.hpp"   // level 02: stage machine, health, rules
#include "Level02Entities.hpp" // level 02: boat, ball, crocodiles, chidori
#include "Level02Deck.hpp"     // level 02: pirate ship, demons, Demon Lord
#include "Level02HUD.hpp"     // level 02: its own read-outs
#include "Level02.hpp"        // the river crossing (part 1)

// ---------------------------------------------------------------------------
//  Screen dispatch
// ---------------------------------------------------------------------------
void iDraw()
{
	iClear();

	switch (gState) {
	case STATE_MENU:    menuDraw();    break;
	case STATE_KEYS:    keysDraw();    break;
	case STATE_ABOUT:   aboutDraw();   break;
	case STATE_LEVEL_SELECT: levelSelectDraw(); break;
	case STATE_LEVEL01: level01Draw(); break;
	case STATE_LEVEL02: level02Draw(); break;
	}
}

void fixedUpdate()
{
	// M mutes from anywhere.
	if (keyJustPressed('m') || keyJustPressed('M'))
		audioToggleMute();

	switch (gState) {
	case STATE_MENU:    menuUpdate();    break;
	case STATE_KEYS:    keysUpdate();    break;
	case STATE_ABOUT:   aboutUpdate();   break;
	case STATE_LEVEL_SELECT: levelSelectUpdate(); break;
	case STATE_LEVEL01: level01Update(); break;
	case STATE_LEVEL02: level02Update(); break;
	}

	inputEndFrame();   // must stay last -- it snapshots this frame's key state
}

void iMouse(int button, int state, int mx, int my)
{
	switch (gState) {
	case STATE_MENU:    menuMouse(button, state, mx, my);    break;
	case STATE_KEYS:    keysMouse(button, state, mx, my);    break;
	case STATE_ABOUT:   aboutMouse(button, state, mx, my);   break;
	case STATE_LEVEL_SELECT: levelSelectMouse(button, state, mx, my); break;
	case STATE_LEVEL01: level01Mouse(button, state, mx, my); break;
	case STATE_LEVEL02: level02Mouse(button, state, mx, my); break;
	}
}

void iPassiveMouseMove(int mx, int my)
{
	if (gState == STATE_MENU)
		menuPassiveMouseMove(mx, my);
	else if (gState == STATE_LEVEL_SELECT)
		levelSelectPassiveMouseMove(mx, my);
}

void iMouseMove(int mx, int my)
{
	// dragging is unused
}

// ---------------------------------------------------------------------------
//  Menu audio
//
//  The introduction narration loops under the whole menu shell -- title, KEYS
//  and ABOUT alike -- and stops the moment NEW GAME starts the level, where the
//  background music takes over. The flag matters: without it, stepping from
//  KEYS back to the title would restart the narration from the top every time.
// ---------------------------------------------------------------------------
static bool gMenuMusicOn = false;

static void menuAudioStart()
{
	if (gMenuMusicOn) return;
	audioPlayLoop("introvoice");
	gMenuMusicOn = true;
}

static void menuAudioStop()
{
	if (!gMenuMusicOn) return;
	audioStop("introvoice");
	gMenuMusicOn = false;
}

// ---------------------------------------------------------------------------
//  State transitions
// ---------------------------------------------------------------------------
void setState(GameStateId next)
{
	gState = next;

	// A key still held down during the change must not immediately fire on the
	// new screen -- otherwise the ENTER that opened ABOUT closes it again.
	inputClear();

	// The campaign map counts as part of the menu shell, so the intro
	// narration keeps looping across it and only stops when a level starts.
	if (next == STATE_LEVEL01 || next == STATE_LEVEL02)
		menuAudioStop();
	else
		menuAudioStart();

	switch (next) {
	case STATE_MENU:    menuEnter();    break;
	case STATE_KEYS:    keysEnter();    break;
	case STATE_ABOUT:   aboutEnter();   break;
	case STATE_LEVEL_SELECT: levelSelectEnter(); break;
	case STATE_LEVEL01: level01Enter(); break;
	case STATE_LEVEL02: level02Enter(); break;
	}
}

void quitGame()
{
	printf("[game] closing\n");
	audioCloseAll();
	exit(0);   // glutMainLoop() never returns, so there is no other way out
}

// ---------------------------------------------------------------------------
//  Entry point
// ---------------------------------------------------------------------------
int main()
{
	// Unbuffered, so the console shows asset/audio diagnostics the moment they
	// happen rather than whenever the buffer decides to flush.
	setvbuf(stdout, NULL, _IONBF, 0);

	printf("=====================================================\n");
	printf("  NO PROXY TODAY  -  A Dhaka Commute Adventure\n");
	printf("  Team A2_06  -  CSE-1200  -  AUST\n");
	printf("=====================================================\n");

	// 0. seed the obstacle generator from the clock. Left unseeded it deals the
	//    identical sequence of obstacles in the identical order on every single
	//    run, which is how a rickshaw could go a whole session without ever
	//    appearing.
	seedGameRandom((unsigned int)time(NULL));

	// 1. work out where the assets are relative to the working directory
	pathsInit();

	// 2. see which backdrop images the street is made of. This is a directory
	//    scan, not a hard-coded list, so images can be added to or removed from
	//    the folder without touching any code. It needs no GL context, so it
	//    runs here and the level knows its length before loading starts.
	scanBackgrounds();

	// 3. audio can be opened before the window exists.
	//
	//    Every obstacle has its own WARNING horn, played once as it closes in,
	//    separate from the thud that plays on contact. Most of these sit in the
	//    solution root, and the spaces in their names are real.
	char full[520];

	audioPath(PATH_BG_MUSIC, full, sizeof(full));
	audioOpen("bgsong", full);

	assetPath(PATH_WIN_SOUND, full, sizeof(full));  audioOpen("winsnd", full);
	assetPath(PATH_LOSE_SOUND, full, sizeof(full)); audioOpen("losesnd", full);

	assetPath(PATH_SFX_BIKE, full, sizeof(full)); audioOpen("sfxbike", full);
	assetPath(PATH_SFX_CAR,  full, sizeof(full)); audioOpen("sfxcar",  full);
	assetPath(PATH_SFX_DOG,  full, sizeof(full)); audioOpen("sfxdog",  full);
	assetPath(PATH_SFX_RICK, full, sizeof(full)); audioOpen("sfxrick", full);
	assetPath(PATH_SFX_BIRD, full, sizeof(full)); audioOpen("sfxbird", full);

	assetPath(PATH_INTRO_VOICE, full, sizeof(full));   audioOpen("introvoice", full);
	assetPath(PATH_SFX_COLLISION, full, sizeof(full)); audioOpen("sfxhit", full);

	//    Level 02 adds the rain, which runs under the whole level, and one
	//    voice per crocodile so two can growl at once without cutting each
	//    other off. The level02 folder has its own copy of collision
	//    sound.mpeg, which is not opened: the thud is already loaded above
	//    as "sfxhit" and Level 02 reuses that same alias.
	assetPath(PATH_L02_RAIN, full, sizeof(full)); audioOpen("rain", full);

	//    The pirate song carries the whole of Level 02's second half. It is an
	//    MP3 behind a .mpeg extension like the others, so audioOpen naming the
	//    MPEGVideo device is what lets it open at all.
	assetPath(PATH_L02B_PIRATE_SONG, full, sizeof(full)); audioOpen("piratesong", full);

	//    And Level 02's own ending clip, played over the Hatirjheel boat once
	//    the dakatleader is down. The file is called "loose sound2" but it is
	//    the WIN sting for this level.
	assetPath(PATH_L02B_WIN_SONG, full, sizeof(full)); audioOpen("winsnd2", full);
	for (int c = 0; c < MAX_CROCODILES; c++) {
		assetPath(PATH_L02_CROC_SFX, full, sizeof(full));
		audioOpen(L02_CROC_ALIAS[c], full);
	}

	// Balance the mix. The collision thud is the one that has to cut through.
	audioSetMix("sfxhit",     MIX_COLLISION);
	audioSetMix("sfxbike",    MIX_WARNING);
	audioSetMix("sfxcar",     MIX_WARNING);
	audioSetMix("sfxdog",     MIX_WARNING);
	audioSetMix("sfxrick",    MIX_WARNING);
	audioSetMix("sfxbird",    MIX_WARNING);
	audioSetMix("introvoice", MIX_INTRO);
	audioSetMix("winsnd",     MIX_ENDING);
	audioSetMix("losesnd",    MIX_ENDING);
	audioSetMix("bgsong",     MIX_MUSIC);
	audioSetMix("rain",       MIX_RAIN);
	audioSetMix("piratesong", MIX_PIRATE);
	audioSetMix("winsnd2",    MIX_L02_WIN);
	for (int c = 0; c < MAX_CROCODILES; c++)
		audioSetMix(L02_CROC_ALIAS[c], MIX_CROC);

	// 4. create the window -- this is what gives us an OpenGL context
	iInitialize(WIN_W, WIN_H, (char *)WIN_TITLE);

	// 5. textures need that context, so they load here, never earlier
	assetsLoadMenu();

	// 6. open on the title screen and hand control to GLUT
	setState(STATE_MENU);
	iStart();

	return 0;
}

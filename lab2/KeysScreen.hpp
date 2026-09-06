//
//  KeysScreen.hpp  --  the control reference.
//
//  These are the bindings Level 01 will implement. Menu navigation is live
//  already; the gameplay rows describe the scheme the level code will follow,
//  so this list stays the single place the controls are written down.
//
#ifndef KEYSSCREEN_HPP
#define KEYSSCREEN_HPP


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
#include "Draw.hpp"
#include "Input.hpp"
#include "Assets.hpp"
#include "GameState.hpp"
#include "Page.hpp"
#endif
static const TextLine kKeyLines[] = {
	{ "PLAYING LEVEL 01",                                                LS_HEADING },
	{ "",                                                                LS_SPACER  },
	{ "  SPACE  /  UP  /  W      Jump",                                        LS_KEY },
	{ "  P                       Pause and resume",                            LS_KEY },
	{ "  R                       Restart the level",                           LS_KEY },
	{ "  M                       Mute or unmute",                              LS_KEY },
	{ "  ESC                     Back to the main menu",                       LS_KEY },
	{ "",                                                                LS_SPACER  },
	{ "The character runs by itself on one lane and cannot be moved left,",    LS_DIM },
	{ "right, forward or back. The street comes to you. A well-timed jump is", LS_DIM },
	{ "the only way past anything, and only one obstacle arrives at a time.",  LS_DIM },
	{ "",                                                                LS_SPACER  },
	{ "LISTEN FOR IT",                                                   LS_HEADING },
	{ "",                                                                LS_SPACER  },
	{ "  Motorcycle horn         a bike is closing in",                        LS_KEY },
	{ "  Car horn                a car is closing in",                        LS_KEY },
	{ "  Barking                 a dog is closing in",                        LS_KEY },
	{ "  Rickshaw horn           a rickshaw is closing in",                   LS_KEY },
	{ "  Chirping                birds ahead - look at their height",         LS_KEY },
	{ "",                                                                LS_SPACER  },
	{ "Birds flying low must be jumped. Birds flying high pass safely",        LS_DIM },
	{ "overhead, so do NOT jump into them.",                                   LS_DIM },
	{ "",                                                                LS_SPACER  },
	{ "COLLECTING  (just run into them)",                                LS_HEADING },
	{ "",                                                                LS_SPACER  },
	{ "  COIN                    10 points each",                              LS_KEY },
	{ "  MEDIKIT                 Restores 50 health, up to the 200 maximum",   LS_KEY },
	{ "  SHIELD ORB              The next 2 obstacles cost you no health",     LS_KEY },
	{ "",                                                                LS_SPACER  },
	{ "One life, 200 health, two minutes to reach Hatirjheel.",                LS_DIM }
};

static const int kKeyLineCount = sizeof(kKeyLines) / sizeof(kKeyLines[0]);
static int gKeysScroll = 0;

void keysEnter()
{
	gKeysScroll = 0;
}

void keysDraw()
{
	pageDrawBackdrop("KEYS", "How to play No Proxy Today");
	pageDrawLines(kKeyLines, kKeyLineCount, gKeysScroll);
	pageDrawFooter(kKeyLineCount, gKeysScroll);
}

void keysUpdate()
{
	gKeysScroll = pageHandleScroll(gKeysScroll, kKeyLineCount);

	if (keyJustPressed(27) || keyJustPressed(8))   // ESC or BACKSPACE
		setState(STATE_MENU);
}

void keysMouse(int button, int state, int mx, int my)
{
	// Nothing clickable here yet -- ESC or BACKSPACE goes back.
}

#endif // KEYSSCREEN_HPP

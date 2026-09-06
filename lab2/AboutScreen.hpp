//
//  AboutScreen.hpp  --  a summary of the submitted project proposal
//                       (A2_06_projectProposal.pdf).
//
#ifndef ABOUTSCREEN_HPP
#define ABOUTSCREEN_HPP


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
static const TextLine kAboutLines[] = {
	{ "Course CSE-1200  Software Development I   -   Department of CSE", LS_DIM },
	{ "Ahsanullah University of Science and Technology (AUST), Dhaka",   LS_DIM },
	{ "Submitted to Saha Reno (Assistant Professor) and Md. Zahid Hossain (Lecturer)", LS_DIM },
	{ "Submitted 12 July 2026 by MD Shahidul Islam Noyon (00725105101034),",           LS_DIM },
	{ "Ali Muntakim (00725105101043) and Abu Nahian Rifat (00725105101039).",          LS_DIM },
	{ "",                                                               LS_SPACER },

	{ "OBJECTIVE",                                                      LS_HEADING },
	{ "To entertain players with fun, challenging, fast-paced gameplay, and to",  LS_BODY },
	{ "simulate the everyday journey of an AUST student travelling to campus.",   LS_BODY },
	{ "",                                                               LS_SPACER },

	{ "GAMEPLAY",                                                       LS_HEADING },
	{ "The player starts from Mohanagar or Modhubag and must reach AUST before",  LS_BODY },
	{ "class begins. Along the route you dodge traffic, buses, rickshaws,",       LS_BODY },
	{ "potholes, Hatirjheel and road barriers, and complete mini-tasks before",   LS_BODY },
	{ "the countdown runs out. Finishing a task quickly awards bonus time and score.", LS_BODY },
	{ "",                                                               LS_SPACER },

	{ "LEVELS",                                                         LS_HEADING },
	{ "Level 1    Basic obstacles, easy tasks, a generous time limit.",           LS_BODY },
	{ "Level 2    Heavier traffic, more obstacles, harder tasks, a shorter timer.", LS_BODY },
	{ "Level 3    Dense traffic, multiple checkpoints, very limited time.",       LS_BODY },
	{ "",                                                               LS_SPACER },

	{ "WIN AND LOSS",                                                   LS_HEADING },
	{ "You win by completing every required task and reaching AUST before the",   LS_BODY },
	{ "countdown ends.        \"Congratulations! You made it!\"",                 LS_BODY },
	{ "You lose if time runs out, collisions pile up, or tasks are skipped.",     LS_BODY },
	{ "                       \"You are cooked!\"",                               LS_BODY },
	{ "",                                                               LS_SPACER },

	{ "CORE FEATURES",                                                  LS_HEADING },
	{ "Real-time countdown timer   -   multiple starting locations   -   obstacle dodging", LS_BODY },
	{ "Mini-task checkpoints   -   scoring   -   rising difficulty   -   pause, resume, restart", LS_BODY },
	{ "",                                                               LS_SPACER },

	{ "INNOVATIVE ELEMENTS",                                            LS_HEADING },
	{ "Multiple routes and shortcuts   -   weather and time-of-day events", LS_BODY },
	{ "Power-up collectibles",                                          LS_BODY },
	{ "",                                                               LS_SPACER },

	{ "TARGET AUDIENCE",                                                LS_HEADING },
	{ "University students, especially AUST students, plus teenagers, young",     LS_BODY },
	{ "adults and casual gamers who enjoy fast-paced adventure games.",           LS_BODY },
	{ "",                                                               LS_SPACER },

	{ "Inspired by Does Not Commute (Mediocre).",                       LS_DIM }
};

static const int kAboutLineCount = sizeof(kAboutLines) / sizeof(kAboutLines[0]);
static int gAboutScroll = 0;

void aboutEnter()
{
	gAboutScroll = 0;
}

void aboutDraw()
{
	pageDrawBackdrop("ABOUT THE GAME", "NO PROXY TODAY  -  A Dhaka Commute Adventure");
	pageDrawLines(kAboutLines, kAboutLineCount, gAboutScroll);
	pageDrawFooter(kAboutLineCount, gAboutScroll);
}

void aboutUpdate()
{
	gAboutScroll = pageHandleScroll(gAboutScroll, kAboutLineCount);

	if (keyJustPressed(27) || keyJustPressed(8))   // ESC or BACKSPACE
		setState(STATE_MENU);
}

void aboutMouse(int button, int state, int mx, int my)
{
	// Nothing clickable here yet -- ESC or BACKSPACE goes back.
}

#endif // ABOUTSCREEN_HPP

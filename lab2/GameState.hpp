//
//  GameState.hpp  --  which screen is on show, and how to change it.
//
//  Every screen module exposes the same four hooks:
//
//      xxxEnter()                       once, on becoming the active screen
//      xxxUpdate()                      every 16 ms, from fixedUpdate()
//      xxxDraw()                        every frame, from iDraw()
//      xxxMouse(button, state, mx, my)  from iMouse()
//
//  setState() is declared here and defined at the bottom of iMain.cpp, once all
//  the screen modules are visible.
//
#ifndef GAMESTATE_HPP
#define GAMESTATE_HPP

enum GameStateId {
	STATE_MENU = 0,
	STATE_KEYS,
	STATE_ABOUT,
	STATE_LEVEL_SELECT,   // the campaign map NEW GAME opens
	STATE_LEVEL01,
	STATE_LEVEL02        // the river crossing (part 1)
};

GameStateId gState = STATE_MENU;

void setState(GameStateId next);   // defined in iMain.cpp
void quitGame();                   // defined in iMain.cpp

#endif // GAMESTATE_HPP

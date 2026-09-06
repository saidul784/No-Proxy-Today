//
//  Page.hpp  --  the shared look of the two text screens (KEYS, ABOUT).
//
//  Both draw the poster, dim it almost to black, then lay out a list of lines
//  with a scroll offset. Keeping that here means the two screens only have to
//  supply their own content.
//
#ifndef PAGE_HPP
#define PAGE_HPP


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
#endif
// How a line is styled.
enum LineStyle {
	LS_HEADING = 0,   // orange, section title
	LS_BODY,          // normal white
	LS_DIM,           // muted grey
	LS_KEY,           // key name column, drawn accented
	LS_SPACER         // blank line
};

struct TextLine {
	const char *text;
	LineStyle   style;
};

const double PAGE_TITLE_Y   = WIN_H - 74.0;
const double PAGE_BODY_TOP  = WIN_H - 132.0;
const double PAGE_BODY_BOT  = 56.0;

int pageVisibleLineCount()
{
	return (int)((PAGE_BODY_TOP - PAGE_BODY_BOT) / PAGE_LINE_H) + 1;
}

// Poster, dimmed, plus a framed content plate and the screen title.
void pageDrawBackdrop(const char *title, const char *subtitle)
{
	if (TEX_POSTER != 0)
		iShowImage(0, 0, WIN_W, WIN_H, TEX_POSTER);
	else
		dFillRectA(0, 0, WIN_W, WIN_H, C_INK, 1.0);

	dDimScreen(0.88);

	// content plate
	dFillRectA(PAGE_MARGIN_X - 22, PAGE_BODY_BOT - 22,
	           WIN_W - 2 * (PAGE_MARGIN_X - 22), PAGE_BODY_TOP - PAGE_BODY_BOT + 60,
	           C_PANEL, 0.55);
	dRectOutlineA(PAGE_MARGIN_X - 22, PAGE_BODY_BOT - 22,
	              WIN_W - 2 * (PAGE_MARGIN_X - 22), PAGE_BODY_TOP - PAGE_BODY_BOT + 60,
	              C_ACCENT_DIM, 0.5, 1.0);

	dSetColor(C_ACCENT);
	dStrokeText(PAGE_MARGIN_X, PAGE_TITLE_Y, title, 34, 2.5);

	if (subtitle) {
		dSetColor(C_TEXT_DIM);
		dText(PAGE_MARGIN_X + 2, PAGE_TITLE_Y - 22, subtitle, GLUT_BITMAP_HELVETICA_12);
	}

	// accent rule under the title
	dFillRectA(PAGE_MARGIN_X, PAGE_TITLE_Y - 34, WIN_W - 2 * PAGE_MARGIN_X, 2, C_ACCENT, 0.7);
}

// Draws lines[scroll .. scroll+visible] down the page.
void pageDrawLines(const TextLine *lines, int count, int scroll)
{
	const int visible = pageVisibleLineCount();
	double y = PAGE_BODY_TOP;

	for (int i = scroll; i < count && i < scroll + visible; i++) {
		const TextLine &ln = lines[i];

		switch (ln.style) {
		case LS_HEADING:
			dSetColor(C_ACCENT);
			dText(PAGE_MARGIN_X, y, ln.text, GLUT_BITMAP_HELVETICA_12);
			dFillRectA(PAGE_MARGIN_X, y - 5, (double)dTextWidth(ln.text, GLUT_BITMAP_HELVETICA_12), 1,
			           C_ACCENT, 0.5);
			break;

		case LS_KEY:
			dSetColor(C_SKY);
			dText(PAGE_MARGIN_X + 16, y, ln.text, GLUT_BITMAP_9_BY_15);
			break;

		case LS_DIM:
			dSetColor(C_TEXT_DIM);
			dText(PAGE_MARGIN_X, y, ln.text, GLUT_BITMAP_HELVETICA_12);
			break;

		case LS_SPACER:
			break;

		case LS_BODY:
		default:
			dSetColor(C_TEXT);
			dText(PAGE_MARGIN_X, y, ln.text, GLUT_BITMAP_HELVETICA_12);
			break;
		}

		y -= PAGE_LINE_H;
	}
}

// Footer hint, plus a scrollbar when the content does not fit.
void pageDrawFooter(int count, int scroll)
{
	const int visible = pageVisibleLineCount();

	dSetColor(C_TEXT_DIM);
	if (count > visible)
		dText(PAGE_MARGIN_X, 22, "ESC or BACKSPACE  -  back to menu          UP / DOWN  -  scroll",
		      GLUT_BITMAP_HELVETICA_12);
	else
		dText(PAGE_MARGIN_X, 22, "ESC or BACKSPACE  -  back to menu", GLUT_BITMAP_HELVETICA_12);

	if (count > visible) {
		const double trackX = WIN_W - PAGE_MARGIN_X + 4;
		const double trackY = PAGE_BODY_BOT;
		const double trackH = PAGE_BODY_TOP - PAGE_BODY_BOT;
		const double thumbH = trackH * ((double)visible / (double)count);
		const double maxScroll = (double)(count - visible);
		const double thumbY = trackY + (trackH - thumbH) * (1.0 - scroll / maxScroll);

		dFillRectA(trackX, trackY, 5, trackH, C_TEXT_DIM, 0.20);
		dFillRectA(trackX, thumbY, 5, thumbH, C_ACCENT, 0.75);
	}
}

// Shared scroll input. Returns the clamped offset.
int pageHandleScroll(int scroll, int count)
{
	const int visible = pageVisibleLineCount();
	const int maxScroll = (count > visible) ? count - visible : 0;

	if (specialKeyJustPressed(GLUT_KEY_DOWN) || keyJustPressed('s') || keyJustPressed('S'))
		scroll++;
	if (specialKeyJustPressed(GLUT_KEY_UP) || keyJustPressed('w') || keyJustPressed('W'))
		scroll--;
	if (specialKeyJustPressed(GLUT_KEY_PAGE_DOWN))
		scroll += visible - 1;
	if (specialKeyJustPressed(GLUT_KEY_PAGE_UP))
		scroll -= visible - 1;
	if (specialKeyJustPressed(GLUT_KEY_HOME))
		scroll = 0;
	if (specialKeyJustPressed(GLUT_KEY_END))
		scroll = maxScroll;

	if (scroll < 0) scroll = 0;
	if (scroll > maxScroll) scroll = maxScroll;
	return scroll;
}

#endif // PAGE_HPP

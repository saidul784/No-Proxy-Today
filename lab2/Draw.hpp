//
//  Draw.hpp  --  small drawing helpers layered on top of iGraphics.
//
//  Two things iGraphics does not give us and the menus need:
//    1. translucent panels  (iFilledRectangle is always fully opaque)
//    2. text bigger than 24 px  (iText only reaches GLUT_BITMAP_TIMES_ROMAN_24)
//
//  Note: this build of glut.h reports GLUT_API_VERSION 3, which means
//  glutBitmapLength() / glutStrokeLength() are NOT declared. Widths are
//  therefore measured one character at a time.
//
#ifndef DRAW_HPP
#define DRAW_HPP


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
// ---------------------------------------------------------------------------
//  Colour
// ---------------------------------------------------------------------------
void dSetColor(const Color &c)
{
	iSetColor(c.r, c.g, c.b);
}

// ---------------------------------------------------------------------------
//  Translucent shapes
// ---------------------------------------------------------------------------
void dFillRectA(double x, double y, double w, double h, const Color &c, double alpha)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor4f((float)(c.r / 255.0), (float)(c.g / 255.0), (float)(c.b / 255.0), (float)alpha);
	glBegin(GL_QUADS);
		glVertex2d(x,     y);
		glVertex2d(x + w, y);
		glVertex2d(x + w, y + h);
		glVertex2d(x,     y + h);
	glEnd();

	glDisable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

// A 1 px outline, drawn with alpha.
void dRectOutlineA(double x, double y, double w, double h, const Color &c, double alpha, double thickness)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth((GLfloat)thickness);

	glColor4f((float)(c.r / 255.0), (float)(c.g / 255.0), (float)(c.b / 255.0), (float)alpha);
	glBegin(GL_LINE_LOOP);
		glVertex2d(x + 0.5,     y + 0.5);
		glVertex2d(x + w - 0.5, y + 0.5);
		glVertex2d(x + w - 0.5, y + h - 0.5);
		glVertex2d(x + 0.5,     y + h - 0.5);
	glEnd();

	glLineWidth(1.0f);
	glDisable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

// Darkens the whole window so a photographic backdrop can sit behind text.
void dDimScreen(double alpha)
{
	dFillRectA(0, 0, WIN_W, WIN_H, C_INK, alpha);
}

// A soft dark band, strongest at centreX and fading to nothing at the edges.
// Used to hide the join where one scrolling backdrop meets the next: a hard cut
// between two different streets is obvious, a shadow between buildings is not.
void dSeamShade(double centerX, double halfWidth, const Color &c, double peakAlpha)
{
	float r = (float)(c.r / 255.0);
	float g = (float)(c.g / 255.0);
	float b = (float)(c.b / 255.0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBegin(GL_QUADS);
		// left half: transparent -> solid
		glColor4f(r, g, b, 0.0f);
		glVertex2d(centerX - halfWidth, 0);
		glVertex2d(centerX - halfWidth, WIN_H);
		glColor4f(r, g, b, (float)peakAlpha);
		glVertex2d(centerX, WIN_H);
		glVertex2d(centerX, 0);

		// right half: solid -> transparent
		glColor4f(r, g, b, (float)peakAlpha);
		glVertex2d(centerX, 0);
		glVertex2d(centerX, WIN_H);
		glColor4f(r, g, b, 0.0f);
		glVertex2d(centerX + halfWidth, WIN_H);
		glVertex2d(centerX + halfWidth, 0);
	glEnd();

	glDisable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

// ---------------------------------------------------------------------------
//  Images
//
//  iShowImage() is fine for an opaque full-screen backdrop, but it binds
//  GL_REPLACE, so it cannot fade a texture, tint one, or mirror it. Level 01
//  needs all three: cross-fading between backdrops, flashing the player red on
//  a hit, and turning a left-facing sprite around when it overtakes from
//  behind.
//
//  The odd t = 0 at the bottom and t = -1 at the top is copied deliberately
//  from iShowImage. With GL_REPEAT it cancels out stb_image's top-down row
//  order, so the picture lands the right way up. Changing it flips everything.
// ---------------------------------------------------------------------------
void dImageEx(double x, double y, double w, double h, unsigned int texture,
              bool flipX, double r, double g, double b, double alpha)
{
	if (texture == 0) return;

	double s0 = flipX ? 1.0 : 0.0;
	double s1 = flipX ? 0.0 : 1.0;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glColor4f((float)r, (float)g, (float)b, (float)alpha);

	glBegin(GL_QUADS);
		glTexCoord2d(s0,  0.0); glVertex2d(x,     y);
		glTexCoord2d(s1,  0.0); glVertex2d(x + w, y);
		glTexCoord2d(s1, -1.0); glVertex2d(x + w, y + h);
		glTexCoord2d(s0, -1.0); glVertex2d(x,     y + h);
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void dImage(double x, double y, double w, double h, unsigned int texture)
{
	dImageEx(x, y, w, h, texture, false, 1.0, 1.0, 1.0, 1.0);
}

void dImageFlipped(double x, double y, double w, double h, unsigned int texture, bool flipX)
{
	dImageEx(x, y, w, h, texture, flipX, 1.0, 1.0, 1.0, 1.0);
}

void dImageAlpha(double x, double y, double w, double h, unsigned int texture, double alpha)
{
	dImageEx(x, y, w, h, texture, false, 1.0, 1.0, 1.0, alpha);
}

// ---------------------------------------------------------------------------
//  Additive draw, for energy effects photographed on a BLACK field
//
//  Some artwork arrives as a glow on solid black with no alpha at all --
//  "enemy chidori.jpg" and "powerfulchidori" are both like this. The load-time
//  background cut cannot help: it removes pixels that are BRIGHT and neutral,
//  and here the background is the darkest thing in the file.
//
//  Additive blending solves it without touching the pixels. Source and
//  destination are simply summed, so black adds nothing and disappears on its
//  own, while the bright lightning adds to whatever is behind it and glows.
//  That is also how a real energy effect should composite, so this looks better
//  than a cut-out would have.
// ---------------------------------------------------------------------------
void dImageAdditive(double x, double y, double w, double h, unsigned int texture,
                    double intensity)
{
	if (texture == 0) return;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);      // additive
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glColor4f((float)intensity, (float)intensity, (float)intensity, 1.0f);

	// Same odd t = 0 bottom / t = -1 top as dImageEx, for the same reason.
	glBegin(GL_QUADS);
		glTexCoord2d(0.0,  0.0); glVertex2d(x,     y);
		glTexCoord2d(1.0,  0.0); glVertex2d(x + w, y);
		glTexCoord2d(1.0, -1.0); glVertex2d(x + w, y + h);
		glTexCoord2d(0.0, -1.0); glVertex2d(x,     y + h);
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);   // put it back
	glDisable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

// ---------------------------------------------------------------------------
//  Bitmap text
// ---------------------------------------------------------------------------
int dTextWidth(const char *s, void *font)
{
	int w = 0;
	for (int i = 0; s[i]; i++)
		w += glutBitmapWidth(font, (unsigned char)s[i]);
	return w;
}

void dText(double x, double y, const char *s, void *font)
{
	iText(x, y, (char *)s, font);   // iText takes char*, not const char*
}

void dTextCentered(double centerX, double y, const char *s, void *font)
{
	dText(centerX - dTextWidth(s, font) / 2.0, y, s, font);
}

void dTextRight(double rightX, double y, const char *s, void *font)
{
	dText(rightX - dTextWidth(s, font), y, s, font);
}

// ---------------------------------------------------------------------------
//  Stroke text  --  vector glyphs, so any size we like
// ---------------------------------------------------------------------------
const double STROKE_FONT_HEIGHT = 119.05;   // cap height of GLUT_STROKE_ROMAN

int dStrokeTextWidth(const char *s, double pixelHeight)
{
	int raw = 0;
	for (int i = 0; s[i]; i++)
		raw += glutStrokeWidth(GLUT_STROKE_ROMAN, (unsigned char)s[i]);
	return (int)(raw * (pixelHeight / STROKE_FONT_HEIGHT));
}

void dStrokeText(double x, double y, const char *s, double pixelHeight, double thickness)
{
	double scale = pixelHeight / STROKE_FONT_HEIGHT;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth((GLfloat)thickness);

	glPushMatrix();
	glTranslated(x, y, 0.0);
	glScaled(scale, scale, scale);
	for (int i = 0; s[i]; i++)
		glutStrokeCharacter(GLUT_STROKE_ROMAN, (unsigned char)s[i]);
	glPopMatrix();

	glLineWidth(1.0f);
	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_BLEND);
}

void dStrokeTextCentered(double centerX, double y, const char *s, double pixelHeight, double thickness)
{
	dStrokeText(centerX - dStrokeTextWidth(s, pixelHeight) / 2.0, y, s, pixelHeight, thickness);
}

#endif // DRAW_HPP

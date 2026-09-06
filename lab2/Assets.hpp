//
//  Assets.hpp  --  texture loading, with background removal done at load time.
//
//  Nearly every sprite in game_assetes is a jpg (or a flattened png) on a solid
//  white or checkerboard field. The alpha test is binary, so drawn raw each one
//  would be an opaque rectangle sitting on top of the street. Rather than keep
//  a set of hand-prepared cut-out files that go stale every time the artwork
//  changes, the cut is done here, on load, straight out of the pixel buffer.
//  Drop a new jpg in the folder and it just works.
//
//  Why not iLoadImage()? It does not check whether stbi_load succeeded: on a
//  missing file it hands a NULL pixel pointer and two UNINITIALISED width and
//  height ints to glTexImage2D.
//
//  IMPORTANT: every load must happen AFTER iInitialize() (there is no GL
//  context before it) and BEFORE iStart() (which never returns).
//
#ifndef ASSETS_HPP
#define ASSETS_HPP


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
#include "BackgroundScan.hpp"
#endif
#include <stdlib.h>
#include <string.h>

// A texture plus the size it was loaded at, so it can be drawn at its true
// aspect ratio without hard-coding numbers that go stale with the artwork.
struct Sprite {
	unsigned int tex;
	int w, h;
};

static Sprite emptySprite()
{
	Sprite s;
	s.tex = 0; s.w = 0; s.h = 0;
	return s;
}

double spriteAspect(const Sprite &s)
{
	if (s.h <= 0) return 1.0;
	return (double)s.w / (double)s.h;
}

// --- menu ------------------------------------------------------------------
unsigned int TEX_POSTER = 0;
unsigned int TEX_LEVELMAP = 0;                 // campaign map background
int SZ_LEVELMAP_W = 0, SZ_LEVELMAP_H = 0;      // its true pixel size

// --- level 01 --------------------------------------------------------------
unsigned int TEX_CHARACTER[CHARACTER_FRAME_COUNT] = { 0 };
unsigned int TEX_BACKGROUND[MAX_BACKGROUNDS]      = { 0 };

Sprite SPR_BIKE_COMING, SPR_BIKE_GOING;
Sprite SPR_CAR_COMING,  SPR_CAR_GOING;
Sprite SPR_DOG_COMING,  SPR_DOG_GOING;
Sprite SPR_RICK_COMING, SPR_RICK_GOING;
Sprite SPR_BIRDS;
Sprite SPR_COIN;
Sprite SPR_PU_HEALTH, SPR_PU_SHIELD;

// --- level 02 --------------------------------------------------------------
// The boy is drawn inside all four boat poses, so there is no separate
// character sprite on this level.
unsigned int TEX_L02_BG = 0;
Sprite SPR_L02_BOAT_1, SPR_L02_BOAT_2;      // rowing, two oar strokes
Sprite SPR_L02_BOAT_STAND;                  // standing, after the ball lands
Sprite SPR_L02_BOAT_THROW;                  // hand raised, mid-throw
Sprite SPR_L02_CHIDORI;
Sprite SPR_L02_DRAGON;
Sprite SPR_L02_CROCODILE;
Sprite SPR_L02_COIN;

// --- level 02, second half: the pirate ship --------------------------------
// The character walking the deck is the SAME six frames Level 01 runs on
// (TEX_CHARACTER above), so there is no second walk cycle anywhere in the
// project. That is also why those frames no longer belong to Level 01 alone --
// see assetsLoadCharacterFrames() below.
unsigned int TEX_L02B_DECK = 0;             // boatside view.jpg, the interior
Sprite SPR_L02B_SHIP;                       // the ship that rams the boat
Sprite SPR_L02B_THROW;                      // his throwing pose, off the boat

Sprite SPR_L02B_FIGHTER[3];                 // fighter_01, _02, _03 -- the dakat
Sprite SPR_L02B_ICON;                       // buys him chidori to throw
Sprite SPR_L02B_POWERUP;                    // grants the powerful chidori
Sprite SPR_L02B_LEADER;                     // dakatleader, the boss

Sprite SPR_L02B_CHIDORI;                    // the bolt he throws at a dakat
Sprite SPR_L02B_POWERFUL;                   // the boss-killer
Sprite SPR_L02B_ENEMY_CHI;                  // what the boss throws back

// ---------------------------------------------------------------------------
//  Upload
// ---------------------------------------------------------------------------
static unsigned int uploadTexture(unsigned char *pixels, int w, int h)
{
	unsigned int texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	return texture;
}

// Averages each divisor x divisor block down to one pixel, in place.
static void boxDownscale(unsigned char *src, int w, int h, int divisor, int *outW, int *outH)
{
	int nw = w / divisor, nh = h / divisor;

	for (int y = 0; y < nh; y++)
		for (int x = 0; x < nw; x++) {
			int acc[4] = { 0, 0, 0, 0 };
			for (int sy = 0; sy < divisor; sy++)
				for (int sx = 0; sx < divisor; sx++) {
					const unsigned char *p = src + (((y * divisor + sy) * w) + (x * divisor + sx)) * 4;
					acc[0] += p[0]; acc[1] += p[1]; acc[2] += p[2]; acc[3] += p[3];
				}
			int n = divisor * divisor;
			unsigned char *d = src + ((y * nw) + x) * 4;
			d[0] = (unsigned char)(acc[0] / n);
			d[1] = (unsigned char)(acc[1] / n);
			d[2] = (unsigned char)(acc[2] / n);
			d[3] = (unsigned char)(acc[3] / n);
		}

	*outW = nw;
	*outH = nh;
}

// ---------------------------------------------------------------------------
//  Background removal
// ---------------------------------------------------------------------------
// Background here is always white or neutral grey -- a plain field, or the
// grey/white chequerboard some of these files have baked in where transparency
// used to be. Requiring the pixel to be NEUTRAL as well as bright is what lets
// the threshold go low enough to swallow the jpeg ringing around each chequer
// square without also eating warm, pale artwork like a cream-coloured dog.
static bool pixelIsBackground(const unsigned char *p, int cut)
{
	int r = p[0], g = p[1], b = p[2];

	int lo = r < g ? (r < b ? r : b) : (g < b ? g : b);
	int hi = r > g ? (r > b ? r : b) : (g > b ? g : b);

	return lo >= cut && (hi - lo) <= 16;
}

// Clears every near-white pixel CONNECTED TO THE BORDER. White inside the
// subject -- a dog's coat, the gleam on a coin -- is not reachable from the
// edge, so it survives. This is also what kills the grey/white checkerboard
// some of these files have baked into them where transparency used to be.
static void floodClearBackground(unsigned char *px, int w, int h, int cut)
{
	int total = w * h;
	unsigned char *seen = (unsigned char *)calloc(total, 1);
	int *stack = (int *)malloc(sizeof(int) * total);
	if (seen == 0 || stack == 0) { free(seen); free(stack); return; }

	int top = 0;

	// Seed from every border pixel that looks like background.
	for (int x = 0; x < w; x++) {
		int a = x, b = (h - 1) * w + x;
		if (!seen[a] && pixelIsBackground(px + a * 4, cut)) { seen[a] = 1; stack[top++] = a; }
		if (!seen[b] && pixelIsBackground(px + b * 4, cut)) { seen[b] = 1; stack[top++] = b; }
	}
	for (int y = 0; y < h; y++) {
		int a = y * w, b = y * w + (w - 1);
		if (!seen[a] && pixelIsBackground(px + a * 4, cut)) { seen[a] = 1; stack[top++] = a; }
		if (!seen[b] && pixelIsBackground(px + b * 4, cut)) { seen[b] = 1; stack[top++] = b; }
	}

	while (top > 0) {
		int id = stack[--top];
		int x = id % w, y = id / w;
		px[id * 4 + 3] = 0;

		int nb[4];
		int n = 0;
		if (x > 0)     nb[n++] = id - 1;
		if (x < w - 1) nb[n++] = id + 1;
		if (y > 0)     nb[n++] = id - w;
		if (y < h - 1) nb[n++] = id + w;

		for (int i = 0; i < n; i++) {
			int j = nb[i];
			if (seen[j]) continue;
			if (!pixelIsBackground(px + j * 4, cut)) continue;
			seen[j] = 1;
			stack[top++] = j;
		}
	}

	free(seen);
	free(stack);
}

// Pushes colour outward into the cleared pixels. Without this, GL_LINEAR
// samples the original white while fading out and leaves a bright halo around
// every sprite.
static void bleedEdges(unsigned char *px, int w, int h, int passes)
{
	int bytes = w * h * 4;
	unsigned char *copy = (unsigned char *)malloc(bytes);
	if (copy == 0) return;

	for (int pass = 0; pass < passes; pass++) {
		memcpy(copy, px, bytes);

		for (int y = 0; y < h; y++)
			for (int x = 0; x < w; x++) {
				int id = (y * w + x) * 4;
				if (copy[id + 3] != 0) continue;

				int r = 0, g = 0, b = 0, n = 0;
				for (int dy = -1; dy <= 1; dy++)
					for (int dx = -1; dx <= 1; dx++) {
						int nx = x + dx, ny = y + dy;
						if (nx < 0 || ny < 0 || nx >= w || ny >= h) continue;
						int j = (ny * w + nx) * 4;
						if (copy[j + 3] == 0) continue;
						r += copy[j]; g += copy[j + 1]; b += copy[j + 2]; n++;
					}
				if (n == 0) continue;
				px[id] = (unsigned char)(r / n);
				px[id + 1] = (unsigned char)(g / n);
				px[id + 2] = (unsigned char)(b / n);
			}
	}

	free(copy);
}

// Trims the buffer down to the opaque part, in place. Returns the new size.
//
// The threshold matters on artwork that came with a watermark: enemy throw.png
// has a stock-site mark stamped over it at a low alpha, so at the usual 12 the
// box grows to enclose the mark and the flame ends up small inside it. Raising
// the threshold ignores the mark and crops to the flame.
static void cropToContentT(unsigned char *px, int w, int h, int *outW, int *outH,
                           int alphaThreshold)
{
	int minX = w, minY = h, maxX = -1, maxY = -1;

	for (int y = 0; y < h; y++)
		for (int x = 0; x < w; x++)
			if (px[(y * w + x) * 4 + 3] > alphaThreshold) {
				if (x < minX) minX = x;
				if (x > maxX) maxX = x;
				if (y < minY) minY = y;
				if (y > maxY) maxY = y;
			}

	if (maxX < 0) { *outW = w; *outH = h; return; }   // nothing survived

	int nw = maxX - minX + 1;
	int nh = maxY - minY + 1;

	for (int y = 0; y < nh; y++)
		memmove(px + (y * nw) * 4, px + ((y + minY) * w + minX) * 4, nw * 4);

	*outW = nw;
	*outH = nh;
}

static void cropToContent(unsigned char *px, int w, int h, int *outW, int *outH)
{
	cropToContentT(px, w, h, outW, outH, 12);
}

// Cuts a sub-rectangle out of the buffer, in place, before anything else looks
// at it. This is what makes a sprite SHEET usable: one figure is lifted out of
// a page of them first, and the rest of the pipeline never sees the others.
static void cropToRect(unsigned char *px, int w, int h, int *outW, int *outH,
                       int rx, int ry, int rw, int rh)
{
	if (rw <= 0 || rh <= 0) return;          // 0 means "the whole image"

	if (rx < 0) rx = 0;
	if (ry < 0) ry = 0;
	if (rx + rw > w) rw = w - rx;
	if (ry + rh > h) rh = h - ry;
	if (rw <= 0 || rh <= 0) return;

	for (int y = 0; y < rh; y++)
		memmove(px + (y * rw) * 4, px + ((y + ry) * w + rx) * 4, rw * 4);

	*outW = rw;
	*outH = rh;
}

// ---------------------------------------------------------------------------
//  Loaders
// ---------------------------------------------------------------------------
unsigned int loadTextureScaled(const char *relativePath, int divisor, int *outW, int *outH)
{
	char full[520];
	assetPath(relativePath, full, sizeof(full));

	int w = 0, h = 0, channels = 0;
	unsigned char *pixels = stbi_load(full, &w, &h, &channels, 4);

	if (pixels == 0) {
		printf("[assets] FAILED: %s\n", full);
		printf("[assets]   stb_image says: %s\n", stbi_failure_reason());
		if (outW) *outW = 0;
		if (outH) *outH = 0;
		return 0;
	}

	if (divisor > 1 && w >= divisor && h >= divisor)
		boxDownscale(pixels, w, h, divisor, &w, &h);

	unsigned int texture = uploadTexture(pixels, w, h);
	stbi_image_free(pixels);

	if (outW) *outW = w;
	if (outH) *outH = h;
	return texture;
}

unsigned int loadTexture(const char *relativePath, int *outW = 0, int *outH = 0)
{
	int w = 0, h = 0;
	unsigned int t = loadTextureScaled(relativePath, 1, &w, &h);
	if (t) printf("[assets] loaded %-32s %4d x %-4d\n", relativePath, w, h);
	if (outW) *outW = w;
	if (outH) *outH = h;
	return t;
}

// Loads a sprite and cuts its background away.
//
//   cut         how bright a pixel must be to count as background (0-255).
//               Lower values bite deeper: needed for a baked checkerboard or a
//               watermark, dangerous on pale artwork.
//   cropBottom  fraction of the image height to discard first, for the stock
//               photo watermark bars that sit along the bottom edge.
//   clearEnclosed
//               also clear background-coloured pixels the border flood cannot
//               reach -- the chequer pocket trapped between a dog's legs, for
//               instance. Only safe on artwork with no bright neutral tones of
//               its own: switch it on for a brown dog, never for a white one.
//   subX/subY/subW/subH
//               lift this rectangle out of the file FIRST and discard the rest.
//               A subW or subH of 0 means "the whole image". This is what makes
//               a sprite SHEET usable: one figure can be taken out of a page of
//               them before anything else looks at the pixels.
//   divisor     average down by this factor before the cut, for files far
//               larger than they will ever be drawn.
Sprite loadSpriteEx(const char *relativePath, int cut, double cropBottom,
                    bool clearEnclosed, int subX, int subY, int subW, int subH,
                    int divisor)
{
	Sprite sprite = emptySprite();

	char full[520];
	assetPath(relativePath, full, sizeof(full));

	int w = 0, h = 0, channels = 0;
	unsigned char *pixels = stbi_load(full, &w, &h, &channels, 4);

	if (pixels == 0) {
		printf("[assets] FAILED: %s\n", full);
		printf("[assets]   stb_image says: %s\n", stbi_failure_reason());
		return sprite;
	}

	// The sheet cut comes first, so everything after it works on one subject.
	// stb hands back rows top-down, which is the order the rectangle was
	// measured in, so the coordinates need no flipping.
	if (subW > 0 && subH > 0)
		cropToRect(pixels, w, h, &w, &h, subX, subY, subW, subH);

	if (divisor > 1 && w >= divisor && h >= divisor)
		boxDownscale(pixels, w, h, divisor, &w, &h);

	// The bottom of the image is the LAST rows in stb's top-down buffer.
	if (cropBottom > 0.0 && cropBottom < 0.5)
		h = h - (int)(h * cropBottom);

	floodClearBackground(pixels, w, h, cut);

	if (clearEnclosed) {
		int count = w * h;
		for (int i = 0; i < count; i++)
			if (pixels[i * 4 + 3] != 0 && pixelIsBackground(pixels + i * 4, cut))
				pixels[i * 4 + 3] = 0;
	}

	bleedEdges(pixels, w, h, 2);
	cropToContent(pixels, w, h, &w, &h);

	sprite.tex = uploadTexture(pixels, w, h);
	sprite.w = w;
	sprite.h = h;
	stbi_image_free(pixels);

	printf("[assets] cut out %-32s %4d x %-4d (cut %d)\n", relativePath, w, h, cut);
	return sprite;
}

// Loads a sprite that ALREADY carries its own alpha channel and needs no cut.
// chidori.png is a 32-bit PNG whose surround is genuinely transparent, so
// running the brightness cut over it would be pointless at best -- its border
// pixels are dark, not bright, so nothing would be removed anyway. All this
// does is trim the empty margin so the bolt fills the rect it is drawn into.
// The plain form every Level 01 sprite uses: the whole file, no downscale.
Sprite loadSprite(const char *relativePath, int cut, double cropBottom, bool clearEnclosed)
{
	return loadSpriteEx(relativePath, cut, cropBottom, clearEnclosed, 0, 0, 0, 0, 1);
}

//   alphaCut  pixels at or below this alpha are erased outright before the
//             trim. Zero leaves the file exactly as it is. It exists for
//             enemy throw.png, which carries a faint stock-site watermark: at
//             the usual "alpha > 12" test the sprite's box grows to enclose the
//             mark and the flame ends up small and off-centre inside it.
//   divisor   average down by this factor first, for the very large files.
Sprite loadSpriteAlphaEx(const char *relativePath, int alphaCut, int divisor)
{
	Sprite sprite = emptySprite();

	char full[520];
	assetPath(relativePath, full, sizeof(full));

	int w = 0, h = 0, channels = 0;
	unsigned char *pixels = stbi_load(full, &w, &h, &channels, 4);

	if (pixels == 0) {
		printf("[assets] FAILED: %s\n", full);
		printf("[assets]   stb_image says: %s\n", stbi_failure_reason());
		return sprite;
	}

	if (divisor > 1 && w >= divisor && h >= divisor)
		boxDownscale(pixels, w, h, divisor, &w, &h);

	if (alphaCut > 0) {
		int count = w * h;
		for (int i = 0; i < count; i++)
			if (pixels[i * 4 + 3] <= alphaCut) pixels[i * 4 + 3] = 0;
	}

	cropToContentT(pixels, w, h, &w, &h, alphaCut > 0 ? alphaCut : 12);

	sprite.tex = uploadTexture(pixels, w, h);
	sprite.w = w;
	sprite.h = h;
	stbi_image_free(pixels);

	printf("[assets] alpha    %-32s %4d x %-4d\n", relativePath, w, h);
	return sprite;
}

// The plain form: keep the file's own alpha exactly as it is.
Sprite loadSpriteAlpha(const char *relativePath)
{
	return loadSpriteAlphaEx(relativePath, 0, 1);
}

// No cut, no trim, no alpha -- the file exactly as it is, wrapped in a Sprite so
// its true size travels with it. This is for artwork that is meant to be drawn
// ADDITIVELY: a glow on solid black, where the background disappears because
// black adds nothing, and where cropping or cutting would only do harm.
Sprite loadSpriteRaw(const char *relativePath)
{
	Sprite sprite = emptySprite();

	int w = 0, h = 0;
	sprite.tex = loadTextureScaled(relativePath, 1, &w, &h);
	sprite.w = w;
	sprite.h = h;

	if (sprite.tex)
		printf("[assets] raw      %-32s %4d x %-4d\n", relativePath, w, h);
	return sprite;
}

void freeTexture(unsigned int *texture)
{
	if (texture && *texture) {
		glDeleteTextures(1, texture);
		*texture = 0;
	}
}

// Everything the main menu, keys and about screens need.
void assetsLoadMenu()
{
	printf("[assets] --- loading menu assets ---\n");
	TEX_POSTER = loadTexture(PATH_POSTER);

	// The campaign map loads here too, with the menu, because NEW GAME opens
	// it immediately -- loading it on demand would stall that transition.
	// Its real size is kept so the screen can fit it without distorting it.
	TEX_LEVELMAP = loadTexture(PATH_LEVELMAP, &SZ_LEVELMAP_W, &SZ_LEVELMAP_H);
}

// The six walk frames used to belong to Level 01 alone. The second half of
// Level 02 walks the same character down the pirate ship's deck, so they are
// shared now, and this is the one place they load.
//
// It is guarded on the frames themselves, so whichever level is entered first
// pays for them and the other finds them already there.
bool assetsCharacterFramesLoaded()
{
	return TEX_CHARACTER[0] != 0;
}

void assetsLoadCharacterFrames()
{
	if (assetsCharacterFramesLoaded()) return;

	for (int i = 0; i < CHARACTER_FRAME_COUNT; i++) {
		char relative[128];
		sprintf_s(relative, sizeof(relative), PATH_CHARACTER_FMT, i + 1);
		TEX_CHARACTER[i] = loadTexture(relative);
	}
}

// This asks a question ONLY Level 01 can answer. It used to test the character
// frames, which was correct while they belonged to Level 01 -- but now that
// Level 02 loads them too, entering Level 02 first would have set that flag and
// Level 01 would then have skipped loading its entire street.
bool assetsLevel01Loaded()
{
	return SPR_BIKE_COMING.tex != 0;
}

typedef void (*LoadProgressFn)(const char *stage, int done, int total);

void assetsLoadLevel01(LoadProgressFn progress)
{
	if (assetsLevel01Loaded()) return;

	const int total = CHARACTER_FRAME_COUNT + 13 + gBgCount;
	int done = 0;

	printf("[assets] --- loading level 01 ---\n");

	// Shared with Level 02's deck section, so they may already be here. The
	// progress bar still counts them either way, so it does not jump.
	for (int i = 0; i < CHARACTER_FRAME_COUNT; i++) {
		if (TEX_CHARACTER[i] == 0) {
			char relative[128];
			sprintf_s(relative, sizeof(relative), PATH_CHARACTER_FMT, i + 1);
			TEX_CHARACTER[i] = loadTexture(relative);
		}
		if (progress) progress("character", ++done, total);
	}

	// Per-asset cut thresholds, because these files are not consistent:
	//   dog_is_going and coin and the cars have a transparency checkerboard
	//   baked into the pixels, which needs a deeper bite;
	//   dog_is_coming is a near-white dog on white, so the bite must be shallow
	//   or the dog goes with the background;
	//   rickshaw_is_coming is a watermarked stock image -- the faint diagonal
	//   mark needs a deeper bite and the caption bar along the bottom is
	//   cropped off before the cut.
	SPR_BIKE_COMING = loadSprite(PATH_BIKE_COMING, 228, 0.0,  false);  if (progress) progress("traffic", ++done, total);
	SPR_BIKE_GOING  = loadSprite(PATH_BIKE_GOING,  228, 0.0,  false);  if (progress) progress("traffic", ++done, total);
	SPR_CAR_COMING  = loadSprite(PATH_CAR_COMING,  200, 0.0,  true);  if (progress) progress("traffic", ++done, total);
	SPR_CAR_GOING   = loadSprite(PATH_CAR_GOING,   200, 0.0,  false);  if (progress) progress("traffic", ++done, total);
	// dog_is_coming is a near-white dog on white, and white is neutral too, so
	// the bite has to stay shallow or the dog goes with the background.
	SPR_DOG_COMING  = loadSprite(PATH_DOG_COMING,  244, 0.0,  false);  if (progress) progress("traffic", ++done, total);
	// dog_is_going is a warm brown dog behind a grey chequerboard, so the bite
	// can go deep: 168 clears the chequers and the jpeg ringing between them,
	// and the dog's colours are nowhere near neutral.
	SPR_DOG_GOING   = loadSprite(PATH_DOG_GOING,   168, 0.0,  true);  if (progress) progress("traffic", ++done, total);
	// rickshaw_is_coming is a watermarked stock image. The faint diagonal mark
	// sits at about 210 on white, and jpeg ringing around it dips lower still,
	// so at 206 thin grey lines survived all the way to the edges and nothing
	// cropped. 188 clears the mark and its ringing; the rickshaw's own white
	// canopy is enclosed by dark framework, so the border flood cannot reach it.
	SPR_RICK_COMING = loadSprite(PATH_RICK_COMING, 188, 0.10, false); if (progress) progress("traffic", ++done, total);
	SPR_RICK_GOING  = loadSprite(PATH_RICK_GOING,  228, 0.0,  false);  if (progress) progress("traffic", ++done, total);
	SPR_BIRDS       = loadSprite(PATH_BIRDS,       228, 0.0,  true);  if (progress) progress("birds",   ++done, total);

	SPR_COIN        = loadSprite(PATH_COIN,        200, 0.0,  true);  if (progress) progress("coins",   ++done, total);
	SPR_PU_HEALTH   = loadSprite(PATH_PU_HEALTH,   250, 0.0,  false);  if (progress) progress("power-ups", ++done, total);
	SPR_PU_SHIELD   = loadSprite(PATH_PU_SHIELD,   250, 0.0,  false);  if (progress) progress("power-ups", ++done, total);
	if (progress) progress("power-ups", ++done, total);

	// Backdrops load at full resolution: each scrolls across the whole window,
	// so any softening would show.
	for (int i = 0; i < gBgCount; i++) {
		int w = 0, h = 0;
		TEX_BACKGROUND[i] = loadTextureScaled(gBgFiles[i], 1, &w, &h);
		printf("[assets] backdrop %2d  %-34s %4d x %-4d\n", i + 1, gBgFiles[i], w, h);
		if (progress) progress("the street", ++done, total);
	}

	printf("[assets] level 01 ready\n");
}

// ---------------------------------------------------------------------------
//  Level 02
//
//  Every sprite here except chidori.png sits on a plain white field -- the
//  corner pixels sample at 253-255 across all three channels -- so they all go
//  through the same load-time cut the Level 01 traffic uses.
//
//  The boat art is line-drawn with white highlights on the hull and the boy's
//  shirt, and those whites are ENCLOSED by dark outlines, so the border flood
//  cannot reach them. That is why clearEnclosed stays off for the boats: it
//  would punch holes straight through the hull.
// ---------------------------------------------------------------------------
bool assetsLevel02Loaded()
{
	return TEX_L02_BG != 0;
}

void assetsLoadLevel02(LoadProgressFn progress)
{
	if (assetsLevel02Loaded()) return;

	const int total = 22;
	int done = 0;

	printf("[assets] --- loading level 02 ---\n");

	// The river backdrop fills the window and scrolls, so it loads whole.
	TEX_L02_BG = loadTexture(PATH_L02_BG);
	if (progress) progress("the river", ++done, total);

	SPR_L02_BOAT_1     = loadSprite(PATH_L02_BOAT_1,     236, 0.0, false); if (progress) progress("the boat", ++done, total);
	SPR_L02_BOAT_2     = loadSprite(PATH_L02_BOAT_2,     236, 0.0, false); if (progress) progress("the boat", ++done, total);
	SPR_L02_BOAT_STAND = loadSprite(PATH_L02_BOAT_STAND, 236, 0.0, false); if (progress) progress("the boat", ++done, total);
	SPR_L02_BOAT_THROW = loadSprite(PATH_L02_BOAT_THROW, 236, 0.0, false); if (progress) progress("the boat", ++done, total);

	// Already transparent -- no cut, just a trim.
	SPR_L02_CHIDORI    = loadSpriteAlpha(PATH_L02_CHIDORI);               if (progress) progress("chidori",  ++done, total);

	// The medallion and the crocodile are flat colour on white, so a shallow
	// bite is enough and keeps their pale highlights.
	SPR_L02_DRAGON     = loadSprite(PATH_L02_DRAGON,     240, 0.0, false); if (progress) progress("dragon power", ++done, total);
	SPR_L02_CROCODILE  = loadSprite(PATH_L02_CROCODILE,  238, 0.0, false); if (progress) progress("crocodiles",   ++done, total);

	// This coin has the same grey chequerboard baked in as the Level 01 one,
	// which is what the deeper bite and clearEnclosed are for.
	SPR_L02_COIN       = loadSprite(PATH_L02_COIN,       200, 0.0, true);  if (progress) progress("coins", ++done, total);

	// -----------------------------------------------------------------------
	//  Second half: the pirate ship
	//
	//  Every cut threshold below was chosen from the pixels, not by eye:
	//
	//    pirates ship.png   a 4-bit indexed PNG with a white/grey chequerboard
	//                       baked in where transparency used to be. The ship is
	//                       black throughout, so a deep bite with the enclosed
	//                       pass is safe and clears the chequers between the
	//                       rigging as well as around it.
	//    fighter_01..03     already transparent -- no cut, only a trim.
	//    icon.jpg           a bolt in a strongly coloured ring on plain white,
	//                       so a shallow bite lifts the white without biting the
	//                       ring. The bolt itself IS white, but it is enclosed
	//                       by the ring, so the enclosed pass stays off.
	//    powerup.png        already transparent.
	//    dakatleader.jpeg   a warm off-white field sampling around (239,244,237)
	//                       at the corners, so the cut has to reach below that.
	//                       Measuring the content box at several thresholds, 225
	//                       gives a stable crop of the leader and his crates; by
	//                       232 it starts swallowing jpeg noise in the field and
	//                       the box grows sideways by a quarter.
	//    chidori.png        already transparent.
	//    powerfulchidori    glows on SOLID BLACK with no alpha, so no cut can
	//    enemy chidori      help -- the background is the DARKEST thing in the
	//                       file, not the brightest. They are loaded raw and
	//                       drawn additively, which makes the black disappear on
	//                       its own. See dImageAdditive() in Draw.hpp.
	// -----------------------------------------------------------------------
	TEX_L02B_DECK = loadTexture(PATH_L02B_DECK);
	if (progress) progress("the deck", ++done, total);

	// The walk cycle is Level 01's, shared rather than duplicated.
	assetsLoadCharacterFrames();
	if (progress) progress("the walk", ++done, total);

	SPR_L02B_SHIP  = loadSprite(PATH_L02B_SHIP,  200, 0.0, true);   if (progress) progress("pirate ship", ++done, total);
	SPR_L02B_THROW = loadSprite(PATH_L02B_THROW, 236, 0.0, false);  if (progress) progress("the throw",   ++done, total);

	// The three dakat used to be 32-bit PNGs that were already transparent and
	// went through loadSpriteAlpha. The replacements are JPEGs, which carry no
	// alpha at all, so drawn that way each one would have been an opaque grey
	// rectangle. They are cut instead, like the rest of the flat artwork.
	//
	// Their field samples 244-246 across all four corners and the content box
	// does not move at all between cuts of 200, 225 and 235, so there is clear
	// air between the background and the darkest thing in the picture.
	//
	// The enclosed pass is ON, and the numbers say it is safe. Counting the
	// interior neutral pixels -- the ones the border flood can NEVER reach --
	// fighter_02 and fighter_03 have none at all above 225, while fighter_01
	// has about 690 sitting at 240-250: a solid white pocket tucked behind his
	// waist, between his back, his arm and his lungi. Without this pass it drew
	// as an opaque white blob stuck to him.
	//
	// The swords survive because they are nowhere near that bright: across all
	// three files only a single interior pixel reaches 215. A cut of 226 is
	// comfortably above the blades and comfortably below the pocket, so it
	// takes the pocket and its jpeg ringing and leaves the steel alone.
	SPR_L02B_FIGHTER[0] = loadSprite(PATH_L02B_FIGHTER1, 226, 0.0, true);  if (progress) progress("the dakat", ++done, total);
	SPR_L02B_FIGHTER[1] = loadSprite(PATH_L02B_FIGHTER2, 226, 0.0, true);  if (progress) progress("the dakat", ++done, total);
	SPR_L02B_FIGHTER[2] = loadSprite(PATH_L02B_FIGHTER3, 226, 0.0, true);  if (progress) progress("the dakat", ++done, total);

	// icon.jpg is a bolt in a coloured ring on plain white. Its ring is bright
	// but strongly coloured, so a shallow bite lifts the white without biting
	// into it, and the enclosed pass stays off because the bolt itself is white.
	SPR_L02B_ICON = loadSprite(PATH_L02B_ICON, 242, 0.0, false);
	if (progress) progress("chidori icon", ++done, total);

	// Already transparent.
	SPR_L02B_POWERUP = loadSpriteAlpha(PATH_L02B_POWERUP);
	if (progress) progress("the powerup", ++done, total);

	// dakatleader.jpeg sits on a slightly warm off-white -- its corners sample
	// around (239,244,237), so the cut has to reach below that. Measuring the
	// content box at several thresholds, 225 gives a stable 135 x 113 crop of
	// the leader and his crates; at 232 it starts swallowing jpeg noise in the
	// background and the box grows sideways by a quarter.
	SPR_L02B_LEADER = loadSprite(PATH_L02B_LEADER, 225, 0.0, false);
	if (progress) progress("the dakatleader", ++done, total);

	// chidori.png carries real alpha, like the one the boat half throws.
	SPR_L02B_CHIDORI = loadSpriteAlpha(PATH_L02B_CHIDORI);
	if (progress) progress("chidori", ++done, total);

	// These two last are glows on SOLID BLACK with no alpha, so no cut can help
	// -- the background is the darkest thing in the file, not the brightest.
	// They are loaded raw and drawn additively instead, which makes the black
	// disappear on its own. See dImageAdditive() in Draw.hpp.
	SPR_L02B_POWERFUL  = loadSpriteRaw(PATH_L02B_POWERFUL);   if (progress) progress("powerful chidori", ++done, total);
	SPR_L02B_ENEMY_CHI = loadSpriteRaw(PATH_L02B_ENEMY_CHI);  if (progress) progress("enemy chidori",    ++done, total);

	printf("[assets] level 02 ready\n");
}

#endif // ASSETS_HPP

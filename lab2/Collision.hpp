//
//  Collision.hpp  --  axis-aligned bounding boxes.
//
//  Everything that can be hit -- the character, every obstacle, every piece of
//  falling garbage, every power-up -- reports a Rect, and one function decides
//  whether two of them touch. Nothing compares raw sprite positions.
//
#ifndef COLLISION_HPP
#define COLLISION_HPP

struct Rect {
	double x, y, w, h;    // x,y is the bottom-left corner (iGraphics origin)
};

Rect makeRect(double x, double y, double w, double h)
{
	Rect r;
	r.x = x; r.y = y; r.w = w; r.h = h;
	return r;
}

// Pulls a box in towards its own centre. Sprites carry a lot of empty space --
// a swinging arm, the glow around a power-up -- and colliding on the artwork's
// full extent feels wrong to play.
Rect shrinkRect(const Rect &r, double keepX, double keepY)
{
	Rect s;
	s.w = r.w * keepX;
	s.h = r.h * keepY;
	s.x = r.x + (r.w - s.w) * 0.5;
	s.y = r.y + (r.h - s.h) * 0.5;
	return s;
}

bool checkCollision(const Rect &a, const Rect &b)
{
	return a.x < b.x + b.w &&
	       a.x + a.w > b.x &&
	       a.y < b.y + b.h &&
	       a.y + a.h > b.y;
}

#endif // COLLISION_HPP

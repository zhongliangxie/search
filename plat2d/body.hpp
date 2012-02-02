#ifndef _BODY_HPP_
#define _BODY_HPP_

#include <cassert>
#include "../utils/geom2d.hpp"

struct Lvl;

struct Line1d {
	Line1d(double _a, double _b) : a(_a), b(_b) { }

	double isection(const Line1d &o) const {
		if (Geom2d::between(o.a, o.b, b))
			return b - o.a;
		else if (Geom2d::between(a, b, o.b))
			return o.b - a;
		return -1.0;
	}

	double a, b;
};

struct Isect {
	Isect(void) : is(false), dx(0.0), dy(0.0) { }

	Isect(double _dx, double _dy) : is(true), dx(_dx), dy(_dy) { }

	Isect(const Isect &o) : is(o.is), dx(o.dx), dy(o.dy) { }

	double area(void) { return dx * dy; }

	bool is;
	double dx, dy;
};

struct Bbox : public Geom2d::Rectangle {

	Bbox(void) { }

	Bbox(unsigned int x0, unsigned int y0, unsigned int x1,
			unsigned int y1) : Geom2d::Rectangle(x0, y0, x1, y1) { }

	Isect isection(const Bbox &o) const {
		double ix = projx().isection(o.projx());
		if(ix > 0.0) {
			double iy = projy().isection(o.projy());
			if(iy > 0.0)
				return Isect(ix, iy);
		}
		return Isect();
	}

	Line1d projx(void) const { return Line1d(min.x, max.x); }

	Line1d projy(void) const { return Line1d(min.y, max.y); }
};

struct Body {
	enum { Maxdy = 12 };

	Body(void) { }

	Body(unsigned int x, unsigned int y, unsigned int w, unsigned int h) :
		bbox(x, y, x + w, y + h), dy(0), fall(false) { }

	Body(const Body &o) : bbox(o.bbox), dy(o.dy), fall(o.fall) { }

	bool operator==(const Body &o) const {
		return fall == o.fall && bbox == o.bbox && Geom2d::doubleeq(dy, o.dy);
	}

	void output(FILE *out) const {
		fprintf(out, "%g,%g,	dy=%g,	fall=%d\n", bbox.min.x, bbox.min.y,
			dy, fall);
	}

	void move(const Lvl&, double dx);
	
	Bbox bbox;
	double dy;
	bool fall;

private:

	Geom2d::Point step(const Geom2d::Point&);

	void dofall(const Lvl&, const Isect&);
};

#endif	// _BODY_HPP_
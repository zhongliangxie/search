#include "ui.hpp"
#include "../utils/utils.hpp"
#include <cstdio>
#include <SDL/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>

Ui::Ui(unsigned int w, unsigned int h) : scene(w, h) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
		fatal("failed to initialiaze SDL: %s", SDL_GetError());

	int flags = SDL_HWSURFACE | SDL_GL_DOUBLEBUFFER | SDL_OPENGL;
	unsigned int color = 32;
	screen = SDL_SetVideoMode(scene.width, scene.height, color, flags);
	if (!screen)
		fatal("failed to create a window: %s", SDL_GetError());

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POINT_SMOOTH);
	glTranslated(-1.0, -1.0, 0.0);
	glScaled(2.0 / scene.width, 2.0 / scene.height, 0.0);
	glClearColor(1.0, 1.0, 1.0, 0.0);
}

void Ui::run(unsigned long rate) {
	for ( ; ; ) {
		long tick = SDL_GetTicks();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		frame();
		scene.render();
		glFlush();
		SDL_GL_SwapBuffers();

		if (handleevents())
			break;

		long delay = tick + rate - SDL_GetTicks();
		if (delay > 0)
			SDL_Delay(delay);
	}
}

bool Ui::handleevents(void) {
	for ( ; ; ) {
		SDL_Event e;
		int p = SDL_PollEvent(&e);
		if (!p)
			break;

		switch(e.type) {
		case SDL_QUIT:
			return true;
		case SDL_KEYDOWN:
			key(e.key.keysym.sym, true);
			break;
		case SDL_KEYUP:
			key(e.key.keysym.sym, false);
			break;
		case SDL_MOUSEMOTION:
			motion(e.motion.x, e.motion.y, e.motion.xrel, e.motion.yrel);
			break;
		case SDL_MOUSEBUTTONDOWN:
			click(e.button.x, e.button.y, e.button.button, true);
			break;
		case SDL_MOUSEBUTTONUP:
			click(e.button.x, e.button.y, e.button.button, false);
			break;
		default:
			break;
		}
	}
	return false;
}

void Ui::frame(void) {
	scene.clear();
	Geom2d::Point center(scene.width / 2, scene.height / 2);

	unsigned int color = 0;

	Geom2d::Polygon p = Geom2d::Polygon::random(10, 0, 0, 1);
	p.scale(scene.width / 2, scene.height / 2);
	p.translate(scene.width / 2, scene.height / 2);
	scene.add(new Scene::Polygon(p, somecolors[color++ % Nsomecolors], 1));

	Geom2d::Arc a = Geom2d::Arc(center, scene.width / 10, 0, M_PI);
	scene.add(new Scene::Arc(a, somecolors[color++ % Nsomecolors], 1));

	scene.add(new Scene::Point(Geom2d::Point(0.0, 0.0), Image::blue, 10, 1));
	scene.add(new Scene::Point(Geom2d::Point(scene.width, scene.height), Image::green, 10, 1));
}

void Ui::key(int key, bool down) {
	printf("key %d %s\n", key, down ? "down" : "up");
}

void Ui::motion(int x, int y, int dx, int dy) {
	printf("mouse motion: (%d, %d) delta=(%d, %d)\n", x, y, dx, dy);
}

void Ui::click(int x ,int y, int button, bool down) {
	printf("mouse click: button %d at (%d, %d) %s\n", button, x, y, down ? "down" : "up");
}
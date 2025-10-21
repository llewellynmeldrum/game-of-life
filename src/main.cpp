// Metal
#include "SDL_metal.h"
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define NS_PRIVATE_IMPLEMENTATION
#include "metal-cpp_sdl.hpp"

#include <iostream>
#include <cstdlib>

#include "log.h"

int main() {
	SDMTL engine = SDMTL();
	engine.init();
	engine.run();
	engine.cleanup();
	exit(EXIT_SUCCESS);
}

/* public */
void SDMTL::init() {
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");
	if (SDL_Init(SDL_INIT_EVERYTHING)) {
		logsdl("Failed to initialize SDL");
		logexit(EXIT_FAILURE);
	}
	int render_flags = 0;
	int window_flags = SDL_WINDOW_SHOWN & SDL_WINDOW_METAL & SDL_WINDOW_INPUT_FOCUS;

	sdl.win = SDL_CreateWindow("Metal hello world", 0, 0, sdl.width_px, sdl.height_px, window_flags);
	if (!sdl.win) logsdl_exit("Failed to initialize window");

	sdl.renderer = SDL_CreateRenderer(sdl.win, -1, render_flags);
	if (!sdl.renderer) logsdl_exit("Failed to initialize renderer");


	mtl.layer = (CA::MetalLayer*)SDL_RenderGetMetalLayer(sdl.renderer);
	mtl.device = mtl.layer->device();
	auto name = mtl.device->name();
	// make sure we grabbed device
	std::cerr << "device name: " << name->utf8String() << std::endl;
}

void SDMTL::run() {
	while(sdl.running) {
		handle_input();
		SDL_RenderPresent(sdl.renderer);
	}
}

void SDMTL::cleanup() {
	SDL_DestroyRenderer(sdl.renderer);
	SDL_DestroyWindow(sdl.win);
	SDL_Quit();
	mtl.device->release();
}

/* private */


void SDMTL::handle_input() {
	SDL_Event event;
	while( SDL_PollEvent( &event ) ) {
		switch( event.type ) {
		case SDL_QUIT:
			sdl.running = false;
		case SDL_KEYDOWN:
			handle_keypress(event.key.keysym);
			break;

		case SDL_KEYUP:
			break;

		default:
			break;
		}
	}
}

void SDMTL::handle_keypress(SDL_Keysym keysym) {
	switch (keysym.sym) {
	case SDLK_ESCAPE:
		sdl.running = false;
		break;
	case 'c':
	case 'C':
		if (keysym.mod & KMOD_CTRL) sdl.running = false;
		break;
	}
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <SDL.h>
#include "log.h"
#define PROGNAME "SDL2 cpu only GOL"
#define SCR_WIDTH 800
#define SCR_HEIGHT 600

#define min(a,b) (a<=b?a:b)
#define max(a,b) (a>=b?a:b)


#define SDL_RENDER_FLAGS SDL_RENDERER_SOFTWARE
// #define SDL_RENDER_FLAGS SDL_RENDERER_ACCELERATED

static void init_SDL();
static void exit_SDL();

static void handle_input();
static void update();
static void draw();

static SDL_Window *window;
static SDL_Renderer *renderer;
bool running = true;

int main() {
	init_SDL();

	while (running) {
		handle_input();
		update();
		draw();
	}

	exit_SDL();
}

static void exit_SDL() {
	SDL_Quit();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	exit(EXIT_SUCCESS);
}

char *SDLMod_to_str(uint16_t mod) {
	if( mod & KMOD_NUM 	) 	return  "NUMLOCK ";
	if( mod & KMOD_CAPS 	) 	return  "CAPSLOCK ";
	if( mod & KMOD_LCTRL 	) 	return  "LCTRL ";
	if( mod & KMOD_RCTRL 	) 	return  "RCTRL ";
	if( mod & KMOD_RSHIFT 	) 	return  "RSHIFT ";
	if( mod & KMOD_LSHIFT 	) 	return  "LSHIFT ";
	if( mod & KMOD_RALT 	) 	return  "RALT ";
	if( mod & KMOD_LALT 	) 	return  "LALT ";
	if( mod & KMOD_CTRL 	) 	return  "CTRL ";
	if( mod & KMOD_SHIFT 	) 	return  "SHIFT ";
	if( mod & KMOD_ALT 	) 	return  "ALT ";
	return "";
}

void log_key_info(SDL_Keysym key) {
	char *mod = SDLMod_to_str(key.mod);
	log("key down: %s + ", mod);
	log("%d\n", key.sym);
}


void handle_keypress(SDL_Keysym key) {
	switch (key.sym) {
	case SDLK_ESCAPE:
		running = false;
		break;
	case 'c':
	case 'C':
		if (key.mod & KMOD_CTRL)  running = false;
		break;
	default:
		log_key_info(key);
		break;
	}
}

static void handle_input() {
	SDL_Event event;
	while( SDL_PollEvent( &event ) ) {
		switch( event.type ) {
		case SDL_QUIT:
			running = false;
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

static void update() {

}

static void draw() {
	// Initialize renderer color white for the background
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	// Clear screen
	SDL_RenderClear(renderer);
	// Update screen
	SDL_RenderPresent(renderer);
}

static void init_SDL() {
	if (SDL_Init(SDL_INIT_VIDEO & SDL_INIT_EVENTS)) {
		logsdl("Failed to initialize SDL");
		logexit(EXIT_FAILURE);
	}

	const int WIN_FLAGS = SDL_WINDOW_SHOWN & SDL_WINDOW_METAL & SDL_WINDOW_INPUT_FOCUS;
	window = SDL_CreateWindow(PROGNAME, 0, 0, SCR_WIDTH, SCR_HEIGHT, WIN_FLAGS);
	if (!window) logsdl_exit("Failed to initialize window");

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDER_FLAGS);
	if (!renderer) logsdl_exit("Failed to initialize renderer");
}

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include "SDL_timer.h"
#include "log.h"
#include "alloc.h"
#include "ring_buffer.h"
#define PROGNAME "SDL2 cpu only GOL"
#define SCR_WIDTH 800
#define SCR_HEIGHT 600

#define min(a,b) (a<=b?a:b)
#define max(a,b) (a>=b?a:b)


#define SDL_RENDER_FLAGS SDL_RENDERER_SOFTWARE
// #define SDL_RENDER_FLAGS SDL_RENDERER_ACCELERATED // enable for gpu acceleration
/* SDL_COLORS */
const SDL_Color white = { 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE };
const SDL_Color black = { 0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE };



/* SDL_HELPERS */
#define SDL_RGBA(r,g,b,a) 	SDL_SetRenderDrawColor(renderer, r, g, b, a)
#define SDL_RGB(r,g,b) 		SDL_RGBA(col.r,col.g,col.b,SDL_ALPHA_OPAQUE)
#define SDL_Color(col) 		SDL_RGBA(col.r,col.g,col.b,col.a)
#define SDL_Point(x, y) SDL_RenderDrawPoint(renderer, x, y)

typedef enum {
	DEAD,
	ALIVE
} CELL_STATE;
/* LOCAL HELPERS */
#define get_cell(x,y) (cells[y][x])




static void init_SDL();
static void exit_SDL();

void init_cells();
static void handle_input();
static void update();
static void draw();

static SDL_Window *window;
static SDL_Renderer *renderer;
static int cells_cols, cells_rows;
static int cell_width_px, cell_height_px;
bool running = true;

static int scaling_factor = 4;
static CELL_STATE **cells;

// perf
uint64_t SDL_CLOCKS_PER_SEC;
#define measure_time() SDL_GetPerformanceCounter()

#define ms_between(t0, t1) ( ((double)(t1-t0))/ SDL_CLOCKS_PER_SEC )*1000.0
#define s_between(t0, t1) ( ((double)(t1-t0))/ SDL_CLOCKS_PER_SEC )



static size_t generation_count = 0;
static size_t global_pop_count = 0;
RingBuffer_t ms_input_rb, ms_update_rb, ms_draw_rb, ms_frametime_rb, fps_rb;

void init_metrics() {
	RB_COUNT = 0;
	ms_input_rb = 		init_RingBuffer("INPUT(MS)");
	ms_update_rb = 		init_RingBuffer("UPDATE(MS)");
	ms_draw_rb = 		init_RingBuffer("DRAW(MS)");
	ms_frametime_rb = 	init_RingBuffer("FRAMETIME(MS)");
	fps_rb = 		init_RingBuffer("FPS");
}

void update_metrics(double ms_input, double ms_update, double ms_draw) {
	double ms_frametime = ms_input + ms_update + ms_draw;
	double fps = 1.0 / (ms_frametime / 1000.0);
	//

	RingBuffer_put(&ms_input_rb, ms_input);
	RingBuffer_put(&ms_update_rb, ms_update);
	RingBuffer_put(&ms_draw_rb, ms_draw);
	RingBuffer_put(&ms_frametime_rb, ms_frametime);
	RingBuffer_put(&fps_rb, fps);
}
static inline void set_cell(int x, int y, CELL_STATE new_val) {
	CELL_STATE prev_val = cells[y][x];
	cells[y][x] = new_val;
	// dead->alive ++
	// alive->dead --
	if (prev_val == ALIVE && new_val == DEAD) global_pop_count--;
	if (prev_val == DEAD && new_val == ALIVE) global_pop_count++;
}

int main() {
	init_SDL();
	init_cells();
	init_metrics();

	double ms_input, ms_update, ms_draw, s_execution_time, ms_execution_time;
	uint64_t prog_t0 = measure_time();
	uint64_t t0, frame_count;
	frame_count = 0;
	while (running) {
		t0 = measure_time();
		handle_input();
		ms_input = ms_between(t0, measure_time());

		t0 = measure_time();
		update();
		ms_update = ms_between(t0, measure_time());


		t0 = measure_time();
		draw();
		ms_draw = ms_between(t0, measure_time());

		update_metrics(ms_input, ms_update, ms_draw);
		frame_count++;
	}
	s_execution_time = s_between(prog_t0, measure_time());
	ms_execution_time = ms_between(prog_t0, measure_time());
	// dump ring buffers
//	RingBuffer_log(ms_input_rb, "ms");
//	RingBuffer_log(ms_update_rb, "ms");
//	RingBuffer_log(ms_draw_rb, "ms");
//	RingBuffer_log(ms_frametime_rb, "ms");
//	RingBuffer_log(fps_rb, "fps");
	log("framecount:%llu, took %0.0lfms\n", frame_count, s_execution_time * 1000.0);
	log("averages:\n");



	double ms_input_avg 		= RingBuffer_avg(ms_input_rb);
	double ms_update_avg 		= RingBuffer_avg(ms_update_rb);
	double ms_draw_avg 		= RingBuffer_avg(ms_draw_rb);
	double ms_frametime_avg 	= RingBuffer_avg(ms_frametime_rb);
	double fps_avg 			= RingBuffer_avg(fps_rb);

	log("\tinput:               %0.2lfms              %0.1lf%%\n ", 	ms_input_avg 	, ms_input_avg / ms_frametime_avg * 100.0);
	log("\tupdate:              %0.2lfms              %0.1lf%%\n ", 	ms_update_avg 	, ms_update_avg / ms_frametime_avg * 100.0);
	log("\tdraw:                %0.2lfms              %0.1lf%%\n ", 	ms_draw_avg 	, ms_draw_avg / ms_frametime_avg * 100.0);
	log("\tframetime/overall:   %0.2lfms/%0.2lfms      100.0%%\n ",    	ms_frametime_avg, (double)ms_execution_time / frame_count);
	log("\tfps/overall:         %0.2lf/%0.2lffps\n",	fps_avg, 	((double)frame_count / s_execution_time));
	log("\tpopcount:            %zu\n",	global_pop_count);
	log("\tgeneration:          %zu\n",	generation_count);
	exit_SDL();
}

void init_cells() {
	int window_width, window_height;
	SDL_GetWindowSize(window, &window_width, &window_height);
	log("win     size: %dx%d\n", window_width, window_height);

	assert(cells_cols % scaling_factor == 0);
	assert(cells_rows % scaling_factor == 0);
	cells_cols = window_width / scaling_factor;
	cells_rows = window_height / scaling_factor;

	cell_width_px = scaling_factor;
	cell_height_px = scaling_factor;
	log("cells   size: %dx%d\n", cells_cols, cells_rows);
	log("cell    size: %dx%d\n", cell_width_px, cell_height_px);
	cells = ALLOC(cells_rows, sizeof(CELL_STATE*));
	if (!cells) logfatalerrno("calloc1");

	for (int y = 0; y < cells_rows; y++) {
		cells[y] = ALLOC(cells_cols, sizeof(CELL_STATE));
		if (!cells[y]) logfatalerrno("calloc1[%d]", y);
	}

	// temp to check if rendering works
	for (int y = 0; y < cells_rows; y++) {
		for (int x = 0; x < cells_cols; x++) {
			if ((x + (y * cells_cols)) % 2 == 0) {
				set_cell(x, y, true);
			}
		}
	}
}
static void exit_SDL() {
	SDL_Quit();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	exit(EXIT_SUCCESS);
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
		//log_key_info(key);
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

size_t population_count(int init_x, int init_y) {
	size_t pop_count = 0;
	for (int xi = -1; xi <= 1; xi++) {
		int x = init_x + xi;
		if (x >= cells_cols || x < 0) continue;
		for (int iter_y = -1; iter_y <= 1; iter_y++) {
			int y = init_y + iter_y;
			if (y >= cells_rows || y < 0 ) continue;
			if (get_cell(x, y)) pop_count++;
		}
	}
	return pop_count;
}

// popcount:
// fps:
// frametime:
// drawtime:
// update time:
//
static void update() {
	for (int y = 0; y < cells_rows; y++) {
		for (int x = 0; x < cells_cols; x++) {
			size_t pop_count = population_count(x, y);
			if (pop_count < 2) set_cell(x, y, DEAD); // underpopulation
			if (pop_count >= 4) set_cell(x, y, DEAD); // overpopulation
			if (pop_count == 3) set_cell(x, y, ALIVE);
		}
	}
	generation_count++;
}

void draw_cell(int x, int y) {
	for (int xi = 0; xi < cell_width_px; xi++) {
		for (int yi = 0; yi < cell_height_px; yi++) {
			int px = x * scaling_factor;
			int py = y * scaling_factor;
			SDL_Point(px + xi, py + yi);
		}
	}
}

static void draw_cells() {
	SDL_Color(black);
	for (int y = 0; y < cells_rows; y++) {
		for (int x = 0; x < cells_cols; x++) {
			if (cells[y][x] == true) {
				draw_cell(x, y);
			}
		}
	}
}

static void draw() {
	// Initialize renderer color white for the background
	SDL_Color(white);
	SDL_RenderClear(renderer);
	draw_cells();
	SDL_RenderPresent(renderer);
}

static void init_fonts() {

}

static void init_SDL() {
	if (SDL_Init(SDL_INIT_EVERYTHING)) {
		logsdl("Failed to initialize SDL");
		logexit(EXIT_FAILURE);
	}

	const int WIN_FLAGS = SDL_WINDOW_SHOWN & SDL_WINDOW_METAL & SDL_WINDOW_INPUT_FOCUS;
	window = SDL_CreateWindow(PROGNAME, 0, 0, SCR_WIDTH, SCR_HEIGHT, WIN_FLAGS);
	if (!window) logsdl_exit("Failed to initialize window");

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDER_FLAGS);
	if (!renderer) logsdl_exit("Failed to initialize renderer");
	init_fonts();
	SDL_CLOCKS_PER_SEC = SDL_GetPerformanceFrequency();
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


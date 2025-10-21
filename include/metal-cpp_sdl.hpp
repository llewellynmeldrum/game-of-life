#pragma once


#include <Metal/Metal.hpp>
#include <QuartzCore/CAMetalLayer.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <simd/simd.h>

// windowing, input
#include <SDL.h>


class SDMTL {
  public:
	// sdl state
	struct SDL_Context {
		SDL_Window *win = nullptr;
		SDL_Renderer *renderer = nullptr;
		SDL_MetalView metal_view = nullptr;
		bool running = true;
		int width_px =  800;
		int height_px = 600;
		int win_flags = SDL_WINDOW_METAL & SDL_WINDOW_INPUT_FOCUS;
		int renderer_flags = SDL_RENDERER_PRESENTVSYNC;
	};

	// metal state
	struct MTL_Context {
		MTL::Device *device;
		CA::MetalLayer *layer;
	};

	SDL_Context sdl;
	MTL_Context mtl;

	void init();
	void run();
	void cleanup();
  private:

	void load_library(NS::String* shader_path);
	void create_command_queue();
	void create_render_pipeline();

	void encode_render_command(MTL::RenderCommandEncoder* render_encoder);
	void send_render_cmd();
	void draw();

	void handle_input();
	void handle_keypress(SDL_Keysym key);
	void sdl_draw();
};


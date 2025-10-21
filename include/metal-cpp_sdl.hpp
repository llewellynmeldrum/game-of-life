#pragma once


#include <Metal/Metal.hpp>
#include <QuartzCore/CAMetalLayer.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <simd/simd.h>

// windowing, input
#include <SDL.h>

ssize_t syscall_file_size(const char* filename);
char *read_file(const char* filename);

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
		CA::MetalDrawable *drawable;
		MTL::Library *lib;
		MTL::CommandQueue *command_queue;
		MTL::CommandBuffer *command_buf;
		MTL::RenderPipelineState *pipeline_state;
		MTL::Buffer *triangle_vertex_buf;

		MTL::ClearColor clear_col =
		{41.0f / 255.0f, 42.0f / 255.0f, 48.0f / 255.0f, 1.0};
	};
	struct Triangle {
		MTL::Buffer *vertex_buf;
	};

	SDL_Context sdl;
	MTL_Context mtl;
	Triangle triangle;

	void init();
	void run();
	void cleanup();
  private:
	NS::Error *err;

	// metal
	void load_library(const char* msl_path);
	void create_render_pipeline();

	void encode_render_command(MTL::RenderCommandEncoder* render_encoder);
	void send_render_cmd();
	void draw();

	void init_triangle(Triangle t);
	void handle_input();
	void handle_keypress(SDL_Keysym key);
	void mtl_draw();
	void sdl_draw();

};


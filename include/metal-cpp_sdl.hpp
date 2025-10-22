#pragma once


#include "SDL_render.h"
#include <Metal/Metal.hpp>
#include <QuartzCore/CAMetalLayer.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <simd/simd.h>

// windowing, input
#include <SDL.h>

ssize_t syscall_file_size(const char* filename);
char *read_file(const char* filename);




struct SDL_Context {
	SDL_Window *win = nullptr;
	SDL_Renderer *renderer = nullptr;
	SDL_MetalView metal_view = nullptr;
	bool running = true;
	int width_px =  800;
	int height_px = 600;
	int win_flags = SDL_WINDOW_METAL & SDL_WINDOW_INPUT_FOCUS;
	int renderer_flags = SDL_RENDERER_PRESENTVSYNC & SDL_RENDERER_ACCELERATED;
};

// metal state
struct MTL_Context {
	CA::MetalLayer *layer;
	MTL::Device *device;
	MTL::CommandQueue *command_queue;
	CA::MetalDrawable *drawable;
	MTL::RenderPipelineState *pipeline_state;
	MTL::Buffer *vertex_buf;

	MTL::Texture *gen_a;
	MTL::Texture *gen_b;

	NS::UInteger vertex_offset = 0;
	NS::UInteger vertex_count = 6;
	MTL::PrimitiveType primitive_type = MTL::PrimitiveTypeTriangle;

	MTL::ClearColor clear_col = {41.0f / 255.0f, 42.0f / 255.0f, 48.0f / 255.0f, 1.0};
};
// *INDENT-OFF*
class GOL {
  public:
	void init(const char* mtl_shader_path);
	void run();
	void cleanup();

	size_t generation = 0;
	MTL::Texture *create_texture();
	inline MTL::Texture *get_current_gen_texture(){
		return (generation%2==0 ? mtl.gen_a : mtl.gen_b);
	}

	SDL_Context sdl;
	MTL_Context mtl;

  private:
	void mtl_init(const char* shader_path);
	void sdl_init();
	void update();
	void draw();

	NS::Error *err;

	MTL::Function *setup_fragment_fn(MTL::Library* lib, MTL::RenderPipelineDescriptor* pld, const char* fragment_fn_name_cstr);
	MTL::Function *setup_vertex_fn(MTL::Library* lib, MTL::RenderPipelineDescriptor* pld, const char* vertex_fn_name_cstr);
	MTL::Buffer *setup_vertex_buffer();
	// metal
	MTL::Library* load_mtl_lib_from_src(const char* msl_path);
	void create_render_pipeline();

	void encode_render_command(MTL::RenderCommandEncoder* render_encoder);
	void send_render_cmd();

	void init_textures();
	void handle_input();
	void handle_keypress(SDL_Keysym key);
	void mtl_draw();

};


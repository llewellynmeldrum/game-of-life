#pragma once
#include <simd/simd.h>


#include "Foundation/Foundation.hpp"
#include "Metal/MTLCommandQueue.hpp"
#include "Metal/MTLRenderCommandEncoder.hpp"
#include "Metal/MTLRenderPipeline.hpp"
#include "Metal/Metal.hpp"
#include "QuartzCore/CAMetalDrawable.hpp"
#include "QuartzCore/QuartzCore.hpp"

#include <SDL.h>

class MTLEngine {
  public:
	void init();
	void run();
	void cleanup();
  private:

	void create_triangle();
	void create_default_lib();
	void create_cmd_queue();
	void create_render_pipeline();

	void encode_render_command(MTL::RenderCommandEncoder* render_encoder);
	void send_render_cmd();
	void draw();

	SDL_Renderer *sdl_renderer;
	SDL_Window *sdl_window;

	CA::MetalLayer *mtl_layer;
	CA::MetalDrawable *mtl_drawable;

	MTL::Device *mtl_device;
	MTL::Library *mtl_default_lib;
	MTL::CommandQueue *mtl_cmd_queue;
	MTL::CommandBuffer *mtl_cmd_buf;
	MTL::RenderPipelineState *metal_render_pso;
	MTL::Buffer *triangle_vtx_buf;


	bool sdl_running = true;
	const size_t screen_width_px = 800;
	const size_t screen_height_px = 600;
	void init_mtl_sdl();
	void sdl_handle_input();
	void sdl_draw();
};


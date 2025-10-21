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

	void init_mtl_sdl();
	void sdl_handle_input();
	void sdl_draw();
};


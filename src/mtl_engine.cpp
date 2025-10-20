
#include "Foundation/NSString.hpp"
#include "Metal/MTLDrawable.hpp"
#include "Metal/MTLPixelFormat.hpp"
#include "Metal/MTLRenderCommandEncoder.hpp"
#include "Metal/MTLRenderPass.hpp"
#include "Metal/MTLRenderPipeline.hpp"
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define NS_PRIVATE_IMPLEMENTATION

#include "mtl_engine.hpp"
#include <SDL.h>
#include <cstdlib>
#include "log.h"
#include <iostream>

static bool is_exit_keypress(SDL_Keysym key);

void MTLEngine::sdl_handle_input() {
	SDL_Event event;
	while( SDL_PollEvent( &event ) ) {
		switch( event.type ) {
		case SDL_QUIT:
			sdl_running = false;
		case SDL_KEYDOWN:
			sdl_running = is_exit_keypress(event.key.keysym);
			break;

		case SDL_KEYUP:
			break;

		default:
			break;
		}
	}
}

void MTLEngine::run() {
	while(sdl_running) {
		mtl_drawable = mtl_layer->nextDrawable();
		send_render_cmd();
		sdl_handle_input();
		SDL_RenderPresent(sdl_renderer);
	}
}

void MTLEngine::init() {
	init_mtl_sdl();
	create_triangle();
	create_default_lib();
	create_cmd_queue();
}

void MTLEngine::create_triangle() {
	simd::float3 triangle_vtxs[] = {
		{-0.5f, -0.5f, 0.0f},
		{ 0.5f, -0.5f, 0.0f},
		{ 0.0f,  0.5f, 0.0f},
	};
	triangle_vtx_buf = mtl_device->newBuffer(
	                       &triangle_vtxs,
	                       sizeof(triangle_vtxs),
	                       MTL::ResourceStorageModeShared);
}

void MTLEngine::create_default_lib() {
	mtl_default_lib = mtl_device->newDefaultLibrary();
	if (!mtl_default_lib) {
		std::cerr << "Failed to initialize metal default library.";
		std::exit(EXIT_FAILURE);
	}
}

void MTLEngine::create_cmd_queue() {
	mtl_cmd_queue = mtl_device->newCommandQueue();
}

void MTLEngine::create_render_pipeline() {
	auto *vertex_fn_name = NS::String::string("vertex_shader", NS::ASCIIStringEncoding);
	auto *fragment_fn_name = NS::String::string("fragment_shader", NS::ASCIIStringEncoding);

	auto vertex_shader = mtl_default_lib->newFunction(vertex_fn_name);
	auto fragment_shader = mtl_default_lib->newFunction(fragment_fn_name);
	assert(vertex_shader);
	assert(fragment_shader);

	auto pipeline_descriptor = MTL::RenderPipelineDescriptor::alloc()->init();
	auto render_pipeline_label = NS::String::string("fragment_shader", NS::ASCIIStringEncoding);
	pipeline_descriptor->setLabel(render_pipeline_label);
	pipeline_descriptor->setVertexFunction(vertex_shader);
	pipeline_descriptor->setFragmentFunction(fragment_shader);
	assert(pipeline_descriptor);

	MTL::PixelFormat pixel_fmt = (MTL::PixelFormat)mtl_layer->pixelFormat();
	pipeline_descriptor->colorAttachments()->object(0)->setPixelFormat(pixel_fmt);

	NS::Error* ns_err;
	metal_render_pso = mtl_device->newRenderPipelineState(pipeline_descriptor, &ns_err);

	pipeline_descriptor->release();
}


void MTLEngine::send_render_cmd() {
	mtl_cmd_buf = mtl_cmd_queue->commandBuffer();

	auto render_pass_descriptor = MTL::RenderPassDescriptor::alloc()->init();
	auto cd = render_pass_descriptor->colorAttachments()->object(0);

	cd->setTexture(mtl_drawable->texture());
	cd->setLoadAction(MTL::LoadActionClear);
	cd->setClearColor(MTL::ClearColor(41.0f / 255.0f, 42.0f / 255.0f, 48.0f / 255.0f, 1.0));
	cd->setStoreAction(MTL::StoreActionStore);


	MTL::RenderCommandEncoder* renderCommandEncoder = mtl_cmd_buf->renderCommandEncoder(render_pass_descriptor);
	encode_render_command(renderCommandEncoder);
	renderCommandEncoder->endEncoding();


	mtl_cmd_buf->presentDrawable((const MTL::Drawable*)mtl_drawable);
	mtl_cmd_buf->commit();
	mtl_cmd_buf->waitUntilCompleted();

	render_pass_descriptor->release();

}
void MTLEngine::encode_render_command(MTL::RenderCommandEncoder* cmd_encoder) {
	cmd_encoder->setRenderPipelineState(metal_render_pso);
	cmd_encoder->setVertexBuffer(triangle_vtx_buf, 0, 0);
	MTL::PrimitiveType typeTriangle = MTL::PrimitiveTypeTriangle;
	NS::UInteger vtx_start = 0;
	NS::UInteger vtx_count = 3;
	cmd_encoder->drawPrimitives(typeTriangle, vtx_start, vtx_count);
}

void MTLEngine::cleanup() {
	SDL_Quit();
	SDL_Metal_DestroyView(mtl_layer);
	SDL_DestroyRenderer(sdl_renderer);
	SDL_DestroyWindow(sdl_window);
	mtl_device->release();
}


void MTLEngine::init_mtl_sdl() {
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "mtl");
	if (SDL_Init(SDL_INIT_EVERYTHING)) {
		logsdl("Failed to initialize SDL");
		logexit(EXIT_FAILURE);
	}
	int render_flags = 0;
	int window_flags = SDL_WINDOW_SHOWN & SDL_WINDOW_METAL & SDL_WINDOW_INPUT_FOCUS;

	sdl_window = SDL_CreateWindow("Metal hello world", 0, 0, screen_width_px, screen_height_px, window_flags);
	if (!sdl_window) logsdl_exit("Failed to initialize window");

	sdl_renderer = SDL_CreateRenderer(sdl_window, -1, render_flags);
	if (!sdl_renderer) logsdl_exit("Failed to initialize renderer");

	// mtl stuff
	mtl_layer = (CA::MetalLayer*)SDL_RenderGetMetalLayer(sdl_renderer);
	mtl_device = mtl_layer->device();
	auto name = mtl_device->name();
	std::cerr << "device name: " << name->utf8String() << std::endl;
}


static bool is_exit_keypress(SDL_Keysym key) {
	switch (key.sym) {
	case SDLK_ESCAPE:
		return false;
		break;
	case 'c':
	case 'C':
		if (key.mod & KMOD_CTRL) return false;
		break;
	default:
		return true;
		break;
	}
	return true;
}

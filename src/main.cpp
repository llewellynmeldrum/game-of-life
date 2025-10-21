// Metal
#include "SDL_metal.h"
#include <unistd.h>
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define NS_PRIVATE_IMPLEMENTATION
#include "metal-cpp_sdl.hpp"

#include <iostream>
#include <cstdlib>

#include "log.h"
#include <sys/stat.h>

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

	load_library("shaders/triangle.metal");
	init_triangle(triangle);
	mtl.command_queue = mtl.device->newCommandQueue();

	auto pipeline_descriptor = MTL::RenderPipelineDescriptor::alloc()->init();
	if (!pipeline_descriptor) {
		logfatal("Failed to create pipeline!\n");
		logexit(EXIT_FAILURE);
	}

	auto vertex_fn_name = NS::String::string("vertex_shader", NS::ASCIIStringEncoding);
	auto vertex_fn = mtl.lib->newFunction(vertex_fn_name);
	pipeline_descriptor->setVertexFunction(vertex_fn);

	auto frag_fn_name = NS::String::string("fragment_shader", NS::ASCIIStringEncoding);
	auto frag_fn = mtl.lib->newFunction(frag_fn_name);
	pipeline_descriptor->setFragmentFunction(frag_fn);
	assert(pipeline_descriptor);


	auto pixel_format = (MTL::PixelFormat)mtl.layer->pixelFormat();
	pipeline_descriptor->colorAttachments()->object(0)->setPixelFormat(pixel_format);
	mtl.pipeline_state = mtl.device->newRenderPipelineState(pipeline_descriptor, &err);
	if (!mtl.pipeline_state) {
		logfatal("Failed to create pipeline!\n");
		logexit(EXIT_FAILURE);
	}

	pipeline_descriptor->release();

}




void SDMTL::run() {
	while(sdl.running) {
		handle_input();
		mtl_draw();
	}
}

void SDMTL::mtl_draw() {
	/* init command buffer from command queue */
	auto auto_release_pool = NS::AutoreleasePool::alloc()->init();
	mtl.drawable = mtl.layer->nextDrawable();
	mtl.command_buf = mtl.command_queue->commandBuffer();

	/* create render pass */
	auto render_pass = MTL::RenderPassDescriptor::alloc()->init();
	/* add color attachments for the pass */
	auto pass_color_attachment = render_pass->colorAttachments()->object(0);
	pass_color_attachment->setTexture(mtl.drawable->texture());
	pass_color_attachment->setLoadAction(MTL::LoadActionClear);
	pass_color_attachment->setClearColor(MTL::ClearColor(mtl.clear_col));
	pass_color_attachment->setStoreAction(MTL::StoreActionStore);


	/* command encoding */
	auto command_encoder = mtl.command_buf->renderCommandEncoder(render_pass);
	command_encoder->setRenderPipelineState(mtl.pipeline_state);
	command_encoder->setVertexBuffer(triangle.vertex_buf, 0, 0);
	NS::UInteger vertex_offset = 0;
	NS::UInteger vertex_count = 3;
	command_encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, vertex_offset, vertex_count);
	command_encoder->endEncoding();

	mtl.command_buf->presentDrawable(mtl.drawable);
	mtl.command_buf->commit();
//	mtl.command_buf->waitUntilCompleted();

	auto_release_pool->release();


}

void SDMTL::cleanup() {
	SDL_DestroyRenderer(sdl.renderer);
	SDL_DestroyWindow(sdl.win);
	SDL_Quit();
	mtl.device->release();
}

/* private */

void SDMTL::load_library(const char* msl_path) {
	char *msl_src = read_file(msl_path);
	if (!msl_src) {
		logsdl("Failed to read file '%s' \n", msl_path);
		logexit(EXIT_FAILURE);
	}
	auto msl_NS_FORMAT = NS::String::string(msl_src, NS::ASCIIStringEncoding);

	auto compile_opts = MTL::CompileOptions::alloc()->init();
	mtl.lib = mtl.device->newLibrary(msl_NS_FORMAT, compile_opts, &err);
	if (!mtl.lib) {
		logfatal("Failed to initialize library '%s'.", msl_path);
		logfn("\n%s:%s\n", err->domain()->utf8String(), err->localizedDescription()->utf8String());
		logexit(EXIT_FAILURE);
	}
}
void SDMTL::init_triangle(Triangle &t) {
	simd::float3 triangle_verticies[] = {
		{-0.5f, -0.5f, 0.0f},
		{ 0.5f, -0.5f, 0.0f},
		{ 0.0f,  0.5f, 0.0f}
	};

	t.vertex_buf = mtl.device->newBuffer(&triangle_verticies,
	                                     sizeof(triangle_verticies),
	                                     MTL::ResourceStorageModeShared);

}

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

ssize_t syscall_file_size(const char* filename) {
	struct stat s;
	stat(filename, &s);
	return s.st_size;
}

char *read_file(const char* filename) {
	size_t const MAX_FILE_SZ = 50'000'000; // 50MB
	size_t file_size = syscall_file_size(filename);

	FILE * file_ptr = fopen(filename, "rb");

	if (!file_ptr) {
		logfatalerrno("Unable to open file '%s'.\n", filename);
		return NULL;
	}

	if (file_size == 0) {
		logfatal("syscall_file_size(%s) ret=0.\n", filename);
		fclose(file_ptr);
		return NULL;
	}

	if (file_size > MAX_FILE_SZ) {
		logfatal("Requested file '%s' is too large! (%zu>%zu)\n", filename, file_size, MAX_FILE_SZ);
		fclose(file_ptr);
		return NULL;
	}
	char *file_contents = (char*)malloc(sizeof(char) * file_size);
	if (!file_contents) {
		logfatal("Unable to alloc buffer for file '%s'.\n", filename);
		fclose(file_ptr);
		return NULL;
	}

	int n_read = fread(file_contents, file_size, 1, file_ptr);
	if (n_read != 1) {
		logfatal("Unable to read file contents for file '%s'.\n", filename);
		free(file_contents);
		fclose(file_ptr);
		return NULL;
	}
	fclose(file_ptr);
	return file_contents;
}

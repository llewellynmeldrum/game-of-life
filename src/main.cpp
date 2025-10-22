// Metal
#include "SDL_metal.h"
#include <unistd.h>
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define NS_PRIVATE_IMPLEMENTATION
#include "metal-cpp_sdl.hpp"

#include <iostream>
#include <functional>
#include <cstdlib>

#include "log.h"
#include <sys/stat.h>

int main() {
	GOL sim = GOL();
	sim.init("shaders/shader_test.metal");
	sim.run();
	sim.cleanup();
	exit(EXIT_SUCCESS);
}


void GOL::init(const char* shader_src_path) {
	sdl_init();

	auto init_pool = NS::AutoreleasePool::alloc()->init();
	mtl_init(shader_src_path);
	init_pool->release();
	reset_sim();
}

MTL::Function *GOL::setup_vertex_fn(MTL::Library* lib, MTL::RenderPipelineDescriptor* pld, const char* vertex_fn_name_cstr) {
	auto vertex_fn_name = NS::String::string(vertex_fn_name_cstr, NS::ASCIIStringEncoding);
	auto vertex_fn = lib->newFunction(vertex_fn_name);
	pld->setVertexFunction(vertex_fn);
	return vertex_fn;

}

MTL::Function *GOL::setup_fragment_fn(MTL::Library* lib, MTL::RenderPipelineDescriptor* pld, const char* fragment_fn_name_cstr) {
	auto fragment_fn_name = NS::String::string(fragment_fn_name_cstr, NS::ASCIIStringEncoding);
	auto fragment_fn = lib->newFunction(fragment_fn_name);
	pld->setFragmentFunction(fragment_fn);
	return fragment_fn;

}

MTL::Buffer *GOL::setup_vertex_buffer() {
	simd::float4 verticies[] = {
		{-1.0, -1.0, 0.0, 1.0,	},
		{   1.0, -1.0, 0.0, 1.0,},
		{  -1.0,  1.0, 0.0, 1.0,},
		{  -1.0,  1.0, 0.0, 1.0,},
		{   1.0, -1.0, 0.0, 1.0,},
		{   1.0,  1.0, 0.0, 1.0 },
	};
	return mtl.device->newBuffer(&verticies, sizeof(verticies), MTL::ResourceStorageModeShared);

}
void GOL::mtl_init(const char* shader_src_path) {

	mtl.layer = (CA::MetalLayer*)SDL_RenderGetMetalLayer(sdl.renderer);
	mtl.device = mtl.layer->device();					/* MUST BE RELEASED LATER */
	mtl.command_queue = mtl.device->newCommandQueue();			/* MUST BE RELEASED */

	mtl.vertex_buf = setup_vertex_buffer();
	auto lib = load_mtl_lib_from_src(shader_src_path);

	auto pipeline_descriptor = MTL::RenderPipelineDescriptor::alloc()->init();
	if (!pipeline_descriptor) {
		logfatal("Failed to create pipeline!\n");
		logexit(EXIT_FAILURE);
	}

	auto vert_fn = setup_vertex_fn(lib, pipeline_descriptor, "vertex_shader");
	auto frag_fn = setup_fragment_fn(lib, pipeline_descriptor, "fragment_shader");


	auto pixel_format = (MTL::PixelFormat)mtl.layer->pixelFormat();
	pipeline_descriptor->colorAttachments()->object(0)->setPixelFormat(pixel_format);
	mtl.render_pipeline_state = mtl.device->newRenderPipelineState(pipeline_descriptor, &err);
	if (!mtl.render_pipeline_state) {
		logfatal("Failed to create pipeline state!\n");
		logexit(EXIT_FAILURE);
	}

	mtl.gen_a = GOL::create_texture();
	mtl.gen_b = GOL::create_texture();



	pipeline_descriptor->release();
	vert_fn->release();
	frag_fn->release();
	lib->release();

}

void GOL::reset_sim() {

	int *bytes = (int*)malloc(sizeof(int) * 800);
	get_current_gen_texture()->replaceRegion(MTL::Region::Make2D(0, 0, 800, 600), 0, bytes, 800 * 4);
	// TODO:
}

MTL::Texture *GOL::create_texture() {
	auto td = MTL::TextureDescriptor::alloc()->init();
	td->setStorageMode(MTL::StorageModeShared);
	td->setUsage(MTL::TextureUsageShaderRead & MTL::TextureUsageShaderWrite);
	td->setPixelFormat(mtl.layer->pixelFormat());
	td->setWidth((NS::UInteger)sdl.width_px);
	td->setHeight((NS::UInteger)sdl.height_px);
	td->setDepth((NS::UInteger)1);

	MTL::Texture* texture = mtl.device->newTexture(td);

	td->release();
	return texture;
}

void GOL::sdl_init() {
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

}


void GOL::run() {
	while(sdl.running) {
		handle_input();
		auto draw_pool = NS::AutoreleasePool::alloc()->init();
		mtl_draw();
		draw_pool->release();
	}
}

void GOL::mtl_draw() {
	/* init command buffer from command queue */
	mtl.drawable = mtl.layer->nextDrawable();
	auto buffer = mtl.command_queue->commandBuffer();

	/* create render pass */
	auto render_pass = MTL::RenderPassDescriptor::alloc()->init();
	auto pass_color_attachment = render_pass->colorAttachments()->object(0);
	pass_color_attachment->setLoadAction(MTL::LoadActionClear);
	pass_color_attachment->setStoreAction(MTL::StoreActionStore);
	pass_color_attachment->setClearColor(MTL::ClearColor(mtl.clear_col));
	pass_color_attachment->setTexture(mtl.drawable->texture());


	/* command encoding */
	auto command_encoder = buffer->renderCommandEncoder(render_pass);
	render_pass->release();

	command_encoder->setRenderPipelineState(mtl.render_pipeline_state);
	command_encoder->setVertexBuffer(mtl.vertex_buf, 0, 0);
	command_encoder->setFragmentTexture(get_current_gen_texture(), 0);
	command_encoder->drawPrimitives(mtl.primitive_type, mtl.vertex_offset, mtl.vertex_count);
	command_encoder->endEncoding();

	/* do compute encoder stuff */


	buffer->presentDrawable(mtl.drawable);
	buffer->commit();
//	buffer->waitUntilCompleted();

//	command_buf->release();
	generation += 1;
}


/* private */

MTL::Library *GOL::load_mtl_lib_from_src(const char* msl_path) {
	char *msl_src = read_file(msl_path);
	if (!msl_src) {
		logsdl("Failed to read file '%s' \n", msl_path);
		logexit(EXIT_FAILURE);
	}
	//log("read file successfully:\n\n%s\n\n", msl_src);
	auto msl_src_ascii = NS::String::string(msl_src, NS::ASCIIStringEncoding);

	//logwarning("converted file \n");
	auto compile_opts = MTL::CompileOptions::alloc()->init();
	MTL::Library* lib = mtl.device->newLibrary(msl_src_ascii, compile_opts, &err);
	if (!lib) {
		logfatal("Failed to initialize library '%s'.", msl_path);
		logfn("\n%s:%s\n", err->domain()->utf8String(), err->localizedDescription()->utf8String());
		logexit(EXIT_FAILURE);
	}
	free(msl_src);
	compile_opts->release();
	return lib;
}

void GOL::cleanup() {
	SDL_DestroyRenderer(sdl.renderer);
	SDL_DestroyWindow(sdl.win);
	SDL_Quit();
	mtl.device->release();
}

void GOL::init_textures() {
}

void GOL::handle_input() {
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

void GOL::handle_keypress(SDL_Keysym keysym) {
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
		fclose(file_ptr);
		logfatal("file '%s' is empty!\n", filename);
		return NULL;
	}


	if (file_size > MAX_FILE_SZ) {
		logfatal("Requested file '%s' is too large! (%zu>%zu)\n", filename, file_size, MAX_FILE_SZ);
		fclose(file_ptr);
		return NULL;
	}
	char *file_contents = (char*)calloc(file_size + 1, sizeof(char));
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

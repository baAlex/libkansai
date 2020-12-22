/*-----------------------------

MIT License

Copyright (c) 2019 Alexander Brandt

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

-------------------------------

 [context/window.c]
 - Alexander Brandt 2019-2020
-----------------------------*/

#include "private.h"


#define DEFAULT_WIDTH 640
#define DEFAULT_HEIGHT 480
#define DEFAULT_FULLSCREEN 0
#define DEFAULT_VSYNC 1


static inline void sPrintWarning(struct jaStatus* st) // Only make noise if the cvar exists
{
	if (st->code != JA_STATUS_INVALID_ARGUMENT && st->code != JA_STATUS_SUCCESS)
		jaStatusPrint("LibKansai", *st);
}


int kaWindowCreate(const struct jaConfiguration* cfg, void (*init_callback)(struct kaWindow*, void*, struct jaStatus*),
                   void (*frame_callback)(struct kaWindow*, struct kaEvents, float, void*, struct jaStatus*),
                   void (*resize_callback)(struct kaWindow*, int, int, void*, struct jaStatus*),
                   void (*function_callback)(struct kaWindow*, int, void*, struct jaStatus*),
                   void (*close_callback)(struct kaWindow*, void*), void* user_data, struct jaStatus* st)
{
	struct kaWindow* window = NULL;

	int cfg_width = DEFAULT_WIDTH;
	int cfg_height = DEFAULT_HEIGHT;
	int cfg_fullscreen = DEFAULT_FULLSCREEN;
	int cfg_vsync = DEFAULT_VSYNC;
	const char* cfg_caption = "LibKansai";

	jaStatusSet(st, "kaWindowCreate", JA_STATUS_SUCCESS, NULL);

	// Configurations
	if (cfg != NULL)
	{
		struct jaStatus cfg_st = {0};

		jaCvarGetValueInt(jaCvarGet(cfg, "render.width"), &cfg_width, &cfg_st);
		sPrintWarning(&cfg_st);
		jaCvarGetValueInt(jaCvarGet(cfg, "render.height"), &cfg_height, &cfg_st);
		sPrintWarning(&cfg_st);
		jaCvarGetValueInt(jaCvarGet(cfg, "render.fullscreen"), &cfg_fullscreen, &cfg_st);
		sPrintWarning(&cfg_st);
		jaCvarGetValueInt(jaCvarGet(cfg, "render.vsync"), &cfg_vsync, &cfg_st);
		sPrintWarning(&cfg_st);
		jaCvarGetValueString(jaCvarGet(cfg, "kansai.caption"), &cfg_caption, &cfg_st);
		sPrintWarning(&cfg_st);
	}

	// Window
	if ((window = InternalAllocWindow()) == NULL)
	{
		jaStatusSet(st, "kaWindowCreate", JA_STATUS_MEMORY_ERROR, NULL);
		goto return_failure;
	}

	window->frame_callback = frame_callback;
	window->resize_callback = resize_callback;
	window->function_callback = function_callback;
	window->close_callback = close_callback;
	window->user_data = user_data;

	// SDL2
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	if ((window->sdl_window = SDL_CreateWindow(cfg_caption, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, cfg_width,
	                                           cfg_height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE)) == NULL)
	{
		fprintf(stderr, "\n%s\n", SDL_GetError());
		jaStatusSet(st, "kaWindowCreate", JA_STATUS_ERROR, "SDL_CreateWindow()");
		goto return_failure;
	}

	if ((window->gl_context = SDL_GL_CreateContext(window->sdl_window)) == NULL)
	{
		fprintf(stderr, "\n%s\n", SDL_GetError());
		jaStatusSet(st, "kaWindowCreate", JA_STATUS_ERROR, "SDL_GL_CreateContext()");
		goto return_failure;
	}

	SDL_SetWindowMinimumSize(window->sdl_window, 320, 240);
	InternalFocusWindow(window);

	SDL_GL_SetSwapInterval(cfg_vsync);

	if (cfg_fullscreen != 0)
		kaSwitchFullscreen(window);

	// GLAD (after context creation)
	if (InternalSwitchContext(window, st) != 0)
		goto return_failure;

	if (InternalInitGlad() != 0)
	{
		jaStatusSet(st, "kaWindowCreate", JA_STATUS_ERROR, "gladLoad()");
		goto return_failure;
	}

	// OpenGL
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glEnableVertexAttribArray(ATTRIBUTE_POSITION);
	glEnableVertexAttribArray(ATTRIBUTE_COLOR);
	glEnableVertexAttribArray(ATTRIBUTE_UV);

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Default objects and values
	{
		uint16_t raw_index[] = {2, 1, 0, 3, 2, 0};
		struct kaVertex raw_vertices[] = {
		    {.position = {-0.5f, -0.5f, 0.0f}, .color = {1.0f, 0.0f, 0.0f, 1.0f}, .uv = {0.0f, 1.0f}},
		    {.position = {-0.5f, +0.5f, 0.0f}, .color = {0.0f, 1.0f, 0.0f, 1.0f}, .uv = {0.0f, 0.0f}},
		    {.position = {+0.5f, +0.5f, 0.0f}, .color = {0.0f, 0.0f, 1.0f, 1.0f}, .uv = {1.0f, 0.0f}},
		    {.position = {+0.5f, -0.5f, 0.0f}, .color = {0.5f, 0.5f, 0.5f, 1.0f}, .uv = {1.0f, 1.0f}}};

		const char* vertex_code =
		    "#version 100\n"
		    "attribute vec3 vertex_position; attribute vec4 vertex_color; attribute vec2 vertex_uv;"
		    "uniform mat4 world; uniform mat4 camera; uniform mat4 local;"
		    "varying vec4 color; varying vec2 uv;"

		    "void main() { color = vertex_color; uv = vertex_uv;"
		    "gl_Position = world * camera * local * vec4(vertex_position, 1.0); }";

		const char* fragment_code =
		    "#version 100\n"
		    "uniform sampler2D texture0;"
		    "varying lowp vec4 color; varying lowp vec2 uv;"

		    "lowp vec4 Overlay(lowp vec4 a, lowp vec4 b) {"
		    "return mix((1.0 - 2.0 * (1.0 - a) * (1.0 - b)), (2.0 * a * b), step(a, vec4(0.5)));}"

		    "void main() { gl_FragColor = Overlay(color, texture2D(texture0, uv)); }";

		struct jaImage image = {0};
		uint8_t image_data[] = {128 + 16, 128 - 16, 128 - 16, 128 + 16};

		image.channels = 1;
		image.format = JA_IMAGE_U8;
		image.width = 2;
		image.height = 2;
		image.size = sizeof(image_data);
		image.data = image_data;

		if (kaIndexInit(window, raw_index, 6, &window->default_index, st) != 0 ||
		    kaVerticesInit(window, raw_vertices, 4, &window->default_vertices, st) != 0 ||
		    kaProgramInit(window, vertex_code, fragment_code, &window->default_program, st) != 0 ||
		    kaTextureInitImage(window, &image, KA_FILTER_BILINEAR, KA_REPEAT, &window->default_texture, st) != 0)
			goto return_failure;

		kaSetProgram(window, &window->default_program);
		kaSetVertices(window, &window->default_vertices);
		kaSetTexture(window, 0, &window->default_texture);

		kaSetWorld(window, jaMatrix4Identity());
		kaSetCameraMatrix(window, jaMatrix4Orthographic(-1.0f, +1.0f, -1.0f, +1.0f, 0.0f, 2.0f), (struct jaVector3){0.0f, 0.0f, 0.0f});
		kaSetLocal(window, jaMatrix4Identity());

		kaSetCleanColor(window, (struct kaRgb){0.0f, 0.0f, 0.0f});
	}

	// First callbacks
	{
		struct jaStatus callback_st = {0};
		callback_st.code = JA_STATUS_SUCCESS; // Assume success

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Init
		if (init_callback != NULL)
			init_callback(window, window->user_data, &callback_st);

		if (callback_st.code != JA_STATUS_SUCCESS)
		{
			jaStatusCopy(&callback_st, st);
			goto return_failure;
		}

		// Resize
		if (window->resize_callback != NULL)
			window->resize_callback(window, cfg_width, cfg_height, window->user_data, &callback_st);

		if (callback_st.code != JA_STATUS_SUCCESS)
		{
			jaStatusCopy(&callback_st, st);
			goto return_failure;
		}
	}

	// Bye!
	SDL_GL_SwapWindow(window->sdl_window);
	return 0;

return_failure:
	if (window != NULL)
		InternalFreeWindow(window);

	return 1;
}


inline void kaWindowDelete(struct kaWindow* window)
{
	window->delete_mark = true;
}


struct jaImage* kaScreenshot(struct kaWindow* window, struct jaStatus* st)
{
	struct jaImage* image = NULL;
	int window_w;
	int window_h;

	jaStatusSet(st, "kaScreenshot", JA_STATUS_SUCCESS, NULL);
	SDL_GetWindowSize(window->sdl_window, &window_w, &window_h);

	// Create a generic image
	if (window->temp_image != NULL)
		jaImageDelete(window->temp_image);

	if ((window->temp_image = jaImageCreate(JA_IMAGE_U8, (size_t)window_w, (size_t)window_h + 1, 4)) == NULL)
	{
		jaStatusSet(st, "kaScreenshot", JA_STATUS_MEMORY_ERROR, NULL);
		goto return_failure;
	}

	image = window->temp_image;

	// Read from OpenGL buffer
	glReadPixels(0, 0, window_w, window_h, GL_RGBA, GL_UNSIGNED_BYTE, image->data);

	if (glGetError() != GL_NO_ERROR)
	{
		// TODO, glReadPixels has tons of corners where it can fail.
		jaStatusSet(st, "kaScreenshot", JA_STATUS_ERROR, NULL);
		goto return_failure;
	}

	for (size_t i = 0;; i++)
	{
		if (i >= (image->height - 1) / 2)
			break;

		// The image has an extra row
		memcpy((uint8_t*)image->data + (image->width * 4) * (image->height - 1),
		       (uint8_t*)image->data + (image->width * 4) * i, (image->width * 4));
		memcpy((uint8_t*)image->data + (image->width * 4) * i,
		       (uint8_t*)image->data + (image->width * 4) * (image->height - 2 - i), (image->width * 4));
		memcpy((uint8_t*)image->data + (image->width * 4) * (image->height - 2 - i),
		       (uint8_t*)image->data + (image->width * 4) * (image->height - 1), (image->width * 4));
	}

	// Bye!
	image->height -= 1; // ;)
	return image;

return_failure:
	if (image != NULL)
		jaImageDelete(image);

	return NULL;
}


inline void kaSwitchFullscreen(struct kaWindow* window)
{
	if (window->is_fullscreen == false)
	{
		SDL_SetWindowFullscreen(window->sdl_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
		window->is_fullscreen = true;
	}
	else
	{
		SDL_SetWindowFullscreen(window->sdl_window, 0);
		window->is_fullscreen = false;
	}

	window->resized_mark = true;
}

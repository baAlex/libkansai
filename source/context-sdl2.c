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

 [context-sdl2.c]
 - Alexander Brandt 2019-2020
-----------------------------*/

#include "context-private.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define WINDOW_MIN_WIDTH 320
#define WINDOW_MIN_HEIGHT 240


static inline void sFreeWindow(struct kaWindow* window)
{
	if (window->temp_image != NULL)
		jaImageDelete(window->temp_image);

	kaProgramFree(window, &window->default_program);
	kaVerticesFree(window, &window->default_vertices);
	kaTextureFree(window, &window->default_texture);

	if (window->gl_context != NULL)
		SDL_GL_DeleteContext(window->gl_context);

	if (window->sdl_window != NULL)
		SDL_DestroyWindow(window->sdl_window);
}


static inline int sSwitchContext(struct kaWindow* window, struct jaStatus* st)
{
	if (g_context.windows.items_no == 0) // No need of switch if there is a single window
		return 0;

	if (SDL_GL_MakeCurrent(window->sdl_window, window->gl_context) != 0)
	{
		fprintf(stderr, "\n%s\n", SDL_GetError());
		jaStatusSet(st, "SwitchWindow", JA_STATUS_ERROR, "SDL_GL_MakeCurrent()");
		return 1;
	}

	return 0;
}


int kaContextStart(const struct jaConfiguration* cfg, struct jaStatus* st)
{
	(void)cfg;

	jaStatusSet(st, "kaContextStart", JA_STATUS_SUCCESS, NULL);

	if (g_context.sdl_references == 0)
	{
		if (SDL_Init(SDL_INIT_VIDEO) != 0)
		{
			fprintf(stderr, "\n%s\n", SDL_GetError());
			jaStatusSet(st, "kaContextStart", JA_STATUS_ERROR, "SDL_Init()");
			return 1;
		}

		g_context.cfg_vsync = true;
		g_context.cfg_filter = FILTER_NONE;
	}

	g_context.sdl_references += 1;
	return 0;
}


void kaContextStop()
{
	g_context.sdl_references -= 1;

	if (g_context.sdl_references == 0)
	{
		if (g_context.windows.items_no != 0)
			fprintf(stderr, "Deleting %s...\n", (g_context.windows.items_no > 1) ? "windows" : "window");

		while (g_context.windows.items_no != 0)
		{
			sFreeWindow((struct kaWindow*)(g_context.windows.last->data));
			jaListRemove(g_context.windows.last);
		}

		SDL_Quit();
		memset(&g_context, 0, sizeof(struct kaContext));
	}
}


int kaContextUpdate(struct jaStatus* st)
{
	struct kaWindow* window = NULL;
	struct jaListItem* item = NULL;
	struct jaStatus callback_st = {0};
	uint16_t function_keys = 0;

	jaStatusSet(st, "kaContextUpdate", JA_STATUS_SUCCESS, NULL);

	// Refuse to work without a window (not an error)
	if (g_context.windows.items_no == 0)
		return 1;

	// 1 - Receive and process input
	{
		SDL_Event e = {0};

		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_KEYDOWN)
			{
				switch (e.key.keysym.scancode)
				{
				case SDL_SCANCODE_Z: g_context.events.a = true; break;
				case SDL_SCANCODE_Y: g_context.events.b = true; break;
				case SDL_SCANCODE_A: g_context.events.x = true; break;
				case SDL_SCANCODE_S: g_context.events.y = true; break;

				case SDL_SCANCODE_RETURN: g_context.events.start = true; break;
				case SDL_SCANCODE_SPACE: g_context.events.select = true; break;

				case SDL_SCANCODE_UP: g_context.events.pad_u = true; break;
				case SDL_SCANCODE_DOWN: g_context.events.pad_d = true; break;
				case SDL_SCANCODE_LEFT: g_context.events.pad_l = true; break;
				case SDL_SCANCODE_RIGHT: g_context.events.pad_r = true; break;

				default: break;
				}
			}
			else if (e.type == SDL_KEYUP)
			{
				switch (e.key.keysym.scancode)
				{
				case SDL_SCANCODE_Z: g_context.events.a = false; break;
				case SDL_SCANCODE_Y: g_context.events.b = false; break;
				case SDL_SCANCODE_A: g_context.events.x = false; break;
				case SDL_SCANCODE_S: g_context.events.y = false; break;

				case SDL_SCANCODE_RETURN: g_context.events.start = false; break;
				case SDL_SCANCODE_SPACE: g_context.events.select = false; break;

				case SDL_SCANCODE_UP: g_context.events.pad_u = false; break;
				case SDL_SCANCODE_DOWN: g_context.events.pad_d = false; break;
				case SDL_SCANCODE_LEFT: g_context.events.pad_l = false; break;
				case SDL_SCANCODE_RIGHT: g_context.events.pad_r = false; break;

				case SDL_SCANCODE_F1: function_keys = (function_keys | (0x01)); break;
				case SDL_SCANCODE_F2: function_keys = (function_keys | (0x01 << 1)); break;
				case SDL_SCANCODE_F3: function_keys = (function_keys | (0x01 << 2)); break;
				case SDL_SCANCODE_F4: function_keys = (function_keys | (0x01 << 3)); break;
				case SDL_SCANCODE_F5: function_keys = (function_keys | (0x01 << 4)); break;
				case SDL_SCANCODE_F6: function_keys = (function_keys | (0x01 << 5)); break;
				case SDL_SCANCODE_F7: function_keys = (function_keys | (0x01 << 6)); break;
				case SDL_SCANCODE_F8: function_keys = (function_keys | (0x01 << 7)); break;
				case SDL_SCANCODE_F9: function_keys = (function_keys | (0x01 << 8)); break;
				case SDL_SCANCODE_F10: function_keys = (function_keys | (0x01 << 9)); break;
				case SDL_SCANCODE_F11: function_keys = (function_keys | (0x01 << 10)); break;
				case SDL_SCANCODE_F12: function_keys = (function_keys | (0x01 << 11)); break;

				default: break;
				}
			}
			else if (e.type == SDL_WINDOWEVENT)
			{
				// TODO: rather than find the kaWindow ourself, see if SDL offers something like 'user_data'
				for (item = g_context.windows.first; item != NULL; item = item->next)
					if (e.window.windowID == SDL_GetWindowID(((struct kaWindow*)item->data)->sdl_window))
					{
						window = item->data;
						break;
					}

				if (e.window.event == SDL_WINDOWEVENT_CLOSE)
					window->delete_mark = true;
				else if (e.window.event == SDL_WINDOWEVENT_RESIZED)
					window->resized_mark = true;
				else if (e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
					g_context.focused_window = window;
			}
		}

		g_context.events.pad.y = ((g_context.events.pad_u) ? 1.0f : 0.0f) - ((g_context.events.pad_d) ? 1.0f : 0.0f);
		g_context.events.pad.x = ((g_context.events.pad_r) ? 1.0f : 0.0f) - ((g_context.events.pad_l) ? 1.0f : 0.0f);
	}

	// 2 - Flip windows buffer and call non-frequent callbacks
	{
		struct jaListState it = {0};
		int width;
		int height;

		it.start = g_context.windows.first;

		while ((item = jaListIterate(&it)) != NULL) // ListIterate() allows delete items inside it
		{
			window = item->data;

			if (sSwitchContext(window, st) != 0)
				return 1;

			if (g_context.focused_window == window || (g_context.frame_no % 4) == 0) // HARDCODED
			{
				SDL_GL_SwapWindow(window->sdl_window);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			}

			// Following callbacks after SwitchContext(), in case a draw function is called from one of them

			// Delete window
			if (window->delete_mark == true)
			{
				if (window->close_callback != NULL)
					window->close_callback(window, window->user_data);

				sFreeWindow(window);
				jaListRemove(item);
				continue;
			}

			// Resize
			if (window->resized_mark == true)
			{
				window->resized_mark = false;

				SDL_GetWindowSize(window->sdl_window, &width, &height);
				glViewport(0, 0, width, height);

				if (window->resize_callback != NULL)
				{
					callback_st.code = JA_STATUS_SUCCESS; // Assume success
					window->resize_callback(window, width, height, window->user_data, &callback_st);

					if (callback_st.code != JA_STATUS_SUCCESS)
						goto callback_failure;
				}
			}
		}
	}

	// 3 - Call windows frame callback
	{
		uint32_t ms_betwen = 0;
		float delta = 0.0f;

		for (item = g_context.windows.first; item != NULL; item = item->next)
		{
			window = item->data;

			if (sSwitchContext(window, st) != 0)
				return 1;

			// Frame
			if (window->frame_callback != NULL)
			{
				callback_st.code = JA_STATUS_SUCCESS; // Assume success

				ms_betwen = SDL_GetTicks() - window->last_frame_ms;
				delta = (float)ms_betwen / 33.3333f;

				if (g_context.focused_window == window)
				{
					window->frame_callback(window, g_context.events, delta, window->user_data, &callback_st);
					window->last_frame_ms = SDL_GetTicks();
				}
				else if ((g_context.frame_no % 4) == 0) // HARDCODED
				{
					window->frame_callback(window, (struct kaEvents){0}, delta, window->user_data, &callback_st);
					window->last_frame_ms = SDL_GetTicks();
				}

				if (callback_st.code != JA_STATUS_SUCCESS)
					goto callback_failure;
			}

			// Function keys (if any)
			if (function_keys != 0 && g_context.focused_window == window && window->function_callback != NULL)
			{
				int key_no = 1;

				for (uint16_t a = function_keys; a != 0; a = a >> 1)
				{
					callback_st.code = JA_STATUS_SUCCESS; // Assume success

					if ((a & 0x01) == 1)
						window->function_callback(window, key_no, window->user_data, &callback_st);

					if (callback_st.code != JA_STATUS_SUCCESS)
						goto callback_failure;

					key_no += 1;
				}
			}
		}
	}

	// Bye!
	g_context.frame_no += 1;
	return 0;

callback_failure:
	memcpy(st, &callback_st, sizeof(struct jaStatus));
	return 2;
}


int kaWindowCreate(const char* caption, void (*init_callback)(struct kaWindow*, void*, struct jaStatus*),
                   void (*frame_callback)(struct kaWindow*, struct kaEvents, float, void*, struct jaStatus*),
                   void (*resize_callback)(struct kaWindow*, int, int, void*, struct jaStatus*),
                   void (*function_callback)(struct kaWindow*, int, void*, struct jaStatus*),
                   void (*close_callback)(struct kaWindow*, void*), void* user_data, struct jaStatus* st)
{
	struct jaListItem* item = NULL;
	struct kaWindow* window = NULL;

	jaStatusSet(st, "kaWindowCreate", JA_STATUS_SUCCESS, NULL);

	// 1 - Basic object/container
	if ((item = jaListAdd(&g_context.windows, NULL, sizeof(struct kaWindow))) == NULL)
	{
		jaStatusSet(st, "kaWindowCreate", JA_STATUS_MEMORY_ERROR, NULL);
		goto return_failure;
	}

	window = item->data;
	memset(window, 0, sizeof(struct kaWindow));

	window->init_callback = init_callback;
	window->frame_callback = frame_callback;
	window->resize_callback = resize_callback;
	window->function_callback = function_callback;
	window->close_callback = close_callback;
	window->user_data = user_data;

	// 2 - SDL2 objects
	if ((window->sdl_window = SDL_CreateWindow(caption, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH,
	                                           WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE)) == NULL)
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

	if (sSwitchContext(window, st) != 0)
		goto return_failure;

	SDL_SetWindowMinimumSize(window->sdl_window, WINDOW_MIN_WIDTH, WINDOW_MIN_HEIGHT);
	SDL_GL_SetSwapInterval((g_context.cfg_vsync == true) ? 1 : 0);

	SDL_RaiseWindow(window->sdl_window);
	g_context.focused_window = window;

	// 3 - Initialize GLAD (after context creation)
	if (g_context.windows.items_no == 1)
	{
		// FIXME, this probably don't survive to multiple contexts
		if (gladLoadGLES2Loader(SDL_GL_GetProcAddress) == 0)
		{
			jaStatusSet(st, "kaWindowCreate", JA_STATUS_ERROR, "gladLoad()");
			goto return_failure;
		}

		printf("\n%s\n", glGetString(GL_VENDOR));
		printf("%s\n", glGetString(GL_RENDERER));
		printf("%s\n\n", glGetString(GL_VERSION));
	}

	// 4 - Configure OpenGL
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glEnableVertexAttribArray(ATTRIBUTE_POSITION);
	glEnableVertexAttribArray(ATTRIBUTE_COLOUR);
	glEnableVertexAttribArray(ATTRIBUTE_UV);

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// 5 - Create some default OpenGL objects
	{
		uint16_t raw_index[] = {2, 1, 0, 3, 2, 0};
		struct kaVertex raw_vertices[] = {
		    {.position = {-0.5f, -0.5f, 0.0f}, .colour = {1.0f, 0.0f, 0.0f, 1.0f}, .uv = {0.0f, 1.0f}},
		    {.position = {-0.5f, +0.5f, 0.0f}, .colour = {0.0f, 1.0f, 0.0f, 1.0f}, .uv = {0.0f, 0.0f}},
		    {.position = {+0.5f, +0.5f, 0.0f}, .colour = {0.0f, 0.0f, 1.0f, 1.0f}, .uv = {1.0f, 0.0f}},
		    {.position = {+0.5f, -0.5f, 0.0f}, .colour = {0.5f, 0.5f, 0.5f, 1.0f}, .uv = {1.0f, 1.0f}}};

		const char* vertex_code =
		    "#version 100\n"
		    "attribute vec3 vertex_position; attribute vec4 vertex_colour; attribute vec2 vertex_uv;"
		    "uniform mat4 world; uniform mat4 camera; uniform vec3 camera_position;"
		    "uniform vec3 local_position; uniform vec3 local_scale;"
		    "varying vec4 colour; varying vec2 uv;"

		    "void main() { colour = vertex_colour; uv = vertex_uv;"
		    "gl_Position = world * camera * vec4(local_position + (vertex_position * local_scale), 1.0); }";

		const char* fragment_code =
		    "#version 100\n"
		    "uniform sampler2D texture0;"
		    "varying lowp vec4 colour; varying lowp vec2 uv;"

		    "lowp vec4 Overlay(lowp vec4 a, lowp vec4 b) {"
		    "return mix((1.0 - 2.0 * (1.0 - a) * (1.0 - b)), (2.0 * a * b), step(a, vec4(0.5)));}"

		    "void main() { gl_FragColor = Overlay(colour, texture2D(texture0, uv)); }";

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
		    kaTextureInitImage(window, &image, &window->default_texture, st) != 0)
			goto return_failure;

		kaSetProgram(window, &window->default_program);
		kaSetVertices(window, &window->default_vertices);
		kaSetTexture(window, 0, &window->default_texture);
	}

	// 6 - First callback
	{
		struct jaStatus callback_st = {0};
		callback_st.code = JA_STATUS_SUCCESS; // Assume success

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (window->init_callback != NULL)
			window->init_callback(window, window->user_data, &callback_st);

		if (callback_st.code != JA_STATUS_SUCCESS)
		{
			memcpy(st, &callback_st, sizeof(struct jaStatus));
			goto return_failure;
		}

		SDL_GL_SwapWindow(window->sdl_window);
	}

	// Bye!
	return 0;

return_failure:
	if (window != NULL)
	{
		sFreeWindow(window);
		jaListRemove(item);
	}
	return 1;
}


inline void kaWindowDelete(struct kaWindow* window)
{
	window->delete_mark = true;
}


struct jaImage* kaTakeScreenshot(struct kaWindow* window, struct jaStatus* st)
{
	int width;
	int height;

	SDL_GetWindowSize(window->sdl_window, &width, &height);

	// Create a generic jaImage object
	struct jaImage* image = NULL;

	if (window->temp_image != NULL)
		jaImageDelete(window->temp_image);

	if ((window->temp_image = jaImageCreate(JA_IMAGE_U8, (size_t)width, (size_t)height + 1, 4)) == NULL)
	{
		jaStatusSet(st, "kaTakeScreenshot", JA_STATUS_MEMORY_ERROR, NULL);
		goto return_failure;
	}

	image = window->temp_image;

	// Read from OpenGL buffer
	GLenum error;
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, image->data);

	if ((error = glGetError()) != GL_NO_ERROR)
	{
		// TODO, glReadPixels has tons of corners where it can fail.
		jaStatusSet(st, "kaTakeScreenshot", JA_STATUS_ERROR, NULL);
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


inline size_t kaGetTime(struct kaWindow* window)
{
	(void)window;
	return (size_t)SDL_GetTicks();
}


inline size_t kaGetFrame(struct kaWindow* window)
{
	(void)window;
	return (size_t)g_context.frame_no;
}

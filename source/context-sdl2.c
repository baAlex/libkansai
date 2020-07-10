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


static void sFreeWindow(struct kaWindow* window)
{
	if (window->gl_context != NULL)
		SDL_GL_DeleteContext(window->gl_context);

	if (window->sdl_window != NULL)
		SDL_DestroyWindow(window->sdl_window);
}


static int sSwitchWindow(struct kaWindow* window, struct jaStatus* st)
{
	g_context.current_window = window;

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
		g_context.cfg_filter = FILTER_BILINEAR;
		g_context.cfg_wireframe = false;
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
	SDL_Event e = {0};
	struct kaWindow* window = NULL;
	struct jaListItem* item = NULL;

	jaStatusSet(st, "kaContextUpdate", JA_STATUS_SUCCESS, NULL);

	// Refuse to work without a window (not an error)
	if (g_context.windows.items_no == 0)
		return 1;

	// Receive and process input
	while (SDL_PollEvent(&e) != 0)
	{
		if (e.type == SDL_KEYDOWN)
		{
			switch (e.key.keysym.scancode)
			{
			case SDL_SCANCODE_A: g_context.events.a = true; break;
			case SDL_SCANCODE_S: g_context.events.b = true; break;
			case SDL_SCANCODE_Z: g_context.events.x = true; break;
			case SDL_SCANCODE_X: g_context.events.y = true; break;
			case SDL_SCANCODE_Q: g_context.events.lb = true; break;
			case SDL_SCANCODE_W: g_context.events.rb = true; break;
			default: break;
			}
		}
		else if (e.type == SDL_KEYUP)
		{
			switch (e.key.keysym.scancode)
			{
			case SDL_SCANCODE_A: g_context.events.a = false; break;
			case SDL_SCANCODE_S: g_context.events.b = false; break;
			case SDL_SCANCODE_Z: g_context.events.x = false; break;
			case SDL_SCANCODE_X: g_context.events.y = false; break;
			case SDL_SCANCODE_Q: g_context.events.lb = false; break;
			case SDL_SCANCODE_W: g_context.events.rb = false; break;
			default: break;
			}
		}
		else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE)
		{
			for (item = g_context.windows.first; item != NULL; item = item->next)
			{
				window = item->data;

				if (e.window.windowID == SDL_GetWindowID(window->sdl_window))
				{
					window->delete_mark = true;
					break;
				}
			}
		}
	}

	// Flip windows buffer
	struct jaListState it = {0};
	it.start = g_context.windows.first;

	while ((item = jaListIterate(&it)) != NULL) // ListIterate() allows delete items inside it
	{
		window = item->data;

		if (g_context.windows.items_no != 0)
		{
			if (sSwitchWindow(window, st) != 0)
				return 1;
		}

		SDL_GL_SwapWindow(window->sdl_window);

		// Truly delete a marked window
		if (window->delete_mark == true)
		{
			// Done here, after MakeCurrent(), so in case the close callback
			// has a draw function, this one will be successfully executed

			if (window->close_callback != NULL)
				window->close_callback(window, window->user_data);

			sFreeWindow(window);
			jaListRemove(item);
			continue;
		}
	}

	// Call windows for a new frame
	for (item = g_context.windows.first; item != NULL; item = item->next)
	{
		window = item->data;

		if (g_context.windows.items_no != 0)
		{
			if (sSwitchWindow(window, st) != 0)
				return 1;
		}

		glClear(GL_COLOR_BUFFER_BIT);

		if (window->frame_callback != NULL)
			window->frame_callback(window, &g_context.events, window->user_data);
	}

	return 0;
}


int kaWindowCreate(const char* caption, void (*close_callback)(struct kaWindow*, void*),
                   void (*init_callback)(struct kaWindow*, void*),
                   void (*frame_callback)(struct kaWindow*, const struct kaEvents*, void*), void* user_data,
                   struct jaStatus* st)
{
	struct jaListItem* item = NULL;
	struct kaWindow* window = NULL;

	jaStatusSet(st, "kaWindowCreate", JA_STATUS_SUCCESS, NULL);

	if ((item = jaListAdd(&g_context.windows, NULL, sizeof(struct kaWindow))) == NULL)
	{
		jaStatusSet(st, "kaWindowCreate", JA_STATUS_MEMORY_ERROR, NULL);
		goto return_failure;
	}

	window = item->data;
	memset(window, 0, sizeof(struct kaWindow));

	window->close_callback = close_callback;
	window->init_callback = init_callback;
	window->frame_callback = frame_callback;
	window->user_data = user_data;

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

	if (sSwitchWindow(window, st) != 0)
		goto return_failure;

	SDL_SetWindowMinimumSize(window->sdl_window, WINDOW_MIN_WIDTH, WINDOW_MIN_HEIGHT);
	SDL_GL_SetSwapInterval((g_context.cfg_vsync == true) ? 1 : 0);

	// Initialize GLAD (after context creation)
	// FIXME, this probably don't survive to multiple contexts
	if (g_context.windows.items_no == 1)
	{
		if (gladLoadGLES2Loader(SDL_GL_GetProcAddress) == 0)
		{
			jaStatusSet(st, "kaWindowCreate", JA_STATUS_ERROR, "gladLoad()");
			goto return_failure;
		}

		printf("\n%s\n", glGetString(GL_VENDOR));
		printf("%s\n", glGetString(GL_RENDERER));
		printf("%s\n\n", glGetString(GL_VERSION));
	}

	glDisable(GL_DITHER);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	glEnableVertexAttribArray(ATTRIBUTE_COLOUR);
	glEnableVertexAttribArray(ATTRIBUTE_POSITION);
	glEnableVertexAttribArray(ATTRIBUTE_NORMAL);
	glEnableVertexAttribArray(ATTRIBUTE_UV);

	glClear(GL_COLOR_BUFFER_BIT);

	if (window->init_callback != NULL)
		window->init_callback(window, window->user_data);

	SDL_GL_SwapWindow(window->sdl_window);

	// Bye!
	return 0;

return_failure:
	return 1;
}


void kaWindowDelete(struct kaWindow* window)
{
	window->delete_mark = true;
}


int kaTakeScreenshot(const struct kaWindow* window, const char* filename, struct jaStatus* st)
{
	(void)window;
	(void)filename;
	(void)st;
	return 1;

#if 0
	struct jaImage* image = NULL;
	GLenum error;

	if ((image = jaImageCreate(JA_IMAGE_U8, (size_t)context->events.window_size.x,
	                           (size_t)context->events.window_size.y + 1, 4)) == NULL)
	{
		jaStatusSet(st, "TakeScreenshot", JA_STATUS_MEMORY_ERROR, NULL);
		goto return_failure;
	}

	glReadPixels(0, 0, context->events.window_size.x, context->events.window_size.y, GL_RGBA, GL_UNSIGNED_BYTE,
	             image->data);

	if ((error = glGetError()) != GL_NO_ERROR)
	{
		// TODO, glReadPixels has tons of corners where it can fail.
		jaStatusSet(st, "TakeScreenshot", JA_STATUS_ERROR, NULL);
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

	image->height -= 1; // ;)

	if ((jaImageSaveSgi(image, filename, st)) != 0)
		goto return_failure;

	jaImageDelete(image);
	return 0;

return_failure:
	if (image != NULL)
		jaImageDelete(image);

	return 1;
#endif
}

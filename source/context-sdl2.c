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


int kaContextStart(struct jaStatus* st)
{
	jaStatusSet(st, "kaContextStart", JA_STATUS_SUCCESS, NULL);

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		fprintf(stderr, "\n%s\n", SDL_GetError());
		jaStatusSet(st, "kaContextStart", JA_STATUS_ERROR, "SDL_Init()");
		return 1;
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
			kaWindowDelete((struct kaWindow*)(g_context.windows.last->data));

		SDL_Quit();
		memset(&g_context, 0, sizeof(struct Context));
	}
}


int kaContextUpdate(struct kaEvents* out_events)
{
	SDL_Event e = {0};

	// Refuse to work whitout a window
	if (g_context.windows.items_no == 0)
		return 1;

	// Update screen
	// SDL_GL_SwapWindow(context->window);
	// glClear(GL_COLOR_BUFFER_BIT);

	// Receive input
	while (SDL_PollEvent(&e) != 0)
	{
		if (e.type == SDL_WINDOWEVENT)
		{
			// Notify that the user want to close and delete window
			if (e.window.event == SDL_WINDOWEVENT_CLOSE)
			{
				for (struct jaListItem* item = g_context.windows.first; item != NULL; item = item->next)
				{
					struct kaWindow* window = item->data;

					if (e.window.windowID == SDL_GetWindowID(window->sdl_window))
					{
						window->close_callback(window);
						kaWindowDelete(window);
						break;
					}
				}
			}
		}
		else if (e.type == SDL_KEYDOWN)
		{
			switch (e.key.keysym.scancode)
			{
			case SDL_SCANCODE_A: out_events->a = true; break;
			case SDL_SCANCODE_S: out_events->b = true; break;
			case SDL_SCANCODE_Z: out_events->x = true; break;
			case SDL_SCANCODE_X: out_events->y = true; break;
			case SDL_SCANCODE_Q: out_events->lb = true; break;
			case SDL_SCANCODE_W: out_events->rb = true; break;
			default: break;
			}
		}
		else if (e.type == SDL_KEYUP)
		{
			switch (e.key.keysym.scancode)
			{
			case SDL_SCANCODE_A: out_events->a = false; break;
			case SDL_SCANCODE_S: out_events->b = false; break;
			case SDL_SCANCODE_Z: out_events->x = false; break;
			case SDL_SCANCODE_X: out_events->y = false; break;
			case SDL_SCANCODE_Q: out_events->lb = false; break;
			case SDL_SCANCODE_W: out_events->rb = false; break;
			default: break;
			}
		}
	}

	return 0;
}


void kaSleep(int milliseconds)
{
	SDL_Delay((Uint32)milliseconds);
}


struct kaWindow* kaWindowCreate(const struct jaConfiguration* cfg, const char* caption,
                                void (*close_callback)(const struct kaWindow*), struct jaStatus* st)
{
	struct jaListItem* item = NULL;
	struct kaWindow* window = NULL;
	(void)cfg;

	jaStatusSet(st, "kaWindowCreate", JA_STATUS_SUCCESS, NULL);

	if ((item = jaListAdd(&g_context.windows, NULL, sizeof(struct kaWindow))) == NULL)
	{
		jaStatusSet(st, "kaWindowCreate", JA_STATUS_MEMORY_ERROR, NULL);
		goto return_failure;
	}

	window = item->data;
	memset(window, 0, sizeof(struct kaWindow));

	window->item = item;
	window->close_callback = close_callback;

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

	if (SDL_GL_MakeCurrent(window->sdl_window, window->gl_context) != 0)
	{
		fprintf(stderr, "\n%s\n", SDL_GetError());
		jaStatusSet(st, "kaWindowCreate", JA_STATUS_ERROR, "SDL_GL_MakeCurrent()");
		goto return_failure;
	}

	SDL_SetWindowMinimumSize(window->sdl_window, WINDOW_MIN_WIDTH, WINDOW_MIN_HEIGHT);
	SDL_GL_SetSwapInterval((window->cfg_vsync == true) ? 1 : 0);

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

	glEnableVertexAttribArray(ATTRIBUTE_POSITION);
	glEnableVertexAttribArray(ATTRIBUTE_UV);

	glClear(GL_COLOR_BUFFER_BIT);
	SDL_GL_SwapWindow(window->sdl_window);

	// Bye!
	return window;

return_failure:
	return NULL;
}


void kaWindowDelete(struct kaWindow* window)
{
	if (window->gl_context != NULL)
		SDL_GL_DeleteContext(window->gl_context);

	if (window->sdl_window != NULL)
		SDL_DestroyWindow(window->sdl_window);

	jaListRemove(window->item);
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

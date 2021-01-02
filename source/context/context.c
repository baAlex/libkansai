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

 [context/context.c]
 - Alexander Brandt 2019-2020
-----------------------------*/

#include "private.h"


#define ACCUMULATOR_LEN 128
#define MAX_WINDOWS 4

#define KEY_PRESS_CODE 128
#define KEY_RELEASE_CODE 0


struct kaContext
{
	size_t frame_no;

	bool glad_initialized;
	int sdl_references;

	uint8_t keyboard_accumulator[ACCUMULATOR_LEN];

	uint8_t windows_no;
	struct kaWindow* windows[MAX_WINDOWS];
	struct kaWindow* focused_window;

} g_context = {0}; // Globals! nooooo!


struct kaWindow* InternalAllocWindow()
{
	struct kaWindow* window = calloc(1, sizeof(struct kaWindow));

	// Add it to our global list
	if (window != NULL)
	{
		size_t i = 0;

		for (i = 0; i < MAX_WINDOWS; i++)
		{
			if (g_context.windows[i] == NULL)
			{
				g_context.windows[i] = window;
				break;
			}
		}

		if (i == MAX_WINDOWS)
		{
			free(window);
			return NULL;
		}
	}

	g_context.windows_no += 1;
	return window;
}


void InternalFreeWindow(struct kaWindow* window)
{
	// TODO: the following routine is out of place in this file,
	// fits better in 'window.c' but the globals and callbacks
	// make that... complicate
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

	// Normal free routine from here
	free(window);

	// Remove from global list
	for (size_t i = 0; i < MAX_WINDOWS; i++)
	{
		if (g_context.windows[i] == window)
		{
			g_context.windows[i] = NULL;
			break;
		}
	}

	g_context.windows_no -= 1;
}


int InternalSwitchContext(struct kaWindow* window, struct jaStatus* st)
{
	if (SDL_GL_MakeCurrent(window->sdl_window, window->gl_context) != 0)
	{
		fprintf(stderr, "\n%s\n", SDL_GetError());
		jaStatusSet(st, "SwitchContext", JA_STATUS_ERROR, "SDL_GL_MakeCurrent()");
		return 1;
	}

	return 0;
}


void InternalFocusWindow(struct kaWindow* window)
{
	SDL_RaiseWindow(window->sdl_window);
	g_context.focused_window = window;
}


int InternalInitGlad()
{
	if (g_context.glad_initialized == false)
	{
		// FIXME, this probably don't survive to multiple contexts
		if (gladLoadGLES2Loader(SDL_GL_GetProcAddress) == 0)
			return 1;

		printf("\n%s\n", glGetString(GL_VENDOR));
		printf("%s\n", glGetString(GL_RENDERER));
		printf("%s\n", glGetString(GL_VERSION));
		printf("%s\n\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

		g_context.glad_initialized = true;
	}

	return 0;
}


int kaContextStart(struct jaStatus* st)
{
	jaStatusSet(st, "kaContextStart", JA_STATUS_SUCCESS, NULL);

	if (g_context.sdl_references == 0)
	{
		if (SDL_Init(SDL_INIT_VIDEO) != 0)
		{
			fprintf(stderr, "\n%s\n", SDL_GetError());
			jaStatusSet(st, "kaContextStart", JA_STATUS_ERROR, "SDL_Init()");
			return 1;
		}
	}

	g_context.sdl_references += 1;
	return 0;
}


void kaContextStop()
{
	g_context.sdl_references -= 1;

	if (g_context.sdl_references == 0)
	{
		for (size_t i = 0; i < MAX_WINDOWS; i++)
		{
			if (g_context.windows[i] != NULL)
				InternalFreeWindow(g_context.windows[i]);
		}

		SDL_Quit();
		memset(&g_context, 0, sizeof(struct kaContext));
	}
}


static inline void sKeyboardPress(SDL_Scancode sc)
{
	if (sc > ACCUMULATOR_LEN) // We only support an subset
		return;

	g_context.keyboard_accumulator[sc] = KEY_PRESS_CODE + g_context.windows_no;
}


static inline void sKeyboardRelease(SDL_Scancode sc)
{
	if (sc > ACCUMULATOR_LEN)
		return;

	g_context.keyboard_accumulator[sc] = KEY_RELEASE_CODE + g_context.windows_no;
}


int kaContextUpdate(struct jaStatus* st)
{
	struct kaWindow* window = NULL;
	struct jaStatus callback_st = {0};
	SDL_Event e = {0};

	jaStatusSet(st, "kaContextUpdate", JA_STATUS_SUCCESS, NULL);

	// Receive and process input
	while (SDL_PollEvent(&e) != 0)
	{
		if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
		{
			sKeyboardPress(e.key.keysym.scancode);
		}
		else if (e.type == SDL_KEYUP)
		{
			sKeyboardRelease(e.key.keysym.scancode);
		}
		else if (e.type == SDL_WINDOWEVENT)
		{
			for (size_t i = 0; i < MAX_WINDOWS; i++)
			{
				if (g_context.windows[i] != NULL &&
				    e.window.windowID == SDL_GetWindowID(g_context.windows[i]->sdl_window))
				{
					window = g_context.windows[i];
					break;
				}
			}

			if (window == NULL)
				continue;

			if (e.window.event == SDL_WINDOWEVENT_CLOSE)
				window->delete_mark = true;
			else if (e.window.event == SDL_WINDOWEVENT_RESIZED)
				window->resized_mark = true;
			else if (e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
				g_context.focused_window = window;
		}
	}

	struct kaEvents events = {0};

	events.a = (g_context.keyboard_accumulator[KA_KEY_RETURN] >= 128) ? 1 : 0;
	events.b = (g_context.keyboard_accumulator[KA_KEY_BACKSPACE] >= 128) ? 1 : 0;
	events.x = (g_context.keyboard_accumulator[KA_KEY_Z] >= 128) ? 1 : 0;
	events.y = (g_context.keyboard_accumulator[KA_KEY_X] >= 128) ? 1 : 0;

	events.select = (g_context.keyboard_accumulator[KA_KEY_SPACE] >= 128) ? 1 : 0;
	events.start = (g_context.keyboard_accumulator[KA_KEY_ESCAPE] >= 128) ? 1 : 0;

	events.pad_u = (g_context.keyboard_accumulator[KA_KEY_UP] >= 128) ? 1 : 0;
	events.pad_d = (g_context.keyboard_accumulator[KA_KEY_DOWN] >= 128) ? 1 : 0;
	events.pad_l = (g_context.keyboard_accumulator[KA_KEY_LEFT] >= 128) ? 1 : 0;
	events.pad_r = (g_context.keyboard_accumulator[KA_KEY_RIGHT] >= 128) ? 1 : 0;

	events.pad.x = 0.0f; // TODO
	events.pad.y = 0.0f;

	// Windows iteration
	float delta = 0.0f;
	int alive_windows = 0;

	for (size_t i = 0; i < MAX_WINDOWS; i++)
	{
		if ((window = g_context.windows[i]) == NULL)
			continue;

		// Flip screen
		if (InternalSwitchContext(window, st) != 0)
			return 1;

		if (g_context.focused_window == window || (g_context.frame_no % 4) == 0) // HARDCODED
		{
			SDL_GL_SwapWindow(window->sdl_window);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		// Delete window, delete callback
		if (window->delete_mark == true)
		{
			if (window->close_callback != NULL)
				window->close_callback(window, window->user_data);

			InternalFreeWindow(window);
			continue;
		}

		// Resize, resize callback
		if (window->resized_mark == true)
		{
			int window_w = 0;
			int window_h = 0;

			window->resized_mark = false;

			SDL_GetWindowSize(window->sdl_window, &window_w, &window_h);
			glViewport(0, 0, window_w, window_h);

			if (window->resize_callback != NULL)
			{
				callback_st.code = JA_STATUS_SUCCESS; // Assume success
				window->resize_callback(window, window_w, window_h, window->user_data, &callback_st);

				if (callback_st.code != JA_STATUS_SUCCESS)
					goto callback_failure;
			}
		}

		// Frame callback
		if (window->frame_callback != NULL)
		{
			callback_st.code = JA_STATUS_SUCCESS; // Assume success
			delta = (float)(SDL_GetTicks() - window->last_frame_ms) / 33.3333f;

			if (g_context.focused_window == window)
			{
				window->frame_callback(window, events, delta, window->user_data, &callback_st);
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

		// Keyboard callback (if any)
		if (window->keyboard_callback != NULL)
		{
			for (unsigned i = 0; i < ACCUMULATOR_LEN; i++)
			{
				callback_st.code = JA_STATUS_SUCCESS; // Assume success

				if (g_context.keyboard_accumulator[i] == KEY_PRESS_CODE ||
				    g_context.keyboard_accumulator[i] == KEY_RELEASE_CODE)
					continue;

				if (g_context.keyboard_accumulator[i] > KEY_PRESS_CODE)
				{
					window->keyboard_callback(window, i, KA_PRESSED, window->user_data, &callback_st);
					g_context.keyboard_accumulator[i] -= 1;
				}
				else
				{
					window->keyboard_callback(window, i, KA_RELEASED, window->user_data, &callback_st);
					g_context.keyboard_accumulator[i] -= 1;
				}

				if (callback_st.code != JA_STATUS_SUCCESS)
					goto callback_failure;
			}
		}

		// Window survives all callbacks!
		alive_windows += 1;
	}

	// Bye!
	if (alive_windows == 0)
		return 1;

	g_context.frame_no += 1;
	return 0;

callback_failure:
	jaStatusCopy(&callback_st, st);
	return 2;
}


inline unsigned kaGetTime()
{
	return SDL_GetTicks();
}


inline size_t kaGetFrame()
{
	return g_context.frame_no;
}


inline void kaSleep(unsigned ms)
{
	SDL_Delay(ms);
}


inline bool kaWindowInFocus(const struct kaWindow* window)
{
	return (g_context.focused_window == window) ? true : false;
}

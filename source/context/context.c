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


struct kaContext
{
	size_t frame_no;

	bool glad_initialized;
	int sdl_references;

	struct kaEvents events;

	struct kaWindow* windows[MAX_WINDOWS];
	struct kaWindow* focused_window;

} g_context; // Globals! nooooo!


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


int kaContextUpdate(struct jaStatus* st)
{
	struct kaWindow* window = NULL;
	struct jaStatus callback_st = {0};
	uint16_t function_keys = 0;
	SDL_Event e = {0};

	jaStatusSet(st, "kaContextUpdate", JA_STATUS_SUCCESS, NULL);

	// Receive and process input
	while (SDL_PollEvent(&e) != 0)
	{
		if (e.type == SDL_KEYDOWN)
		{
			switch (e.key.keysym.scancode)
			{
			case SDL_SCANCODE_RETURN: g_context.events.a = true; break;
			case SDL_SCANCODE_BACKSPACE: g_context.events.b = true; break;
			case SDL_SCANCODE_Z: g_context.events.x = true; break;
			case SDL_SCANCODE_X: g_context.events.y = true; break;

			case SDL_SCANCODE_ESCAPE: g_context.events.start = true; break;
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
			case SDL_SCANCODE_RETURN: g_context.events.a = false; break;
			case SDL_SCANCODE_BACKSPACE: g_context.events.b = false; break;
			case SDL_SCANCODE_Z: g_context.events.x = false; break;
			case SDL_SCANCODE_X: g_context.events.y = false; break;

			case SDL_SCANCODE_ESCAPE: g_context.events.start = false; break;
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

	g_context.events.pad.y = ((g_context.events.pad_u) ? 1.0f : 0.0f) - ((g_context.events.pad_d) ? 1.0f : 0.0f);
	g_context.events.pad.x = ((g_context.events.pad_r) ? 1.0f : 0.0f) - ((g_context.events.pad_l) ? 1.0f : 0.0f);

	// Windows iteration
	float delta = 0.0f;
	int live_windows = 0;

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

		// Function keys callback (if any)
		if (window->function_callback != NULL && function_keys != 0 && g_context.focused_window == window)
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

		// Window survives all callbacks!
		live_windows += 1;
	}

	// Bye!
	if (live_windows == 0)
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

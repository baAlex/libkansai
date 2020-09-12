/*-----------------------------

 [context-private.h]
 - Alexander Brandt 2019-2020
-----------------------------*/

#ifndef CONTEXT_PRIVATE_H
#define CONTEXT_PRIVATE_H

#include "kansai-context.h"
#include <string.h>

#include "glad/glad.h" // Before SDL2
#include <SDL2/SDL.h>

#define ATTRIBUTE_POSITION 10
#define ATTRIBUTE_COLOR 11
#define ATTRIBUTE_UV 12

#define MAX_WINDOWS 4


struct kaWindow
{
	// ---- Agnostic side ----

	void (*frame_callback)(struct kaWindow*, struct kaEvents, float, void*, struct jaStatus*);
	void (*resize_callback)(struct kaWindow*, int, int, void*, struct jaStatus*);
	void (*function_callback)(struct kaWindow*, int, void*, struct jaStatus*);
	void (*close_callback)(struct kaWindow*, void*);

	void* user_data;
	bool delete_mark;
	bool resized_mark;
	uint32_t last_frame_ms;
	bool is_fullscreen;

	struct jaMatrix4 world;
	struct jaMatrix4 camera;
	struct jaVector3 camera_position;
	struct jaMatrix4 local;

	const struct kaProgram* current_program;
	const struct kaVertices* current_vertices;
	const struct kaTexture* current_texture;

	struct
	{
		GLint world;
		GLint local;
		GLint camera;
		GLint camera_position;
		GLint texture[8];

	} uniform; // For current program

	struct kaVertices default_vertices;
	struct kaIndex default_index;
	struct kaProgram default_program;
	struct kaTexture default_texture;

	struct jaImage* temp_image;


	// ---- SDL2 side ----

	SDL_Window* sdl_window;
	SDL_GLContext* gl_context;
};

struct kaContext
{
	// ---- Agnostic side ----

	bool cfg_vsync;
	size_t frame_no;


	// ---- SDL2 side ----

	bool glad_initialized;
	size_t sdl_references;
	struct kaEvents events;

	struct kaWindow* windows[MAX_WINDOWS];
	struct kaWindow* focused_window;

} g_context; // Globals! nooooo!

#endif

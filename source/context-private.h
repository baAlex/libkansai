/*-----------------------------

 [context-private.h]
 - Alexander Brandt 2019-2020
-----------------------------*/

#ifndef CONTEXT_PRIVATE_H
#define CONTEXT_PRIVATE_H

#include "japan-list.h"
#include "kansai-context.h"

#include <string.h>

#include "glad/glad.h" // Before SDL2
#include <SDL2/SDL.h>

#define ATTRIBUTE_POSITION 10
#define ATTRIBUTE_COLOUR 11
#define ATTRIBUTE_UV 12

enum Filter
{
	FILTER_BILINEAR,
	FILTER_TRILINEAR,
	FILTER_PIXEL_BILINEAR,
	FILTER_PIXEL_TRILINEAR,
	FILTER_NONE
};

struct kaWindow
{
	// ---- Agnostic side ----
	void (*init_callback)(struct kaWindow*, void*);
	void (*frame_callback)(struct kaWindow*, struct kaEvents, void*);
	void (*resize_callback)(struct kaWindow*, int, int, void*);
	void (*function_callback)(struct kaWindow*, int, void*);
	void (*close_callback)(struct kaWindow*, void*);
	void* user_data;
	bool delete_mark;
	bool resized_mark;

	struct jaMatrix4 world;
	struct jaMatrix4 camera;
	struct jaVector3 camera_position;

	const struct kaProgram* current_program;
	const struct kaVertices* current_vertices;
	const struct kaTexture* current_texture;

	struct
	{
		GLint local_position;
		GLint local_scale;

		GLint world;
		GLint camera;
		GLint camera_position;
		GLint texture[8];
	} uniform; // For current program

	struct kaVertices default_vertices;
	struct kaIndex default_index;
	struct kaProgram default_program;
	struct kaTexture default_texture;

	// ---- SDL2 side ----
	SDL_Window* sdl_window;
	SDL_GLContext* gl_context;
};

struct kaContext
{
	// ---- Agnostic side ----
	enum Filter cfg_filter;
	bool cfg_vsync;
	bool cfg_wireframe;

	// ---- SDL2 side ----
	size_t sdl_references;
	struct jaList windows;
	struct kaEvents events;

	struct kaWindow* current_window;
	struct kaWindow* focused_window;

} g_context; // Globals! nooooo!

#endif

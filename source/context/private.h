/*-----------------------------

 [context/private.h]
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

struct kaContext;

struct kaWindow
{
	void (*frame_callback)(struct kaWindow*, struct kaEvents, float, void*, struct jaStatus*);
	void (*resize_callback)(struct kaWindow*, int, int, void*, struct jaStatus*);
	void (*keyboard_callback)(struct kaWindow*, enum kaKey, enum kaKeyMode, void*, struct jaStatus*);
	void (*close_callback)(struct kaWindow*, void*);
	void* user_data;

	bool delete_mark;
	bool resized_mark;
	bool is_fullscreen;
	uint32_t last_frame_ms;

	struct jaMatrixF4 world;
	struct jaMatrixF4 camera;
	struct jaVectorF3 camera_position;
	struct jaMatrixF4 local;

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

	SDL_Window* sdl_window;
	SDL_GLContext* gl_context;
};

struct kaWindow* InternalAllocWindow();
void InternalFreeWindow(struct kaWindow* window);
int InternalSwitchContext(struct kaWindow* window, struct jaStatus* st);
void InternalFocusWindow(struct kaWindow* window);
int InternalInitGlad();

#endif

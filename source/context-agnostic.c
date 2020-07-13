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

 [context-agnostic.c]
 - Alexander Brandt 2019-2020
-----------------------------*/

#include "context-private.h"


/*-----------------------------

 Context state
-----------------------------*/
inline void kaSetProgram(const struct kaProgram* program)
{
	if (program != NULL && program != g_context.current_window->current_program)
	{
		g_context.current_window->current_program = program;

		g_context.current_window->uniform.local_position = glGetUniformLocation(program->glptr, "local_position");
		g_context.current_window->uniform.local_scale = glGetUniformLocation(program->glptr, "local_scale");

		//

		g_context.current_window->uniform.world = glGetUniformLocation(program->glptr, "world");
		g_context.current_window->uniform.camera = glGetUniformLocation(program->glptr, "camera");
		g_context.current_window->uniform.camera_position = glGetUniformLocation(program->glptr, "camera_position");

		g_context.current_window->uniform.texture[0] = glGetUniformLocation(program->glptr, "texture0");
		g_context.current_window->uniform.texture[1] = glGetUniformLocation(program->glptr, "texture1");
		g_context.current_window->uniform.texture[2] = glGetUniformLocation(program->glptr, "texture2");
		g_context.current_window->uniform.texture[3] = glGetUniformLocation(program->glptr, "texture3");
		g_context.current_window->uniform.texture[4] = glGetUniformLocation(program->glptr, "texture4");
		g_context.current_window->uniform.texture[5] = glGetUniformLocation(program->glptr, "texture5");
		g_context.current_window->uniform.texture[6] = glGetUniformLocation(program->glptr, "texture6");
		g_context.current_window->uniform.texture[7] = glGetUniformLocation(program->glptr, "texture7");

		glUseProgram(program->glptr);

		glUniformMatrix4fv(g_context.current_window->uniform.world, 1, GL_FALSE,
		                   &g_context.current_window->world.e[0][0]);
		glUniformMatrix4fv(g_context.current_window->uniform.camera, 1, GL_FALSE,
		                   &g_context.current_window->camera.e[0][0]);
		glUniform3fv(g_context.current_window->uniform.camera_position, 1,
		             (float*)&g_context.current_window->camera_position);
		glUniform1i(g_context.current_window->uniform.texture[0], 0);
		glUniform1i(g_context.current_window->uniform.texture[1], 1);
		glUniform1i(g_context.current_window->uniform.texture[2], 2);
		glUniform1i(g_context.current_window->uniform.texture[3], 3);
		glUniform1i(g_context.current_window->uniform.texture[4], 4);
		glUniform1i(g_context.current_window->uniform.texture[5], 5);
		glUniform1i(g_context.current_window->uniform.texture[6], 6);
		glUniform1i(g_context.current_window->uniform.texture[7], 7);
	}
}


inline void kaSetVertices(const struct kaVertices* vertices)
{
	if (vertices != NULL && vertices != g_context.current_window->current_vertices)
	{
		g_context.current_window->current_vertices = vertices;

		glBindBuffer(GL_ARRAY_BUFFER, vertices->glptr);
		glVertexAttribPointer(ATTRIBUTE_COLOUR, 4, GL_FLOAT, GL_FALSE, sizeof(struct kaVertex), NULL);
		glVertexAttribPointer(ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(struct kaVertex), ((float*)NULL) + 4);
		glVertexAttribPointer(ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(struct kaVertex), NULL);
		glVertexAttribPointer(ATTRIBUTE_UV, 2, GL_FLOAT, GL_FALSE, sizeof(struct kaVertex), NULL);
	}
}


inline void kaSetContextTexture(int unit, const struct kaTexture* texture)
{
	if (texture != NULL && texture != g_context.current_window->current_texture)
	{
		g_context.current_window->current_texture = texture;

		glActiveTexture((GLenum)(GL_TEXTURE0 + unit));
		glBindTexture(GL_TEXTURE_2D, texture->glptr);
	}
}


inline void kaSetWorld(struct jaMatrix4 matrix)
{
	memcpy(&g_context.current_window->world, &matrix, sizeof(struct jaMatrix4));

	if (g_context.current_window->current_program != NULL)
		glUniformMatrix4fv(g_context.current_window->uniform.world, 1, GL_FALSE,
		                   &g_context.current_window->world.e[0][0]);
}


inline void kaSetCameraLookAt(struct jaVector3 target, struct jaVector3 origin)
{
	g_context.current_window->camera_position = origin;
	g_context.current_window->camera = jaMatrix4LookAt(origin, target, (struct jaVector3){0.0f, 0.0f, 1.0f});

	if (g_context.current_window->current_program != NULL)
	{
		glUniformMatrix4fv(g_context.current_window->uniform.camera, 1, GL_FALSE,
		                   &g_context.current_window->camera.e[0][0]);
		glUniform3fv(g_context.current_window->uniform.camera_position, 1,
		             (float*)&g_context.current_window->camera_position);
	}
}


inline void kaSetCameraMatrix(struct jaMatrix4 matrix, struct jaVector3 origin)
{
	g_context.current_window->camera_position = origin;
	g_context.current_window->camera = matrix;

	if (g_context.current_window->current_program != NULL)
	{
		glUniformMatrix4fv(g_context.current_window->uniform.camera, 1, GL_FALSE,
		                   &g_context.current_window->camera.e[0][0]);
		glUniform3fv(g_context.current_window->uniform.camera_position, 1,
		             (float*)&g_context.current_window->camera_position);
	}
}


inline void kaDraw(const struct kaIndex* index)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index->glptr);
	glDrawElements((g_context.cfg_wireframe == false) ? GL_TRIANGLES : GL_LINES, (GLsizei)index->length,
	               GL_UNSIGNED_SHORT, NULL);
}


inline void kaDrawSprite(struct jaVector3 position, struct jaVector3 scale)
{
	// Incredible inefficient!
	const struct kaVertices* prev_vertices = g_context.current_window->current_vertices;
	kaSetVertices(&g_context.current_window->generic_vertices);

	glUniform3fv(g_context.current_window->uniform.local_position, 1, (float*)&position);
	glUniform3fv(g_context.current_window->uniform.local_scale, 1, (float*)&scale);
	kaDraw(&g_context.current_window->generic_index);

	if (prev_vertices != &g_context.current_window->generic_vertices)
		kaSetVertices(prev_vertices);
}


/*-----------------------------

 OpenGL abstractions
-----------------------------*/
static inline int sCompileShader(GLuint shader, struct jaStatus* st)
{
	GLint success = GL_FALSE;

	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (success == GL_FALSE)
	{
		jaStatusSet(st, "kaProgramInit", JA_STATUS_ERROR, NULL);
		glGetShaderInfoLog(shader, JA_STATUS_EXPL_LEN, NULL, st->explanation);
		return 1;
	}

	return 0;
}


int kaProgramInit(const char* vertex_code, const char* fragment_code, struct kaProgram* out, struct jaStatus* st)
{
	GLint success = GL_FALSE;
	GLuint vertex = 0;
	GLuint fragment = 0;

	jaStatusSet(st, "kaProgramInit", JA_STATUS_SUCCESS, NULL);

	// Compile shaders
	if ((vertex = glCreateShader(GL_VERTEX_SHADER)) == 0 || (fragment = glCreateShader(GL_FRAGMENT_SHADER)) == 0)
	{
		jaStatusSet(st, "kaProgramInit", JA_STATUS_ERROR, "Creating GL shader\n");
		goto return_failure;
	}

	glShaderSource(vertex, 1, &vertex_code, NULL);
	glShaderSource(fragment, 1, &fragment_code, NULL);

	if (sCompileShader(vertex, st) != 0 || sCompileShader(fragment, st) != 0)
		goto return_failure;

	// Create program
	if ((out->glptr = glCreateProgram()) == 0)
	{
		jaStatusSet(st, "kaProgramInit", JA_STATUS_ERROR, "Creating GL program\n");
		goto return_failure;
	}

	glAttachShader(out->glptr, vertex);
	glAttachShader(out->glptr, fragment);

	glBindAttribLocation(out->glptr, ATTRIBUTE_COLOUR, "vertex_colour"); // Before link!
	glBindAttribLocation(out->glptr, ATTRIBUTE_POSITION, "vertex_position");
	glBindAttribLocation(out->glptr, ATTRIBUTE_NORMAL, "vertex_normal");
	glBindAttribLocation(out->glptr, ATTRIBUTE_UV, "vertex_uv");

	// Link
	glLinkProgram(out->glptr);
	glGetProgramiv(out->glptr, GL_LINK_STATUS, &success);

	if (success == GL_FALSE)
	{
		jaStatusSet(st, "kaProgramInit", JA_STATUS_ERROR, NULL);
		glGetProgramInfoLog(out->glptr, JA_STATUS_EXPL_LEN, NULL, st->explanation);
		goto return_failure;
	}

	// Bye!
	glDeleteShader(vertex); // Set shader to be deleted when glDeleteProgram() happens
	glDeleteShader(fragment);
	return 0;

return_failure:
	if (vertex != 0)
		glDeleteShader(vertex);
	if (fragment != 0)
		glDeleteShader(fragment);
	if (out->glptr != 0)
		glDeleteProgram(out->glptr);

	return 1;
}


inline void kaProgramFree(struct kaProgram* program)
{
	if (program->glptr != 0)
	{
		glDeleteProgram(program->glptr);
		program->glptr = 0;
	}
}


int kaVerticesInit(const struct kaVertex* data, uint16_t length, struct kaVertices* out, struct jaStatus* st)
{
	GLint reported_size = 0;
	GLint old_bind = 0;

	jaStatusSet(st, "kaVerticesInit", JA_STATUS_SUCCESS, NULL);

	glGenBuffers(1, &out->glptr);
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &old_bind);
	glBindBuffer(GL_ARRAY_BUFFER, out->glptr); // Before ask if is!

	if (glIsBuffer(out->glptr) == GL_FALSE)
	{
		jaStatusSet(st, "kaVerticesInit", JA_STATUS_ERROR, "Creating GL buffer");
		goto return_failure;
	}

	glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(sizeof(struct kaVertex) * length), data, GL_STREAM_DRAW);
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &reported_size);

	if ((size_t)reported_size != (sizeof(struct kaVertex) * length))
	{
		jaStatusSet(st, "kaVerticesInit", JA_STATUS_ERROR, "Attaching data");
		goto return_failure;
	}

	out->length = length;

	// Bye!
	glBindBuffer(GL_ARRAY_BUFFER, (GLuint)old_bind);
	return 0;

return_failure:
	glBindBuffer(GL_ARRAY_BUFFER, (GLuint)old_bind);

	if (out->glptr != 0)
		glDeleteBuffers(1, &out->glptr);

	return 1;
}


inline void kaVerticesFree(struct kaVertices* vertices)
{
	if (vertices->glptr != 0)
	{
		glDeleteBuffers(1, &vertices->glptr);
		vertices->glptr = 0;
	}
}


int kaIndexInit(const uint16_t* data, size_t length, struct kaIndex* out, struct jaStatus* st)
{
	GLint reported_size = 0;
	GLint old_bind = 0;

	jaStatusSet(st, "kaIndexInit", JA_STATUS_SUCCESS, NULL);

	glGenBuffers(1, &out->glptr);
	glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &old_bind);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, out->glptr); // Before ask if is!

	if (glIsBuffer(out->glptr) == GL_FALSE)
	{
		jaStatusSet(st, "kaIndexInit", JA_STATUS_ERROR, "Creating GL buffer");
		goto return_failure;
	}

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(sizeof(uint16_t) * length), data, GL_STREAM_DRAW);
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &reported_size);

	if ((size_t)reported_size != (sizeof(uint16_t) * length))
	{
		jaStatusSet(st, "kaIndexInit", JA_STATUS_ERROR, "Attaching data");
		goto return_failure;
	}

	out->length = length;

	// Bye!
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)old_bind);
	return 0;

return_failure:
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)old_bind);

	if (out->glptr != 0)
		glDeleteBuffers(1, &out->glptr);

	return 1;
}


inline void kaIndexFree(struct kaIndex* index)
{
	if (index->glptr != 0)
	{
		glDeleteBuffers(1, &index->glptr);
		index->glptr = 0;
	}
}


int kaTextureInitImage(const struct jaImage* image, struct kaTexture* out, struct jaStatus* st)
{
	GLint old_bind = 0;

	jaStatusSet(st, "kaTextureInitImage", JA_STATUS_SUCCESS, NULL);

	if (image->format != JA_IMAGE_U8)
	{
		jaStatusSet(st, "kaTextureInitImage", JA_STATUS_ERROR, "Only 8 bits per component images supported");
		return 1;
	}

	glGenTextures(1, &out->glptr);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &old_bind);
	glBindTexture(GL_TEXTURE_2D, out->glptr); // Before ask if is!

	if (glIsTexture(out->glptr) == GL_FALSE)
	{
		jaStatusSet(st, "kaTextureInitImage", JA_STATUS_ERROR, "Creating GL texture");
		return 1;
	}

	switch (g_context.cfg_filter)
	{
	case FILTER_BILINEAR:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;
	case FILTER_TRILINEAR:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;

	case FILTER_PIXEL_BILINEAR:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		break;
	case FILTER_PIXEL_TRILINEAR:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		break;

	case FILTER_NONE:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		break;
	}

	switch (image->channels)
	{
	case 1:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, (GLsizei)image->width, (GLsizei)image->height, 0, GL_LUMINANCE,
		             GL_UNSIGNED_BYTE, image->data);
		break;
	case 2:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, (GLsizei)image->width, (GLsizei)image->height, 0,
		             GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, image->data);
		break;
	case 3:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)image->width, (GLsizei)image->height, 0, GL_RGB,
		             GL_UNSIGNED_BYTE, image->data);
		break;
	case 4:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)image->width, (GLsizei)image->height, 0, GL_RGBA,
		             GL_UNSIGNED_BYTE, image->data);
		break;
	default: break;
	}

	if (g_context.cfg_filter != FILTER_NONE)
		glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, (GLuint)old_bind);
	return 0;
}


int kaTextureInitFilename(const char* image_filename, struct kaTexture* out, struct jaStatus* st)
{
	struct jaImage* image = NULL;
	jaStatusSet(st, "kaTextureInitFilename", JA_STATUS_SUCCESS, NULL);

	if ((image = jaImageLoad(image_filename, st)) == NULL)
		return 1;

	if (kaTextureInitImage(image, out, st) != 0)
	{
		jaImageDelete(image);
		return 1;
	}

	jaImageDelete(image);
	return 0;
}


inline void kaTextureFree(struct kaTexture* texture)
{
	if (texture->glptr != 0)
	{
		glDeleteTextures(1, &texture->glptr);
		texture->glptr = 0;
	}
}

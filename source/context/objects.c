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

 [context/objects.c]
 - Alexander Brandt 2019-2020
-----------------------------*/

#include "private.h"


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


int kaProgramInit(struct kaWindow* window, const char* vertex_code, const char* fragment_code, struct kaProgram* out,
                  struct jaStatus* st)
{
	(void)window;
	GLint success = GL_FALSE;
	GLuint vertex = 0;
	GLuint fragment = 0;

	jaStatusSet(st, "kaProgramInit", JA_STATUS_SUCCESS, NULL);

	if (vertex_code == NULL || fragment_code == NULL)
	{
		jaStatusSet(st, "kaProgramInit", JA_STATUS_INVALID_ARGUMENT, NULL);
		goto return_failure;
	}

	// Compile shaders
	if ((vertex = glCreateShader(GL_VERTEX_SHADER)) == 0 || (fragment = glCreateShader(GL_FRAGMENT_SHADER)) == 0)
	{
		jaStatusSet(st, "kaProgramInit", JA_STATUS_ERROR, "creating GL shader\n");
		goto return_failure;
	}

	glShaderSource(vertex, 1, &vertex_code, NULL);
	glShaderSource(fragment, 1, &fragment_code, NULL);

	if (sCompileShader(vertex, st) != 0 || sCompileShader(fragment, st) != 0)
		goto return_failure;

	// Create program
	if ((out->glptr = glCreateProgram()) == 0)
	{
		jaStatusSet(st, "kaProgramInit", JA_STATUS_ERROR, "creating GL program\n");
		goto return_failure;
	}

	glAttachShader(out->glptr, vertex);
	glAttachShader(out->glptr, fragment);

	glBindAttribLocation(out->glptr, ATTRIBUTE_POSITION, "vertex_position"); // Before link!
	glBindAttribLocation(out->glptr, ATTRIBUTE_COLOR, "vertex_color");
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


inline void kaProgramFree(struct kaWindow* window, struct kaProgram* program)
{
	(void)window;

	if (program != NULL && program->glptr != 0)
	{
		glDeleteProgram(program->glptr);
		program->glptr = 0;
	}
}


int kaVerticesInit(struct kaWindow* window, const struct kaVertex* data, uint16_t length, struct kaVertices* out,
                   struct jaStatus* st)
{
	(void)window;
	GLint reported_size = 0;
	GLint old_bind = 0;

	jaStatusSet(st, "kaVerticesInit", JA_STATUS_SUCCESS, NULL);

	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &old_bind);
	glGenBuffers(1, &out->glptr);
	glBindBuffer(GL_ARRAY_BUFFER, out->glptr); // Before ask if is!

	if (glIsBuffer(out->glptr) == GL_FALSE)
	{
		jaStatusSet(st, "kaVerticesInit", JA_STATUS_ERROR, "creating GL buffer");
		goto return_failure;
	}

	glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(sizeof(struct kaVertex) * length), data, GL_STREAM_DRAW);
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &reported_size);

	if ((size_t)reported_size != (sizeof(struct kaVertex) * length))
	{
		jaStatusSet(st, "kaVerticesInit", JA_STATUS_ERROR, "attaching data");
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


inline void kaVerticesFree(struct kaWindow* window, struct kaVertices* vertices)
{
	(void)window;

	if (vertices != NULL && vertices->glptr != 0)
	{
		glDeleteBuffers(1, &vertices->glptr);
		vertices->glptr = 0;
	}
}


int kaIndexInit(struct kaWindow* window, const uint16_t* data, size_t length, struct kaIndex* out, struct jaStatus* st)
{
	(void)window;
	GLint reported_size = 0;
	GLint old_bind = 0;

	jaStatusSet(st, "kaIndexInit", JA_STATUS_SUCCESS, NULL);

	glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &old_bind);
	glGenBuffers(1, &out->glptr);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, out->glptr); // Before ask if is!

	if (glIsBuffer(out->glptr) == GL_FALSE)
	{
		jaStatusSet(st, "kaIndexInit", JA_STATUS_ERROR, "creating GL buffer");
		goto return_failure;
	}

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(sizeof(uint16_t) * length), data, GL_STREAM_DRAW);
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &reported_size);

	if ((size_t)reported_size != (sizeof(uint16_t) * length))
	{
		jaStatusSet(st, "kaIndexInit", JA_STATUS_ERROR, "attaching data");
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


inline void kaIndexFree(struct kaWindow* window, struct kaIndex* index)
{
	(void)window;

	if (index != NULL && index->glptr != 0)
	{
		glDeleteBuffers(1, &index->glptr);
		index->glptr = 0;
	}
}


int kaTextureInitImage(struct kaWindow* window, const struct jaImage* image, enum kaTextureFilter filter,
                       enum kaTextureWrap wrap, struct kaTexture* out, struct jaStatus* st)
{
	(void)window;
	GLint old_bind = 0;

	jaStatusSet(st, "kaTextureInit", JA_STATUS_SUCCESS, NULL);

	if (image->format != JA_IMAGE_U8)
	{
		jaStatusSet(st, "kaTextureInit", JA_STATUS_ERROR, "only 8 bits per component images supported");
		return 1;
	}

	glGetIntegerv(GL_TEXTURE_BINDING_2D, &old_bind);
	glGenTextures(1, &out->glptr);
	glBindTexture(GL_TEXTURE_2D, out->glptr); // Before ask if is!

	if (glIsTexture(out->glptr) == GL_FALSE)
	{
		jaStatusSet(st, "kaTextureInit", JA_STATUS_ERROR, "creating GL texture");
		return 1;
	}

	out->filter = filter;
	out->wrap = wrap;

	switch (filter)
	{
	case KA_FILTER_BILINEAR:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;
	case KA_FILTER_TRILINEAR:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;
	case KA_FILTER_PIXEL_BILINEAR:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		break;
	case KA_FILTER_PIXEL_TRILINEAR:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		break;
	case KA_FILTER_NONE:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		break;
	default: break;
	}

	switch (wrap)
	{
	case KA_CLAMP:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		break;
	case KA_MIRRORED_REPEAT:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		break;
	default: break; // KA_REPEAT is the default value
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

	if (filter != KA_FILTER_NONE)
		glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, (GLuint)old_bind);
	return 0;
}


int kaTextureInitFilename(struct kaWindow* window, const char* image_filename, enum kaTextureFilter filter,
                          enum kaTextureWrap wrap, struct kaTexture* out, struct jaStatus* st)
{
	struct jaImage* image = NULL;
	jaStatusSet(st, "kaTextureInit", JA_STATUS_SUCCESS, NULL);

	if ((image = jaImageLoad(image_filename, st)) == NULL)
		return 1;

	if (kaTextureInitImage(window, image, filter, wrap, out, st) != 0)
	{
		jaImageDelete(image);
		return 1;
	}

	jaImageDelete(image);
	return 0;
}


void kaTextureUpdate(struct kaWindow* window, const struct jaImage* image, size_t x, size_t y, size_t width,
                     size_t height, struct kaTexture* out)
{
	(void)window;
	GLint old_bind = 0;

	glGetIntegerv(GL_TEXTURE_BINDING_2D, &old_bind);
	glBindTexture(GL_TEXTURE_2D, out->glptr);

	switch (image->channels)
	{
	case 1:
		glTexSubImage2D(GL_TEXTURE_2D, 0, (GLsizei)x, (GLsizei)y, (GLsizei)width, (GLsizei)height, GL_LUMINANCE,
		                GL_UNSIGNED_BYTE, image->data);
		break;
	case 2:
		glTexSubImage2D(GL_TEXTURE_2D, 0, (GLsizei)x, (GLsizei)y, (GLsizei)width, (GLsizei)height, GL_LUMINANCE_ALPHA,
		                GL_UNSIGNED_BYTE, image->data);
		break;
	case 3:
		glTexSubImage2D(GL_TEXTURE_2D, 0, (GLsizei)x, (GLsizei)y, (GLsizei)width, (GLsizei)height, GL_RGB,
		                GL_UNSIGNED_BYTE, image->data);
		break;
	case 4:
		glTexSubImage2D(GL_TEXTURE_2D, 0, (GLsizei)x, (GLsizei)y, (GLsizei)width, (GLsizei)height, GL_RGBA,
		                GL_UNSIGNED_BYTE, image->data);
		break;
	default: break;
	}

	if (out->filter != KA_FILTER_NONE)
		glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, (GLuint)old_bind);
}


inline void kaTextureFree(struct kaWindow* window, struct kaTexture* texture)
{
	(void)window;

	if (texture != NULL && texture->glptr != 0)
	{
		glDeleteTextures(1, &texture->glptr);
		texture->glptr = 0;
	}
}

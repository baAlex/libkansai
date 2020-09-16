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

 [context/state.c]
 - Alexander Brandt 2019-2020
-----------------------------*/

#include "private.h"


void kaSetProgram(struct kaWindow* window, const struct kaProgram* program)
{
	if (window == NULL || program == NULL)
		return;

	if (program != window->current_program)
	{
		window->current_program = program;

		window->uniform.world = glGetUniformLocation(program->glptr, "world");
		window->uniform.local = glGetUniformLocation(program->glptr, "local");
		window->uniform.camera = glGetUniformLocation(program->glptr, "camera");
		window->uniform.camera_position = glGetUniformLocation(program->glptr, "camera_position");

		window->uniform.texture[0] = glGetUniformLocation(program->glptr, "texture0");
		window->uniform.texture[1] = glGetUniformLocation(program->glptr, "texture1");
		window->uniform.texture[2] = glGetUniformLocation(program->glptr, "texture2");
		window->uniform.texture[3] = glGetUniformLocation(program->glptr, "texture3");
		window->uniform.texture[4] = glGetUniformLocation(program->glptr, "texture4");
		window->uniform.texture[5] = glGetUniformLocation(program->glptr, "texture5");
		window->uniform.texture[6] = glGetUniformLocation(program->glptr, "texture6");
		window->uniform.texture[7] = glGetUniformLocation(program->glptr, "texture7");

		glUseProgram(program->glptr);

		glUniformMatrix4fv(window->uniform.world, 1, GL_FALSE, &window->world.e[0][0]);
		glUniformMatrix4fv(window->uniform.local, 1, GL_FALSE, &window->local.e[0][0]);
		glUniformMatrix4fv(window->uniform.camera, 1, GL_FALSE, &window->camera.e[0][0]);
		glUniform3fv(window->uniform.camera_position, 1, (float*)&window->camera_position);
		glUniform1i(window->uniform.texture[0], 0);
		glUniform1i(window->uniform.texture[1], 1);
		glUniform1i(window->uniform.texture[2], 2);
		glUniform1i(window->uniform.texture[3], 3);
		glUniform1i(window->uniform.texture[4], 4);
		glUniform1i(window->uniform.texture[5], 5);
		glUniform1i(window->uniform.texture[6], 6);
		glUniform1i(window->uniform.texture[7], 7);
	}
}


inline void kaSetVertices(struct kaWindow* window, const struct kaVertices* vertices)
{
	if (window == NULL || vertices == NULL)
		return;

	if (vertices != window->current_vertices)
	{
		window->current_vertices = vertices;

		glBindBuffer(GL_ARRAY_BUFFER, vertices->glptr);
		glVertexAttribPointer(ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(struct kaVertex), NULL);
		glVertexAttribPointer(ATTRIBUTE_COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(struct kaVertex), ((float*)NULL) + 3);
		glVertexAttribPointer(ATTRIBUTE_UV, 2, GL_FLOAT, GL_FALSE, sizeof(struct kaVertex), ((float*)NULL) + 7);
	}
}


inline void kaSetTexture(struct kaWindow* window, int unit, const struct kaTexture* texture)
{
	if (window == NULL || texture == NULL)
		return;

	if (texture != window->current_texture)
	{
		window->current_texture = texture;

		glActiveTexture((GLenum)(GL_TEXTURE0 + unit));
		glBindTexture(GL_TEXTURE_2D, texture->glptr);
	}
}


inline void kaSetWorld(struct kaWindow* window, struct jaMatrix4 matrix)
{
	if (window == NULL)
		return;

	memcpy(&window->world, &matrix, sizeof(struct jaMatrix4));

	if (window->current_program != NULL)
		glUniformMatrix4fv(window->uniform.world, 1, GL_FALSE, &window->world.e[0][0]);
}


inline void kaSetCameraLookAt(struct kaWindow* window, struct jaVector3 target, struct jaVector3 origin)
{
	if (window == NULL)
		return;

	window->camera_position = origin;
	window->camera = jaMatrix4LookAt(origin, target, (struct jaVector3){0.0f, 0.0f, 1.0f});

	if (window->current_program != NULL)
	{
		glUniformMatrix4fv(window->uniform.camera, 1, GL_FALSE, &window->camera.e[0][0]);
		glUniform3fv(window->uniform.camera_position, 1, (float*)&window->camera_position);
	}
}


inline void kaSetCameraMatrix(struct kaWindow* window, struct jaMatrix4 matrix, struct jaVector3 origin)
{
	if (window == NULL)
		return;

	window->camera_position = origin;
	window->camera = matrix;

	if (window->current_program != NULL)
	{
		glUniformMatrix4fv(window->uniform.camera, 1, GL_FALSE, &window->camera.e[0][0]);
		glUniform3fv(window->uniform.camera_position, 1, (float*)&window->camera_position);
	}
}


inline void kaSetLocal(struct kaWindow* window, struct jaMatrix4 matrix)
{
	if (window == NULL)
		return;

	memcpy(&window->local, &matrix, sizeof(struct jaMatrix4));

	if (window->current_program != NULL)
		glUniformMatrix4fv(window->uniform.local, 1, GL_FALSE, &window->local.e[0][0]);
}


inline void kaSetCleanColor(struct kaWindow* window, uint8_t r, uint8_t g, uint8_t b)
{
	(void)window;
	glClearColor((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, 1.0f);
}


inline void kaDraw(struct kaWindow* window, const struct kaIndex* index)
{
	(void)window;

	if (index != NULL)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index->glptr);
		glDrawElements(GL_TRIANGLES, (GLsizei)index->length, GL_UNSIGNED_SHORT, NULL);
	}
}


inline void kaDrawDefault(struct kaWindow* window)
{
	if (window != NULL)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, window->default_index.glptr);
		glDrawElements(GL_TRIANGLES, (GLsizei)window->default_index.length, GL_UNSIGNED_SHORT, NULL);
	}
}

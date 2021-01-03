
#include <stdio.h>
#include <stdlib.h>

#include "japan-image.h"
#include "japan-matrix.h"
#include "japan-utilities.h"
#include "japan-vector.h"
#include "kansai-context.h"

#define NAME "Viewer"


static char* s_vertex_code = "#version 100\n"
                             "attribute vec3 vertex_position; attribute vec2 vertex_uv;"
                             "uniform mat4 world; uniform mat4 camera; uniform mat4 local;"
                             "varying vec2 uv;"

                             "void main() { uv = vertex_uv;"
                             "gl_Position = world * camera * local * vec4(vertex_position, 1.0); }";

static char* s_fragment_code = "#version 100\n"
                               "uniform sampler2D texture0;"
                               "varying lowp vec2 uv;"

                               "void main() { gl_FragColor = texture2D(texture0, uv); }";


struct WindowData
{
	const char* filename;
	struct jaImage* image;
	struct kaTexture texture;
	struct kaProgram program;
};


static void sInit(struct kaWindow* w, void* raw_data, struct jaStatus* st)
{
	(void)w;
	(void)st;

	struct WindowData* data = raw_data;

	if (data->filename == NULL)
	{
		printf("No input specified\n");
		return;
	}

	if ((data->image = jaImageLoad(data->filename, st)) == NULL ||
	    kaTextureInitImage(w, data->image, KA_FILTER_NONE, KA_CLAMP, &data->texture, st) != 0 ||
	    kaProgramInit(w, s_vertex_code, s_fragment_code, &data->program, st) != 0)
		return;

	kaSetTexture(w, 0, &data->texture);
	kaSetProgram(w, &data->program);
}


static void sFrame(struct kaWindow* w, struct kaEvents e, float delta, void* raw_data, struct jaStatus* st)
{
	(void)e;
	(void)delta;
	(void)raw_data;
	(void)st;

	kaDrawDefault(w);
}


void sResize(struct kaWindow* w, int width, int height, void* raw_data, struct jaStatus* st)
{
	(void)st;

	struct WindowData* data = raw_data;
	float aspect = (float)data->image->width / (float)data->image->height;

	// Camera matrix, just centre the origin in the window middle
	kaSetCameraMatrix(w,
	                  jaMatrixOrthographicF4(-((float)width / 2.0f), ((float)width / 2.0f), -((float)height / 2.0f),
	                                        ((float)height / 2.0f), 0.0f, 2.0f),
	                  (struct jaVectorF3){0.0f, 0.0f, 0.0f});

	// Calculate a scale respecting the aspect ratio
	float scale = ((float)width / (float)height > aspect) ? (float)height / (float)data->image->height
	                                                      : (float)width / (float)data->image->width;

	// Scale the local matrix for the screen
	kaSetLocal(w,
	           jaMatrixScaleAnsioF4(jaMatrixF4Identity(), (struct jaVectorF3){(float)data->image->width * scale,
	                                                                       (float)data->image->height * scale, 1.0f}));
}


static void sClose(struct kaWindow* w, void* raw_data)
{
	struct WindowData* data = raw_data;

	kaTextureFree(w, &data->texture);
	kaProgramFree(w, &data->program);
}


int main(int argc, char* argv[])
{
	struct jaStatus st = {0};
	struct WindowData data = {0};

	if (argc >= 2)
		data.filename = argv[1];

	if (kaContextStart(&st) != 0)
		goto return_failure;

	if (kaWindowCreate(NULL, sInit, sFrame, sResize, NULL, sClose, &data, &st) != 0)
		goto return_failure;

	while (1)
	{
		if (kaContextUpdate(&st) != 0)
			break;
	}

	if (st.code != JA_STATUS_SUCCESS)
		goto return_failure;

	// Bye!
	kaContextStop();
	return EXIT_SUCCESS;

return_failure:
	jaStatusPrint(NAME, st);
	kaContextStop();
	return EXIT_FAILURE;
}

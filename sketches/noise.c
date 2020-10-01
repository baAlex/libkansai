
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "japan-image.h"
#include "japan-vector.h"
#include "kansai-context.h"
#include "kansai-random.h"

#define NAME "Noise"
#define SIZE 256


struct WindowData
{
	struct kaXorshift rng;

	struct jaImage* image;
	struct kaTexture texture;
	struct kaProgram program;

	size_t previous_median;
	size_t previous_reset;
};


static inline void sReset(struct WindowData* data)
{
	uint64_t* p = data->image->data;
	uint64_t* end = (uint64_t*)data->image->data + (data->image->size >> 3);

	for (; p < end; p++)
		*p = kaRandom(&data->rng);
}


static inline void sMedian(struct WindowData* data)
{
	uint8_t* pixel = data->image->data;
	unsigned a = 0;

	for (size_t i = 0; i < (data->image->size); i++)
	{
		// Median neighbours, kinda
		a = 0;
		a += pixel[(i + 1) % data->image->size];
		a += pixel[(i - 1) % data->image->size];
		a += pixel[(i + SIZE) % data->image->size];
		a += pixel[(i - SIZE) % data->image->size];
		a = a >> 2;

		// Median with the pixel itself
		a += pixel[i];
		a = a >> 1;

		// Mix with noise
		a = a << 1;

		if (a > 4)
			a = a - (unsigned)(kaRandom(&data->rng) % 4);
		else
			a = a + (unsigned)(kaRandom(&data->rng) % 4);

		a = a >> 1;

		// Enjoy!
		pixel[i] = (uint8_t)(a % 255);
	}
}


static void sInit(struct kaWindow* w, void* user_data, struct jaStatus* st)
{
	struct WindowData* data = user_data;

	kaSeed(&data->rng, 0);
	kaSetCameraMatrix(w, jaMatrix4Orthographic(-0.5f, +0.5f, -0.5f, +0.5f, 0.0f, 2.0f), jaVector3Clean());

	// Create an image
	if ((data->image = jaImageCreate(JA_IMAGE_U8, SIZE, SIZE, 1)) == NULL)
	{
		st->code = JA_STATUS_ERROR;
		return;
	}

	sReset(data);

	// Upload it as a texture
	if (kaTextureInitImage(w, data->image, KA_FILTER_BILINEAR, KA_CLAMP, &data->texture, st) != 0)
		return;

	kaSetTexture(w, 0, &data->texture);

	// A program
	{
		const char* vertex_code =
		    "#version 100\n"
		    "attribute vec3 vertex_position; attribute vec4 vertex_color; attribute vec2 vertex_uv;"
		    "uniform mat4 world; uniform mat4 camera; uniform mat4 local;"
		    "varying vec4 color; varying vec2 uv;"

		    "void main() { color = vertex_color; uv = vertex_uv;"
		    "gl_Position = world * camera * local * vec4(vertex_position, 1.0); }";

		const char* fragment_code =
		    "#version 100\n"
		    "uniform sampler2D texture0;"
		    "varying lowp vec2 uv;"

		    "lowp float LinearStep(lowp float edge0, lowp float edge1, lowp float v) {"
		    "return clamp((v - edge0) / (edge1 - edge0), 0.0, 1.0); }"

		    "void main() { lowp float l = texture2D(texture0, uv).r;"
		    "gl_FragColor = mix(vec4(1.0, 1.0, 1.0, 1.0), vec4(1.0, 0.0, 0.0, 1.0), LinearStep(0.1, 0.2, l));"
		    "gl_FragColor = mix(gl_FragColor, vec4(0.0, 0.0, 0.0, 1.0), LinearStep(0.2, 0.4, l));"
		    "gl_FragColor = mix(gl_FragColor, vec4(1.0, 1.0, 1.0, 1.0), LinearStep(0.4, 0.5, l));"
		    "/* Test */ /*gl_FragColor = texture2D(texture0, uv);*/ }";

		if (kaProgramInit(w, vertex_code, fragment_code, &data->program, st) != 0)
			return;

		kaSetProgram(w, &data->program);
	}
}


static void sFrame(struct kaWindow* w, struct kaEvents e, float delta, void* user_data, struct jaStatus* st)
{
	(void)st;
	(void)e;
	(void)delta;

	struct WindowData* data = user_data;
	size_t current = kaGetTime(w);

	if (current > (data->previous_reset + 6000)) // 6 seconds
	{
		sReset(data);
		data->previous_reset = current;
	}

	if (current > (data->previous_median + (1000 / 12))) // 12 fps
	{
		sMedian(data);
		kaTextureUpdate(w, data->image, 0, 0, data->image->width, data->image->height, &data->texture);
		data->previous_median = current;
	}

	kaDrawDefault(w);
}


int main()
{
	struct jaStatus st = {0};
	struct WindowData data = {0};

	if (kaContextStart(&st) != 0)
		goto return_failure;

	if (kaWindowCreate(NULL, sInit, sFrame, NULL, NULL, NULL, &data, &st) != 0)
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

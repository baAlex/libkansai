
#include <stdio.h>
#include <stdlib.h>

#include "japan-utilities.h"
#include "japan-vector.h"
#include "kansai-context.h"
#include "kansai-random.h"

#define NAME "Square"
#define EPSILON 0.1


struct WindowData
{
	struct jaVectorF2 position;
	struct kaXorshift rng;
	bool start_press;
};


static void sInit(struct kaWindow* w, void* user_data, struct jaStatus* st)
{
	(void)st;
	struct WindowData* data = user_data;

	data->position = (struct jaVectorF2){160.0f, 120.0f};
	kaSeed(&data->rng, 0);

	kaSetWorld(w, jaMatrixF4Identity());
	kaSetLocal(w, jaMatrixRotateZF4(jaMatrixF4Identity(), jaDegToRad(45.0f)));
	kaSetCameraMatrix(w, jaMatrixOrthographicF4(-0.5f, +0.5f, -0.5f, +0.5f, 0.0f, 2.0f), (struct jaVectorF3){0.0f, 0.0f, 0.0f});
}


static void sFrame(struct kaWindow* w, struct kaEvents e, float delta, void* user_data, struct jaStatus* st)
{
	(void)st;
	struct WindowData* data = user_data;

	if (jaVectorLengthF2(e.pad) > EPSILON)
	{
		struct jaVectorF2 movement = jaVectorScaleF2(jaVectorNormalizeF2(e.pad), 4.0f * delta);
		data->position = jaVectorAddF2(data->position, movement);
	}

	if (e.start == false)
		data->start_press = false;
	else
	{
		if (data->start_press == false)
		{
			data->start_press = true;
			kaSetCleanColor(w, kaRgbRandom(&data->rng));
		}
	}

	kaDrawDefault(w);
}


static void sArgumentsCallback(enum jaStatusCode code, int i, const char* key, const char* value)
{
	printf("[Warning] %s, argument %i ['%s' = '%s']\n", jaStatusCodeMessage(code), i, key, value);
}


int main(int argc, const char* argv[])
{
	struct jaStatus st = {0};
	struct WindowData data = {0};

	if (kaContextStart(&st) != 0)
		goto return_failure;

	{
		struct jaConfiguration* cfg = jaConfigurationCreate();

		if (cfg == NULL)
			goto return_failure;

		if (jaCvarCreateInt(cfg, "render.width", 720, 320, INT_MAX, &st) == NULL ||
		    jaCvarCreateInt(cfg, "render.height", 480, 240, INT_MAX, &st) == NULL ||
		    jaCvarCreateInt(cfg, "render.fullscreen", 0, 0, 1, &st) == NULL ||
		    jaCvarCreateInt(cfg, "render.vsync", 1, 0, 1, &st) == NULL ||
		    jaCvarCreateString(cfg, "kansai.caption", "Square", NULL, NULL, &st) == NULL)
			goto return_failure;

		jaConfigurationArgumentsEx(cfg, JA_UTF8, JA_SKIP_FIRST, sArgumentsCallback, argc, argv);

		if (kaWindowCreate(cfg, sInit, sFrame, NULL, NULL, NULL, &data, &st) != 0)
			goto return_failure;

		jaConfigurationDelete(cfg);
	}

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

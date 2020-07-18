
#include <stdio.h>
#include <stdlib.h>

#include "japan-vector.h"
#include "kansai-context.h"
#include "kansai-random.h"

#define NAME "Square"
#define EPSILON 0.1


struct WindowData
{
	struct jaVector2 position;
	struct kaXorshift rng;
	bool start_press;
};


static void sInit(struct kaWindow* w, void* user_data, struct jaStatus* st)
{
	(void)st;
	struct WindowData* data = user_data;

	data->position = jaVector2Set(160.0f, 120.0f);
	kaSeed(&data->rng, 0);

	kaSetCameraMatrix(w, jaMatrix4Identity(), jaVector3Clean());
	kaSetWorld(w, jaMatrix4Orthographic(0.0f, 320.0f, 0.0f, 240.0f, 0.0f, 2.0f));
}


static void sFrame(struct kaWindow* w, struct kaEvents e, float delta, void* user_data, struct jaStatus* st)
{
	(void)st;
	struct WindowData* data = user_data;

	if (jaVector2Length(e.pad) > EPSILON)
	{
		struct jaVector2 movement = jaVector2Scale(jaVector2Normalize(e.pad), 4.0f * delta);
		data->position = jaVector2Add(data->position, movement);
	}

	if (e.start == false)
		data->start_press = false;
	else
	{
		if (data->start_press == false)
		{
			data->start_press = true;
			kaSetCleanColor(w, (uint8_t)(kaRandom(&data->rng) % 255), (uint8_t)(kaRandom(&data->rng) % 255),
			                (uint8_t)(kaRandom(&data->rng) % 255));
		}
	}

	kaDrawSprite(w, (struct jaVector3){data->position.x, data->position.y, 1.0f},
	             (struct jaVector3){64.0f, 64.0f, 0.0f});
}


int main()
{
	struct jaStatus st = {0};
	struct WindowData data = {0};

	if (kaContextStart(NULL, &st) != 0)
		goto return_failure;

	if (kaWindowCreate(NAME, sInit, sFrame, NULL, NULL, NULL, &data, &st) != 0)
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

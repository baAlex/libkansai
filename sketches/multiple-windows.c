
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "japan-version.h"
#include "kansai-context.h"
#include "kansai-version.h"

#define NAME "Multiple windows"


struct WindowData
{
	int id;
	int screenshot_counter;
	bool x_press;
	float phase;
};


static void sInit(struct kaWindow* w, void* user_data, struct jaStatus* st)
{
	(void)user_data;
	(void)w;
	jaStatusSet(st, "sInit", JA_STATUS_SUCCESS, NULL);
}


static void sFrame(struct kaWindow* w, struct kaEvents e, float delta, void* user_data, struct jaStatus* st)
{
	struct WindowData* data = user_data;
	jaStatusSet(st, "sFrame", JA_STATUS_SUCCESS, NULL);

	struct jaVectorF3 pos;
	pos.x = sinf(data->phase);
	pos.y = sinf(data->phase / 4.0f);

	kaSetLocal(w, jaMatrixTranslationF4(pos));
	kaDrawDefault(w);

	data->phase += 0.125f * delta;

	if (data->id == 0)
	{
		kaSetLocal(w, jaMatrixF4Identity());
		kaDrawDefault(w);
	}
}


static void sResize(struct kaWindow* w, int width, int height, void* user_data, struct jaStatus* st)
{
	(void)w;
	(void)st;

	struct WindowData* data = user_data;
	printf("Window %i resized to %i, %i\n", data->id, width, height);
}


static void sKeyboard(struct kaWindow* w, enum kaKey key, enum kaGesture mode, void* user_data, struct jaStatus* st)
{
	struct WindowData* data = user_data;

	printf("Window %i, %s key %i\n", data->id, (mode == KA_PRESSED) ? "pressed" : "released", key);

	if (kaWindowInFocus(w) == true && key == KA_KEY_F12 && mode == KA_RELEASED)
	{
		struct jaImage* image = NULL;
		char tstr[64];
		char filename[256];

		time_t t;
		time(&t);

		strftime(tstr, 64, "%Y-%m-%e, %H:%M:%S", localtime(&t));
		sprintf(filename, "%s - %s (%i%i).sgi", NAME, tstr, data->id, data->screenshot_counter);

		data->screenshot_counter += 1;

		if ((image = kaScreenshot(w, st)) == NULL)
			return;

		if (jaImageSaveSgi(image, filename, st) != 0)
			return;
	}
}


static void sMouse(struct kaWindow* w, int button, enum kaGesture mode, void* user_data, struct jaStatus* st)
{
	(void)w;
	(void)st;

	struct WindowData* data = user_data;

	printf("Window %i, %s mouse button %i\n", data->id, (mode == KA_PRESSED) ? "Pressed" : "Released", button);
}


static void sClose(struct kaWindow* w, void* user_data)
{
	(void)w;

	struct WindowData* data = user_data;
	printf("Close request for window %i...\n", data->id);
}


int main()
{
	struct jaStatus st = {0};
	struct WindowData a = {0};
	struct WindowData b = {0};

	a.id = 0;
	b.id = 1;

	printf("%s\n", NAME);
	printf(" - %s\n", jaVersionString());
	printf(" - %s\n", kaVersionString());

	if (kaContextStart(&st) != 0)
		goto return_failure;

	if (kaWindowCreate(NULL, sInit, sFrame, sResize, sKeyboard, sMouse, sClose, &a, &st) != 0)
		goto return_failure;

	if (kaWindowCreate(NULL, sInit, sFrame, sResize, sKeyboard, sMouse, sClose, &b, &st) != 0)
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

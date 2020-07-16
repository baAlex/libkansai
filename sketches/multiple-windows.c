
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
};


static void sInit(struct kaWindow* window, void* user_data)
{
	(void)window;
	(void)user_data;
	kaSetCameraMatrix(jaMatrix4Identity(), jaVector3Clean());
	kaSetWorld(jaMatrix4Orthographic(0.0f, 320.0f, 0.0f, 240.0f, 0.0f, 1.0f));
}


static void sFrame(struct kaWindow* window, struct kaEvents e, void* user_data)
{
	(void)window;
	(void)e;

	struct WindowData* data = user_data;
	kaDrawSprite((struct jaVector3){160.0f, 120.0f, 0.0f}, (struct jaVector3){320.0f, 240.0f, 0.0f});

	if (data->id == 0)
		kaDrawSprite((struct jaVector3){160.0f, 120.0f, 0.5f}, (struct jaVector3){64.0f, 64.0f, 0.0f});

	if (e.x != data->x_press)
	{
		printf("Event X = %s on window %i\n", (e.x == true) ? "true" : "false", data->id);
		data->x_press = e.x;
	}
}


static void sResize(struct kaWindow* window, int width, int height, void* user_data)
{
	(void)window;

	struct WindowData* data = user_data;
	printf("Window %i resized to %i, %i\n", data->id, width, height);
}


static void sFunction(struct kaWindow* window, int f, void* user_data)
{
	struct WindowData* data = user_data;
	printf("F%i pressed on window %i\n", f, data->id);

	if (f == 12)
	{
		char tstr[64];
		char filename[256];

		time_t t;
		time(&t);

		strftime(tstr, 64, "%Y-%m-%e, %H:%M:%S", localtime(&t));
		sprintf(filename, "%s - %s (%i%i).sgi", NAME, tstr, data->id, data->screenshot_counter);

		kaTakeScreenshot(window, filename, NULL);
		data->screenshot_counter += 1;
	}
}


static void sClose(struct kaWindow* window, void* user_data)
{
	(void)window;

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

	if (kaContextStart(NULL, &st) != 0)
		goto return_failure;

	if (kaWindowCreate("Window A", sInit, sFrame, sResize, sFunction, sClose, &a, &st) != 0)
		goto return_failure;

	if (kaWindowCreate("Window B", sInit, sFrame, sResize, sFunction, sClose, &b, &st) != 0)
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

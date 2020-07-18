
#include <stdio.h>
#include <stdlib.h>

#include "kansai-context.h"

#define NAME "Square"


static void sInit(struct kaWindow* window, void* user_data, struct jaStatus* st)
{
	(void)window;
	(void)user_data;
	(void)st;
	kaSetCameraMatrix(jaMatrix4Identity(), jaVector3Clean());
	kaSetWorld(jaMatrix4Orthographic(0.0f, 320.0f, 0.0f, 240.0f, 0.0f, 1.0f));
}


static void sFrame(struct kaWindow* window, struct kaEvents e, float delta, void* user_data, struct jaStatus* st)
{
	(void)window;
	(void)e;
	(void)delta;
	(void)user_data;
	(void)st;
	kaDrawSprite((struct jaVector3){160.0f, 120.0f, 0.0f}, (struct jaVector3){64.0f, 64.0f, 0.0f});
}


int main()
{
	struct jaStatus st = {0};

	if (kaContextStart(NULL, &st) != 0)
		goto return_failure;

	if (kaWindowCreate(NAME, sInit, sFrame, NULL, NULL, NULL, NULL, &st) != 0)
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

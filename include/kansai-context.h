/*-----------------------------

 [kansai-context.h]
 - Alexander Brandt 2020
-----------------------------*/

#ifndef KANSAI_CONTEXT_H
#define KANSAI_CONTEXT_H

#ifdef KA_EXPORT_SYMBOLS
	#if defined(__clang__) || defined(__GNUC__)
	#define KA_EXPORT __attribute__((visibility("default")))
	#elif defined(_MSC_VER)
	#define KA_EXPORT __declspec(dllexport)
	#endif
#else
	#define KA_EXPORT // Whitespace
#endif

#include <stdbool.h>
#include <stdint.h>

#include "japan-configuration.h"
#include "japan-image.h"
#include "japan-matrix.h"
#include "japan-status.h"
#include "japan-vector.h"

struct kaWindow;

struct kaEvents
{
	bool a, b, x, y;
	bool lb, rb;
	bool view, menu, guide; // TODO
	bool ls, rs;            // TODO

	struct { float h, v; } pad; // TODO
	struct { float h, v, t; } left_analog; // TODO
	struct { float h, v, t; } right_analog; // TODO
};

struct kaProgram
{
	unsigned int glptr;
};

struct kaVertex
{
	struct jaVector4 colour;
	struct jaVector3 position;
	struct jaVector3 normal;
	struct jaVector2 uv;
};

struct kaVertices
{
	unsigned int glptr;
	uint16_t length; // In elements
};

struct kaIndex
{
	unsigned int glptr;
	size_t length; // In elements
};

struct kaTexture
{
	unsigned int glptr;
};


KA_EXPORT int kaContextStart(const struct jaConfiguration*, struct jaStatus* st);
KA_EXPORT void kaContextStop();
KA_EXPORT int kaContextUpdate(struct jaStatus* st);

KA_EXPORT int kaWindowCreate(const char* caption, void (*init_callback)(struct kaWindow*, void*),
                             void (*frame_callback)(struct kaWindow*, const struct kaEvents*, void*),
                             void (*resize_callback)(struct kaWindow*, int, int, void*),
                             void (*function_callback)(struct kaWindow*, int, void*),
                             void (*close_callback)(struct kaWindow*, void*), void* user_data, struct jaStatus* st);
KA_EXPORT void kaWindowDelete(struct kaWindow* window);

KA_EXPORT int kaTakeScreenshot(const struct kaWindow* window, const char* filename, struct jaStatus* st);

//

KA_EXPORT int kaProgramInit(const char* vertex_code, const char* fragment_code, struct kaProgram* out,
                            struct jaStatus* st);
KA_EXPORT void kaProgramFree(struct kaProgram* program);

KA_EXPORT int kaVerticesInit(const struct kaVertex* data, uint16_t length, struct kaVertices* out, struct jaStatus* st);
KA_EXPORT void kaVerticesFree(struct kaVertices* vertices);

KA_EXPORT int kaIndexInit(const uint16_t* data, size_t length, struct kaIndex* out, struct jaStatus* st);
KA_EXPORT void kaIndexFree(struct kaIndex* index);

KA_EXPORT int kaTextureInitImage(const struct jaImage* image, struct kaTexture* out, struct jaStatus* st);
KA_EXPORT int kaTextureInitFilename(const char* image_filename, struct kaTexture* out, struct jaStatus* st);
KA_EXPORT void kaTextureFree(struct kaTexture* texture);

KA_EXPORT void kaSetProgram(const struct kaProgram* program);
KA_EXPORT void kaSetVertices(const struct kaVertices* vertices);
KA_EXPORT void SetTexture(int unit, const struct kaTexture* texture);

KA_EXPORT void kaSetWorld(struct jaMatrix4 matrix);
KA_EXPORT void kaSetCameraLookAt(struct jaVector3 target, struct jaVector3 origin);
KA_EXPORT void kaSetCameraMatrix(struct jaMatrix4 matrix, struct jaVector3 origin);

KA_EXPORT void kaDraw(const struct kaIndex* index);
KA_EXPORT void kaDrawSprite(struct jaVector3 position, struct jaVector3 scale);

#endif

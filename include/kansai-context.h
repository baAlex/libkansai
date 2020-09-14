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

#include <stdint.h>

#include "japan-image.h"
#include "japan-matrix.h"
#include "japan-status.h"
#include "japan-vector.h"
#include "kansai-color.h"

struct kaWindow;

struct kaEvents
{
	uint8_t a : 1;
	uint8_t b : 1;
	uint8_t x : 1;
	uint8_t y : 1;

	uint8_t select : 1;
	uint8_t start : 1;

	uint8_t pad_u : 1;
	uint8_t pad_d : 1;
	uint8_t pad_l : 1;
	uint8_t pad_r : 1;

	struct jaVector2 pad;
};

struct kaProgram
{
	unsigned int glptr;
};

struct kaVertex
{
	struct jaVector3 position;
	struct kaRgba color;
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

enum kaTextureFilter
{
	KA_FILTER_BILINEAR,
	KA_FILTER_TRILINEAR,
	KA_FILTER_PIXEL_BILINEAR,
	KA_FILTER_PIXEL_TRILINEAR,
	KA_FILTER_NONE
};

enum kaTextureWrap
{
	KA_REPEAT,
	KA_CLAMP,
	KA_MIRRORED_REPEAT
};

struct kaTexture
{
	unsigned int glptr;
	enum kaTextureFilter filter;
	enum kaTextureWrap wrap;
};


KA_EXPORT int kaContextStart(struct jaStatus* st);
KA_EXPORT void kaContextStop();
KA_EXPORT int kaContextUpdate(struct jaStatus* st);

KA_EXPORT int kaWindowCreate(const char* caption, void (*init_callback)(struct kaWindow*, void*, struct jaStatus*),
                             void (*frame_callback)(struct kaWindow*, struct kaEvents, float, void*, struct jaStatus*),
                             void (*resize_callback)(struct kaWindow*, int, int, void*, struct jaStatus*),
                             void (*function_callback)(struct kaWindow*, int, void*, struct jaStatus*),
                             void (*close_callback)(struct kaWindow*, void*), void* user_data, struct jaStatus* st);
KA_EXPORT void kaWindowDelete(struct kaWindow* window);

//

KA_EXPORT int kaProgramInit(struct kaWindow* window, const char* vertex_code, const char* fragment_code,
                            struct kaProgram* out, struct jaStatus* st);
KA_EXPORT void kaProgramFree(struct kaWindow* window, struct kaProgram* program);

KA_EXPORT int kaVerticesInit(struct kaWindow* window, const struct kaVertex* data, uint16_t length,
                             struct kaVertices* out, struct jaStatus* st);
KA_EXPORT void kaVerticesFree(struct kaWindow* window, struct kaVertices* vertices);

KA_EXPORT int kaIndexInit(struct kaWindow* window, const uint16_t* data, size_t length, struct kaIndex* out,
                          struct jaStatus* st);
KA_EXPORT void kaIndexFree(struct kaWindow* window, struct kaIndex* index);

KA_EXPORT int kaTextureInitImage(struct kaWindow* window, const struct jaImage* image, enum kaTextureFilter,
                                 enum kaTextureWrap, struct kaTexture* out, struct jaStatus* st);
KA_EXPORT int kaTextureInitFilename(struct kaWindow* window, const char* image_filename, enum kaTextureFilter,
                                    enum kaTextureWrap, struct kaTexture* out, struct jaStatus* st);
KA_EXPORT void kaTextureUpdate(struct kaWindow* window, const struct jaImage*, size_t x, size_t y, size_t width,
                               size_t height, struct kaTexture* out);
KA_EXPORT void kaTextureFree(struct kaWindow* window, struct kaTexture* texture);

//

KA_EXPORT void kaSetProgram(struct kaWindow* window, const struct kaProgram* program);
KA_EXPORT void kaSetVertices(struct kaWindow* window, const struct kaVertices* vertices);
KA_EXPORT void kaSetTexture(struct kaWindow* window, int unit, const struct kaTexture* texture);

KA_EXPORT void kaSetWorld(struct kaWindow* window, struct jaMatrix4 matrix);
KA_EXPORT void kaSetCameraLookAt(struct kaWindow* window, struct jaVector3 target, struct jaVector3 origin);
KA_EXPORT void kaSetCameraMatrix(struct kaWindow* window, struct jaMatrix4 matrix, struct jaVector3 origin);
KA_EXPORT void kaSetLocal(struct kaWindow* window, struct jaMatrix4 matrix);

KA_EXPORT void kaDraw(struct kaWindow* window, const struct kaIndex* index);
KA_EXPORT void kaDrawDefault(struct kaWindow* window);

KA_EXPORT struct jaImage* kaTakeScreenshot(struct kaWindow* window, struct jaStatus* st);

KA_EXPORT void kaSetCleanColor(struct kaWindow* window, uint8_t r, uint8_t g, uint8_t b);
KA_EXPORT unsigned kaGetTime(struct kaWindow* window);
KA_EXPORT size_t kaGetFrame(struct kaWindow* window);
KA_EXPORT void kaSleep(unsigned ms);

KA_EXPORT void kaSwitchFullscreen(struct kaWindow* window);

#endif

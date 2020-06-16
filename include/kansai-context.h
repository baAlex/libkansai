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
#include <stddef.h>

#include "japan-aabounding.h"
#include "japan-configuration.h"
#include "japan-image.h"
#include "japan-matrix.h"
#include "japan-status.h"
#include "japan-vector.h"

enum Filter
{
	FILTER_BILINEAR,
	FILTER_TRILINEAR,
	FILTER_PIXEL_BILINEAR,
	FILTER_PIXEL_TRILINEAR,
	FILTER_NONE
};

struct Program
{
	unsigned int glptr;
};

struct Vertex
{
	struct jaVector3 pos;
	struct jaVector2 uv;
};

struct Vertices
{
	unsigned int glptr;
	uint16_t length; // In elements
};

struct Index
{
	unsigned int glptr;
	size_t length; // In elements
};

struct Texture
{
	unsigned int glptr;
};

struct ContextEvents
{
	// Input
	bool a, b, x, y;
	bool lb, rb;
	bool view, menu, guide; // TODO
	bool ls, rs;            // TODO

	struct { float h, v; } pad;             // TODO
	struct { float h, v, t; } left_analog;  // TODO
	struct { float h, v, t; } right_analog; // TODO

	// Window
	bool close;
	bool resized;                  // TODO
	struct jaVector2i window_size; // TODO
};


// Context as object

KA_EXPORT struct Context* ContextCreate(const struct jaConfiguration*, const char* caption, struct jaStatus* st);
KA_EXPORT int ContextUpdate(struct Context* context, struct ContextEvents* out_events, struct jaStatus* st);
KA_EXPORT void ContextDelete(struct Context* context);

KA_EXPORT int TakeScreenshot(const struct Context* context, const char* filename, struct jaStatus* st);


// Context state

KA_EXPORT void SetProgram(struct Context* context, const struct Program* program);
KA_EXPORT void SetVertices(struct Context* context, const struct Vertices* vertices);
KA_EXPORT void SetTexture(struct Context* context, int unit, const struct Texture* texture);
KA_EXPORT void SetProjection(struct Context* context, struct jaMatrix4 matrix);
KA_EXPORT void SetCameraLookAt(struct Context* context, struct jaVector3 target, struct jaVector3 origin);
KA_EXPORT void SetCameraMatrix(struct Context* context, struct jaMatrix4 matrix, struct jaVector3 origin);

KA_EXPORT void Draw(struct Context* context, const struct Index* index);
KA_EXPORT void DrawAABB(struct Context* context, struct jaAABBox box, struct jaVector3 pos);


// OpenGL abstractions

KA_EXPORT int ProgramInit(const char* vertex_code, const char* fragment_code, struct Program* out, struct jaStatus* st);
KA_EXPORT void ProgramFree(struct Program* program);

KA_EXPORT int VerticesInit(const struct Vertex* data, uint16_t length, struct Vertices* out, struct jaStatus* st);
KA_EXPORT void VerticesFree(struct Vertices* vertices);

KA_EXPORT int IndexInit(const uint16_t* data, size_t length, struct Index* out, struct jaStatus* st);
KA_EXPORT void IndexFree(struct Index* index);

KA_EXPORT int TextureInitImage(const struct Context*, const struct jaImage* image, struct Texture* out, struct jaStatus* st);
KA_EXPORT int TextureInitFilename(const struct Context*, const char* image_filename, struct Texture* out, struct jaStatus* st);
KA_EXPORT void TextureFree(struct Texture* texture);

#if __STDC_VERSION__ >= 201112L

#define SetCamera(context, val, origin) \
	_Generic((val), \
		struct jaVector3: SetCameraLookAt, \
		struct jaMatrix4: SetCameraMatrix, \
		default: SetCameraLookAt \
	)(context, val, origin)

#define TextureInit(context, src, out, st) \
	_Generic((src), \
		const char*: TextureInitFilename, \
		char*: TextureInitFilename, \
		const struct jaImage*: TextureInitImage, \
		struct jaImage*: TextureInitImage, \
		default: TextureInitImage \
	)(context, src, out, st)

#endif

#endif

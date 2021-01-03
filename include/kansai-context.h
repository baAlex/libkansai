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

#define JA_WIP
#include "japan-configuration.h"

struct kaWindow;

enum kaKey // A subset of 'SDL_scancode.h'
{
	KA_KEY_A = 4,
	KA_KEY_B = 5,
	KA_KEY_C = 6,
	KA_KEY_D = 7,
	KA_KEY_E = 8,
	KA_KEY_F = 9,
	KA_KEY_G = 10,
	KA_KEY_H = 11,
	KA_KEY_I = 12,
	KA_KEY_J = 13,
	KA_KEY_K = 14,
	KA_KEY_L = 15,
	KA_KEY_M = 16,
	KA_KEY_N = 17,
	KA_KEY_O = 18,
	KA_KEY_P = 19,
	KA_KEY_Q = 20,
	KA_KEY_R = 21,
	KA_KEY_S = 22,
	KA_KEY_T = 23,
	KA_KEY_U = 24,
	KA_KEY_V = 25,
	KA_KEY_W = 26,
	KA_KEY_X = 27,
	KA_KEY_Y = 28,
	KA_KEY_Z = 29,

	KA_KEY_1 = 30,
	KA_KEY_2 = 31,
	KA_KEY_3 = 32,
	KA_KEY_4 = 33,
	KA_KEY_5 = 34,
	KA_KEY_6 = 35,
	KA_KEY_7 = 36,
	KA_KEY_8 = 37,
	KA_KEY_9 = 38,
	KA_KEY_0 = 39,

	KA_KEY_RETURN = 40,
	KA_KEY_ESCAPE = 41,
	KA_KEY_BACKSPACE = 42,
	KA_KEY_TAB = 43,
	KA_KEY_SPACE = 44,

	KA_KEY_MINUS = 45,
	KA_KEY_EQUALS = 46,
	KA_KEY_LEFTBRACKET = 47,
	KA_KEY_RIGHTBRACKET = 48,
	KA_KEY_BACKSLASH = 49,
	KA_KEY_SEMICOLON = 51,
	KA_KEY_APOSTROPHE = 52,
	KA_KEY_GRAVE = 53,
	KA_KEY_COMMA = 54,
	KA_KEY_PERIOD = 55,
	KA_KEY_SLASH = 56,

	KA_KEY_CAPSLOCK = 57,

	KA_KEY_F1 = 58,
	KA_KEY_F2 = 59,
	KA_KEY_F3 = 60,
	KA_KEY_F4 = 61,
	KA_KEY_F5 = 62,
	KA_KEY_F6 = 63,
	KA_KEY_F7 = 64,
	KA_KEY_F8 = 65,
	KA_KEY_F9 = 66,
	KA_KEY_F10 = 67,
	KA_KEY_F11 = 68,
	KA_KEY_F12 = 69,

	KA_KEY_RIGHT = 79,
	KA_KEY_LEFT = 80,
	KA_KEY_DOWN = 81,
	KA_KEY_UP = 82,

	KA_KEY_KP_DIVIDE = 84,
	KA_KEY_KP_MULTIPLY = 85,
	KA_KEY_KP_MINUS = 86,
	KA_KEY_KP_PLUS = 87,
	KA_KEY_KP_ENTER = 88,
	KA_KEY_KP_1 = 89,
	KA_KEY_KP_2 = 90,
	KA_KEY_KP_3 = 91,
	KA_KEY_KP_4 = 92,
	KA_KEY_KP_5 = 93,
	KA_KEY_KP_6 = 94,
	KA_KEY_KP_7 = 95,
	KA_KEY_KP_8 = 96,
	KA_KEY_KP_9 = 97,
	KA_KEY_KP_0 = 98,
	KA_KEY_KP_PERIOD = 99,
};

enum kaKeyMode
{
	KA_PRESSED,
	KA_RELEASED
};

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

	struct jaVectorF2 pad;
};

struct kaProgram
{
	unsigned int glptr;
};

struct kaVertex
{
	struct jaVectorF3 position;
	struct kaRgba color;
	struct jaVectorF2 uv;
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

// context/sdl2.c

KA_EXPORT int kaContextStart(struct jaStatus*);
KA_EXPORT void kaContextStop();

KA_EXPORT int kaContextUpdate(struct jaStatus*);

KA_EXPORT unsigned kaGetTime();
KA_EXPORT size_t kaGetFrame();
KA_EXPORT void kaSleep(unsigned ms);

// context/window.c

KA_EXPORT int
kaWindowCreate(const struct jaConfiguration*, void (*init_callback)(struct kaWindow*, void*, struct jaStatus*),
               void (*frame_callback)(struct kaWindow*, struct kaEvents, float, void*, struct jaStatus*),
               void (*resize_callback)(struct kaWindow*, int, int, void*, struct jaStatus*),
               void (*keyboard_callback)(struct kaWindow*, enum kaKey, enum kaKeyMode, void*, struct jaStatus*),
               void (*close_callback)(struct kaWindow*, void*), void* user_data, struct jaStatus*);

KA_EXPORT void kaWindowDelete(struct kaWindow*);
KA_EXPORT struct jaImage* kaScreenshot(struct kaWindow*, struct jaStatus*);
KA_EXPORT void kaSwitchFullscreen(struct kaWindow*);
KA_EXPORT bool kaWindowInFocus(const struct kaWindow*);

// context/objects.c

KA_EXPORT int kaProgramInit(struct kaWindow*, const char* vertex_code, const char* fragment_code, struct kaProgram* out,
                            struct jaStatus*);
KA_EXPORT int kaVerticesInit(struct kaWindow*, const struct kaVertex* data, uint16_t length, struct kaVertices* out,
                             struct jaStatus*);
KA_EXPORT int kaIndexInit(struct kaWindow*, const uint16_t* data, size_t length, struct kaIndex* out, struct jaStatus*);
KA_EXPORT int kaTextureInitImage(struct kaWindow*, const struct jaImage* image, enum kaTextureFilter,
                                 enum kaTextureWrap, struct kaTexture* out, struct jaStatus*);
KA_EXPORT int kaTextureInitFilename(struct kaWindow*, const char* filename, enum kaTextureFilter, enum kaTextureWrap,
                                    struct kaTexture* out, struct jaStatus*);
KA_EXPORT void kaTextureUpdate(struct kaWindow*, const struct jaImage* image, size_t x, size_t y, size_t width,
                               size_t height, struct kaTexture* out);

KA_EXPORT void kaProgramFree(struct kaWindow*, struct kaProgram*);
KA_EXPORT void kaVerticesFree(struct kaWindow*, struct kaVertices*);
KA_EXPORT void kaIndexFree(struct kaWindow*, struct kaIndex*);
KA_EXPORT void kaTextureFree(struct kaWindow*, struct kaTexture*);

// context/state.c

KA_EXPORT void kaSetProgram(struct kaWindow*, const struct kaProgram*);
KA_EXPORT void kaSetVertices(struct kaWindow*, const struct kaVertices*);
KA_EXPORT void kaSetTexture(struct kaWindow*, int unit, const struct kaTexture*);

KA_EXPORT void kaSetWorld(struct kaWindow*, struct jaMatrixF4);
KA_EXPORT void kaSetCameraLookAt(struct kaWindow*, struct jaVectorF3 target, struct jaVectorF3 origin);
KA_EXPORT void kaSetCameraMatrix(struct kaWindow*, struct jaMatrixF4 matrix, struct jaVectorF3 origin);
KA_EXPORT void kaSetLocal(struct kaWindow*, struct jaMatrixF4);

KA_EXPORT void kaSetCleanColor(struct kaWindow*, struct kaRgb);

KA_EXPORT void kaDraw(struct kaWindow*, const struct kaIndex*);
KA_EXPORT void kaDrawDefault(struct kaWindow*);

#endif

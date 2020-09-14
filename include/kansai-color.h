/*-----------------------------

 [kansai-color.h]
 - Alexander Brandt 2020
-----------------------------*/

#ifndef KANSAI_COLOR_H
#define KANSAI_COLOR_H

#ifdef KA_EXPORT_SYMBOLS
	#if defined(__clang__) || defined(__GNUC__)
	#define KA_EXPORT __attribute__((visibility("default")))
	#elif defined(_MSC_VER)
	#define KA_EXPORT __declspec(dllexport)
	#endif
#else
	#define KA_EXPORT // Whitespace
#endif

#include "kansai-random.h"

struct kaRgba
{
	float r;
	float g;
	float b;
	float a;
};

struct kaRgb
{
	float r;
	float g;
	float b;
};

KA_EXPORT struct kaRgb kaRgbFromHsv(float h, float s, float v);
KA_EXPORT struct kaRgba kaRgbaFromHsv(float h, float s, float v, float a);

KA_EXPORT struct kaRgb kaRgbRandom(struct KA_RANDOM_STATE*);
KA_EXPORT struct kaRgba kaRgbaRandom(struct KA_RANDOM_STATE*, float a); // TODO, what about alpha?

#endif

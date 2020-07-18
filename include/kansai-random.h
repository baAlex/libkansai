/*-----------------------------

 [kansai-random.h]
 - Alexander Brandt 2020
-----------------------------*/

#ifndef KANSAI_RANDOM_H
#define KANSAI_RANDOM_H

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

struct kaXorshift // xorshift*
{
	uint64_t a;
};


KA_EXPORT uint64_t kaRandomXorshift(struct kaXorshift* state);
KA_EXPORT void kaSeedXorshift(struct kaXorshift* state, uint64_t seed);

#define kaRandom(state) kaRandomXorshift(state)
#define kaSeed(state, seed) kaSeedXorshift(state, seed)

#endif

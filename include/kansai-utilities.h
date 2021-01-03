/*-----------------------------

 [kansai-utilities.h]
 - Alexander Brandt 2020
-----------------------------*/

#ifndef KANSAI_UTILITIES_H
#define KANSAI_UTILITIES_H

#ifdef KA_EXPORT_SYMBOLS
	#if defined(__clang__) || defined(__GNUC__)
	#define KA_EXPORT __attribute__((visibility("default")))
	#elif defined(_MSC_VER)
	#define KA_EXPORT __declspec(dllexport)
	#endif
#else
	#define KA_EXPORT // Whitespace
#endif

#include "japan-vector.h"
#include "japan-utilities.h"

KA_EXPORT void kaVectorAxes(struct jaVectorF3 angle, struct jaVectorF3* forward, struct jaVectorF3* left,
                            struct jaVectorF3* up);

#endif

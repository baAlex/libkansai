/*-----------------------------

 [kansai-version.h]
 - Alexander Brandt 2020
-----------------------------*/

#ifndef KANSAI_VERSION_H
#define KANSAI_VERSION_H

#ifdef KA_EXPORT_SYMBOLS
	#if defined(__clang__) || defined(__GNUC__)
	#define KA_EXPORT __attribute__((visibility("default")))
	#elif defined(_MSC_VER)
	#define KA_EXPORT __declspec(dllexport)
	#endif
#else
	#define KA_EXPORT // Whitespace
#endif

KA_EXPORT int kaVersionMajor();
KA_EXPORT int kaVersionMinor();
KA_EXPORT int kaVersionPatch();

KA_EXPORT char* kaVersionString();

#endif

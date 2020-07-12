/*-----------------------------

 [kansai-aabounding.h]
 - Alexander Brandt 2019-2020
-----------------------------*/

#ifndef KANSAI_AABOUNDING_H
#define KANSAI_AABOUNDING_H

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

struct kaAABBox
{
	struct jaVector3 min;
	struct jaVector3 max;
};

struct kaAABRectangle
{
	struct jaVector2 min;
	struct jaVector2 max;
};

struct kaSphere
{
	struct jaVector3 origin;
	float radius;
};

struct kaCircle
{
	struct jaVector2 origin;
	float radius;
};

KA_EXPORT bool kaAABCollisionRectRect(struct kaAABRectangle a, struct kaAABRectangle b);
KA_EXPORT bool kaAABCollisionRectCircle(struct kaAABRectangle, struct kaCircle);

KA_EXPORT bool kaAABCollisionBoxBox(struct kaAABBox a, struct kaAABBox b);
KA_EXPORT bool kaAABCollisionBoxSphere(struct kaAABBox, struct kaSphere);

KA_EXPORT struct kaAABRectangle kaAABToRectangle(struct kaAABBox);
KA_EXPORT struct kaAABBox kaAABToBox(struct kaAABRectangle, float min_z, float max_z);

KA_EXPORT struct jaVector2 kaAABMiddleRect(struct kaAABRectangle);
KA_EXPORT struct jaVector3 kaAABMiddleBox(struct kaAABBox);

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L

#define kaAABCollision(a, b)\
	_Generic((a),\
		struct kaAABRectangle : _Generic((b),\
			struct kaAABRectangle : kaAABCollisionRectRect,\
			struct kaCircle : kaAABCollisionRectCircle,\
			default : kaAABCollisionRectRect),\
		struct kaAABBox : _Generic((b),\
			struct kaAABBox : kaAABCollisionBoxBox,\
			struct kaSphere : kaAABCollisionBoxSphere,\
			default : kaAABCollisionBoxBox)\
	)(a, b)

#define kaAABMiddle(obj)\
	_Generic((obj),\
		struct kaAABRectangle : kaAABMiddleRect,\
		struct kaAABBox : kaAABMiddleBox,\
		default : kaAABMiddleRect\
	)(obj)

#endif

#endif

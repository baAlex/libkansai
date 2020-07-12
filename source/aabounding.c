/*-----------------------------

MIT License

Copyright (c) 2019 Alexander Brandt

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

-------------------------------

 [aabounding.c]
 - Alexander Brandt 2019-2020
-----------------------------*/

#include "kansai-aabounding.h"
#include <math.h>


inline bool kaAABCollisionRectRect(struct kaAABRectangle a, struct kaAABRectangle b)
{
	if (a.max.x < b.min.x || a.min.x > b.max.x || a.max.y < b.min.y || a.min.y > b.max.y)
		return false;

	return true;
}


inline bool kaAABCollisionRectCircle(struct kaAABRectangle rect, struct kaCircle c)
{

	if (c.origin.x < (rect.min.x - c.radius) || c.origin.x > (rect.max.x + c.radius) ||
	    c.origin.y < (rect.min.y - c.radius) || c.origin.y > (rect.max.y + c.radius))
		return false;

	return true;
}


inline bool kaAABCollisionBoxBox(struct kaAABBox a, struct kaAABBox b)
{
	if (a.max.x < b.min.x || a.min.x > b.max.x || a.max.y < b.min.y || a.min.y > b.max.y || a.max.z < b.min.z ||
	    a.min.z > b.max.z)
		return false;

	return true;
}


inline bool kaAABCollisionBoxSphere(struct kaAABBox box, struct kaSphere s)
{
	if (s.origin.x < (box.min.x - s.radius) || s.origin.x > (box.max.x + s.radius) ||
	    s.origin.y < (box.min.y - s.radius) || s.origin.y > (box.max.y + s.radius) ||
	    s.origin.z < (box.min.z - s.radius) || s.origin.z > (box.max.z + s.radius))
		return false;

	return true;
}


inline struct kaAABRectangle kaAABToRectangle(struct kaAABBox box)
{
	return (struct kaAABRectangle){.min = {box.min.x, box.min.y}, .max = {box.max.x, box.max.y}};
}


inline struct kaAABBox kaAABToBox(struct kaAABRectangle rect, float min_z, float max_z)
{
	return (struct kaAABBox){.min = {rect.min.x, rect.min.y, min_z}, .max = {rect.max.x, rect.max.y, max_z}};
}


inline struct jaVector2 kaAABMiddleRect(struct kaAABRectangle rect)
{
	return (struct jaVector2){.x = (rect.min.x + (rect.max.x - rect.min.x) / 2.0f),
	                          .y = (rect.min.y + (rect.max.y - rect.min.y) / 2.0f)};
}


inline struct jaVector3 kaAABMiddleBox(struct kaAABBox box)
{
	return (struct jaVector3){.x = (box.min.x + (box.max.x - box.min.x) / 2.0f),
	                          .y = (box.min.y + (box.max.y - box.min.y) / 2.0f),
	                          .z = (box.min.z + (box.max.z - box.min.z) / 2.0f)};
}

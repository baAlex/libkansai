/*-----------------------------

MIT License

Copyright (c) 2020 Alexander Brandt

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

 [utilities.c]
 - Alexander Brandt 2020
-----------------------------*/

#include <math.h>
#include <stddef.h>

#include "kansai-utilities.h"


void kaVectorAxes(struct jaVectorF3 angle, struct jaVectorF3* forward, struct jaVectorF3* left, struct jaVectorF3* up)
{
	// http://www.songho.ca/opengl/gl_anglestoaxes.html#anglestoaxes
	// Saving all differences on axis disposition

	// x = Pith
	// y = Roll
	// z = Yaw

	float cx = cosf(jaDegToRad(angle.x));
	float sx = sinf(jaDegToRad(angle.x));
	float cy = cosf(jaDegToRad(angle.y)); // TODO: broken... maybe
	float sy = sinf(jaDegToRad(angle.y)); // "
	float cz = cosf(jaDegToRad(angle.z));
	float sz = sinf(jaDegToRad(angle.z));

	if (forward != NULL)
	{
		forward->x = sz * -sx;
		forward->y = cz * -sx;
		forward->z = -cx;
	}

	if (left != NULL)
	{
		left->x = (sz * sy * sx) + (cy * cz);
		left->y = (cz * sy * sx) + (cy * -sz);
		left->z = sx * sy;
	}

	if (up != NULL)
	{
		up->x = (sz * cy * cx) + -(sy * cz);
		up->y = (cz * cy * cx) + -(sy * -sz);
		up->z = -sx * cy;
	}
}

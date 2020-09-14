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

 [color.c]
 - Alexander Brandt 2020
-----------------------------*/

#include "kansai-color.h"
#include <math.h>


struct kaRgb kaRgbFromHsv(float h, float s, float v)
{
	// https://en.wikipedia.org/wiki/HSL_and_HSV#HSV_to_RGB

	float c = v * s;
	float h1 = (h * 360.0f) / 60.0f;
	float x = c * (1.0f * fabsf(fmodf(h1, 2.0f) - 1.0f));

	struct kaRgb ret;

	if (h1 > 0.0f && h1 < 1.0f)
		ret = (struct kaRgb){c, x, 0.0f};
	else if (h1 > 1.0f && h1 < 2.0f)
		ret = (struct kaRgb){x, c, 0.0f};
	else if (h1 > 2.0f && h1 < 3.0f)
		ret = (struct kaRgb){0.0f, c, x};
	else if (h1 > 3.0f && h1 < 4.0f)
		ret = (struct kaRgb){0.0f, x, c};
	else if (h1 > 4.0f && h1 < 5.0f)
		ret = (struct kaRgb){x, 0.0f, c};
	else if (h1 > 5.0f && h1 < 6.0f)
		ret = (struct kaRgb){c, 0.0f, x};
	else
		ret = (struct kaRgb){0.0f, 0.0f, 0.0f};

	ret.r += (v - c);
	ret.g += (v - c);
	ret.b += (v - c);

	return ret;
}


inline struct kaRgba kaRgbaFromHsv(float h, float s, float v, float a)
{
	struct kaRgb ret = kaRgbFromHsv(h, s, v);
	return (struct kaRgba){ret.r, ret.g, ret.b, a};
}


inline struct kaRgb kaRgbRandom(struct KA_RANDOM_STATE* state)
{
	// TODO, is UINT64_MAX representable with a float?
	return kaRgbFromHsv((float)kaRandom(state) / (float)UINT64_MAX, (float)kaRandom(state) / (float)UINT64_MAX,
	                    (float)kaRandom(state) / (float)UINT64_MAX);
}


inline struct kaRgba kaRgbaRandom(struct KA_RANDOM_STATE* state, float a)
{
	struct kaRgb ret = kaRgbRandom(state);
	return (struct kaRgba){ret.r, ret.g, ret.b, a};
}

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

 [random.c]
 - Alexander Brandt 2020
-----------------------------*/

#include "kansai-random.h"


inline uint64_t kaRandomXorshift(struct kaXorshift* state)
{
	// https://en.wikipedia.org/wiki/Xorshift#xorshift*

	uint64_t x = state->a;
	x ^= x >> 12; // a
	x ^= x << 25; // b
	x ^= x >> 27; // c
	state->a = x;
	return x * (uint64_t)(0x2545F4914F6CDD1D);
}

inline void kaSeedXorshift(struct kaXorshift* state, uint64_t seed)
{
	state->a = (seed != 0) ? seed : 1;
}

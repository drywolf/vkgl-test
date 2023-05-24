/*
 * Copyright © 2020 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Author:
 *    Eleni Maria Stea <estea@igalia.com>
 */

#version 430
#extension GL_ARB_separate_shader_objects : enable

#if PIGLIT_TEST_CUSTOMIZATIONS_ENABLED

// This is our custom code that we can experiment with!

const float sz = 0.1;
//const float d = 0.830001;
const float d = 0.66;
const vec3 vdata[] = vec3[] (
		vec3(sz, sz, d), // top-right
		vec3(sz, -sz, d), // bottom-right
		vec3(-sz, sz, d), // top-left
		vec3(-sz, -sz, d+0.0000005)); // bottom-left
void main()
{
	gl_Position = vec4(vdata[gl_VertexIndex], 1.0);
}

#else // PIGLIT_TEST_CUSTOMIZATIONS_ENABLED

// The original unmodified piglit shader code
// !!! DO NOT CHANGE THIS !!!

const vec2 vdata[] = vec2[] (
		vec2(0.5, 0.5),
		vec2(0.5, -0.5),
		vec2(-0.5, 0.5),
		vec2(-0.5, -0.5));
void main()
{
	gl_Position = vec4(vdata[gl_VertexIndex], 0.33, 1.0);
}

#endif // PIGLIT_TEST_CUSTOMIZATIONS_ENABLED

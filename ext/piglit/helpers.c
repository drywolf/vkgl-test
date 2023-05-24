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
 *    Tapani Pälli <tapani.palli@intel.com>
 */

#include <glad/glad.h>

#include "helpers.h"

char*
load_shader(const char *shader_file,
	    unsigned int *size)
{
	static char filepath[4096];
	if (!shader_file)
		return NULL;

	//piglit_join_paths(filepath, sizeof(filepath), 5,
	//		  piglit_source_dir(),
	//		  "tests",
	//		  "spec",
	//		  "ext_external_objects",
	//		  shader_file);

	strcpy(filepath, shader_file);

	// IMPORTANT: read file as binary !!! reading of binary files in ASCII mode will lead to corrupted SPV VK shader modules on Windows !!!
	char *result =
		piglit_load_text_file(filepath, size, "rb");

	if (!result)
		fprintf(stderr, "Failed to load shader source [%s].\n", filepath);

	return result;
}

bool
check_bound_fbo_status(void)
{
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		switch(status) {
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			fprintf(stderr, "GL FBO status: GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT\n");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
			fprintf(stderr, "GL FBO status: GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS\n");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			fprintf(stderr, "GL FBO status: GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT\n");
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			fprintf(stderr, "GL FBO status: GL_FRAMEBUFFER_UNSUPPORTED\n");
			break;
		default:
			fprintf(stderr, "GL FBO status: Unknown\n");
		}
		return false;
	}
	return true;
}

bool
dump_image_rgba(const char *fname,
		int w, int h,
		unsigned char *data)
{
	FILE *fp;
	int i;

	if (!(fp = fopen(fname, "wb"))) {
		fprintf(stderr, "Failed to open file: %s.\n", fname);
		return false;
	}

	fprintf(fp, "P3\n%d %d\n255\n", w, h);
	for (i = 0; i < w * h; i++) {
		fprintf(fp, "%d %d %d\n", data[0], data[1], data[2]);
		data += 4;
	}
	fclose(fp);
	return true;
}

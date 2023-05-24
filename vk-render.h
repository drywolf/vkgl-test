#pragma once

#include <glad/glad.h>

#include <stdint.h>
#include <iostream>

#define VK_CHECK(x)                                                 \
	do                                                              \
	{                                                               \
		VkResult err = x;                                           \
		if (err)                                                    \
		{                                                           \
			std::cerr << __FILE__ << ": " << __LINE__ << std::endl; \
			std::cerr << "VK_CHECK ERROR: " << err << std::endl;	\
			throw new std::runtime_error("VK ERROR");               \
		}                                                           \
	} while (0)

#define GL_CHECK(x)                                                 \
	do                                                              \
	{                                                               \
		x;                                                          \
		GLenum err = glGetError();                                  \
		if (err)                                                    \
		{                                                           \
			std::cerr << __FILE__ << ": " << __LINE__ << std::endl; \
			std::cerr << "GL_CHECK ERROR: " << err << std::endl;	\
			throw new std::runtime_error("GL ERROR");               \
		}                                                           \
	} while (0)

bool vk_init(uint32_t w, uint32_t h, uint32_t num_samples, GLuint* OUT_gl_color_tex_id, GLuint* OUT_gl_depth_tex_id);
void vk_render();
void vk_shutdown();

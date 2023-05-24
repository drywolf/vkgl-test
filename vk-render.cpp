#include "vk-render.h"

#include <ext/piglit/vk.h>
#include <ext/piglit/piglit-util.h>
#include <ext/piglit/helpers.h>

#include "vk_gl_interop_helpers.h"
#include "interop.h"

static struct vk_ctx vk_core;
static struct vk_image_att vk_color_att;
static struct vk_image_att vk_depth_att;
static struct vk_renderer vk_rnd;

static char* vs_src;
static char* fs_src;
static unsigned int vs_sz;
static unsigned int fs_sz;

uint32_t d = 1;
uint32_t num_levels = 1;
uint32_t num_layers = 1;

VkImageTiling color_tiling = VK_IMAGE_TILING_OPTIMAL;
VkImageTiling depth_tiling = VK_IMAGE_TILING_OPTIMAL;
VkImageLayout color_in_layout = VK_IMAGE_LAYOUT_UNDEFINED;
VkImageLayout color_end_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
VkImageLayout depth_in_layout = VK_IMAGE_LAYOUT_UNDEFINED;
VkImageLayout depth_end_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

const struct vkgl_format color_format = { "RGBA8", GL_RGBA8, VK_FORMAT_R8G8B8A8_UNORM };
const struct vkgl_format depth_format = { "D32S8", GL_DEPTH32F_STENCIL8, VK_FORMAT_D32_SFLOAT_S8_UINT };

// INTEROP
static GLuint gl_color_mem_obj;
static GLuint gl_color_tex;

static GLuint gl_depth_mem_obj;
static GLuint gl_depth_tex;

bool vk_init(uint32_t w, uint32_t h, uint32_t num_samples)
{
    if (!vk_init_ctx_for_rendering(&vk_core, true)) {
        fprintf(stderr, "Failed to create Vulkan context.\n");
        return false;
    }

    if (!vk_check_gl_compatibility(&vk_core)) {
        fprintf(stderr, "Mismatch in driver/device UUID\n");
        return false;
    }

    if (!(vs_src = load_shader("vk_shader.vert.spv", &vs_sz))) {
        fprintf(stderr, "Failed to load VS source.\n");
        return false;
    }

    if (!(fs_src = load_shader("vk_shader.frag.spv", &fs_sz))) {
        fprintf(stderr, "Failed to load FS source.\n");
        free(vs_src);
        return false;
    }

    /* Vulkan interop extensions init */
    if (!vk_load_interop_functions(vk_core.dev)) {
        fprintf(stderr, "Failed to initialize Vulkan-GL interop extension functions.\n");
        return false;
    }

    if (!vk_fill_ext_image_props(&vk_core,
        w, h, d,
        num_samples,
        num_levels,
        num_layers,
        color_format.vk_fmt,
        color_tiling,
        color_in_layout,
        color_end_layout,
        true,
        &vk_color_att.props)) {
        fprintf(stderr, "Unsupported color image properties.\n");
        return false;
    }
    if (!vk_create_ext_image(&vk_core, &vk_color_att.props, &vk_color_att.obj)) {
        fprintf(stderr, "Failed to create color image.\n");
        return false;
    }

    if (!vk_fill_ext_image_props(&vk_core,
        w, h, d,
        num_samples,
        num_levels,
        num_layers,
        depth_format.vk_fmt,
        depth_tiling,
        depth_in_layout,
        depth_end_layout,
        true,
        &vk_depth_att.props)) {
        fprintf(stderr, "Unsupported depth image properties.\n");
        return false;
    }

    if (!vk_create_ext_image(&vk_core, &vk_depth_att.props, &vk_depth_att.obj)) {
        fprintf(stderr, "Failed to create depth image.\n");
        return false;
    }

    if (!vk_create_renderer(&vk_core, vs_src, vs_sz, fs_src, fs_sz,
        true, false,
        &vk_color_att, &vk_depth_att, 0, &vk_rnd)) {
        fprintf(stderr, "Failed to create Vulkan renderer.\n");
        return false;
    }

    /* interoperability */
    // COLOR
    if (!gl_create_mem_obj_from_vk_mem(&vk_core, &vk_color_att.obj.mobj,
        &gl_color_mem_obj)) {
        fprintf(stderr, "Failed to create GL memory object from Vulkan memory. (COLOR)\n");
        return false;
    }

    if (!gl_gen_tex_from_mem_obj(&vk_color_att.props,
        color_format.gl_fmt,
        gl_color_mem_obj, 0, &gl_color_tex)) {
        fprintf(stderr, "Failed to create GL texture from Vulkan memory object. (COLOR)\n");
        return false;
    }

    // DEPTH-STENCIL
    if (!gl_create_mem_obj_from_vk_mem(&vk_core, &vk_depth_att.obj.mobj,
        &gl_depth_mem_obj)) {
        fprintf(stderr, "Failed to create GL memory object from Vulkan memory. (DEPTH)\n");
        return false;
    }

    if (!gl_gen_tex_from_mem_obj(&vk_depth_att.props,
        depth_format.gl_fmt,
        gl_depth_mem_obj, 0, &gl_depth_tex)) {
        fprintf(stderr, "Failed to create GL texture from Vulkan memory object. (DEPTH)\n");
        return false;
    }

    return true;
}


void vk_shutdown()
{
    vk_destroy_ext_image(&vk_core, &vk_color_att.obj);
    vk_destroy_ext_image(&vk_core, &vk_depth_att.obj);

    vk_destroy_renderer(&vk_core, &vk_rnd);

    free(vs_src);
    free(fs_src);

    vk_cleanup_ctx(&vk_core);
}
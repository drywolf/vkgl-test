#if WIN32
#define NOMINMAX
#endif

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
VkImageLayout color_in_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
VkImageLayout color_end_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
VkImageLayout depth_in_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
VkImageLayout depth_end_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

const struct vkgl_format color_format = { "RGBA8", GL_RGBA8, VK_FORMAT_R8G8B8A8_UNORM };
const struct vkgl_format depth_format = { "D32S8", GL_DEPTH32F_STENCIL8, VK_FORMAT_D32_SFLOAT_S8_UINT };
//const struct vkgl_format depth_format = { "D24S8", GL_DEPTH24_STENCIL8, VK_FORMAT_D24_UNORM_S8_UINT };

uint32_t w = 0, h = 0;

// INTEROP TEXTURES
static GLuint gl_color_mem_obj = 0;
static GLuint gl_color_tex = 0;

static GLuint gl_depth_mem_obj = 0;
static GLuint gl_depth_tex = 0;

// INTEROP SEMAPHORES
static struct gl_ext_semaphores gl_sem;
static struct vk_semaphores vk_sem;
static bool vk_sem_has_wait = true;
static bool vk_sem_has_signal = true;

VkSampleCountFlags vk_max_supported_msaa_samples(VkPhysicalDevice pdev)
{
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(pdev, &physicalDeviceProperties);

    VkSampleCountFlags counts =
        physicalDeviceProperties.limits.framebufferColorSampleCounts
        & physicalDeviceProperties.limits.framebufferDepthSampleCounts
        & physicalDeviceProperties.limits.framebufferStencilSampleCounts;

    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

    return VK_SAMPLE_COUNT_1_BIT;
}

bool vk_init(uint32_t width, uint32_t height, int& msaa_samples, bool enable_validation, GLuint* OUT_gl_color_tex_id, GLuint* OUT_gl_depth_tex_id)
{
    *OUT_gl_color_tex_id = 0;
    *OUT_gl_depth_tex_id = 0;

    w = width;
    h = height;

    if (!vk_init_ctx_for_rendering(&vk_core, enable_validation)) {
        fprintf(stderr, "Failed to create Vulkan context.\n");
        return false;
    }

    std::cout << "requested MSAA sample-count: " << msaa_samples << std::endl;

    VkSampleCountFlags vk_max_msaa_samples = vk_max_supported_msaa_samples(vk_core.pdev);
    std::cout << "VK max-msaa-samples: " << vk_max_msaa_samples << std::endl;

    GLint gl_max_msaa_samples = 0;
    glGetIntegerv(GL_MAX_SAMPLES, &gl_max_msaa_samples);
    std::cout << "GL max-msaa-samples: " << gl_max_msaa_samples << std::endl;

    const int requested_msaa_samples = msaa_samples;
    msaa_samples = std::min(std::min(msaa_samples, (int)vk_max_msaa_samples), gl_max_msaa_samples);

    if (msaa_samples != requested_msaa_samples)
    {
        std::cout << "WARNING: MSAA sample-count has been reduced to " << msaa_samples << " samples (because of GPU limits)" << std::endl;
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
        msaa_samples,
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
        msaa_samples,
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

    // INTEROP TEXTURES
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

    if (!gl_color_tex || !gl_depth_tex)
    {
        fprintf(stderr, "Uninitialized GL color or depth texture-id\n");
        return false;
    }

    // INTEROP SEMAPHORES
    if (!vk_create_semaphores(&vk_core, &vk_sem)) {
        fprintf(stderr, "Failed to create semaphores.\n");
        return false;
    }

    if (!gl_create_semaphores_from_vk(&vk_core, &vk_sem, &gl_sem)) {
        fprintf(stderr, "Failed to import semaphores from Vulkan.\n");
        return false;
    }

    std::cout << "VK INIT DONE" << std::endl;

    *OUT_gl_color_tex_id = gl_color_tex;
    *OUT_gl_depth_tex_id = gl_depth_tex;

    return true;
}

void vk_clear_fbo()
{
    GLuint in_layouts[] = {
        gl_get_layout_from_vk(color_in_layout),
        gl_get_layout_from_vk(depth_in_layout),
    };

    GLuint interop_textures[] = {
        gl_color_tex,
        gl_depth_tex,
    };

    if (vk_sem_has_wait) {
        glSignalSemaphoreEXT(gl_sem.gl_frame_ready, 0, 0, 1,
            interop_textures, in_layouts);
        glFlush();
    }

    struct vk_image_att images[] = { vk_color_att, vk_depth_att };
    static float vk_fb_color[4] = { 0.0, 1.0, 0.0, 1.0 };

    vk_clear_color(&vk_core, 0, &vk_rnd, vk_fb_color, 4, &vk_sem,
        vk_sem_has_wait, vk_sem_has_signal, images,
        ARRAY_SIZE(images), 0, 0, w, h);

    GLuint end_layouts[] = {
        gl_get_layout_from_vk(color_end_layout),
        gl_get_layout_from_vk(depth_end_layout),
    };

    if (vk_sem_has_signal) {
        glWaitSemaphoreEXT(gl_sem.vk_frame_done, 0, 0, 1,
            interop_textures, end_layouts);
        glFlush();
    }
}

void vk_draw_cube(const glm::mat4& mvp_matrix)
{
    GLuint in_layouts[] = {
        gl_get_layout_from_vk(color_in_layout),
        gl_get_layout_from_vk(depth_in_layout),
    };

    GLuint interop_textures[] = {
        gl_color_tex,
        gl_depth_tex,
    };

    if (vk_sem_has_wait) {
        glSignalSemaphoreEXT(gl_sem.gl_frame_ready, 0, 0, 1,
            interop_textures, in_layouts);
        glFlush();
    }

    struct vk_image_att images[] = { vk_color_att, vk_depth_att };
    static float vk_fb_color[4] = { 0.0, 1.0, 0.0, 1.0 };

    struct vk_push_constants pc;
    memcpy(&pc.mvp_matrix, &mvp_matrix, sizeof(glm::mat4));

    vk_draw(&vk_core, 0, &vk_rnd, vk_fb_color, 4, &vk_sem,
        vk_sem_has_wait, vk_sem_has_signal, images, ARRAY_SIZE(images), &pc, 0, 0, w, h);

    GLuint end_layouts[] = {
        gl_get_layout_from_vk(color_end_layout),
        gl_get_layout_from_vk(depth_end_layout),
    };

    if (vk_sem_has_signal) {
        glWaitSemaphoreEXT(gl_sem.vk_frame_done, 0, 0, 1,
            interop_textures, end_layouts);
        glFlush();
    }
}

void vk_shutdown()
{
    vk_destroy_ext_image(&vk_core, &vk_color_att.obj);
    vk_destroy_ext_image(&vk_core, &vk_depth_att.obj);

    vk_destroy_semaphores(&vk_core, &vk_sem);

    vk_destroy_renderer(&vk_core, &vk_rnd);

    free(vs_src);
    free(fs_src);

    vk_cleanup_ctx(&vk_core);
}
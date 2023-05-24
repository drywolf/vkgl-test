#include "vk-render.h"

#include <ext/piglit/vk.h>
#include <ext/piglit/piglit-util.h>
#include <ext/piglit/helpers.h>

#include "vk_gl_interop_helpers.h"

static struct vk_ctx vk_core;

static char* vs_src;
static char* fs_src;
static unsigned int vs_sz;
static unsigned int fs_sz;

bool vk_init()
{
    //piglit_require_extension("GL_EXT_memory_object");
    //piglit_require_extension(GL_EXT_memory_object_PLATFORM_EXT_NAME);
    //piglit_require_extension("GL_EXT_semaphore");
    //piglit_require_extension(GL_EXT_semaphore_PLATFORM_EXT_NAME);

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

    return true;
}

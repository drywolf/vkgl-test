#pragma once

struct VkGlAppOptions
{
    const uint32_t width = 800;
    const uint32_t height = 600;

    const bool enable_msaa = true;

    // NOTE: only enable this when running on systems where you have a Vulkan SDK installed !!!
    const bool ENABLE_VULKAN_VALIDATION_LAYER = false;
};

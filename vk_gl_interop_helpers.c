#include "vk_gl_interop_helpers.h"

#include <vulkan/vulkan.h>

#include <stdio.h>

#if WIN32
    PFN_vkGetMemoryWin32HandleKHR _vkGetMemoryInteropHandle = 0;
    PFN_vkGetSemaphoreWin32HandleKHR _vkGetSemaphoreInteropHandle = 0;
#else
    PFN_vkGetMemoryFdKHR _vkGetMemoryInteropHandle = 0;
    PFN_vkGetSemaphoreFdKHR _vkGetSemaphoreInteropHandle = 0;
#endif

bool vk_load_interop_functions(VkDevice vk_device)
{
#if WIN32
    _vkGetMemoryInteropHandle =
        (PFN_vkGetMemoryWin32HandleKHR)vkGetDeviceProcAddr(vk_device, "vkGetMemoryWin32HandleKHR");

    _vkGetSemaphoreInteropHandle =
        (PFN_vkGetSemaphoreWin32HandleKHR)vkGetDeviceProcAddr(vk_device, "vkGetSemaphoreWin32HandleKHR");
#else
    _vkGetMemoryInteropHandle =
        (PFN_vkGetMemoryFdKHR)vkGetDeviceProcAddr(vk_device, "vkGetMemoryFdKHR");

    _vkGetSemaphoreInteropHandle =
        (PFN_vkGetSemaphoreFdKHR)vkGetDeviceProcAddr(vk_device, "vkGetSemaphoreFdKHR");
#endif

    if (!_vkGetMemoryInteropHandle) {
        fprintf(stderr, "_vkGetMemoryInteropHandle not found\n");
        return false;
    }

    if (!_vkGetSemaphoreInteropHandle) {
        fprintf(stderr, "_vkGetSemaphoreInteropHandle not found\n");
        return false;
    }

    return true;
}

VkResult vkGetMemoryInteropHandle(VkDevice device, const VkMemoryGetInteropHandleInfo* pGetInteropHandleInfo, VkInteropHandle* pHandle)
{
    return _vkGetMemoryInteropHandle(device, pGetInteropHandleInfo, pHandle);
}

VkResult vkGetSemaphoreInteropHandle(VkDevice device, const VkSemaphoreGetInteropHandleInfo* pGetInteropHandleInfo, VkInteropHandle* pHandle)
{
    return _vkGetSemaphoreInteropHandle(device, pGetInteropHandleInfo, pHandle);
}

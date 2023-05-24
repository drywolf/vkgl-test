#pragma once

#include <stdbool.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OpenGL Interop API
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <glad/glad.h>

#ifdef WIN32
#   define GL_EXT_memory_object_PLATFORM_EXT_NAME "GL_EXT_memory_object_win32"
#   define GL_EXT_semaphore_PLATFORM_EXT_NAME "GL_EXT_semaphore_win32"
#   define glImportSemaphore glImportSemaphoreWin32HandleEXT
#   define glImportMemory glImportMemoryWin32HandleEXT
#   define GL_INTEROP_HANDLE_TYPE GL_HANDLE_TYPE_OPAQUE_WIN32_EXT
#else
#   define GL_EXT_memory_object_PLATFORM_EXT_NAME "GL_EXT_memory_object_fd"
#   define GL_EXT_semaphore_PLATFORM_EXT_NAME "GL_EXT_semaphore_fd"
#   define glImportSemaphore glImportSemaphoreFdEXT
#   define glImportMemory glImportMemoryFdEXT
#   define GL_INTEROP_HANDLE_TYPE GL_HANDLE_TYPE_OPAQUE_FD_EXT
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Vulkan Interop API
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <vulkan/vulkan.h>

// Interop-Handle type-alias
#ifdef WIN32
#   define VkInteropHandle HANDLE
#else
#   define VkInteropHandle int
static const VkInteropHandle INVALID_HANDLE_VALUE = -1;
#endif

// Interop-Extension name-strings
#ifdef WIN32
#   define VK_KHR_EXTERNAL_MEMORY_SYSTEM_HANDLE_EXTENSION_NAME VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME
#   define VK_KHR_EXTERNAL_SEMAPHORE_SYSTEM_HANDLE_EXTENSION_NAME VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME
#else
#   define VK_KHR_EXTERNAL_MEMORY_SYSTEM_HANDLE_EXTENSION_NAME VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME
#   define VK_KHR_EXTERNAL_SEMAPHORE_SYSTEM_HANDLE_EXTENSION_NAME VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME
#endif

// Interop-Handle bit-flags
#ifdef WIN32
#   define VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_BIT VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT
#   define VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_BIT VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT
#else
#   define VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_BIT VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT_KHR
#   define VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_BIT VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR
#endif

// Structure type-enums
#ifdef WIN32
#	  define VK_STRUCTURE_TYPE_MEMORY_GET_INTEROP_HANDLE_INFO VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR
#   define VK_STRUCTURE_TYPE_SEMAPHORE_GET_INTEROP_HANDLE_INFO VK_STRUCTURE_TYPE_SEMAPHORE_GET_WIN32_HANDLE_INFO_KHR
#else
#	  define VK_STRUCTURE_TYPE_MEMORY_GET_INTEROP_HANDLE_INFO VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR
#   define VK_STRUCTURE_TYPE_SEMAPHORE_GET_INTEROP_HANDLE_INFO VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR
#endif

// Interop-API Parameter-Structs
#if WIN32
#   define VkMemoryGetInteropHandleInfo VkMemoryGetWin32HandleInfoKHR
#   define VkSemaphoreGetInteropHandleInfo VkSemaphoreGetWin32HandleInfoKHR
#else
#   define VkMemoryGetInteropHandleInfo VkMemoryGetFdInfoKHR
#   define VkSemaphoreGetInteropHandleInfo VkSemaphoreGetFdInfoKHR
#endif

#if __cplusplus
extern "C" {
#endif

// initialize the Interop API extension functions
bool vk_load_interop_functions(VkDevice vk_device);

VkResult vkGetMemoryInteropHandle(VkDevice device, const VkMemoryGetInteropHandleInfo* pGetInteropHandleInfo, VkInteropHandle* pHandle);
VkResult vkGetSemaphoreInteropHandle(VkDevice device, const VkSemaphoreGetInteropHandleInfo* pGetInteropHandleInfo, VkInteropHandle* pHandle);

#if __cplusplus
} // extern "C"
#endif

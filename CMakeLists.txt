cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

project(amd-vkgl)

# GLOBAL PROPERTIES
set(CMAKE_CXX_STANDARD 17)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# EXTERNAL DEPENDENCIES
find_package(glfw3 CONFIG REQUIRED) # via vcpkg
find_package(glm CONFIG REQUIRED) # via vcpkg
find_package(Stb REQUIRED) # via vcpkg
find_package(Vulkan REQUIRED)

add_subdirectory(ext)

# VULKAN SHADERS
find_program(GLSLANG_VALIDATOR NAMES glslangValidator)

if (NOT GLSLANG_VALIDATOR)
    message(FATAL_ERROR "required program 'glslangValidator' could not be found")
endif()

message(STATUS "GLSLANG_VALIDATOR: ${GLSLANG_VALIDATOR}")

set(VK_SHADER_VERT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/vk_shader.vert)
set(VK_SHADER_VERT_OUT ${CMAKE_CURRENT_SOURCE_DIR}/vk_shader.vert.spv)

add_custom_command(
    COMMAND ${GLSLANG_VALIDATOR} -V ${VK_SHADER_VERT_SRC} -o ${VK_SHADER_VERT_OUT}
    DEPENDS ${VK_SHADER_VERT_SRC}
    OUTPUT ${VK_SHADER_VERT_OUT}
)

set(VK_SHADER_FRAG_SRC ${CMAKE_CURRENT_SOURCE_DIR}/vk_shader.frag)
set(VK_SHADER_FRAG_OUT ${CMAKE_CURRENT_SOURCE_DIR}/vk_shader.frag.spv)

add_custom_command(
    COMMAND ${GLSLANG_VALIDATOR} -V ${VK_SHADER_FRAG_SRC} -o ${VK_SHADER_FRAG_OUT}
    DEPENDS ${VK_SHADER_FRAG_SRC}
    OUTPUT ${VK_SHADER_FRAG_OUT}
)

# amd-vkgl EXECUTABLE
add_executable(amd-vkgl
    amd-vkgl.cpp
    ext/piglit/helpers.c
    ext/piglit/helpers.h
    ext/piglit/interop.c
    ext/piglit/interop.h
    ext/piglit/piglit-log.c
    ext/piglit/piglit-log.h
    ext/piglit/piglit-util.c
    ext/piglit/piglit-util.h
    ext/piglit/sized-internalformats.c
    ext/piglit/sized-internalformats.h
    ext/piglit/vk.c
    ext/piglit/vk.h
    vk_gl_interop_helpers.c
    vk_gl_interop_helpers.h
    vk-render.cpp
    vk-render.h
    ${VK_SHADER_VERT_SRC}
    ${VK_SHADER_VERT_OUT}
    ${VK_SHADER_FRAG_SRC}
    ${VK_SHADER_FRAG_OUT}
)

set_target_properties(amd-vkgl PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:amd-vkgl>"
)

target_link_libraries(amd-vkgl
PUBLIC
    learnopengl-framework
    glad::glad
    glm::glm
    glfw
    Vulkan::Vulkan
)

target_include_directories(amd-vkgl
PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/ext/piglit"
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT amd-vkgl)

# https://github.com/KhronosGroup/Vulkan-Samples/blob/f6dd68c1fa053bff3f4a850062968070d1388b08/third_party/CMakeLists.txt#L53-L86
if(ANDROID)
    target_compile_definitions(amd-vkgl PUBLIC VK_USE_PLATFORM_ANDROID_KHR)
elseif(WIN32)
    target_compile_definitions(amd-vkgl PUBLIC VK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
    target_compile_definitions(amd-vkgl PUBLIC VK_USE_PLATFORM_METAL_EXT)
elseif(UNIX)
    # TODO: this could be a more sophisticated selection of the used xlib
    target_compile_definitions(amd-vkgl PUBLIC VK_USE_PLATFORM_XCB_KHR)
    target_compile_definitions(amd-vkgl PUBLIC VK_USE_PLATFORM_XLIB_KHR)
endif()

# COPY-RESOURCES
add_custom_target(copy-resources
                  COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:amd-vkgl>/"
                  # OpenGL shaders
                  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/5.1.framebuffers.vs" "$<TARGET_FILE_DIR:amd-vkgl>/"
                  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/5.1.framebuffers.fs" "$<TARGET_FILE_DIR:amd-vkgl>/"
                  # Vulkan shaders
                  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${VK_SHADER_VERT_OUT}" "$<TARGET_FILE_DIR:amd-vkgl>/"
                  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${VK_SHADER_FRAG_OUT}" "$<TARGET_FILE_DIR:amd-vkgl>/"
                  # OpenGL Textures
                  COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:amd-vkgl>/resources/textures/"
                  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/resources/textures/container.jpg" "$<TARGET_FILE_DIR:amd-vkgl>/resources/textures/"
                  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/resources/textures/metal.png" "$<TARGET_FILE_DIR:amd-vkgl>/resources/textures/"
                  COMMENT "Copying shaders & resources..."
                  SOURCES
                    "${CMAKE_CURRENT_SOURCE_DIR}/5.1.framebuffers.vs"
                    "${CMAKE_CURRENT_SOURCE_DIR}/5.1.framebuffers.fs"
                    "${VK_SHADER_VERT_OUT}"
                    "${VK_SHADER_FRAG_OUT}"
)

add_dependencies(amd-vkgl copy-resources)

set_target_properties(copy-resources PROPERTIES FOLDER res)

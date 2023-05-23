cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

project(amd-vkgl)

set(CMAKE_CXX_STANDARD 17)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_package(glfw3 CONFIG REQUIRED) # via vcpkg
find_package(glm CONFIG REQUIRED) # via vcpkg
find_package(Stb REQUIRED) # via vcpkg
find_package(Vulkan REQUIRED)

add_subdirectory(ext)

add_executable(amd-vkgl amd-vkgl.cpp)

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
)

add_custom_target(copy-resources
                  COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:amd-vkgl>/"
                  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/5.1.framebuffers.vs" "$<TARGET_FILE_DIR:amd-vkgl>/"
                  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/5.1.framebuffers.fs" "$<TARGET_FILE_DIR:amd-vkgl>/"
                  COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:amd-vkgl>/resources/textures/"
                  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/resources/textures/container.jpg" "$<TARGET_FILE_DIR:amd-vkgl>/resources/textures/"
                  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/resources/textures/metal.png" "$<TARGET_FILE_DIR:amd-vkgl>/resources/textures/"
                  COMMENT "Copying resources..."
                  SOURCES
                    "${CMAKE_CURRENT_SOURCE_DIR}/5.1.framebuffers.vs"
                    "${CMAKE_CURRENT_SOURCE_DIR}/5.1.framebuffers.fs"
)

add_dependencies(amd-vkgl copy-resources)

set_target_properties(copy-resources PROPERTIES FOLDER res)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT amd-vkgl)

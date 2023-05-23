
# LEARN-OPENGL
file(GLOB learnopengl_source_files "${CMAKE_CURRENT_SOURCE_DIR}/learnopengl/*.h")

if(NOT learnopengl_source_files)
    message(FATAL_ERROR "Missing learnopengl files")
endif()

add_library(learnopengl-framework INTERFACE
    ${learnopengl_source_files}
)

target_include_directories(learnopengl-framework
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}"
)

set_target_properties(learnopengl-framework PROPERTIES FOLDER ext)

# GLAD
add_library(glad STATIC
    glad/glad.c
    glad/glad/glad.h
    glad/KHR/khrplatform.h
)
add_library(glad::glad ALIAS glad)

target_include_directories(glad PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/glad")

set_target_properties(glad PROPERTIES FOLDER ext)

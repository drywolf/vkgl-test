#version 430
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform _pc {
    mat4 mvp_matrix;
} pc;

// cube vertex positions
const vec3 pos[] = vec3[] (
    vec3(-0.5, -0.5, -0.5),
    vec3( 0.5, -0.5, -0.5),
    vec3( 0.5,  0.5, -0.5),
    vec3( 0.5,  0.5, -0.5),
    vec3(-0.5,  0.5, -0.5),
    vec3(-0.5, -0.5, -0.5),

    vec3(-0.5, -0.5,  0.5),
    vec3( 0.5, -0.5,  0.5),
    vec3( 0.5,  0.5,  0.5),
    vec3( 0.5,  0.5,  0.5),
    vec3(-0.5,  0.5,  0.5),
    vec3(-0.5, -0.5,  0.5),

    vec3(-0.5,  0.5,  0.5),
    vec3(-0.5,  0.5, -0.5),
    vec3(-0.5, -0.5, -0.5),
    vec3(-0.5, -0.5, -0.5),
    vec3(-0.5, -0.5,  0.5),
    vec3(-0.5,  0.5,  0.5),

    vec3( 0.5,  0.5,  0.5),
    vec3( 0.5,  0.5, -0.5),
    vec3( 0.5, -0.5, -0.5),
    vec3( 0.5, -0.5, -0.5),
    vec3( 0.5, -0.5,  0.5),
    vec3( 0.5,  0.5,  0.5),

    vec3(-0.5, -0.5, -0.5),
    vec3( 0.5, -0.5, -0.5),
    vec3( 0.5, -0.5,  0.5),
    vec3( 0.5, -0.5,  0.5),
    vec3(-0.5, -0.5,  0.5),
    vec3(-0.5, -0.5, -0.5),

    vec3(-0.5,  0.5, -0.5),
    vec3( 0.5,  0.5, -0.5),
    vec3( 0.5,  0.5,  0.5),
    vec3( 0.5,  0.5,  0.5),
    vec3(-0.5,  0.5,  0.5),
    vec3(-0.5,  0.5, -0.5)
);

// cube uv-coordinates
const vec2 uv[] = vec2[] (
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(1.0, 1.0),
    vec2(1.0, 1.0),
    vec2(0.0, 1.0),
    vec2(0.0, 0.0),

    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(1.0, 1.0),
    vec2(1.0, 1.0),
    vec2(0.0, 1.0),
    vec2(0.0, 0.0),

    vec2(1.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 1.0),
    vec2(0.0, 1.0),
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),

    vec2(1.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 1.0),
    vec2(0.0, 1.0),
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),

    vec2(0.0, 1.0),
    vec2(1.0, 1.0),
    vec2(1.0, 0.0),
    vec2(1.0, 0.0),
    vec2(0.0, 0.0),
    vec2(0.0, 1.0),

    vec2(0.0, 1.0),
    vec2(1.0, 1.0),
    vec2(1.0, 0.0),
    vec2(1.0, 0.0),
    vec2(0.0, 0.0),
    vec2(0.0, 1.0)
);

layout(location = 0) out vec2 uv_coord;

void main()
{
	gl_Position = pc.mvp_matrix * vec4(pos[gl_VertexIndex], 1.0);
    uv_coord = uv[gl_VertexIndex];

    // see: https://matthewwellings.com/blog/the-new-vulkan-coordinate-system/
    // NOTE: this is only needed if we don't pre-multiply the correction matrix in the C++ code already
    //gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
}

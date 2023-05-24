#version 430
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform _pc {
	float quad_z;
} pc;

const float sz = 0.3;

const vec2 vdata[] = vec2[] (
		vec2(sz, sz),
		vec2(sz, -sz),
		vec2(-sz, sz),
		vec2(-sz, -sz)
);

void main()
{
	gl_Position = vec4(vdata[gl_VertexIndex], pc.quad_z, 1.0);
}

#version 450

layout(location = 0) in vec2 uv_coord;

layout(location = 0) out vec4 f_color;

void main()
{
	f_color = vec4(uv_coord, 1.0, 1.0);
}

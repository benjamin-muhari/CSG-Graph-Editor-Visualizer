#version 410

in vec2 vs_in_pos;

layout(location = 0) out vec3 vs_out_col;

void main()
{
	vec2 p = vs_in_pos;
	gl_Position = vec4(p, 0, 1);
	vs_out_col = vec3(1, 0, 0);
}

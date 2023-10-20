#version 410

in vec3 vs_in_pos;
in vec3 vs_in_col;

layout(location = 0) out vec3 vs_out_col;

uniform mat4 mvp;

void main()
{
	gl_Position = mvp * vec4(vs_in_pos, 1);
	vs_out_col = vs_in_col;
}

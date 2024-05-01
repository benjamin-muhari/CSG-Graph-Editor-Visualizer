#version 460

layout(location = 0) in vec2 vs_in_pos;

layout(location = 0) out vec2 vs_out_tex;

void main()
{
	gl_Position = vec4(vs_in_pos, 0, 1);

	vs_out_tex = vs_in_pos*0.5+0.5;
}

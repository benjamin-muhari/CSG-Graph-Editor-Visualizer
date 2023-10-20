#version 410

layout(location = 0) in vec3 fs_in_col;
out vec4 fs_out_col;

void main()
{
	fs_out_col = vec4(fs_in_col,1);
}

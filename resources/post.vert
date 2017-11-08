#version 330

in vec3 pos;

void main()
{
	gl_Position = vec4(pos,1);
}

#version 330

flat in vec4 flatColor;

out vec4 fragColor;

void main()
{
	fragColor = flatColor;
}

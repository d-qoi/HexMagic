#version 330

uniform mat4 P;  //projection matrix
uniform mat4 C;  //camera matrix
uniform mat4 mT; //model transform
uniform mat4 mR; //model rotation
uniform mat4 M;  //modelview matrix: M = C * mR * mT
uniform mat3 N;  //inverse transpose of upperleft 3x3 of M
uniform mat4 L;  //light rotation matrix

uniform vec4 lightPos;
uniform vec4 camPos;
uniform int shadingMode;

const int WIDTH = 40;

in vec3 pos;
in vec2 modelId;
in vec2 rectCoord;

flat out vec4 flatColor;

struct RectModel {
	int x;
	int y;
	float zOffset;
	int zLength;
	int highlighted;
	int padding;
	int padding2;
	int padding3;
};

layout (std140) uniform RectBlock {
	RectModel rects[WIDTH*WIDTH];
};

RectModel getModel() {
	return rects[int(rectCoord.y) * WIDTH + int(rectCoord.x)];
}

vec3 offsetPos() {
	return vec3(0, getModel().zOffset, 0);
}

void main()
{
	gl_Position = P * M * vec4(pos + offsetPos(), 1);
	flatColor = vec4(0, 0, modelId.x/255.0, modelId.y/255.0);
}

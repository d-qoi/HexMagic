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

const int WIDTH = 16;

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

in vec3 pos;
in vec3 colorIn;
in vec3 normalIn;
in vec2 rectCoord;

out vec3 smoothPos;
out vec3 smoothNorm;
flat out vec3 flatDiffuseColor;

const vec3 lightIntensity = vec3(1, 1, 1);
const vec3 ka = vec3(0.1, 0.1, 0.1);
const vec3 ks = vec3(1.0, 1.0, 1.0);
const float specAlpha = 10;

RectModel getModel() {
	return rects[int(rectCoord.y) * WIDTH + int(rectCoord.x)];
}

vec3 offsetPos() {
	return vec3(0, getModel().zOffset, 0) * 0.5;
}

void main()
{
	//hack to preserve inputs/output
	vec3 mpos = pos + colorIn * 0 + normalIn * 0 + offsetPos();
	smoothPos = pos;
	smoothNorm = normalIn;

	vec3 color = colorIn;

	if(getModel().highlighted != 0) {
		color = vec3(1,0,0);
	}

	vec4 p = M * vec4(mpos, 1);
	gl_Position = P*p;

	flatDiffuseColor = color;
}

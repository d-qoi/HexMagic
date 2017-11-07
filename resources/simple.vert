#version 410

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

uniform int highlightX;
uniform int highlightY;

in float xCoord;
in float yCoord;
in float zOffset;
in uint zLength;

const int WIDTH = 40;

in vec3 pos;
in vec3 colorIn;
in vec3 normalIn;

out vec3 smoothPos;
out vec3 smoothNorm;
flat out vec3 flatDiffuseColor;
flat out vec3 highlightPos;

const vec3 lightIntensity = vec3(1, 1, 1);
const vec3 ka = vec3(0.1, 0.1, 0.1);
const vec3 ks = vec3(1.0, 1.0, 1.0);
const float specAlpha = 10;

//vec3 offsetPosFor(RectModel model) {
//	return vec3(0, model.zOffset, 0);
//}
//
//vec3 offsetPos() {
//	return offsetPosFor(getModel());
//}

//float getOffsetFromOrig() {
//	RectModel model = getModel();
//	return model.zOffset - float(model.x + model.y);
//}

void main()
{
	vec3 offset = vec3(2*xCoord, zOffset + 0 * xCoord + 0 * yCoord + 0 * zLength * 0, 2*yCoord);
//	float offsetFromOrig = offset.y - float(model.x + model.y);
//	if (pos.y < 0) {
//		offset.y -= float(model.zLength);
//	}

	//hack to preserve inputs/output
	vec3 mpos = (pos + colorIn * 0 + normalIn * 0 + offset) * 0.2;
	
//	vec3 color = vec3(cos(offsetFromOrig*0.3 + 4*3.14159265/3),
//					  cos(offsetFromOrig*0.8),
//					  cos(offsetFromOrig*0.3 + 2*3.13158265/3));
//
//	if(getModel().highlighted != 0) {
//		color = vec3(1,0,0);
//	}
//
//	if(highlightX < 0 || highlightY < 0) {
		highlightPos = vec3(0, 0, 0);
//	} else {
//		highlightPos = offsetPosFor(getModelAt(highlightX, highlightY)) + vec3(0, 0, 0);
//	}

	vec4 p = M * vec4(mpos, 1);
	gl_Position = P*p;

	smoothPos = pos;
	smoothNorm = normalIn;
	flatDiffuseColor = vec3(0,0,0);
}

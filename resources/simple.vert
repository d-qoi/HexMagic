#version 330

uniform mat4 P;  //projection matrix
uniform mat4 M;  //modelview matrix: M = C * mR * mT

uniform float highlightX;
uniform float highlightY;

uniform int colorMode;

in float xCoord;
in float yCoord;
in float zOffset;
in float zLength;

const int WIDTH = 40;

in vec3 pos;
in vec3 normalIn;

out vec3 smoothPos;
out vec3 smoothNorm;
flat out vec3 flatDiffuseColor;

const vec3 lightIntensity = vec3(1, 1, 1);
const vec3 ka = vec3(0.1, 0.1, 0.1);
const vec3 ks = vec3(1.0, 1.0, 1.0);
const float specAlpha = 10;

vec3 offsetPos() {
	// NOTE: If you modify this method, also modify the one in pickTexture.vert
	vec3 modifier = vec3(0, 0, 0);

	if(pos.y < 0) {
		modifier = vec3(0, zLength, 0);
	}

	return (vec3(2 * xCoord, zOffset, 2 * yCoord) - modifier);
}

void main()
{
	vec3 mpos = (pos + offsetPos()) * 0.2;
	vec4 p = M * vec4(mpos, 1);
	gl_Position = P*p;
	
	float offsetFromOrig = zOffset - (xCoord + yCoord);
	
	vec3 color = vec3(0);
	if (colorMode == 0) {
		color = vec3(cos(offsetFromOrig*0.3 + 4*3.14159265/3),
					 cos(offsetFromOrig*0.8),
					 cos(offsetFromOrig*0.3 + 2*3.13158265/3));	
	} else if (colorMode == 1) {
		color = vec3(0.3);
		if (offsetFromOrig > 0.3) {
			color = color + vec3(0,0,cos(offsetFromOrig*0.5 + 4*3.14159265/3));
		} else if (offsetFromOrig < -0.3) {
			color = color + vec3(cos(offsetFromOrig*0.5 + 2*3.13158265/3),0,0);
		} else {
			
		}
	} else if (colorMode == 2) {
		if (abs(offsetFromOrig) > 0.01)
			color = vec3(0.5+0.5/(offsetFromOrig*2), 0, 0.5 - 0.5/(offsetFromOrig));
		else
			color = vec3(1,1,1);
	} else {
		color = vec3(0.4, 0.4, 0.6);
	}
	

	if(xCoord == highlightX && yCoord == highlightY) {
		color = vec3(1,0,0);
	}

	smoothPos = pos;
	smoothNorm = normalIn;
	flatDiffuseColor = color;
}

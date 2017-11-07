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

in float xCoord;
in float yCoord;
in float zOffset;
in float zLength;

in vec3 pos;

flat out vec4 flatColor;

vec3 offsetPos() {
	vec3 modifier = vec3(0, 0, 0);

	if(pos.y < 0) {
		modifier = vec3(0, zLength, 0);
	}

	return (vec3(2 * xCoord, zOffset, 2 * yCoord) - modifier);
}

void main()
{
	gl_Position = P * M * vec4((pos + offsetPos()) * 0.2, 1);
	flatColor = vec4(0, 0, yCoord/255.0, xCoord/255.0);
}

#version 330

uniform mat4 P;  //projection matrix
uniform mat4 M;  //modelview matrix: M = C * mR * mT

in float xCoord;
in float yCoord;
in float zOffset;
in float zLength;

in vec3 pos;

flat out vec4 flatColor;

vec3 offsetPos() {
	// NOTE: If you modify this method, also modify the one in simple.vert
	vec3 modifier = vec3(0, 0, 0);

	if(pos.y < 0) {
		modifier = vec3(0, zLength, 0);
	}

	return (vec3(2 * xCoord, zOffset, 2 * yCoord) - modifier);
}

void main()
{
	gl_Position = P * M * vec4((pos + offsetPos()) * 0.2, 1);
	// This forces a 254x254 limit on cubes
	flatColor = vec4(0, 0, (xCoord + 1)/255.0, (yCoord + 1)/255.0);
}

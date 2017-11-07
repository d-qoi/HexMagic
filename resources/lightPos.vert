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

in vec3 pos;
in vec3 colorIn;

out vec4 smoothColor;

void main()
{    
    //hacky easy way to draw the light position
    gl_Position = P*C*L*lightPos;
    smoothColor = vec4(1,1,1,1);
}

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

flat in vec4 flatColor;
in vec4 smoothColor;
in vec3 smoothPos;
in vec3 smoothNorm;
flat in vec3 flatDiffuseColor;

out vec4 fragColor;

const vec3 lightIntensity = vec3(1, 1, 1);
const vec3 ka = vec3(0.1, 0.1, 0.1);
const vec3 ks = vec3(1.0, 1.0, 1.0);
const float specAlpha = 10;

void main()
{
	if(shadingMode == 1) {
		fragColor = flatColor;
	} else if(shadingMode == 3) {
		vec4 p = M * vec4(smoothPos, 1);
		vec4 light = C*L*lightPos;

		vec4 incident = normalize(light - p);

		vec4 viewer = normalize(C * camPos);
		vec4 normal = normalize(vec4(N * smoothNorm, 1));

		float dotProduct = dot(normal, incident);
		vec4 ambient = vec4(ka * lightIntensity, 1);

		vec4 diffuse = vec4(flatDiffuseColor * lightIntensity * dotProduct, 1);

		vec4 normalReflect = -normalize(incident - 2 * dotProduct * normal);
		float specDot = max(0.0, dot(viewer, normalReflect));
		vec4 specular = vec4(ks * lightIntensity * pow(specDot, specAlpha), 1);

		fragColor = ambient + diffuse + specular;
	} else {
		fragColor = smoothColor;
	}
}
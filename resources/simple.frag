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

uniform int renderHighlight;

flat in vec3 highlightPos;
in vec3 smoothPos;
in vec3 smoothNorm;
flat in vec3 flatDiffuseColor;

out vec4 fragColor;

const vec3 lightIntensity = vec3(1, 1, 1);
const vec3 ka = vec3(0.1, 0.1, 0.1);
const vec3 ks = vec3(1.0, 1.0, 1.0);
const float specAlpha = 10;

float attenuation(float r, float f, float d) {
	float denom = d / r + 1.0;
	float attenuation = 1.0 / (denom*denom);
	float t = (attenuation - f) / (1.0 - f);
	return max(t, 0.0);
}

float attenuation2(float distance) {
	return 1.0 / (1.0 + 0.1*distance + 1*distance*distance);
}

void main()
{
	vec4 p = M * vec4(smoothPos, 1); // point we are manipulating
	// Set light position to camera position
	// used to be C*L*lightPos
	vec4 light = C*camPos; // point for light

	vec4 incident = normalize(light - p); // incedent 

	float lightDistance = length(incident);
	float falloff = attenuation(5, 0.20, lightDistance);

	vec4 viewer = normalize(C * camPos);
	vec4 normal = normalize(vec4(N * smoothNorm, 1));

	float dotProduct = dot(normal, incident);
	vec4 ambient = vec4(ka * lightIntensity, 1);

	vec4 diffuse = vec4(flatDiffuseColor * lightIntensity * dotProduct, 1);

	vec4 normalReflect = -normalize(incident - 2 * dotProduct * normal);
	float specDot = max(0.0, dot(viewer, normalReflect));
	vec4 specular = vec4(ks * lightIntensity * pow(specDot, specAlpha), 1);

	vec4 highlightFinal = vec4(0, 0, 0, 0);

	if(renderHighlight != 0) {
		vec3 highlightIntensity = vec3(1, 0, 0);
		vec4 highlight = C*vec4(highlightPos, 1);
		vec4 highlightIncident = normalize(highlight - p);
		float highlightDistance = length(highlightIncident);
		float highlightFalloff = attenuation2(highlightDistance);

		float highlightDotProduct = dot(normal, highlightIncident);
	//	vec4 highlightAmbient = vec4(ka * vec3(1, 0.1, 0.1), 1);

		vec4 highlightDiffuse = vec4(flatDiffuseColor * highlightIntensity * highlightDotProduct, 1);

		vec4 highlightNormalReflect = -normalize(highlightIncident - 2 * highlightDotProduct * normal);
		float highlightSpecDot = max(0.0, dot(viewer, highlightNormalReflect));
		vec4 highlightSpecular = vec4(ks * highlightIntensity * pow(highlightSpecDot, specAlpha), 1);

		highlightFinal = clamp((highlightDiffuse + highlightSpecular) * highlightFalloff, 0, 1.0);
	}

	fragColor = (ambient + diffuse + specular) * falloff + highlightFinal;
}

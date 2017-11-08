#version 330

uniform vec2 resolution;          //view resolution in pixels

uniform sampler2D texId;

layout(location = 0) out vec4 fragColor;

int kernelSize = 27;  //larger kernels will blur more, but run slower
int halfSize = kernelSize / 2;
mat3 kernel = mat3(1, 1, 1, 1, -8, 1, 1, 1, 1);

vec2 res = vec2(resolution);
vec2 fragCoord = gl_FragCoord.xy;
vec2 texCoord = fragCoord/res;

void main()
{
   fragColor = texture(texId, (texCoord + vec2(-1, -1)/res) * kernel[0][0]) +
		texture(texId, (texCoord + vec2(0, -1)/res)) * kernel[0][1] +
		texture(texId, (texCoord + vec2(1, -1)/res)) * kernel[0][2] +
		texture(texId, (texCoord + vec2(-1, 0)/res)) * kernel[1][0] +
		texture(texId, (texCoord + vec2(0, 0)/res)) * kernel[1][1] +
		texture(texId, (texCoord + vec2(0, 1)/res)) * kernel[1][2] +
		texture(texId, (texCoord + vec2(1, -1)/res)) * kernel[2][0] +
		texture(texId, (texCoord + vec2(1, 0)/res)) * kernel[2][1] +
		texture(texId, (texCoord + vec2(1, 1)/res)) * kernel[2][2];
}


uniform vec3 ViewPosition;

uniform sampler2D DiffuseTexture;
uniform sampler2D SpecularTexture;
uniform sampler2D NormalTexture;
uniform sampler2D DisplacementTexture;

uniform float emissive;


varying vec3 worldNormal;
varying vec3 worldBinormal;
varying vec3 worldTangent;
varying vec2 texcoords;

vec4 encodeDepth(float depth)
{
	float d0 = depth;
	float d1 = depth * 256.0;
	float d2 = depth * 256.0 * 256.0;
	float d3 = depth * 256.0 * 256.0 * 256.0;

	vec4 result;

	result.a = mod(d3, 1.0);
	result.r = mod((d2 - result.a / 256.0), 1.0);
	result.g = mod((d1 - result.r / 256.0), 1.0);
	result.b = d0 - result.g / 256.0;
	return result;
}

void main() {
	mat3 TBN = mat3(worldTangent, worldBinormal, worldNormal);
    vec3 textureNormal = texture2D(NormalTexture, texcoords).xyz*2.0-1.0;
    vec3 NewNormal = normalize(TBN * textureNormal);

	/* Depth value */
	gl_FragData[0] = encodeDepth(gl_FragCoord.z);

	/* World normal */
	gl_FragData[1].r = (NewNormal.x*0.5)+0.5;
	gl_FragData[1].g = (NewNormal.y*0.5)+0.5;
	gl_FragData[1].b = (NewNormal.z*0.5)+0.5;
	
	/* Specularity */
	gl_FragData[1].a = texture2D(SpecularTexture,texcoords).r;

	/* Diffuse color */
	gl_FragData[2].rgb = texture2D(DiffuseTexture,texcoords).rgb;
	gl_FragData[2].a = 1.0;

	/* Normal, tangent and binormal */
	gl_FragData[3].rgb = worldNormal*0.5+0.5;
	gl_FragData[4].rgb = worldTangent*0.5+0.5;
	gl_FragData[5].rgb = worldBinormal*0.5+0.5;

	/* Displacement value */
	gl_FragData[6] = DisplacementTexture;
}
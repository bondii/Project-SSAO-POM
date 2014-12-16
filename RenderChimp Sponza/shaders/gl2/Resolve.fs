#version 150

uniform sampler2D diffuseTextureBuffer;
uniform sampler2D lightBuffer;
uniform sampler2D heightBuffer;

varying vec2 tc;

varying vec3 viewVector;
varying vec3 normal;

#define EXTRACT_DEPTH(cc)	((cc).b + (cc).g / 256.0 + (cc).r / (256.0 * 256.0) + (cc).a / (256.0 * 256.0 * 256.0))

void main()
{
	float fHeightMapScale = 0.8;
	float fParallaxLimit = -length(viewVector.xy) / viewVector.z;
	fParallaxLimit *= fHeightMapScale;

	vec2 vOffsetDir = normalize(viewVector.xy);
	vec2 vMaxOffset = vOffsetDir * fParallaxLimit;

	int nMaxSamples = 20;
	int nMinSamples = 4;
	int nNumSamples = int(mix(nMaxSamples, nMinSamples, dot(viewVector, normal)));

	float fStepSize = 1.0 / float(nNumSamples);

	vec2 dx = dFdx(tc);
	vec2 dy = dFdy(tc);

	float fCurrRayHeight = 1.0;
	vec2 vCurrOffset = vec2(0, 0);
	vec2 vLastOffset = vec2(0, 0);

	float fLastSampledHeight = 1.0;
	float fCurrSampledHeight = 1.0;

	int nCurrSample = 0;

	while (nCurrSample < nNumSamples) {
		fCurrSampledHeight = textureGrad(heightBuffer, tc + vCurrOffset, dx, dy).r;
		if (fCurrSampledHeight > fCurrRayHeight) {
			float delta1 = fCurrSampledHeight - fCurrRayHeight;
			float delta2 = (fCurrRayHeight + fStepSize) - fLastSampledHeight;

			float ratio = delta1/(delta1+delta2);

			vCurrOffset = ratio * vLastOffset + (1.0 - ratio) * vCurrOffset;

			nCurrSample = nNumSamples +1;
		} else {
			nCurrSample++;

			fCurrRayHeight -= fStepSize;

			vLastOffset = vCurrOffset;
			vCurrOffset += fStepSize * vMaxOffset;

			fLastSampledHeight = fCurrSampledHeight;
		}
	}
	
	vec2 vFinalCoords = tc + vCurrOffset;
	
	vec4 diffuseTexture = texture2D(diffuseTextureBuffer, vFinalCoords);
	vec4 light = texture2D(lightBuffer, vFinalCoords);
	vec3 ambient = vec3(0.8);

	/* Final lighting */
	//gl_FragData[0] = vec4((light.rgb+ambient)*diffuseTexture.rgb,1.0);
	gl_FragData[0] = vec4(textureGrad(heightBuffer, tc, dx, dy));

}


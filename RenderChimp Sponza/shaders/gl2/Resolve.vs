
attribute vec2 Vertex;

const int N_LIGHTS = 4;

uniform vec3 ViewPosition;

uniform mat4 World;
uniform mat4 WorldViewProjection;	/* transform to screen space */
uniform mat4 ViewProjectionInverse;

uniform sampler2D normalBuffer;
uniform sampler2D tangentBuffer;
uniform sampler2D binormalBuffer;
uniform sampler2D depthBuffer;
uniform vec2 invRes;

varying vec2 tc;

varying vec3 finalViewVector;
varying vec3 finalNormal;

varying vec3 test;

#define EXTRACT_DEPTH(cc)	((cc).b + (cc).g / 256.0 + (cc).r / (256.0 * 256.0) + (cc).a / (256.0 * 256.0 * 256.0))

void main()
{
	tc = 0.5 + Vertex * 0.5; //this means that Vertex ranges from -1 to 1 and tc from 0 to 1.

	vec4 ScreenPosition;
	vec2 res = vec2(1/invRes.x, 1/invRes.y);
	ScreenPosition.xy = Vertex;
	ScreenPosition.z = EXTRACT_DEPTH(texture2D(depthBuffer, tc))*2.0-1.0;
	ScreenPosition.w = 1.0;

	vec4 Position = (ViewProjectionInverse* ScreenPosition);
	Position = Position/Position.w;
	vec3 worldPos = Position.xyz;

	test = worldPos;

	vec3 viewVector = worldPos - ViewPosition;
	
	vec3 normal =	texture2D(normalBuffer, tc).xyz*2.0-1.0;		//in world space
	vec3 tangent =	texture2D(tangentBuffer, tc).xyz*2.0-1.0;	//in world space
	vec3 binormal =	texture2D(binormalBuffer, tc).xyz*2.0-1.0;	//in world space

	mat3 tTWS;
	tTWS[0] = tangent;
	tTWS[1] = binormal;
	tTWS[2] = normal;

	//mat3 worldToTangentSpace = transpose(tTWS); //apparently doesn't work in this version of glsl
	mat3 worldToTangentSpace = mat3(
                 vec3(tTWS[0].x, tTWS[1].x, tTWS[2].x),
                 vec3(tTWS[0].y, tTWS[1].y, tTWS[2].y),
                 vec3(tTWS[0].z, tTWS[1].z, tTWS[2].z)
                 );

	finalViewVector = worldToTangentSpace * viewVector;
	finalNormal = worldToTangentSpace * normal;

	gl_Position = vec4(Vertex.xy, 0.0, 1.0);
}



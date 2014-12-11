
attribute vec2 Vertex;

const int N_LIGHTS = 4;

uniform vec3 ViewPosition;

uniform mat4 World;
uniform mat4 WorldViewProjection;	/* transform to screen space */

uniform sampler2D normalBuffer;
uniform sampler2D tangentBuffer;
uniform sampler2D binormalBuffer;

varying vec2 tc;

varying vec3 viewVector;
varying vec3 normal;

void main()
{
	vec3 worldPos = (World * vec4(Vertex, 0.0, 1.0)).xyz;
	viewVector = worldPos - ViewPosition;
	
	tc = 0.5 + Vertex * 0.5;
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

	viewVector = worldToTangentSpace * viewVector;
	normal = worldToTangentSpace * normal;

	gl_Position = vec4(Vertex.xy, 0.0, 1.0);
}



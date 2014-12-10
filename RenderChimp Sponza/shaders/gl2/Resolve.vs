
attribute vec2 Vertex;

uniform vec3 ViewPosition;
uniform vec3 LightPosition[];

uniform mat4 World;
uniform mat4 WorldViewProjection;	/* transform to screen space */

varying vec2 tc;

uniform sampler2D normalBuffer;
uniform sampler2D tangentBuffer;
uniform sampler2D binormalBuffer;

varying vec3 viewVector;
varying vec3 normal;
varying vec3 outPos;
varying vec3 lightVector[LightPosition.length()];

void main()
{
	worldPos = (World * vec4(Vertex, 1)).xyz;

	vec3 normal =	texture2D(normalBuffer, tc).xyz*2.0-1.0;		//in world space
	vec3 tangent =	texture2D(tangentBuffer, tc).xyz*2.0-1.0;	//in world space
	vec3 binormal =	texture2D(binormalBuffer, tc).xyz*2.0-1.0;	//in world space

	viewVector = worldPos - ViewPosition;
	

	mat3 tangentToWorldSpace;
	tangentToWorldSpace[0] = tangent;
	tangentToWorldSpace[1] = binormal;
	tangentToWorldSpace[2] = normal;

	mat3 worldToTangentSpace = transpose(tangentToWorldSpace);
	
	outPos = (WorldViewProjection * vec4(Vertex.xy, 0.0, 1.0)).xyz;

	viewVector = worldToTangentSpace * viewVector;
	normal = worldToTangentSpace * normal;

	//vec3 lightVector[LightPosition.length()];
	for (int i=0; i<LightPosition.length(); i++) {
		lightVector[i] = LightPosition[i] - worldPos;
		lightVector[i] = worldToTangentSpace * lightVector[i];
	}

	tc = 0.5 + Vertex * 0.5;
	gl_Position = vec4(Vertex.xy, 0.0, 1.0);
}



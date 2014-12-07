
attribute vec2 Vertex;

uniform vec3 ViewPosition;
uniform vec3 LightPosition[];

uniform mat4 World;
uniform sampler2D normalAndSpecularBuffer;
uniform vec2 invRes;

varying vec2 tc;

void main()
{
	vec3 worldPos = (World * vec4(Vertex, 1)).xyz;
	vec3 normal = normalize(texture2D(normalAndSpecularBuffer, invRes*gl_FragCoord.xy).xyz*2.0-1.0); //in world space
	vec3 viewVector = worldPos - ViewPosition;
	vec3 lightVector[LightPosition.length()];
	for (int i=0; i<lightPosition.length(); i++) {
		lightVector[i] = lightPosition[i] - worldPos;
	}

	mat3 tangentToWorldSpace;

	tangentToWorldSpace[0] = World * normalize(IN.tangent);
	tangentToWorldSpace[1] = World * normalize(IN.binormal);
	tangentToWorldSpace[2] = normal;

	tc = 0.5 + Vertex * 0.5;
	gl_Position = vec4(Vertex.xy, 0.0, 1.0);
}



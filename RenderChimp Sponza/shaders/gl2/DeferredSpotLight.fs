
uniform sampler2D depthBuffer;
uniform sampler2D normalAndSpecularBuffer;


uniform sampler2D shadowMap;

uniform vec2 invRes;

uniform mat4 ViewProjectionInverse;
uniform mat4 ViewProjection;
uniform vec3 ViewPosition;
uniform mat4 shadowViewProjection;

uniform vec3 LightColor;
uniform vec3 LightPosition;
uniform vec3 LightDirection;
uniform float LightIntensity;
uniform float LightAngleFalloff;

uniform vec2 ShadowMapTexelSize;


#define EXTRACT_DEPTH(cc)	((cc).b + (cc).g / 256.0 + (cc).r / (256.0 * 256.0) + (cc).a / (256.0 * 256.0 * 256.0))

void main()
{
vec3 Normals = normalize(texture2D(normalAndSpecularBuffer, invRes*gl_FragCoord.xy).xyz*2.0-1.0);
float Specular = texture2D(normalAndSpecularBuffer, invRes*gl_FragCoord.xy).w;
vec4 ScreenPosition;
ScreenPosition.x = gl_FragCoord.x*invRes.x*2.0-1.0;
ScreenPosition.y = gl_FragCoord.y*invRes.y*2.0-1.0;
ScreenPosition.z = EXTRACT_DEPTH(texture2D(depthBuffer, invRes*gl_FragCoord.xy))*2.0-1.0;
ScreenPosition.w = 1.0;

vec4 Position = (ViewProjectionInverse* ScreenPosition);
Position = Position/Position.w;

vec4 shadowPos = (shadowViewProjection * Position);
shadowPos = shadowPos/shadowPos.w;
float shadowDepth = shadowPos.z;


int nbrShadows=0;
for(int i = -1; i <= 1; i++){
    for(int j = -1; j <= 1; j++){
        vec2 pos ;
        pos.x = i*ShadowMapTexelSize.x+(shadowPos.x)*0.5+0.5;
        pos.y = j*ShadowMapTexelSize.y+(shadowPos.y)*0.5+0.5;
        float depth = EXTRACT_DEPTH(texture2D(shadowMap, pos))*2.0-1.0;
        if (shadowDepth > depth){
           nbrShadows++;
        }
    }
}

float shadowFrac = nbrShadows/9.0;
vec3 LightVector = normalize(LightPosition- Position.xyz);
vec3 Reflection = normalize(-reflect(LightVector, Normals.xyz));
vec3 ViewVector = normalize(ViewPosition-Position.xyz);

float DistanceFalloff = 1.0/(pow(length(LightPosition-Position.xyz), 2.0));
float cosAngle = dot(normalize(LightDirection), -LightVector);
float AngleFalloff = (cosAngle-LightAngleFalloff)/(1-LightAngleFalloff);

float diffuseTerm = max(dot(LightVector, Normals.xyz), 0.0);
float specularTerm = pow(max(dot(Reflection, ViewVector), 0.0), 10) * Specular;
vec3 whiteLight = vec3(1.0, 1.0, 1.0);

/* Outputing final color */
gl_FragData[0] = (1-shadowFrac)*vec4((min((diffuseTerm + specularTerm)*AngleFalloff*DistanceFalloff*LightIntensity, 1.0)*LightColor), 1.0);
}


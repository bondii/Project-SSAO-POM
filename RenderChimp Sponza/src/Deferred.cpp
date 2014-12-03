#include "program.h"

#if defined EDAN35

/*
 * Example 13 - Deferred lighting
 *
 */

#include "RenderChimp.h"

Geometry *sphere;

World *world;
Camera *camera;
Camera *shadowCamera;

Group *sponza;

Geometry *fullScreenQuad;

ShaderProgram *gBufferShader;
ShaderProgram *sBufferShader;

ShaderProgram *spotLightShader;

ShaderProgram *resolveShader;

#define LIGHT_INTENSITY	12.0f
#define LIGHT_CUTOFF 0.05f
#define LIGHT_ANGLE_FALLOFF 0.7f
#define N_LIGHTS		4

#define SHADOW_MAP_RES	512

Light *light_list[N_LIGHTS];

RenderTarget *geometryBuffer;
RenderTarget *lightBuffer;
RenderTarget *shadowBuffer;

Geometry *low_cone;

/* Functions foar loading geometry */
Geometry *loadCone();
Geometry *loadFullscreenQuad();


/* Function for cameracontrols */
void cameraControls();


vec2f prev_pos;
vec2f camera_rotation;



std::vector<Geometry *> geometryList;

void RCInit()
{
	u32 i;


	RenderState *state;
	u32 x, y;
	vec2f near_far;
	vec2f inv_res;
	vec2f res;
	f32 fov, aspect;

	world = SceneGraph::createWorld(0);
	Platform::getDisplaySize(&x, &y);



	fov = 1.2f;
	aspect = 4.0f / 3.0f;
	near_far.x = 0.1f;
	near_far.y = 100.0f;
	inv_res.x = 1.f /  (f32)x;
	inv_res.y = 1.f / (f32)y;

	camera = SceneGraph::createCamera(0);
	camera->setPerspectiveProjection(fov, aspect, near_far.x, near_far.y);
    camera->translate(0,2,0);

	/* Size of the cone */
	float coneSize = sqrt(LIGHT_INTENSITY/LIGHT_CUTOFF); 

	world->attachChild(camera);
	world->setActiveCamera(camera);
	shadowCamera = SceneGraph::createCamera(0);
	shadowCamera->setPerspectiveProjection(0.785398f*2.f, 1.f, 0.1f, coneSize);



	geometryBuffer = SceneGraph::createRenderTarget("GeometryRT", x, y, 3, true, false,TEXTURE_FILTER_NEAREST);
	lightBuffer = SceneGraph::createRenderTarget("LightRT", x, y, 1, false, false,TEXTURE_FILTER_NEAREST);
	shadowBuffer = SceneGraph::createRenderTarget("ShadowRT", SHADOW_MAP_RES, SHADOW_MAP_RES, 1, true, false,TEXTURE_FILTER_NEAREST);
	geometryBuffer->shareDepthBuffer(lightBuffer);

	Renderer::setRenderTarget(geometryBuffer);
	Renderer::clearColor(vec4f(0.f,0.f,0.f,0.f));
	Renderer::clearDepth(1.0f);
	Renderer::setRenderTarget(lightBuffer);
	Renderer::clearColor(vec4f(0.f,0.f,0.f,0.f));
	Renderer::clearDepth(1.0f);
	Renderer::setRenderTarget(shadowBuffer);
	Renderer::clearColor(vec4f(0.f,0.f,0.f,0.f));
	Renderer::clearDepth(1.0f);




	resolveShader = SceneGraph::createShaderProgram("ResolveSP", 0, "Resolve.vs", "Resolve.fs", 0);
	resolveShader->getRenderState()->disableCulling();
	resolveShader->getRenderState()->disableDepthTest();


	spotLightShader = SceneGraph::createShaderProgram("SpotLightingSP", 0, "DeferredSpotLight.vs", "DeferredSpotLight.fs", 0);
	spotLightShader->setValue("invRes", inv_res);
	spotLightShader->setTexture("depthBuffer", geometryBuffer->getTexture(0));
	spotLightShader->setTexture("normalAndSpecularBuffer", geometryBuffer->getTexture(1));
	spotLightShader->setTexture("shadowMap", shadowBuffer->getTexture(0));
	spotLightShader->setValue("ShadowMapTexelSize", vec2f(1.f/(float)(SHADOW_MAP_RES),1.f/(float)(SHADOW_MAP_RES)));

	state = spotLightShader->getRenderState();
	state->enableDepthTest(CMP_GREATER);
	state->disableDepthWrite();
	state->enableBlend(SB_ONE, DB_ONE);
	state->enableCulling(CULL_FRONT);


	gBufferShader = SceneGraph::createShaderProgram("GBufferSP", 0, "GeometryBuffer.vs", "GeometryBuffer.fs", 0);
	state = gBufferShader->getRenderState();
	state->enableBlend(SB_ONE, DB_ZERO);


	sBufferShader = SceneGraph::createShaderProgram("SBufferSP", 0, "ShadowBuffer.vs", "ShadowBuffer.fs", 0);
	state = sBufferShader->getRenderState();
	state->enableDepthTest(CMP_LESS);
	state->enableCulling(CULL_FRONT);
	state->enableBlend(SB_ONE, DB_ZERO);


	sponza = SceneGraph::createGroup("SponzaGroup", "scenes/sponza_lite.pwn");
	world->attachChild(sponza);
	sponza->setScale(0.01f);

	sponza->getChildrenRecursive(geometryList);

	low_cone = loadCone();
	low_cone->setShaderProgram(spotLightShader);
	low_cone->scale(coneSize);

	fullScreenQuad = loadFullscreenQuad();
	fullScreenQuad->setShaderProgram(resolveShader);



	for (i = 0; i < N_LIGHTS; i++) {

		f32 r, g, b;

		r = 0.5f + ((f32)rand() / (f32)RAND_MAX)*0.5f;
		g = 0.5f + ((f32)rand() / (f32)RAND_MAX)*0.5f;
		b = 0.5f + ((f32)rand() / (f32)RAND_MAX)*0.5f;

		light_list[i] = SceneGraph::createLight(0);
		light_list[i]->setColor(r, g, b);

		Geometry* ls = (Geometry *) low_cone->duplicate(0, false);
		ls->setVisible(false);

		ls->setValue("LightColor", light_list[i]->getColor());
		ls->setValue("LightIntensity", LIGHT_INTENSITY); 
		ls->setValue("LightAngleFalloff", LIGHT_ANGLE_FALLOFF); 

		light_list[i]->attachChild(ls);
		world->attachChild(light_list[i]);
	}

	prev_pos = vec2f(0.0f, 0.0f);
	camera_rotation = vec2f(0.0f, 0.0f);

	sphere = SceneGraph::createGeometry("sphere", "scenes/sphere.obj");
	world->attachChild(sphere);
	sphere->setShaderProgram(resolveShader);
	sphere->setScale(1);
	geometryList.push_back(sphere);

}

u32 RCUpdate()
{

	u32 i;

	cameraControls();


	/* Pass 1: Render scene to geometrybuffer */
	Renderer::setRenderTarget(geometryBuffer);
	Renderer::clearColor(vec4f(0.f,0.f,0.f,0.f));
	Renderer::clearDepth(1.0f);
	Renderer::setCameraMatrices(camera);
	Renderer::render(geometryList, gBufferShader);

	/* Pass 2: render lights to lightbuffer */
	Renderer::setRenderTarget(lightBuffer);
	Renderer::clearColor(vec4f(0.f,0.f,0.f,0.f));

	for (i = 0; i < N_LIGHTS; i++) {

		/* Set position and direction of lightsources */
		Geometry *lightBounds = (Geometry *) light_list[i]->getChild(0);
		light_list[i]->setRotate(Platform::getFrameTime()*0.1+i*1.3f, 0.f, 1.f, 0.f);
		light_list[i]->setTranslate(vec3f(cos(Platform::getFrameTime()*0.1+i*2.1f)*4.f,2.f+i*1.f,0.f));
		lightBounds->setValue("LightPosition", light_list[i]->getWorldPosition());
		lightBounds->setValue("LightDirection", light_list[i]->getWorldFront());

		Renderer::setRenderTarget(shadowBuffer);
		Renderer::clearColor(vec4f(0.f,0.f,0.f,0.f));
		Renderer::clearDepth(1.0f);

		light_list[i]->attachChild(shadowCamera);
		Renderer::setCameraMatrices(shadowCamera);
		Renderer::render(geometryList, sBufferShader);
		lightBounds->setValue("shadowViewProjection", shadowCamera->getViewProjectionMatrix());

		light_list[i]->detachChild(shadowCamera);

		Renderer::setRenderTarget(lightBuffer);

		Renderer::setCameraMatrices(camera);
		Renderer::render(*lightBounds, spotLightShader);


	}

	/* Pass 3: Use diffuse part geometrybuffer and lightbuffer and compute final image */
	Renderer::setRenderTarget(0);
	Renderer::clearColor(vec4f(0.f,0.f,0.f,0.f));
	Renderer::clearDepth(1.0f);
	fullScreenQuad->setTexture("lightBuffer", lightBuffer->getTexture(0));
	fullScreenQuad->setTexture("diffuseTextureBuffer", geometryBuffer->getTexture(2));
	fullScreenQuad->render();

	return 0;
}

void RCDestroy()
{
	SceneGraph::deleteNode(world);
}


void cameraControls()
{
	bool *keys = Platform::getKeyState();
	bool *mouse = Platform::getMouseButtonState();
	vec2f pos = Platform::getMousePosition() * 0.002f;
	f32 move, strafe, up;
	vec3f view_front, view_right, view_up;



	move = 0.0f;
	strafe = 0.0f;
	up = 0.0f;

	if (mouse[MouseButtonLeft]) {

		/* Relative position */

		vec2f diff = pos - prev_pos;

		camera_rotation.x -= diff.x * 2.0f;
		camera_rotation.y -= diff.y * 2.0f;

	}
	float speed = 12.0f;
	if (keys[KeyShift])
		speed = 3.0f;
	speed *= Platform::getFrameTimeStep();

	if (keys[KeyW])
		move += speed;
	if (keys[KeyS])
		move -= speed;
	if (keys[KeyA])
		strafe -= speed;
	if (keys[KeyD])
		strafe += speed;
	if (keys[KeyQ])
		up -= speed;
	if (keys[KeyE])
		up += speed;


	view_front = camera->getWorldFront();
	view_right = camera->getWorldRight();
	view_up = camera->getWorldUp();

	camera->setRotateX(camera_rotation.y);
	camera->rotateY(camera_rotation.x);
	camera->translate(view_front * move);
	camera->translate(view_right * strafe);
	camera->translate(view_up * up);

	prev_pos = pos;
}
/* Loads a fullscreen quad */
Geometry *loadFullscreenQuad()
{
	f32 quad[2 * 6] = {
		-1.0f, -1.0f,
		1.0f, -1.0f,
		-1.0f, 1.0f,

		-1.0f, 1.0f,
		1.0f, -1.0f,
		1.0f, 1.0f
	};

	VertexArray *fsquad;
	fsquad = SceneGraph::createVertexArray("FSQuadVA", quad, sizeof(f32) * 2, 6, TRIANGLES, USAGE_STATIC);
	fsquad->setAttribute("Vertex", 0, 2, ATTRIB_FLOAT32, false);

	return  SceneGraph::createGeometry("FSQuad", fsquad, false);
}
/* Loads a cone */
Geometry *loadCone()
{
	float vertexArrayData[] = {	0.f,		1.f,		-1.f,
		0.f,		0.f,		0.f,
		0.38268f,	0.92388f,	-1.f,
		0.f,		0.f,		0.f, 
		0.70711f,	0.70711f,	-1.f,
		0.f,		0.f,		0.f, 
		0.92388f,	0.38268f,	-1.f,
		0.f,		0.f,		0.f, 
		1.f,		0.f,		-1.f,
		0.f,		0.f,		0.f, 
		0.92388f,	-0.38268f,	-1.f,
		0.f,		0.f,		0.f, 
		0.70711f,	-0.70711f,	-1.f,
		0.f,		0.f,		0.f, 
		0.38268f,	-0.92388f,	-1.f,
		0.f,		0.f,		0.f, 
		0.f,		-1.f,		-1.f,
		0.f,		0.f,		0.f, 
		-0.38268f,	-0.92388f,	-1.f,
		0.f,		0.f,		0.f, 
		-0.70711f,	-0.70711f,	-1.f,
		0.f,		0.f,		0.f, 
		-0.92388f,	-0.38268f,	-1.f,
		0.f,		0.f,		0.f, 
		-1.f,		0.f,		-1.f,
		0.f,		0.f,		0.f, 
		-0.92388f,	0.38268f,	-1.f,
		0.f,		0.f,		0.f, 
		-0.70711f,	0.70711f,	-1.f,
		0.f,		0.f,		0.f, 
		-0.38268f,	0.92388f,	-1.f,
		0.f,		1.f,		-1.f,
		0.f,		1.f,		-1.f,
		0.38268f,	0.92388f,	-1.f,
		0.f,		1.f,		-1.f,
		0.70711f,	0.70711f,	-1.f,
		0.f,		0.f,		-1.f,
		0.92388f,	0.38268f,	-1.f,
		0.f,		0.f,		-1.f,
		1.f,		0.f,		-1.f,
		0.f,		0.f,		-1.f,
		0.92388f,	-0.38268f,	-1.f,
		0.f,		0.f,		-1.f,
		0.70711f,	-0.70711f,	-1.f,
		0.f,		0.f,		-1.f,
		0.38268f,	-0.92388f,	-1.f,
		0.f,		0.f,		-1.f,
		0.f,		-1.f,		-1.f,
		0.f,		0.f,		-1.f,
		-0.38268f,	-0.92388f,	-1.f,
		0.f,		0.f,		-1.f,
		-0.70711f,	-0.70711f,	-1.f,
		0.f,		0.f,		-1.f,
		-0.92388f,	-0.38268f,	-1.f,
		0.f,		0.f,		-1.f,
		-1.f,		0.f,		-1.f,
		0.f,		0.f,		-1.f,
		-0.92388f,	0.38268f,	-1.f,
		0.f,		0.f,		-1.f,
		-0.70711f,	0.70711f,	-1.f,
		0.f,		0.f,		-1.f,
		-0.38268f,	0.92388f,	-1.f,
		0.f,		0.f,		-1.f,
		0.f,		1.f,		-1.f,
		0.f,		0.f,		-1.f};

	VertexArray* coneVertexArray = SceneGraph::createVertexArray("SpotLightVertexArray", (void*)vertexArrayData,sizeof(float)*3, 65,TRIANGLE_STRIP, USAGE_STATIC);
	coneVertexArray->setAttribute("Vertex",0,3,ATTRIB_FLOAT32);
	return SceneGraph::createGeometry("SpotLightGeometry",coneVertexArray, false);
}



#endif /* RC_COMPILE_BENCHMARK */


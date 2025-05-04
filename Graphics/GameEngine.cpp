#include "GameEngine.h"

#include "glmAddon.h"
#include "ECSComponents.h"
#include "PhysicsSystem.h"
#include "CollisionSystem.h"
#include "RenderSystem.h"


GameEngine::GameEngine() {
	worldUp = vec3(0, 0, 1);
	camera = Camera(vec3(15, 0, 15), vec3(0.f, 45.f, 180.f), 20.f);

	ambientLighting = vec3(0.4f);
	lightColor = vec3(0.8f);
	lightDirection = normalize(vec3(-1, 1, -1));

	projection = glm::perspective(glm::pi<float>() * 0.25f, 16 / 9.f, 0.1f, 1000.f);

	// Initialize EntityComponentSystem
	ecs.init();

	// Register Components
	ecs.registerComponent<MeshComponent>();
	ecs.registerComponent<TransformComponent>();
	ecs.registerComponent<MaterialComponent>();
	ecs.registerComponent<PhysicsComponent>();
	ecs.registerComponent<CollisionComponent>();

	// Register Systems and their signatures
	ecs.registerSystem<RenderSystem>();
	ecs.addSystemComponentType<RenderSystem, MeshComponent>();
	ecs.addSystemComponentType<RenderSystem, TransformComponent>();
	ecs.addSystemComponentType<RenderSystem, MaterialComponent>();

	ecs.registerSystem<PhysicsSystem>();
	ecs.addSystemComponentType<PhysicsSystem, TransformComponent>();
	ecs.addSystemComponentType<PhysicsSystem, PhysicsComponent>();

	ecs.registerSystem<CollisionSystem>();
	ecs.addSystemComponentType<CollisionSystem, CollisionComponent>();
	ecs.addSystemComponentType<CollisionSystem, TransformComponent>();


	// Create Entities
	ECS::uint floor = ecs.createEntity();
	ecs.addComponent<MeshComponent>(floor, { enumGeometry::PLANE }); // floor mesh
	ecs.addComponent<TransformComponent>(floor, { vec3(0, 0, 0), eulerToQuat(vec3(0, 0, 0)), vec3(20, 20, 1)});
	ecs.addComponent<MaterialComponent>(floor, { MaterialProperties{vec3(0.6f), 0.5f}});
	ecs.addComponent<CollisionComponent>(floor, { enumGeometry::PLANE });
	ECS::uint bunny = ecs.createEntity();
	ecs.addComponent<MeshComponent>(bunny, { 3 }); // bunny mesh
	ecs.addComponent<TransformComponent>(bunny, { vec3(10, 10, 0), eulerToQuat(vec3(0, 0, 0)), vec3(0.5f) });
	ecs.addComponent<MaterialComponent>(bunny, { MaterialProperties{vec3(1, 1, 0.86f), 0.5f} });

	ECS::uint sphere = ecs.createEntity();
	ecs.addComponent<MeshComponent>(sphere, { enumGeometry::SPHERE }); // sphere mesh
	ecs.addComponent<TransformComponent>(sphere, { vec3(-5, 5, 10), eulerToQuat(vec3(0)), vec3(0.5f) });
	ecs.addComponent<MaterialComponent>(sphere, { MaterialProperties{vec3(0.8f, 0.1f, 0.1f), 1.f} });
	ecs.addComponent<PhysicsComponent>(sphere, { vec3(0), vec3(0), 1 / 5.f, vec3(0), vec3(0), glm::identity<mat3>() });
	ecs.addComponent<CollisionComponent>(sphere, { enumGeometry::SPHERE });
	ECS::uint sphere2 = ecs.createEntity();
	ecs.addComponent<MeshComponent>(sphere2, { enumGeometry::SPHERE }); // sphere mesh
	ecs.addComponent<TransformComponent>(sphere2, { vec3(-5, 5, 5), eulerToQuat(vec3(0)), vec3(0.8f) });
	ecs.addComponent<MaterialComponent>(sphere2, { MaterialProperties{vec3(0.1f, 0.8f, 0.1f), 1.f} });
	ecs.addComponent<PhysicsComponent>(sphere2, { vec3(0), vec3(0), 1 / 10.f, vec3(0), vec3(0), glm::identity<mat3>() });
	ecs.addComponent<CollisionComponent>(sphere2, { enumGeometry::SPHERE });

	ECS::uint box = ecs.createEntity();
	ecs.addComponent<MeshComponent>(box, { enumGeometry::BOX }); // cube mesh
	ecs.addComponent<TransformComponent>(box, { vec3(0, 0, 10), eulerToQuat(vec3(45.f, 45.f, 0)), vec3(1, 2, 3) });
	ecs.addComponent<MaterialComponent>(box, { MaterialProperties{vec3(0.2f, 0.2f, 0.8f), 1.f} });
	ecs.addComponent<PhysicsComponent>(box, { vec3(0), vec3(0), 1/100.f, vec3(0), vec3(0), glm::identity<mat3>() });
	ecs.addComponent<CollisionComponent>(box, {enumGeometry::BOX});
	ECS::uint box2 = ecs.createEntity();
	ecs.addComponent<MeshComponent>(box2, { enumGeometry::BOX }); // cube mesh
	ecs.addComponent<TransformComponent>(box2, { vec3(0, 0, 15), eulerToQuat(vec3(80.f, -45.f, 180.f)), vec3(1, 2, 3) });
	ecs.addComponent<MaterialComponent>(box2, { MaterialProperties{vec3(0.7f, 0.2f, 0.7f), 1.f} });
	ecs.addComponent<PhysicsComponent>(box2, { vec3(0), vec3(0), 1 / 50.f, vec3(0), vec3(0), glm::identity<mat3>() });
	ecs.addComponent<CollisionComponent>(box2, { enumGeometry::BOX });

	ECS::uint earth = ecs.createEntity();
	ecs.addComponent<MeshComponent>(earth, { enumGeometry::SPHERE }); // sphere mesh
	ecs.addComponent<TransformComponent>(earth, { vec3(0, 0, 5), eulerToQuat(vec3(0)), vec3(0.8f) });
	ecs.addComponent<MaterialComponent>(earth, { MaterialProperties{vec3(0.6f), 0.5f} });
	ecs.addComponent<PhysicsComponent>(earth, { vec3(0), vec3(0), 1 / 5.f, vec3(0), vec3(0), glm::identity<mat3>() });
	ecs.addComponent<CollisionComponent>(earth, { enumGeometry::SPHERE });
	ECS::uint cobblestone = ecs.createEntity();
	ecs.addComponent<MeshComponent>(cobblestone, { enumGeometry::BOX }); // cube mesh
	ecs.addComponent<TransformComponent>(cobblestone, { vec3(0, 0, 20), eulerToQuat(vec3(45.f, 45.f, 0)), vec3(1.f) });
	ecs.addComponent<MaterialComponent>(cobblestone, { MaterialProperties{vec3(0.6f), 0.5f} });
	ecs.addComponent<PhysicsComponent>(cobblestone, { vec3(0), vec3(0), 1 / 100.f, vec3(0), vec3(0), glm::identity<mat3>() });
	ecs.addComponent<CollisionComponent>(cobblestone, { enumGeometry::BOX });

	ECS::uint van = ecs.createEntity();
	ecs.addComponent<MeshComponent>(van, { 4 }); // van mesh
	ecs.addComponent<TransformComponent>(van, { vec3(5, -10, 1.6f), eulerToQuat(vec3(0, 0, 90.f)), vec3(0.02f) });
	ecs.addComponent<MaterialComponent>(van, { MaterialProperties{vec3(1), 0.5f} });

	vanEntity = van;

	//ECS::uint fluidBounds = ecs.createEntity();
	//ecs.addComponent<MeshComponent>(fluidBounds, { enumGeometry::BOX });
	//ecs.addComponent<TransformComponent>(fluidBounds, { vec3(12, 2, 2), eulerToQuat(vec3(0, 0, 0)), vec3(4.f) });
	//ecs.addComponent<MaterialComponent>(fluidBounds, { MaterialProperties{vec3(1), 0.5f} });

	// ECS testing
	//ecs.removeComponent<PhysicsComponent>(earth);
	//ecs.addComponent<PhysicsComponent>(floor, { vec3(0), vec3(0), 1 / 100.f, vec3(0), vec3(0), glm::identity<mat3>() });
	//ecs.removeComponent<MeshComponent>(earth);

	// Give physics engine access to EntityComponentSystem
	physicsEngine.setEntityComponentSystemPtr(&ecs);

	std::cout << (unsigned int)ecs.getEntityCount() << " entities created" << std::endl;
}

bool GameEngine::init(int windowWidth, int windowHeight) {
	if (!App3D::init(windowWidth, windowHeight)) return false;

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	PhysicsSystem* physicsSystem = ecs.getSystem<PhysicsSystem>();
	physicsSystem->generateInertiaTensors(&ecs);

	meshes[0].generateCube();
	meshes[0].textureID = 1;
	meshes[1].generateSphere();
	meshes[1].textureID = 2;
	meshes[2].generatePlane();
	meshes[3].loadFromFile("models/Bunny.obj");
	meshes[4].loadFromFile("models/UtilityVan_v002.fbx");
	meshes[4].textureID = 3;

	for (int i = 0; i < meshCount; i++) {
		meshes[i].init();
	}

	textures[0].generate1x1(0xFFFFFFFF); // 1x1 white texture
	textures[1].loadFromFile("textures/cobblestone.png");
	textures[2].loadFromFile("textures/earth_diffuse.jpg");
	textures[3].loadFromFile("textures/UtilityVan_v002_bodyShell1_BaseColor.png", true);

	for (int i = 0; i < textureCount; i++) {
		textures[i].init();
	}

	pointLights.init();

	fluidSim.init(vec3(8, -4, 1), vec3(8, 8, 4), physicsEngine.gravity);
	fluidSim.bindConfigUBO(FLUID_CONFIG_UBO);
	fluidSim.bindParticleSSBO(FLUID_DATA_SSBO);

	fluidSim.spawnRandomParticles(8000);
	fluidSim.sendDataToGPU();


	// Uniform Buffer Objects
	pvmUBO.buffer.projection = projection;
	pvmUBO.buffer.projectionInverse = glm::inverse(projection);
	pvmUBO.init();
	pvmUBO.bind(PROJECTIONVIEW_UBO);

	// Shaders
	shadowShader.init("shadow.glsl", "empty.glsl");
	gpassShader.init("gpass_vert.glsl", "gpass_frag.glsl");
	fluidDepthShader.init("fluidDepth_vert.glsl", "fluidDepth_frag.glsl");
	raymarchShader.init("fullscreen_quad.glsl", "raymarch.glsl");
	lightShader.init("fullscreen_quad.glsl", "directional_light.glsl");
	pointLightShader.init("pointLight_vert.glsl", "pointLight_frag.glsl");
	compositeShader.init("fullscreen_quad.glsl", "composite.glsl");

	// Compute Shaders


	// Framebuffers
	shadowFBO.setSize(1024, 1024);
	shadowFBO.genTextureImage(GL_DEPTH_COMPONENT); // Shadow Map
	shadowFBO.init();

	gpassFBO.setSize(windowWidth, windowHeight);
	gpassFBO.genRenderBuffer(GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8);
	gpassFBO.genTextureStorage(GL_RGBA8); // AlbedoSpec
	gpassFBO.genTextureStorage(GL_RGB16F); // Positions
	gpassFBO.genTextureStorage(GL_RGB16F); // Normals
	gpassFBO.init();

	fluidDepthFBO.setSize(windowWidth, windowHeight);
	fluidDepthFBO.genRenderBuffer(GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8);
	fluidDepthFBO.genTextureStorage(GL_RG32F); // Min and Max ray depth
	fluidDepthFBO.init();

	lightFBO.setSize(windowWidth, windowHeight);
	lightFBO.shareRenderBuffer(gpassFBO);
	lightFBO.genTextureStorage(GL_RGB8); // Diffuse Light
	lightFBO.genTextureStorage(GL_RGB8); // Specular Light
	lightFBO.init();

	//fluidSim.update(0.011f);
	return true;
}


bool GameEngine::update()  {
	if (!App3D::update()) return false;
	if (keyPressed(GLFW_KEY_ESCAPE)) return false;
	
	float deltaTime = (float)getFrameTime();

	// Avoid taking absurd time steps
	constexpr float deltaTimeLimit = 1.f;
	if (deltaTime > deltaTimeLimit) deltaTime = 0.f;

	if (keyPressed(GLFW_KEY_W)) { camera.pos += camera.orientation * vec3(1, 0, 0) * camera.movementSpeed * deltaTime; }
	if (keyPressed(GLFW_KEY_S)) { camera.pos += camera.orientation * vec3(1, 0, 0) * -camera.movementSpeed * deltaTime; }
	if (keyPressed(GLFW_KEY_D)) { camera.pos += camera.orientation * vec3(0, 1, 0) * -camera.movementSpeed * deltaTime; }
	if (keyPressed(GLFW_KEY_A)) { camera.pos += camera.orientation * vec3(0, 1, 0) * camera.movementSpeed * deltaTime; }

	auto& transform = ecs.getComponent<TransformComponent>(vanEntity);
	transform.rotation = eulerToQuat(vec3(0, 0, 10.f) * deltaTime) * transform.rotation;

	// Physics engine has inbuilt fixed update system
	physicsEngine.update(deltaTime);
	fluidSim.update(deltaTime);

	return true;
}


void GameEngine::render() {
	view = genViewMatrix(camera.pos, camera.orientation * vec3(1, 0, 0), worldUp);
	pvmUBO.buffer.view = view;
	pvmUBO.buffer.viewInverse = glm::inverse(view);
	pvmUBO.buffer.cameraPos = vec4(camera.pos, 1);
	pvmUBO.subData();


	mat4 lightProjection = glm::ortho(-30.f, 30.f, -30.f, 30.f, 1.f, 60.f);
	mat4 lightView = genViewMatrix(lightDirection * -30.f, lightDirection, worldUp);
	mat4 lightProjectionView = lightProjection * lightView;

	RenderSystem* renderSystem = ecs.getSystem<RenderSystem>();
	renderSystem->addMeshInstances(ecs, meshes);

	//fluidSim.sendDataToGPU();


	// Shadow Pass
	shadowFBO.bind();
	glViewport(0, 0, 1024, 1024);
	glDisable(GL_STENCIL_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	glClear(GL_DEPTH_BUFFER_BIT);

	shadowShader.use();
	shadowShader.bindUniform(lightProjectionView, "LightProjectionView");

	for (int i = 0; i < meshCount; i++) {
		meshes[i].renderInstances();
	}
	glViewport(0, 0, 1600, 900);


	// G-Pass
	gpassFBO.bind();
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);

	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);

	glClearStencil(0);
	glClearColor(0.f, 0.f, 0.f, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Rasterize Meshes
	gpassShader.use();

	for (int i = 0; i < meshCount; i++) {
		textures[meshes[i].textureID].bind();

		meshes[i].renderInstances();
		meshes[i].clearInstances();
		
		textures[0].bind();
	}
	
	// Fluid particle depth pass
	gpassFBO.sendStencilBuffer(fluidDepthFBO);
	fluidDepthFBO.bind();
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);

	glStencilFunc(GL_ALWAYS, 2, 0xFF);
	glStencilMask(0x02);

	glClear(GL_DEPTH_BUFFER_BIT);

	fluidDepthShader.use();
	//glEnable(GL_PROGRAM_POINT_SIZE);
	//glDrawArrays(GL_POINTS, 0, fluidSim.getParticleCount());
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glDrawArraysInstanced(GL_QUADS, 0, 4, fluidSim.getParticleCount()); // change this later


	// Raymarch Particles
	fluidDepthFBO.sendStencilBuffer(gpassFBO);
	gpassFBO.bind();
	glDepthFunc(GL_LESS);
	glDepthMask(GL_FALSE);

	glStencilFunc(GL_EQUAL, 2 | 1, 0x02);
	glStencilMask(0x01);

	raymarchShader.use();

	raymarchShader.bindUniform(0, "fluidDepthPass");
	fluidDepthFBO.getRenderTexture(0)->bind(GL_TEXTURE0);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glDisable(GL_DEPTH_TEST);


	// Light Pass
	lightFBO.bind();
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glStencilFunc(GL_EQUAL, 1, 0x01);
	glStencilMask(0x00);

	glClearColor(0.f, 0.f, 0.f, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	lightShader.use();
	lightShader.bindUniform(lightColor, "LightColor");
	lightShader.bindUniform(vec3(view * vec4(lightDirection, 0)), "LightDirection");
	lightShader.bindUniform(lightProjectionView, "LightProjectionView");

	lightShader.bindUniform(0, "albedoSpecPass");
	gpassFBO.getRenderTexture(0)->bind(GL_TEXTURE0);
	lightShader.bindUniform(1, "positionPass");
	gpassFBO.getRenderTexture(1)->bind(GL_TEXTURE1);
	lightShader.bindUniform(2, "normalPass");
	gpassFBO.getRenderTexture(2)->bind(GL_TEXTURE2);
	lightShader.bindUniform(3, "shadowPass");
	shadowFBO.getRenderTexture(0)->bind(GL_TEXTURE3);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	// Point Light Pass (Reusing same diffuse and specular color buffers)
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_GEQUAL);
	glDepthMask(GL_FALSE);
	glCullFace(GL_FRONT);
	
	pointLightShader.use();

	pointLightShader.bindUniform(0, "albedoSpecPass");
	gpassFBO.getRenderTexture(0)->bind(GL_TEXTURE0);
	pointLightShader.bindUniform(1, "positionPass");
	gpassFBO.getRenderTexture(1)->bind(GL_TEXTURE1);
	pointLightShader.bindUniform(2, "normalPass");
	gpassFBO.getRenderTexture(2)->bind(GL_TEXTURE2);
	
	pointLights.addInstance(vec3(-2, 5, 2), 5.5f, vec3(0.1, 0.8f, 0.4f));
	pointLights.addInstance(vec3(2, 7, 2), 5.5f, vec3(0.8f, 0.2f, 0.2f));
	pointLights.addInstance(vec3(10, 8, 2), 5.5f, vec3(0.1f, 0.1f, 0.9f));
	pointLights.renderInstances();
	pointLights.clearInstances();

	glCullFace(GL_BACK);
	glDepthFunc(GL_LESS);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);


	// Composite Pass
	gpassFBO.sendStencilBuffer(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glStencilFunc(GL_EQUAL, 1, 0x01);
	glStencilMask(0x00);
	
	glClearColor(0.25f, 0.25f, 0.25f, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	compositeShader.use();
	compositeShader.bindUniform(ambientLighting, "AmbientLighting");

	compositeShader.bindUniform(0, "albedoSpecPass");
	gpassFBO.getRenderTexture(0)->bind(GL_TEXTURE0);
	compositeShader.bindUniform(1, "diffuseLightPass");
	lightFBO.getRenderTexture(0)->bind(GL_TEXTURE1);
	compositeShader.bindUniform(2, "specularLightPass");
	lightFBO.getRenderTexture(1)->bind(GL_TEXTURE2);
	compositeShader.bindUniform(3, "shadowPass");
	shadowFBO.getRenderTexture(0)->bind(GL_TEXTURE3);
	
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glDisable(GL_STENCIL_TEST);


	glfwSwapBuffers(window);
	glfwPollEvents();
}

void GameEngine::shutdown() {
	// Can add shutdown code here

	App3D::shutdown();
}

void GameEngine::onMouseMoved(MouseInfo mouse) {
	vec2 input = mouse.pos - mouse.prevPos;
	input *= -0.0006f;

	float alignment = dot(camera.orientation * vec3(1, 0, 0), worldUp);
	input.y = alignment > 0.99f && input.y > 0 ? 0 : input.y;
	input.y = alignment < -0.99f && input.y < 0 ? 0 : input.y;

	quat pitch = quat(cos(-input.y), vec3(0, sin(-input.y), 0));
	quat yaw = quat(cos(input.x), vec3(0, 0, sin(input.x)));

	camera.orientation = yaw * camera.orientation * pitch;
	camera.orientation = normalize(camera.orientation);
}






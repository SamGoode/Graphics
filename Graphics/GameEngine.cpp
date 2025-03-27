#include "GameEngine.h"

#include "glmAddon.h"
#include "Geometry.h"

#include "stb_image.h"



GameEngine::GameEngine() {
	worldUp = vec3(0, 0, 1);
	camera = Camera(vec3(10, 0, 10), vec3(0.f, 45.f, 180.f), 20.f);

	ambientLighting = vec3(0.6f);
	lightColor = vec3(0.8f);
	lightDirection = normalize(vec3(-1, 1, -1));

	PhysicsObject* sphere = new PhysicsObject(vec3(-5, 5, 10), vec3(0, 0, 0), new Sphere(0.5f), 5.f);
	PhysicsObject* sphere2 = new PhysicsObject(vec3(-5, 5, 5), vec3(0, 0, 0), new Sphere(0.8f), 10.f);
	PhysicsObject* box = new PhysicsObject(vec3(0, 0, 10), vec3(45.f, 45.f, 0), new Box(1.f, 2.f, 3.f), 100.f);
	PhysicsObject* box2 = new PhysicsObject(vec3(0, 0, 15), vec3(80.f, -45.f, 180.f), new Box(1.f, 2.f, 3.f), 50.f);
	//PhysicsObject* box = new PhysicsObject(vec3(0, 0, 5), vec3(0, 0, 0), new Box(1.f, 2.f, 1.f), 100.f);
	//PhysicsObject* box2 = new PhysicsObject(vec3(0, 0, 10), vec3(0, 0, 0), new Box(3.f, 1.f, 1.f), 50.f);

	PhysicsObject* earth = new PhysicsObject(vec3(0, 0, 5), vec3(0, 0, 0), new Sphere(0.8f), 5.f);
	PhysicsObject* cobblestone = new PhysicsObject(vec3(0, 0, 20), vec3(45.f, 45.f, 0), new Box(1.f, 1.f, 1.f), 100.f);

	RenderObject* bunny = new RenderObject();
	bunny->pos = vec3(10, 10, 0);
	bunny->meshID = 2;
	bunny->setColor(vec3(1, 1, 0.86f) * 0.2f);
	bunny->setDiffuseColor(vec3(0.98f, 0.52f, 0.84f) * 0.6f);

	sphere->setColor(vec3(0.8f, 0.1f, 0.1f));
	sphere2->setColor(vec3(0.1f, 0.8f, 0.1f));
	box->setColor(vec3(0.1f, 0.1f, 0.8f));
	box2->setColor(vec3(0.5f, 0.1f, 0.5f));

	earth->setColor(vec3(0.2f));
	earth->setDiffuseColor(vec3(0.6f));

	cobblestone->setColor(vec3(0.2f));
	cobblestone->setDiffuseColor(vec3(0.6f));
	
	std::cout << Registry<GameObject>::count << " GameObjects created" << std::endl;
}

bool GameEngine::startup(int windowWidth, int windowHeight) {
	if (!App3D::startup(windowWidth, windowHeight)) return false;

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	meshes[0].generateCube();
	meshes[0].textureID = 1;
	meshes[1].generateSphere();
	meshes[1].textureID = 2;
	meshes[2].loadFromFile("stanford/Bunny.obj");

	for (int i = 0; i < 3; i++) {
		meshes[i].init();
	}

	textures[0].generate1x1(0xFFFFFFFF); // 1x1 white texture
	textures[1].loadFromFile("textures/cobblestone.png");
	textures[2].loadFromFile("textures/earth_diffuse.jpg");

	for (int i = 0; i < 3; i++) {
		textures[i].init();
	}

	gpassShader.init("gpass_vert.glsl", "gpass_frag.glsl");
	lightShader.init("fullscreen_quad.glsl", "directional_light.glsl");
	compositeShader.init("fullscreen_quad.glsl", "composite.glsl");

	gpassFBO.setSize(windowWidth, windowHeight);
	gpassFBO.genRenderBuffer(GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8);
	gpassFBO.genRenderTexture(GL_RGBA8); // Albedo
	gpassFBO.genRenderTexture(GL_RGB8); // Diffuse
	gpassFBO.genRenderTexture(GL_RGBA16F); // Specular
	gpassFBO.genRenderTexture(GL_RGB16F); // Positions
	gpassFBO.genRenderTexture(GL_RGB16F); // Normals
	gpassFBO.init();

	lightFBO.setSize(windowWidth, windowHeight);
	lightFBO.shareRenderBuffer(gpassFBO);
	lightFBO.genRenderTexture(GL_RGB8); // Diffuse Light
	lightFBO.genRenderTexture(GL_RGB8); // Specular Light
	lightFBO.init();


	return true;
}

bool GameEngine::update()  {
	if (!App3D::update()) return false;
	if (keyPressed(GLFW_KEY_ESCAPE)) return false;
	
	float deltaTime = getFrameTime();

	if (keyPressed(GLFW_KEY_W)) { camera.pos += camera.orientation * vec3(1, 0, 0) * camera.movementSpeed * deltaTime; }
	if (keyPressed(GLFW_KEY_S)) { camera.pos += camera.orientation * vec3(1, 0, 0) * -camera.movementSpeed * deltaTime; }
	if (keyPressed(GLFW_KEY_D)) { camera.pos += camera.orientation * vec3(0, 1, 0) * -camera.movementSpeed * deltaTime; }
	if (keyPressed(GLFW_KEY_A)) { camera.pos += camera.orientation * vec3(0, 1, 0) * camera.movementSpeed * deltaTime; }

	//if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) { bodies[1]->applyAngularImpulse(vec3(0, 0, 1)); }
	//if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) { bodies[1]->applyAngularImpulse(vec3(0, 0, -1)); }

	physEng.update(deltaTime);

	return true;
}


void GameEngine::draw() {
	// G-Pass
	gpassFBO.bind();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);

	glClearStencil(0);
	glClearColor(0.f, 0.f, 0.0f, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	view = genViewMatrix(camera.pos, camera.orientation * vec3(1, 0, 0), worldUp);
	mat4 projectionView = projection * view;

	gpassShader.use();
	gpassShader.bindUniform(view, "View");
	gpassShader.bindUniform(projectionView, "ProjectionView");


	int objectCount = Registry<RenderObject>::count;
	for (int i = 0; i < objectCount; i++) {
		RenderObject* obj = Registry<RenderObject>::entries[i];
		int meshID = obj->meshID;
		
		mat4 transform = obj->getTransform();
		MaterialProperties material = obj->material;

		meshes[meshID].addInstance(transform, material);
	}

	for (int i = 0; i < 3; i++) {
		textures[meshes[i].textureID].bind();

		meshes[i].renderInstances();
		meshes[i].clearInstances();

		textures[0].bind();
	}

	glDisable(GL_DEPTH_TEST);


	// Light Pass
	lightFBO.bind();
	glStencilFunc(GL_EQUAL, 1, 0xFF);
	glStencilMask(0x00);

	glClearColor(0.f, 0.f, 0.f, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	vec3 viewLight = vec3(view * vec4(lightDirection, 0));

	lightShader.use();
	lightShader.bindUniform(camera.pos, "CameraPos");
	lightShader.bindUniform(lightColor, "LightColor");
	lightShader.bindUniform(viewLight, "LightDirection");

	lightShader.bindUniform(0, "diffuseTexture");
	gpassFBO.getRenderTexture(1).bind(GL_TEXTURE0);
	lightShader.bindUniform(1, "specularTexture");
	gpassFBO.getRenderTexture(2).bind(GL_TEXTURE1);
	lightShader.bindUniform(2, "positionTexture");
	gpassFBO.getRenderTexture(3).bind(GL_TEXTURE2);
	lightShader.bindUniform(3, "normalTexture");
	gpassFBO.getRenderTexture(4).bind(GL_TEXTURE3);

	glDrawArrays(GL_TRIANGLES, 0, 3);
	
	glDisable(GL_STENCIL_TEST);


	// Composite Pass
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glClearColor(0.25f, 0.25f, 0.25f, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	compositeShader.use();
	compositeShader.bindUniform(ambientLighting, "AmbientLighting");

	compositeShader.bindUniform(0, "albedoTexture");
	gpassFBO.getRenderTexture(0).bind(GL_TEXTURE0);
	compositeShader.bindUniform(1, "diffuseTexture");
	lightFBO.getRenderTexture(0).bind(GL_TEXTURE1);
	compositeShader.bindUniform(2, "specularTexture");
	lightFBO.getRenderTexture(1).bind(GL_TEXTURE2);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glDisable(GL_BLEND);

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

	quat pitch = quat(cos(-input.y), vec3(0, sin(-input.y), 0));
	quat yaw = quat(cos(input.x), vec3(0, 0, sin(input.x)));

	camera.orientation = yaw * camera.orientation * pitch;
	camera.orientation = normalize(camera.orientation);
}






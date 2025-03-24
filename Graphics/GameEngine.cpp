#include "GameEngine.h"

#include "glmAddon.h"
#include "Geometry.h"


GameEngine::GameEngine() {
	worldUp = vec3(0, 0, 1);
	camera = Camera(vec3(10, 0, 10), vec3(0.f, 45.f, 180.f), 20.f);

	PhysicsObject* sphere = new PhysicsObject(vec3(-5, 5, 10), vec3(0, 0, 0), new Sphere(0.5f), 10.f);
	PhysicsObject* sphere2 = new PhysicsObject(vec3(0, 0, 5), vec3(0, 0, 0), new Sphere(0.8f), 5.f);
	PhysicsObject* box = new PhysicsObject(vec3(0, 0, 10), vec3(45.f, 45.f, 0), new Box(1.f, 2.f, 3.f), 100.f);
	PhysicsObject* box2 = new PhysicsObject(vec3(0, 0, 15), vec3(80.f, -45.f, 180.f), new Box(1.f, 2.f, 3.f), 50.f);
	//PhysicsObject* box = new PhysicsObject(vec3(0, 0, 5), vec3(0, 0, 0), new Box(1.f, 2.f, 1.f), 100.f);
	//PhysicsObject* box2 = new PhysicsObject(vec3(0, 0, 10), vec3(0, 0, 0), new Box(3.f, 1.f, 1.f), 50.f);

	PhysicsObject* cobblestone = new PhysicsObject(vec3(0, 0, 20), vec3(45.f, 45.f, 0), new Box(1.f, 1.f, 1.f), 100.f);

	RenderObject* bunny = new RenderObject();
	bunny->pos = vec3(10, 10, 0);
	bunny->meshID = 2;
	bunny->setColor(vec3(0.1f, 0.5f, 0.5f));

	sphere->setColor(vec3(0.8f, 0.1f, 0.1f));
	sphere2->setColor(vec3(0.1f, 0.8f, 0.1f));
	box->setColor(vec3(0.1f, 0.1f, 0.8f));
	box2->setColor(vec3(0.5f, 0.1f, 0.5f));

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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	view = genViewMatrix(camera.pos, camera.orientation * vec3(1, 0, 0), worldUp);
	mat4 projectionView = projection * view;

	// Draws grid
	lineShader.bind();
	lineShader.bindUniform(projectionView, "ProjectionView");
	lineShader.bindUniform(vec3(0.8f), "BaseColor");

	glBindVertexArray(lineVAO);
	glDrawArrays(GL_LINES, 0, 42 * 2);

	// Renders Meshes
	meshShader.bind();
	meshShader.bindUniform(projectionView, "ProjectionView");

	meshShader.bindUniform(directionalLight, "LightDirection");
	meshShader.bindUniform(camera.pos, "CameraPos");

	int objectCount = Registry<RenderObject>::count;
	for (int i = 0; i < objectCount; i++) {
		RenderObject* obj = Registry<RenderObject>::entries[i];
		int meshID = obj->meshID;
		
		mat4 transform = obj->getTransform();
		MaterialProperties material = obj->material;

		meshes[meshID].addInstance(transform, material);
	}

	for (int i = 0; i < 3; i++) {
		int textureID = meshes[i].textureID;
		textures[textureID].bind();

		meshes[i].renderInstances();
		meshes[i].clearInstances();

		textures[0].bind();
	}

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






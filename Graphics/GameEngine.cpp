#include "GameEngine.h"

#include "glmAddon.h"
#include "Geometry.h"

GameEngine::GameEngine() {
	worldUp = vec3(0, 0, 1);
	camera = Camera(vec3(10, 0, 10), vec3(0.f, 45.f, 180.f), 20.f);

	PhysicsObject* sphere = new PhysicsObject(vec3(-5, 5, 10), vec3(0, 0, 0), new Sphere(0.5f), 10.f);
	PhysicsObject* sphere2 = new PhysicsObject(vec3(-5, 5, 5), vec3(0, 0, 0), new Sphere(0.8f), 5.f);
	PhysicsObject* box = new PhysicsObject(vec3(0, 0, 10), vec3(45.f, 45.f, 0), new Box(1.f, 2.f, 3.f), 100.f);
	PhysicsObject* box2 = new PhysicsObject(vec3(0, 0, 15), vec3(80.f, -45.f, 180.f), new Box(1.f, 2.f, 3.f), 50.f);
	//PhysicsObject* box = new PhysicsObject(vec3(0, 0, 5), vec3(0, 0, 0), new Box(2.f, 2.f, 1.f), 100.f);
	//PhysicsObject* box2 = new PhysicsObject(vec3(0, 0, 10), vec3(0, 0, 0), new Box(1.f, 1.f, 1.f), 50.f);

	sphere->setColor(vec4(0.8f, 0.1f, 0.1f, 1));
	sphere2->setColor(vec4(0.1f, 0.8f, 0.1f, 1));
	box->setColor(vec4(0.1f, 0.1f, 0.8f, 1));
	box2->setColor(vec4(0.5f, 0.1f, 0.5f, 1));

	std::cout << Registry<GameObject>::count << " GameObjects created" << std::endl;
}

bool GameEngine::startup(int windowWidth, int windowHeight) {
	if (!App3D::startup(windowWidth, windowHeight)) return false;

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	int objectCount = Registry<RenderObject>::count;
	for (int i = 0; i < objectCount; i++) {
		Registry<RenderObject>::entries[i]->initMesh();
		//drawObject(Registry<RenderObject>::entries[i]);
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

	shader.bind();
	shader.bindUniform(directionalLight, "LightDirection");

	mat4 projectionView = projection * view;

	int objectCount = Registry<RenderObject>::count;
	for (int i = 0; i < objectCount; i++) {
		mat4 transform = Registry<RenderObject>::entries[i]->getTransform();

		mat4 pvm = projectionView * transform;
		shader.bindUniform(pvm, "ProjectionViewModel");
		shader.bindUniform(transform, "ModelTransform");
		shader.bindUniform(vec3(Registry<RenderObject>::entries[i]->color), "BaseColor");

		Registry<RenderObject>::entries[i]->mesh.draw();
	}


	//if (showGrid) {
	//	vec4 white(1);
	//	vec4 black(0, 0, 0, 1);

	//	for (int i = 0; i < 21; i++) {
	//		gl->addLine(vec3(-10 + i, 10, 0), vec3(-10 + i, -10, 0), i == 10 ? white : black);
	//		gl->addLine(vec3(10, -10 + i, 0), vec3(-10, -10 + i, 0), i == 10 ? white : black);
	//	}
	//}

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






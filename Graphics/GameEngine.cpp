#include "GameEngine.h"

#include "glmAddon.h"


GameEngine::GameEngine() {
	worldUp = vec3(0, 0, 1);
	camera = Camera(vec3(10, 0, 10), vec3(0.f, 45.f, 180.f), 20.f);

	physEng.setParentApp(this);

	ground = Plane(vec3(0, 0, 1), 0.f);

	PhysicsBody* sphere = new PhysicsBody(vec3(-5, 5, 10), vec3(0, 0, 0), new Sphere(0.5f), 5.f);
	PhysicsBody* box = new PhysicsBody(vec3(0, 0, 10), vec3(45.f, 45.f, 0), new Box(1.f, 2.f, 3.f), 50.f);

	sphere->setColor(vec4(0.8f, 0, 0, 1));
	box->setColor(vec4(0, 0, 0.8f, 1));

	addGameObject(sphere);
	addGameObject(box);
}

bool GameEngine::startup(int windowWidth, int windowHeight) {
	if (!App3D::startup(windowWidth, windowHeight)) return false;

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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
	startDrawing();

	for (int i = 0; i < bodyCount; i++) {
		bodies[i]->draw();
	}

	view = genViewMatrix(camera.pos, camera.orientation * vec3(1, 0, 0), worldUp);

	endDrawing();
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

GameObject* GameEngine::addGameObject(GameObject* gameObject) {
	if (bodyCount >= maxBodies) {
		return nullptr;
	}

	bodies[bodyCount++] = gameObject;
	return gameObject;
}






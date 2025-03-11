#include "Application.h"

#include <iostream>

using aie::Gizmos;

static void print(vec2 v) { std::cout << "x: " << v.x << ", y: " << v.y << std::endl; }

static mat4 genViewMatrix(vec3 location, vec3 forward, vec3 worldUp) {
	vec3 zaxis = normalize(-forward);
	vec3 xaxis = normalize(cross(normalize(worldUp), zaxis));
	vec3 yaxis = cross(zaxis, xaxis);

	mat4 translation = glm::identity<mat4>();
	translation[3] -= vec4(location, 0);

	// columns and rows are in reverse here
	mat4 rotation = {
		{xaxis.x, yaxis.x, zaxis.x, 0},
		{xaxis.y, yaxis.y, zaxis.y, 0},
		{xaxis.z, yaxis.z, zaxis.z, 0},
		{0,       0,       0,       1}
	};

	return rotation * translation;
}

Application::Application() {
	window = nullptr;
	runtime = 0.f;

	camera = Camera(vec3(10, 10, 10), normalize(vec3(-1, -1, -1)), 20.f);
	worldUp = vec3(0, 1, 0);

	bodies[0] = new Sphere(vec3(0, 10, 0), 5.f, 0.5f);
	bodies[0]->setColor(vec4(0.8f, 0, 0, 1));
	bodyCount++;
}

bool Application::startup(int windowWidth, int windowHeight) {
	if (glfwInit() == false) {
		return false;
	}

	window = glfwCreateWindow(windowWidth, windowHeight, "Graphics Demo", nullptr, nullptr);
	if (window == nullptr) {
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGL()) {
		glfwDestroyWindow(window);
		glfwTerminate();
		return false;
	}

	printf("GL: %i.%i\n", GLVersion.major, GLVersion.minor);

	Gizmos::create(10000, 10000, 0, 0);
	glClearColor(0.25f, 0.25f, 0.25f, 1);
	glEnable(GL_DEPTH_TEST);

	glfwSetWindowUserPointer(window, this);

	auto func = [](GLFWwindow* w, double xpos, double ypos) {
		static_cast<Application*>(glfwGetWindowUserPointer(w))->mouseCursorCallback(w, xpos, ypos);
	};

	glfwSetCursorPosCallback(window, func);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

bool Application::update() {
	float deltaTime = glfwGetTime() - runtime;
	runtime = glfwGetTime();

	if (mouse.input != vec2(0)) {
		vec2 rot = -mouse.input;

		quat q = quatLookAt(vec3(0, 0, -1), worldUp);
		q = rotate(q, glm::radians(rot.x), worldUp);
		q = rotate(q, glm::radians(rot.y), cross(worldUp, -camera.m_forward));

		camera.m_forward = q * camera.m_forward * conjugate(q);

		mouse.input = vec2(0);
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.m_pos += camera.m_forward * camera.m_movementSpeed * deltaTime;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.m_pos += -camera.m_forward * camera.m_movementSpeed * deltaTime;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.m_pos += -cross(worldUp, camera.m_forward) * camera.m_movementSpeed * deltaTime;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.m_pos += cross(worldUp, camera.m_forward) * camera.m_movementSpeed * deltaTime;
	}


	//bodies[0]->acc += vec3(0, -2.f, 0);
	//bodies[0]->update(deltaTime);

	for (int i = 0; i < bodyCount; i++) {
		vec3 gravity = vec3(0, -2.f, 0);

		bodies[i]->acc += gravity;
		bodies[i]->update(deltaTime);
	}

	//ball.update(deltaTime);

	//if (ball.m_pos.y <= ball.m_radius) {
	//	ball.m_vel = -ball.m_vel * 0.95f;
	//}

	return true;
}

void Application::draw() {
	mat4 view = genViewMatrix(camera.m_pos, camera.m_forward, worldUp);
	mat4 projection = glm::perspective(glm::pi<float>() * 0.25f, 16 / 9.f, 0.1f, 1000.f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Gizmos::clear();

	Gizmos::addTransform(glm::mat4(1));

	vec4 white(1);
	vec4 black(0, 0, 0, 1);

	for (int i = 0; i < 21; i++) {
		Gizmos::addLine(vec3(-10 + i, 0, 10), vec3(-10 + i, 0, -10), i == 10 ? white : black);
		Gizmos::addLine(vec3(10, 0, -10 + i), vec3(-10, 0, -10 + i), i == 10 ? white : black);
	}

	//Gizmos::addAABBFilled(vec3(0, 0, 0), vec3(2, 2, 2), vec4(0.8f, 0.8f, 0.8f, 1));

	//ball.draw();

	for (int i = 0; i < bodyCount; i++) {
		bodies[i]->draw();
	}

	Gizmos::draw(projection * view);

	glfwSwapBuffers(window);
	glfwPollEvents();
}

void Application::shutdown() {
	Gizmos::destroy();
	glfwDestroyWindow(window);
	glfwTerminate();
}


void Application::mouseCursorCallback(GLFWwindow* window, double xpos, double ypos) {
	mouse.prevPos = mouse.pos;
	mouse.pos = vec2((float)xpos, (float)ypos);
	vec2 deltaPos = mouse.pos - mouse.prevPos;

	mouse.input += deltaPos * mouse.scaling / (abs(deltaPos.x) + abs(deltaPos.y));

	print(mouse.input);
}

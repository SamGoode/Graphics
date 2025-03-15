#include "Application.h"

#include <iostream>

using aie::Gizmos;

static void print(vec2 v) { std::cout << "x: " << v.x << ", y: " << v.y << std::endl; }
static void print(vec3 v) { std::cout << "x: " << v.x << ", y: " << v.y << ", z: " << v.z << std::endl; }
static void print(quat q) { std::cout << "w: " << q.w << ", x: " << q.x << ", y: " << q.y << ", z: " << q.z << std::endl; }

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

	camera = Camera(vec3(10, 0, 10), vec3(0.f, 45.f, 180.f), 20.f);
	worldUp = vec3(0, 0, 1);

	ground = Plane(vec3(0, 0, 1), 0.f);

	addPhysicsBody((new Sphere(vec3(-5, 5, 10), 5.f, 0.5f))->setColor(vec4(0.8f, 0, 0, 1)));
	addPhysicsBody((new Box(vec3(0, 0, 10), 50.f, vec3(1.f, 2.f, 3.f), vec3(180.f, 45.f, 0)))->setColor(vec4(0.8f, 0, 0, 1)));

	//bodies[1]->rot = rotate(bodies[1]->rot, glm::radians(20.f), vec3(1, 0, 0));
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

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	mouse.pos = vec2((float)xpos, (float)ypos);
	mouse.prevPos = mouse.pos;

	return true;
}

void SolveImpulse(Collision& collision) {
	PhysicsBody* A = collision.bodyA;
	PhysicsBody* B = collision.bodyB;

	vec3 norm = collision.worldNormal;

	vec3 velA = A->vel;
	vec3 angVelA = A->angVel;

	float invMassA = A->invMass;
	mat3 invInertiaA = A->invInertia;

	vec3 radA = collision.pointA - A->pos;

	vec3 velB = vec3(0);
	vec3 angVelB = vec3(0);

	float invMassB = 0.f;
	mat3 invInertiaB = mat3(0);

	vec3 radB = vec3(0);

	if (B) {
		velB = B->vel;
		angVelB = B->angVel;

		invMassB = B->invMass;
		invInertiaB = B->invInertia;

		radB = collision.pointB - B->pos;
	}

	float JV = dot(-norm, velA) + dot(cross(-radA, norm), angVelA) + dot(norm, velB) + dot(cross(radB, norm), angVelB);
	float effMass = invMassA + dot(cross(-radA, norm), A->rot * (invInertiaA * cross(-radA, norm) * A->rot));
	if (B) { effMass += invMassB + dot(cross(radB, norm), B->rot * (invInertiaB * cross(radB, norm) * B->rot)); }

	float lambda = -JV / effMass;
	float newSum = std::max(collision.lambdaSum + lambda, 0.f);

	lambda = newSum - collision.lambdaSum;
	collision.lambdaSum += lambda;

	A->applyImpulse(norm * -lambda, collision.pointA);
	if (B) { B->applyImpulse(norm * lambda, collision.pointB); }
}

bool Application::update() {
	float deltaTime = glfwGetTime() - runtime;
	runtime = glfwGetTime();

	if (mouse.input != vec2(0)) {
		mouse.input *= -1;

		quat pitch = quat(cos(-mouse.input.y), vec3(0, sin(-mouse.input.y), 0));
		quat yaw = quat(cos(mouse.input.x), vec3(0, 0, sin(mouse.input.x)));

		camera.orientation = camera.orientation * pitch;
		camera.orientation = yaw * camera.orientation;
		normalize(camera.orientation);

		mouse.input = vec2(0);
	}

	

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { camera.pos += (camera.orientation * forward) * camera.movementSpeed * deltaTime; }
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { camera.pos += -(camera.orientation * forward) * camera.movementSpeed * deltaTime; }
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { camera.pos += cross(camera.orientation * forward, worldUp) * camera.movementSpeed * deltaTime; }
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { camera.pos += -cross(camera.orientation * forward, worldUp) * camera.movementSpeed * deltaTime; }

	//if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) { bodies[1]->applyAngularImpulse(vec3(0, 0, 1)); }

	for (int i = 0; i < bodyCount; i++) {
		bodies[i]->update(deltaTime);

		//bodies[i]->rot = rotate(bodies[i]->rot, glm::radians(45.f * deltaTime), worldUp);

		//bodies[i]->rot = glm::quat(vec3(0, 0, glm::radians(45.f * deltaTime))) * bodies[i]->rot;//quat(, 0, 0, 1);

		//bodies[i]->rot += ((quat(0, vec3(0, 0, 1)) * bodies[i]->rot) * (deltaTime * 0.5f));

		vec3 gravity = vec3(0, 0, -1.5f);
		bodies[i]->acc += gravity;
	}

	for (int i = 0; i < bodyCount; i++) {
		if (bodies[i]->getID() == 0) {
			Box* box = dynamic_cast<Box*>(bodies[i]);

			vec3 vertices[8] = {
				vec3(box->extents.x, box->extents.y, box->extents.z),
				vec3(box->extents.x, box->extents.y, -box->extents.z),
				vec3(box->extents.x, -box->extents.y, box->extents.z),
				vec3(box->extents.x, -box->extents.y, -box->extents.z),
				vec3(-box->extents.x, box->extents.y, box->extents.z),
				vec3(-box->extents.x, box->extents.y, -box->extents.z),
				vec3(-box->extents.x, -box->extents.y, box->extents.z),
				vec3(-box->extents.x, -box->extents.y, -box->extents.z)
			};

			//quat q = quat(vec3(box->rot.z, box->rot.y, box->rot.x));

			for (int n = 0; n < 8; n++) {
				vec3 vertex = box->pos + (box->rot * vertices[n]);

				if (!ground.isPointUnderPlane(vertex)) { continue; }

				std::cout << "collided" << std::endl;

				Collision collision = {
					.bodyA = box,
					.bodyB = nullptr,
					.worldNormal = -ground.normal,
					.pointA = vertex
				};

				addCollision(collision);
			}
		}
	}

	for (int iteration = 0; iteration < 2; iteration++) {
		for (int i = 0; i < collisionCount; i++) {
			SolveImpulse(collisions[i]);
		}
	}

	clearCollisions();

	for (int i = 0; i < bodyCount; i++) {
		bodies[i]->finaliseUpdate(deltaTime);
	}

	return true;
}

void Application::draw() {
	mat4 view = genViewMatrix(camera.pos, camera.orientation * vec3(1, 0, 0), worldUp);
	mat4 projection = glm::perspective(glm::pi<float>() * 0.25f, 16 / 9.f, 0.1f, 1000.f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Gizmos::clear();

	Gizmos::addTransform(glm::mat4(1));

	vec4 white(1);
	vec4 black(0, 0, 0, 1);

	for (int i = 0; i < 21; i++) {
		Gizmos::addLine(vec3(-10 + i, 10, 0), vec3(-10 + i, -10, 0), i == 10 ? white : black);
		Gizmos::addLine(vec3(10, -10 + i, 0), vec3(-10, -10 + i, 0), i == 10 ? white : black);
	}

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

	mouse.input += deltaPos * mouse.scaling;

	//print(mouse.input);
}

PhysicsBody* Application::addPhysicsBody(PhysicsBody* physicsBody) {
	if (bodyCount >= bodyMaxCount) {
		return nullptr;
	}

	bodies[bodyCount] = physicsBody;
	bodyCount++;

	return physicsBody;
}

void Application::addCollision(Collision collision) {
	collisions[collisionCount] = collision;
	collisionCount++;
}

void Application::clearCollisions() {
	collisionCount = 0;
}
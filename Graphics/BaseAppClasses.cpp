#include "BaseAppClasses.h"


bool App3D::startup(int windowWidth, int windowHeight) {
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


	glfwSetWindowUserPointer(window, this);
	auto func = [](GLFWwindow* w, double xpos, double ypos) {
		static_cast<App3D*>(glfwGetWindowUserPointer(w))->mouseCursorCallback(w, xpos, ypos);
		};
	glfwSetCursorPosCallback(window, func);


	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	mouse.pos = vec2((float)xpos, (float)ypos);
	mouse.prevPos = mouse.pos;


	gl = new GLwrapper();
	glClearColor(0.25f, 0.25f, 0.25f, 0);
	glEnable(GL_DEPTH_TEST);

	projection = glm::perspective(glm::pi<float>() * 0.25f, 16 / 9.f, 0.1f, 1000.f);
	directionalLight = normalize(vec3(-1, 1, -1));

	return true;
}

bool App3D::update() {
	if (glfwWindowShouldClose(window)) {
		return false;
	}

	timeLastFrame = runtime;
	runtime = glfwGetTime();

	return true;
}

void App3D::shutdown() {
	glfwDestroyWindow(window);
	glfwTerminate();
}
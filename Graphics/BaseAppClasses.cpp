#include "BaseAppClasses.h"

// BaseApp Class
bool BaseApp::startup(int windowWidth, int windowHeight) {
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
		static_cast<BaseApp*>(glfwGetWindowUserPointer(w))->mouseCursorCallback(w, xpos, ypos);
		};
	glfwSetCursorPosCallback(window, func);

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	mouse.pos = vec2((float)xpos, (float)ypos);
	mouse.prevPos = mouse.pos;

	return true;
}

bool BaseApp::update() {
	if (glfwWindowShouldClose(window)) {
		return false;
	}

	timeLastFrame = runtime;
	runtime = glfwGetTime();

	return true;
}





// App3D Class
//bool App3D::startup(int windowWidth, int windowHeight) {
//	if (!BaseApp::startup(windowWidth, windowHeight)) {
//		return false;
//	}
//
//	//Gizmos::create(10000, 10000, 0, 0);
//	glClearColor(0.25f, 0.25f, 0.25f, 1);
//	glEnable(GL_DEPTH_TEST);
//
//	projection = glm::perspective(glm::pi<float>() * 0.25f, 16 / 9.f, 0.1f, 1000.f);
//
//	return true;
//}

void App3D::startDrawing() {
	BaseApp::startDrawing();
	//Gizmos::clear();

	//if (showOrigin) {
	//	Gizmos::addTransform(glm::mat4(1));
	//}

	//if (showGrid) {
	//	vec4 white(1);
	//	vec4 black(0, 0, 0, 1);

	//	for (int i = 0; i < 21; i++) {
	//		Gizmos::addLine(vec3(-10 + i, 10, 0), vec3(-10 + i, -10, 0), i == 10 ? white : black);
	//		Gizmos::addLine(vec3(10, -10 + i, 0), vec3(-10, -10 + i, 0), i == 10 ? white : black);
	//	}
	//}
}
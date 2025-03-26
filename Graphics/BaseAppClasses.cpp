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

	lineShader.init("line_vert.glsl", "line_frag.glsl");
	meshShader.init("mesh_vert.glsl", "mesh_frag.glsl");

	glClearColor(0.25f, 0.25f, 0.25f, 0);
	glEnable(GL_DEPTH_TEST);

	projection = glm::perspective(glm::pi<float>() * 0.25f, 16 / 9.f, 0.1f, 1000.f);
	

	// Grid
	line lines[42];
	for (int i = 0; i < 21; i++) {
		lines[i].v0.position = vec4(-10 + i, 10, 0, 1);
		lines[i].v1.position = vec4(-10 + i, -10, 0, 1);
		lines[i + 21].v0.position = vec4(10, -10 + i, 0, 1);
		lines[i + 21].v1.position = vec4(-10, -10 + i, 0, 1);
	}

	glGenBuffers(1, &lineVBO);
	glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
	glBufferData(GL_ARRAY_BUFFER, 42 * sizeof(line), lines, GL_STATIC_DRAW);

	glGenVertexArrays(1, &lineVAO);
	glBindVertexArray(lineVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vert), 0); // position
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vert), (void*)16); // normal

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Mouse input
	glfwSetWindowUserPointer(window, this);
	auto func = [](GLFWwindow* w, double xpos, double ypos) {
		static_cast<App3D*>(glfwGetWindowUserPointer(w))->mouseCursorCallback(w, xpos, ypos);
		};
	glfwSetCursorPosCallback(window, func);

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	mouse.pos = vec2((float)xpos, (float)ypos);
	mouse.prevPos = mouse.pos;


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
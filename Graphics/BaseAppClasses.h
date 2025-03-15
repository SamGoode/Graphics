#pragma once

#include "glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "Gizmos.h"

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::quat;

using aie::Gizmos;


class BaseApp {
protected:
	struct MouseInfo {
		vec2 prevPos;
		vec2 pos;
	};

protected:
	GLFWwindow* window = nullptr;
	double timeLastFrame = 0;
	double runtime = 0;

	MouseInfo mouse;

public:
	virtual ~BaseApp() {}

	virtual bool startup(int windowWidth, int windowHeight);
	virtual bool update();
	virtual void draw() = 0;
	virtual void shutdown() { glfwDestroyWindow(window); glfwTerminate(); }

	double getFrameTime() { return runtime - timeLastFrame; }

	virtual void startDrawing() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }
	virtual void endDrawing() { glfwSwapBuffers(window); glfwPollEvents(); }

	bool keyPressed(int key) { return glfwGetKey(window, key) == GLFW_PRESS; }
	void mouseCursorCallback(GLFWwindow* window, double xpos, double ypos) { mouse.prevPos = mouse.pos; mouse.pos = vec2((float)xpos, (float)ypos); onMouseMoved(mouse); }

	virtual void onMouseMoved(MouseInfo mouse) = 0;
};

class App3D : public BaseApp {
protected:
	mat4 view;
	mat4 projection;

	bool showGrid = true;
	bool showOrigin = true;

public:
	virtual ~App3D() {}

	virtual bool startup(int windowWidth, int windowHeight) override;
	virtual void shutdown() override { Gizmos::destroy(); BaseApp::shutdown(); }

	virtual void startDrawing() override;
	virtual void endDrawing() override { Gizmos::draw(projection * view); BaseApp::endDrawing(); }
};
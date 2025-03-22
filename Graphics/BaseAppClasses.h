#pragma once

#include "glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/fwd.hpp>

#include "GLManager.h"

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::quat;


class BaseApp {
public:
	virtual ~BaseApp() = default;

	virtual bool startup(int windowWidth, int windowHeight) = 0;
	virtual bool update() = 0;
	virtual void draw() = 0;
	virtual void shutdown() = 0; 
};


class App3D : public BaseApp {
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

	mat4 view;
	mat4 projection;

	vec3 directionalLight;

	bool showGrid = true;
	bool showOrigin = true;

	GLwrapper* gl = nullptr;

public:
	virtual ~App3D() { delete gl; }

	virtual bool startup(int windowWidth, int windowHeight) override;
	virtual bool update() override;
	virtual void shutdown() override;

	double getFrameTime() { return runtime - timeLastFrame; }

	virtual void startDrawing() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); gl->clear(); }
	virtual void endDrawing() { gl->draw(projection * view, directionalLight); glfwSwapBuffers(window); glfwPollEvents(); }

	bool keyPressed(int key) { return glfwGetKey(window, key) == GLFW_PRESS; }
	void mouseCursorCallback(GLFWwindow* window, double xpos, double ypos) { mouse.prevPos = mouse.pos; mouse.pos = vec2((float)xpos, (float)ypos); onMouseMoved(mouse); }
	
	virtual void onMouseMoved(MouseInfo mouse) = 0;
};
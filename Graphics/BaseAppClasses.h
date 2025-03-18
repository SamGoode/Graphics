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
	virtual void endDrawing() { 
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	bool keyPressed(int key) { return glfwGetKey(window, key) == GLFW_PRESS; }
	void mouseCursorCallback(GLFWwindow* window, double xpos, double ypos) { mouse.prevPos = mouse.pos; mouse.pos = vec2((float)xpos, (float)ypos); onMouseMoved(mouse); }

	virtual void onMouseMoved(MouseInfo mouse) = 0;
};





class App3D : public BaseApp {
protected:
	mat4 view;
	mat4 projection;

	mat4 m_quadTransform;

	bool showGrid = true;
	bool showOrigin = true;

	GLwrapper* gl = nullptr;

public:
	virtual ~App3D() { delete gl; }

	virtual bool startup(int windowWidth, int windowHeight) override {
		if (!BaseApp::startup(windowWidth, windowHeight)) {
			return false;
		}

		gl = new GLwrapper();
		glClearColor(0.25f, 0.25f, 0.25f, 0);
		glEnable(GL_DEPTH_TEST);

		//view = glm::lookAt(vec3(10, 10, 10), vec3(0), vec3(0, 1, 0));
		projection = glm::perspective(glm::pi<float>() * 0.25f, 16 / 9.f, 0.1f, 1000.f);

		return true;
	}
	virtual void shutdown() override { 
		//Gizmos::destroy(); 
		BaseApp::shutdown(); 
	}

	virtual void startDrawing() override;
	virtual void endDrawing() override {
		gl->draw(projection * view);
		BaseApp::endDrawing(); 
	}
	
	virtual void onMouseMoved(MouseInfo mouse) override {}
};
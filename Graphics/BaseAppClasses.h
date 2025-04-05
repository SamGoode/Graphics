#pragma once

#include "glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/fwd.hpp>

#include "Shader.h"

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::quat;


class BaseApp {
public:
	virtual ~BaseApp() = default;

	virtual bool init(int windowWidth, int windowHeight) = 0;
	virtual bool update() = 0;
	virtual void render() = 0;
	virtual void shutdown() = 0; 
};


class App3D : public BaseApp {
protected:
	struct MouseInfo {
		vec2 prevPos;
		vec2 pos;
	};

	struct vert {
		vec4 position;
		vec4 normal;
	};

	struct line {
		vert v0;
		vert v1;
	};

protected:
	GLFWwindow* window = nullptr;
	double timeLastFrame = 0;
	double runtime = 0;

	Shader lineShader;
	Shader meshShader;

	unsigned int lineVBO = 0;
	unsigned int lineVAO = 0;

	mat4 view;
	mat4 projection;

	bool showGrid = true;
	bool showOrigin = true;

	MouseInfo mouse;

public:
	virtual ~App3D() {
		glDeleteBuffers(1, &lineVBO);
		glDeleteVertexArrays(1, &lineVAO);
	}

	virtual bool init(int windowWidth, int windowHeight) override;
	virtual bool update() override;
	virtual void shutdown() override;

	double getFrameTime() { return runtime - timeLastFrame; }

	bool keyPressed(int key) { return glfwGetKey(window, key) == GLFW_PRESS; }
	void mouseCursorCallback(GLFWwindow* window, double xpos, double ypos) { mouse.prevPos = mouse.pos; mouse.pos = vec2((float)xpos, (float)ypos); onMouseMoved(mouse); }
	
	virtual void onMouseMoved(MouseInfo mouse) = 0;
};
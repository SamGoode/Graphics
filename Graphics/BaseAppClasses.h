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




class GLwrapper {
private:
	unsigned int m_shader;

	unsigned int vbo;
	unsigned int vao;

	struct Vertex {
		glm::vec4 position;
		glm::vec4 normal;
		glm::vec2 texCoord;
	};

	


public:
	GLwrapper() {
		const char* vsSource = "#version 150\n \
					 in vec4 Position; \
					 in vec4 Colour; \
					 out vec4 vColour; \
					 uniform mat4 ProjectionView; \
					 void main() { vColour = Colour; gl_Position = ProjectionView * Position; }";

		const char* fsSource = "#version 150\n \
					 in vec4 vColour; \
                     out vec4 FragColor; \
					 void main() { FragColor = vec4(1); }";


		unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
		unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);

		glShaderSource(vs, 1, (const char**)&vsSource, 0);
		glCompileShader(vs);

		glShaderSource(fs, 1, (const char**)&fsSource, 0);
		glCompileShader(fs);

		m_shader = glCreateProgram();
		glAttachShader(m_shader, vs);
		glAttachShader(m_shader, fs);
		glBindAttribLocation(m_shader, 0, "Position");
		glBindAttribLocation(m_shader, 1, "Colour");
		glLinkProgram(m_shader);

		int success = GL_FALSE;
		glGetProgramiv(m_shader, GL_LINK_STATUS, &success);
		if (success == GL_FALSE) {
			int infoLogLength = 0;
			glGetProgramiv(m_shader, GL_INFO_LOG_LENGTH, &infoLogLength);
			char* infoLog = new char[infoLogLength + 1];

			glGetProgramInfoLog(m_shader, infoLogLength, 0, infoLog);
			printf("Error: Failed to link Gizmo shader program!\n%s\n", infoLog);
			delete[] infoLog;
		}

		glDeleteShader(vs);
		glDeleteShader(fs);
		
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		Vertex vertices[6];
		vertices[0].position = { -0.5f, 0, 0.5f, 1 };
		vertices[1].position = { 0.5f, 0, 0.5f, 1 };
		vertices[2].position = { -0.5f, 0, -0.5f, 1 };
		vertices[3].position = { -0.5f, 0, -0.5f, 1 };
		vertices[4].position = { 0.5f, 0, 0.5f, 1 };
		vertices[5].position = { 0.5f, 0, -0.5f, 1 };

		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(Vertex), vertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

		//glBindVertexArray(0);
		//glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	~GLwrapper() {
		glDeleteBuffers(1, &vbo);
		glDeleteVertexArrays(1, &vao);
		glDeleteProgram(m_shader);
	}

	void draw(const glm::mat4& projectionView) {
		int shader = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &shader);

		glUseProgram(m_shader);


		glDrawArrays(GL_TRIANGLES, 0, 6);

		unsigned int projectionViewUniform = glGetUniformLocation(m_shader, "ProjectionView");
		glUniformMatrix4fv(projectionViewUniform, 1, false, glm::value_ptr(projectionView));

		glUseProgram(shader);

		
	}
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
		//Gizmos::create(10000, 10000, 0, 0);
		glClearColor(0.25f, 0.25f, 0.25f, 0);
		glEnable(GL_DEPTH_TEST);

		view = glm::lookAt(vec3(10, 10, 10), vec3(0), vec3(0, 1, 0));
		projection = glm::perspective(glm::pi<float>() * 0.25f, 16 / 9.f, 0.1f, 1000.f);
		m_quadTransform = {
			10,0,0,0,
			0,10,0,0,
			0,0,10,0,
			0,0,0,1 };
		return true;
	}
	virtual void shutdown() override { 
		//Gizmos::destroy(); 
		BaseApp::shutdown(); 
	}

	virtual void draw() override {
		startDrawing();

		gl->draw(projection * view * m_quadTransform);
		
		endDrawing();
	}

	virtual void startDrawing() override;
	virtual void endDrawing() override { 
		//Gizmos::draw(projection * view); 
		BaseApp::endDrawing(); 
	}
	
	virtual void onMouseMoved(MouseInfo mouse) override {}
};
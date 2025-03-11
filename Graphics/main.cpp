#include <iostream>

#include "Application.h"


int main() {
	Application app;

	if (!app.startup(1280, 720)) {
		return -1;
	}

	while (!app.shouldClose() && !app.keyPressed(GLFW_KEY_ESCAPE)) {
		// Update Loop
		if (!app.update()) {
			return -1;
		}

		// Rendering
		app.draw();
	}

	app.shutdown();

	return 0;
}

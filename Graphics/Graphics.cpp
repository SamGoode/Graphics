#include <iostream>

#include "Application.h"


int main() {
	Application app;

	app.startup(1280, 720);

	while (app.update()) {
		// Rendering
		app.draw();
	}

	return 0;
}

#include <iostream>

#include "GameEngine.h"


int main() {
	BaseApp* app = new App3D();

	if (!app->startup(1280, 720)) return -1;
	while (app->update()) app->draw();
	app->shutdown();

	return 0;
}

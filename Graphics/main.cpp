#include "GameEngine.h"

#include <iostream>

#include "ECS.h"


int main() {
	BaseApp* app = new GameEngine();

	if (!app->init(1600, 900)) return -1;
	while (app->update()) 
		app->render();

	app->shutdown();

	return 0;
}

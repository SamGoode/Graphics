#include "GameEngine.h"

#include <iostream>

#include "ECS.h"


//struct TransformComponent {
//	float x;
//	float y;
//	float z;
//};
//
//struct dumb {
//	char b;
//};

int main() {
	//ComponentManager componentManager;

	//componentManager.registerComponent<TransformComponent>();
	//componentManager.registerComponent<dumb>();

	//TransformComponent transform = { 4.f, 3.f, 2.f };

	//std::cout << componentManager.getComponentID<dumb>() << std::endl;
	//std::cout << componentManager.getComponentID<TransformComponent>() << std::endl;
	//componentManager.registerComponent<TransformComponent>();

	//std::cout << "testing" << std::endl;

	//return 0;


	BaseApp* app = new GameEngine();

	if (!app->startup(1600, 900)) return -1;
	while (app->update()) 
		app->draw();

	app->shutdown();

	return 0;
}

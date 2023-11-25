#include "App.h"

int main()
{
	Vision::App* app = new Vision::App();
	app->Run();
	delete app;
}

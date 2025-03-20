#include <iostream>
#include "app.h"
#include "Math/math.h"

using namespace std;

int main()
{
	
	App* myApp = new App(1920,1080, true);
	myApp->run();
	delete myApp;
	
	/*
	Math::Vector2 w1(1.0f,-1.0f);
	Math::Vector2 w2(2.0f,2.0f);

	std::cout << w1.angleDegrees(w2) << std::endl;
	*/

	return 0;

}

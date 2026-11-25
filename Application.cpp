#include"Application.h"

void Application::Initialize(HINSTANCE hInst)
{
	window_.Initialize(hInst);
	renderer_.Initialize(window_.GetWindowhandle());
	sampleTriangle_.CreateVertexBuffer(renderer_);
}

void Application::Loop()
{
	while (true)
	{
		bool result = window_.MessageLoop();
		if (result == false)break;

		result = gameLoop();
		if (result == false)break;
	}
}

void Application::Terminate()
{
	renderer_.Terminate();
	window_.Terminate();
	
}

bool Application::gameLoop()
{
	renderer_.Draw();
	sampleTriangle_.Draw(renderer_);

	renderer_.Swap();

	return true;
}
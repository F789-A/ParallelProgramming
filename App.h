#include <vector>
#include <thread>
#include <random>

#include "Camera.h"
#include "Renderer.h"

class App
{
	App& operator=(const App&) = delete;
	App(const App&) = delete;
	App& operator=(App&&) = delete;
	App(App&&) = delete;
public:
	App(int w, int h);

	void RunMainLoop();

private:
	void FillStartDataAndCamera();
	void ProcessInput();
	void ProcessData();


	std::vector<VecRgba> data;
	glm::ivec3 dataSize;
	Camera camera;

	std::mt19937 randomGenerator{123};

	Renderer renderer; // also input
	std::unique_ptr<std::thread> rendererThread;
};
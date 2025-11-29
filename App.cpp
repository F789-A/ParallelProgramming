#include "App.h"

App::App(int w, int h) : renderer(w, h, "App")
{
	RunMainLoop();
};

void App::RunMainLoop()
{
	FillStartDataAndCamera();
	renderer.UpdateData(data, dataSize);
	renderer.UpdateCamera(camera);

	rendererThread = std::make_unique<std::thread>(&Renderer::RunRenderer, &renderer);
	while (!renderer.WindowShouldClose())
	{
		ProcessInput();
		renderer.UpdateCamera(camera);
		ProcessData();
		renderer.UpdateData(data, dataSize);
	}
	rendererThread->join();
	rendererThread = nullptr;
}

void App::FillStartDataAndCamera()
{
	std::uniform_real_distribution<float> float01(0, 1.0f);

	dataSize = glm::ivec3(32, 32, 32);
	int plainSize = dataSize.x * dataSize.y * dataSize.z;
	data.resize(plainSize);

	for (int i = 0; i < plainSize; ++i)
	{
		unsigned char green = 255 * float01(randomGenerator);
		unsigned char alpha = 255 * float01(randomGenerator);
		data[i] = VecRgba(0, green, 0, alpha);
	}

	camera.Position.x = -Renderer::cubeHalfSize * static_cast<float>(dataSize.x) * 2.0f;
	camera.Size = -Renderer::cubeHalfSize * static_cast<float>(dataSize.z) * 2.5f;
}

void App::ProcessInput()
{
	static glm::vec2 prevMousePos{-100.0f, -100.0f};

	auto input = renderer.GetInputData();
	if (input.mouseWheelDirection > 0)
	{
		camera.Size += -input.deltaTime / 100.0f;
	}
	if (input.mouseWheelDirection < 0)
	{
		camera.Size += input.deltaTime / 100.0f;
	}

	if (input.leftMouseRepeat && prevMousePos != glm::vec2{ -100.0f, -100.0f })
	{
		auto diff = input.mousePos-prevMousePos;
		glm::mat4 rot;
		rot = glm::rotate(glm::mat4(1.0f), diff.x / 100.0f, { 0.0f, 1.0f, 0.0f });
		rot = glm::rotate(rot, diff.y / 100.0f, glm::cross({ 0.0f, 1.0f, 0.0f }, camera.Position));
		camera.Position = rot * glm::vec4(camera.Position, 1.0f);
	}

	prevMousePos = input.mousePos;
}

void App::ProcessData()
{
	static constexpr float iterationPart = 0.01f;
	const int iterationCount = data.size() * iterationPart;

	std::uniform_int_distribution<int> randomNumber(0, data.size() - 1);
	std::uniform_real_distribution<float> float01(0.0f, 1.0f);

	for (int i = 0; i < iterationCount; ++i)
	{
		int num = randomNumber(randomGenerator);
		unsigned char alpha = 255 * float01(randomGenerator);
		data[num].a = alpha;
	}
}

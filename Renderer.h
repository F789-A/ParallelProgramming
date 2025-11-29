#pragma once
#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"

#include <string>
#include <iostream>
#include <vector>
#include <mutex>
#include <memory>
#include <optional>
#include <bitset>

#include "Camera.h"
#include "GLShader.h"
#include "GLScreenPlaneMesh.h"
#include "GLTexture.h"

using VecRgba = glm::vec<4, unsigned char>;

struct InputData
{
	bool leftMouseRepeat{ false };
	glm::vec2 mousePos{ -100.0f, -100.0f };
	float mouseWheelDirection{ 0.0f };
	float deltaTime;
};

class Renderer
{
	Renderer& operator=(const Renderer&) = delete;
	Renderer(const Renderer&) = delete;
	Renderer& operator=(Renderer&&) = delete;
	Renderer(Renderer&&) = delete;
public:
	static constexpr float cubeHalfSize = 0.01f;

	Renderer(int w, int h, const std::string& name);

	void RunRenderer();
	void UpdateData(const std::vector<VecRgba>& otherdData, const glm::ivec3& otherDataSize);
	void UpdateCamera(const Camera& otherCamera);

	bool WindowShouldClose() const;
	InputData GetInputData() const;

private:
	void RunRendererLoop();
	void SetupWindow(unsigned int weight, unsigned int height, std::string name);
	void Synchronize();
	void Render();
	void CalculateAndLoadPicture();
	void ClearRenderer();

	void ProcessArea(std::vector<VecRgba>& output, int from, int to);
	VecRgba RaytracePixel(const glm::ivec2& pixel);

	std::bitset<6> IntersectWithCube(const glm::vec3& point, const glm::vec3& dir, const glm::ivec3& cubeNum, float eps = 0.0f);

	glm::ivec3 SidedCubeNum(const glm::vec3& point, const glm::vec3& dir);

	void ProcessInput();
	static void ScrollCallback(GLFWwindow* window, GLdouble xoffset, GLdouble yoffse);

	mutable std::mutex synchronizeMutex;

	bool needUpdateData = false;
	bool needUpdateCamera = false;
	std::vector<VecRgba> loadedData;
	glm::ivec3 loadedDataSize;
	Camera loadedCamera;

	std::vector<VecRgba> data;
	glm::ivec3 dataSize;
	Camera camera;

	std::unique_ptr<GLShader> shader;
	std::unique_ptr<GLTexture> texture;
	std::unique_ptr<GLScreenPlaneMesh> screenPlane;

	std::vector<std::thread> workers;

	GLFWwindow* Window;
	int Width;
	int Height;
	std::string name;

	bool souldClose{ false };
	InputData inputData;

	static float wheel;

	static constexpr VecRgba backgroundColor{ 10, 20, 30, 255 };

	static constexpr int WorkersCount = 7;

	float fpsCounter = 0.0f;
	float fpsNum = 0.0f;
};
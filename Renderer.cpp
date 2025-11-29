#include "Renderer.h"

float Renderer::wheel = 0.0f;

Renderer::Renderer(int w, int h, const std::string& n) : Width(w), Height(h), name(n) {};

void Renderer::RunRenderer()
{
	SetupWindow(Width, Height, name);
	shader = std::make_unique<GLShader>("ScreenTextureShader.vert", "ScreenTextureShader.frag");
	texture = std::make_unique<GLTexture>(Width, Height);
	screenPlane = std::make_unique<GLScreenPlaneMesh>();

	RunRendererLoop();
}

void Renderer::RunRendererLoop()
{
	while (!souldClose)
	{
		ProcessInput();
		Synchronize();
		CalculateAndLoadPicture();
		Render();
	}
	ClearRenderer();
}

void Renderer::UpdateData(const std::vector<VecRgba>& otherdData, const glm::ivec3& otherDataSize)
{
	std::lock_guard lock(synchronizeMutex);
	loadedData = otherdData;
	loadedDataSize = otherDataSize;
	needUpdateData = true;
}

void Renderer::UpdateCamera(const Camera& otherCamera)
{
	std::lock_guard lock(synchronizeMutex);
	loadedCamera = otherCamera;
	needUpdateCamera = true;
}

bool Renderer::WindowShouldClose() const
{
	std::lock_guard lock(synchronizeMutex);
	return souldClose;
}

InputData Renderer::GetInputData() const
{
	std::lock_guard lock(synchronizeMutex);
	return inputData;
}

void Renderer::SetupWindow(unsigned int weight, unsigned int height, std::string name)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	Window = glfwCreateWindow(weight, height, name.c_str(), nullptr, nullptr);
	if (Window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
	}
	glfwMakeContextCurrent(Window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		throw std::exception();
	}

	glfwGetFramebufferSize(Window, &Width, &Height);
	glViewport(0, 0, Width, Height);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
}

void Renderer::Synchronize()
{
	std::lock_guard lock(synchronizeMutex);
	if (needUpdateData)
	{
		std::swap(loadedData, data);
		std::swap(loadedDataSize, dataSize);
		needUpdateData = false;
	}
	if (needUpdateCamera)
	{
		std::swap(loadedCamera, camera);
		needUpdateCamera = false;
	}
}

void Renderer::Render()
{
	shader->SetTexture("ScreenTexture", *texture);
	shader->Use();
	screenPlane->Draw();

	glfwSwapBuffers(Window);

	if (glGetError() != GL_NO_ERROR)
	{
		throw std::exception();
	}
}

void Renderer::ClearRenderer()
{
	std::cout << "avg fps " << fpsCounter / fpsNum << std::endl;
	shader = nullptr;
	texture = nullptr;
	screenPlane = nullptr;
	glfwTerminate();
}

void Renderer::CalculateAndLoadPicture()
{
	std::vector<VecRgba> picture(Width * Height);

	const int batchSize = std::ceil(static_cast<float>(picture.size()) / WorkersCount);

	for (int i = 0; i < WorkersCount; ++i)
	{
		workers.push_back(std::thread(&Renderer::ProcessArea, this, std::ref(picture), i * batchSize, std::min((i + 1) * batchSize, static_cast<int>(picture.size()))));
	}

	for (auto& worker : workers)
	{
		worker.join();
	}

	workers.clear();
	texture->WriteData(picture.data(), Width, Height, GL_RGBA, GL_UNSIGNED_BYTE);
}

void Renderer::ProcessArea(std::vector<VecRgba>& output, int from, int to)
{
	for (int i = from; i < to; ++i)
	{
		output[i] = RaytracePixel({ i % Width, i / Width });
	}
}

VecRgba Renderer::RaytracePixel(const glm::ivec2& pixel)
{
	//near, far plane now is not used, only ortho

	auto ToPlainNumber = [&](const glm::vec3& pos)
		{
			return pos.x * dataSize.y * dataSize.z + pos.y * dataSize.z + pos.z;
		};

	auto Dot = [](const glm::ivec3& a, const glm::ivec3& b)
		{
			float r = 0.0f;
			for (int i = 0; i < 3; ++i)
			{
				r += a[i] + b[i];
			}
			return r;
		};
	
	VecRgba color = backgroundColor;

	glm::vec3 startRayPos = { static_cast<float>(pixel.x) / (Width - 1) * 2.0f - 1.0f, static_cast<float>(pixel.y) / (Height - 1) * 2.0f - 1.0f, -1.0f };
	glm::vec3 endRayPos = { static_cast<float>(pixel.x) / (Width - 1) * 2.0f - 1.0f, static_cast<float>(pixel.y) / (Height - 1) * 2.0f - 1.0f, 1.0f };

	glm::mat4 toGlobal = glm::inverse(camera.ProjectionMatrix(static_cast<float>(Width) / Height) * camera.ViewMatrix()); 	//can be cashed

	glm::vec3 endGlobalPos = toGlobal * glm::vec4(endRayPos, 1.0f);
	glm::vec3 startPos = toGlobal * glm::vec4(startRayPos, 1.0f);
	glm::vec3 dir = glm::normalize(endGlobalPos - startPos);

	glm::ivec3 curBox = SidedCubeNum(startPos, dir);

	startPos = startPos - 2.0f * dir * glm::dot(dir, startPos);
	dir = -dir;
	int k = 0;

	while (0 <= curBox.x && curBox.x < dataSize.x && 0 <= curBox.y && curBox.y < dataSize.y && 0 <= curBox.z && curBox.z < dataSize.z)
	{

		auto curColor = data[ToPlainNumber(curBox)];
		float alp = static_cast<float>(curColor.a) / 255;
		color = {
			static_cast<unsigned char>(color.x * (1.0f - alp) + curColor.x * alp),
			static_cast<unsigned char>(color.y * (1.0f - alp) + curColor.y * alp),
			static_cast<unsigned char>(color.z * (1.0f - alp) + curColor.z * alp),
			255
			};

		auto res = IntersectWithCube(startPos, dir, curBox);

		if (res == 0)
		{
			static constexpr float eps = 1e-5f;
			res = IntersectWithCube(startPos, dir, curBox, eps);
		}

		for (int i = 0; i < 3; ++i)
		{
			if (res[i])
			{
				curBox[i] += 1;
			}
			if (res[i + 3])
			{
				curBox[i] -= 1;
			}
		}
	}
	return color;
}

std::bitset<6> Renderer::IntersectWithCube(const glm::vec3& point, const glm::vec3& dir, const glm::ivec3& cubeNum, float eps)
{
	glm::vec3 cubePos = { static_cast<float>(cubeNum.x) - static_cast<float>(dataSize.x) / 2.0f, static_cast<float>(cubeNum.y) - static_cast<float>(dataSize.y) / 2.0f, static_cast<float>(cubeNum.z) - static_cast<float>(dataSize.z) / 2.0f };
	cubePos *= 2.0f * cubeHalfSize;
	cubePos += cubeHalfSize;

	for (int i = 0; i < 3; ++i)
	{
		if (dir[i] == 0.0f)
		{
			continue;
		}

		float offset = dir[i] > 0 ? cubeHalfSize : -cubeHalfSize;

		float t = ((cubePos[i] + offset) - point[i]) / dir[i];

		bool none = false;
		for (int j = 0; j < 3; ++j)
		{
			if (j == i)
			{
				continue;
			}

			if (std::abs((point[j] + dir[j] * t) - cubePos[j]) > cubeHalfSize + eps)
			{
				none = true;
			}
		}

		if (!none)
		{
			return 1 << (i + ((dir[i] > 0.0f) ? 0 : 3));
		}
		
	}
	return 0;
}

glm::ivec3 Renderer::SidedCubeNum(const glm::vec3& point, const glm::vec3& dir)
{
	static constexpr glm::ivec3 noneValue{-1, -1, -1};

	for (int i = 0; i < 3; ++i)
	{
		if (dir[i] == 0.0f)
		{
			continue;
		}

		float offset = dir[i] > 0 ? cubeHalfSize * static_cast<float>(dataSize[i]) : -cubeHalfSize * static_cast<float>(dataSize[i]);

		float t = (offset - point[i]) / dir[i];

		glm::vec3 pos;
		pos[i] = offset;

		bool none = false;
		for (int j = 0; j < 3; ++j)
		{
			if (j == i)
			{
				continue;
			}

			pos[j] = point[j] + dir[j] * t;
			if (std::abs(pos[j]) > cubeHalfSize * static_cast<float>(dataSize[j]))
			{
				none = true;
			}
		}

		if (!none)
		{
			pos /= 2.0f * cubeHalfSize;
			glm::ivec3 cubeNum = { std::floor(pos.x + static_cast<float>(dataSize.x) / 2.0f), std::floor(pos.y + static_cast<float>(dataSize.y) / 2.0f), std::floor(pos.z + static_cast<float>(dataSize.z) / 2.0f) };

			if (cubeNum[i] == dataSize[i])
			{
				--cubeNum[i];
			}

			return cubeNum;
		}

	}

	return noneValue;
}

void Renderer::ScrollCallback(GLFWwindow* window, GLdouble xoffset, GLdouble yoffset)
{
	Renderer::wheel = yoffset;
}

void Renderer::ProcessInput()
{
	glfwPollEvents();

	static GLfloat deltaTime = 0.0f;
	static GLfloat lastFrame = 0.0f;

	GLfloat currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	fpsCounter += deltaTime;
	fpsNum += 1;

	if (glfwGetKey(Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(Window, true);
	}

	bool leftMouseRepeat = inputData.leftMouseRepeat;
	if (glfwGetMouseButton(Window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
	{
		leftMouseRepeat = true;
	}
	else if (glfwGetKey(Window, GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE)
	{
		leftMouseRepeat = false;
	}
	glfwSetScrollCallback(Window, ScrollCallback);

	GLdouble mousePosX;
	GLdouble mousePosY;
	glfwGetCursorPos(Window, &mousePosX, &mousePosY);

	souldClose = glfwWindowShouldClose(Window);

	std::lock_guard lock(synchronizeMutex);
	inputData.deltaTime = deltaTime;
	inputData.leftMouseRepeat = leftMouseRepeat;
	inputData.mousePos = { mousePosX, mousePosY };
	inputData.mouseWheelDirection = wheel;
	wheel = 0.0f;
}
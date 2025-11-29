#pragma once
#include <glad/glad.h> 
#include <GLFW/glfw3.h>

class GLScreenPlaneMesh
{
public:
	GLScreenPlaneMesh();

	GLScreenPlaneMesh& operator=(const GLScreenPlaneMesh&) = delete;
	GLScreenPlaneMesh(const GLScreenPlaneMesh&) = delete;
	GLScreenPlaneMesh& operator=(GLScreenPlaneMesh&&) = delete;
	GLScreenPlaneMesh(GLScreenPlaneMesh&&) = delete;

	~GLScreenPlaneMesh();

	void Draw();

private:
	GLuint VBO;
	GLuint VAO;
	GLuint EBO;
};

#pragma once
#include <glad/glad.h> 
#include <GLFW/glfw3.h>

class GLTexture
{
public:
	GLTexture(int width, int height, int internalFormat = GL_RGBA8, int format = GL_RGBA,
		int wrapS = GL_REPEAT, int wrapT = GL_REPEAT,
		int minFilter = GL_LINEAR, int magFilter = GL_LINEAR, int dataType = GL_UNSIGNED_BYTE);

	GLTexture operator=(const GLTexture&) = delete;
	GLTexture(const GLTexture&) = delete;
	GLTexture(GLTexture&& other) = delete;
	GLTexture& operator=(GLTexture&& other) = delete;

	~GLTexture();

	void Bind();
	void WriteData(void* data, int width, int height, int format, int typet);

private:
	GLuint descr;
};
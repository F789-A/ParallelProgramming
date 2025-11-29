#include "GLTexture.h"

GLTexture::GLTexture(int width, int height, int internalFormat, int format,
	int wrapS, int wrapT, int minFilter, int magFilter, int dataType)
{
	glGenTextures(1, &descr);

	glBindTexture(GL_TEXTURE_2D, descr);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, dataType, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
}

GLTexture::~GLTexture()
{
	glDeleteTextures(1, &descr);
}

void GLTexture::Bind()
{
	glBindTexture(GL_TEXTURE_2D, descr);
}

void GLTexture::WriteData(void* data, int width, int height, int format, int type)
{
	glBindTexture(GL_TEXTURE_2D, descr);

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 120, 64, format, type, data);

	glBindTexture(GL_TEXTURE_2D, 0);
}
#pragma once
#include <glad/glad.h>  
#include <GLFW/glfw3.h>

#include <string>
#include <fstream>
#include <sstream>

#include "GLTexture.h"

class GLShader
{
public:
    GLShader(const GLchar* vertexPath, const GLchar* fragmentPath);

    GLShader& operator=(const GLShader&) = delete;
    GLShader(const GLShader&) = delete;
    GLShader& operator=(GLShader&&) = delete;
    GLShader(GLShader&&) = delete;

    ~GLShader();

    void Use();
    void SetTexture(const std::string& name, GLTexture& map);

private:
    void CreateAndAttachShader(const GLchar* path, const GLint typeShader);
    void Validate();
    GLuint Program;
};
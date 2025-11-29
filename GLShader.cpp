#include "GLShader.h"
#include <iostream>

GLShader::GLShader(const GLchar* vertexPath, const GLchar* fragmentPath)
{
    Program = glCreateProgram();
    CreateAndAttachShader(vertexPath, GL_VERTEX_SHADER);
    CreateAndAttachShader(fragmentPath, GL_FRAGMENT_SHADER);
    glLinkProgram(Program);

    Validate();
}

void GLShader::Validate()
{
    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(Program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(Program, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED " << infoLog << std::endl;
        throw std::exception();
    }
}

void GLShader::CreateAndAttachShader(const GLchar* pathShader, const GLint typeShader)
{
    std::string shaderCode;
    std::ifstream shaderFile;
    shaderFile.exceptions(std::ifstream::badbit);
    try
    {
        shaderFile.open(pathShader);
        std::stringstream shaderStream;
        shaderStream << shaderFile.rdbuf();
        shaderFile.close();
        shaderCode = shaderStream.str();
    }
    catch (std::ifstream::failure e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ " << pathShader << std::endl;;
        throw std::exception();
    }
    const GLchar* cShaderCode = shaderCode.c_str();

    GLuint shader = glCreateShader(typeShader);
    glShaderSource(shader, 1, &cShaderCode, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::" << typeShader << "::COMPILATION_FAILED " << infoLog << std::endl;
        throw std::exception("");
    };

    glAttachShader(Program, shader);

    glDeleteShader(shader);
}

GLShader::~GLShader()
{
    glDeleteProgram(Program);
}

void GLShader::Use()
{
    glUseProgram(Program);
}

void GLShader::SetTexture(const std::string& name, GLTexture& texture)
{
    constexpr int target = 0;

    glUseProgram(Program);
    glActiveTexture(GL_TEXTURE0 + target);
    GLint objectParam = glGetUniformLocation(Program, name.c_str());
    glUniform1i(objectParam, target);
    texture.Bind();

    glActiveTexture(GL_TEXTURE0);
}
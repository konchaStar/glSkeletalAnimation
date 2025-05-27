#include "ShaderProgram.h"

#include <fstream>
#include <glad/glad.h>
#include <sstream>

std::string loadShaderFile(std::string shaderPath) {
    std::ifstream shaderFile(shaderPath);
    if (!shaderFile) {
        return "";
    }
    std::stringstream buffer;
    buffer << shaderFile.rdbuf();
    return buffer.str();
}

ShaderProgram::ShaderProgram() {
}

ShaderProgram::~ShaderProgram() {
}


void ShaderProgram::loadFragmentShader(std::string shaderPath) {
    _fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    std::string shaderCode = loadShaderFile(shaderPath);
    const char* shaderPointer = shaderCode.c_str();
    glShaderSource(_fragmentShader, 1, &shaderPointer, NULL);
    glCompileShader(_fragmentShader);
}

void ShaderProgram::loadVertexShader(std::string shaderPath) {
    _vertexShader = glCreateShader(GL_VERTEX_SHADER);
    std::string shaderCode = loadShaderFile(shaderPath);
    const char* shaderPointer = shaderCode.c_str();
    glShaderSource(_vertexShader, 1, &shaderPointer, NULL);
    glCompileShader(_vertexShader);
}

void ShaderProgram::loadGeometryShader(std::string shaderPath) {
    _geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
    std::string shaderCode = loadShaderFile(shaderPath);
    const char* shaderPointer = shaderCode.c_str();
    glShaderSource(_geometryShader, 1, &shaderPointer, NULL);
    glCompileShader(_geometryShader);
}

void ShaderProgram::link() {
    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, _fragmentShader);
    glAttachShader(shaderProgram, _vertexShader);
    if (_geometryShader) {
        glAttachShader(shaderProgram, _geometryShader);
    }

    glLinkProgram(shaderProgram);

    glDeleteShader(_fragmentShader);
    glDeleteShader(_vertexShader);
    if (_geometryShader) {
        glDeleteShader(_geometryShader);
    }
    _fragmentShader = _vertexShader = _geometryShader = 0;
}

void ShaderProgram::use() {
    glUseProgram(shaderProgram);
}

void ShaderProgram::unbind() {
    glUseProgram(0);
}

void ShaderProgram::setFloat(const char *name, float value) {
    glUniform1f(glGetUniformLocation(shaderProgram, name), value);
}

void ShaderProgram::setInt(const char *name, int value) {
    glUniform1i(glGetUniformLocation(shaderProgram, name), value);
}

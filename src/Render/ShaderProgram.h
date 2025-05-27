#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H
#include <string>

class ShaderProgram {
private:
    unsigned int _vertexShader = 0;
    unsigned int _fragmentShader = 0;
    unsigned int _geometryShader = 0;

public:
    unsigned int shaderProgram;

    ShaderProgram();

    ~ShaderProgram();

    void loadVertexShader(std::string shaderPath);

    void loadFragmentShader(std::string shaderPath);

    void loadGeometryShader(std::string shaderPath);

    void link();

    void use();

    void setFloat(const char *name, float value);

    void setInt(const char *name, int value);

    static void unbind();
};

#endif

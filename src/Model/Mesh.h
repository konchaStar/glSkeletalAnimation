#ifndef MESH_H
#define MESH_H
#define GLM_ENABLE_EXPERIMENTAL
#include <string>
#include <vector>
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include <assimp/Scene.h>
#include "../Render/ShaderProgram.h"

#define MAX_BONE_INFLUENCE 4

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    float m_Weights[MAX_BONE_INFLUENCE];
};

struct Texture {
    unsigned int id;
    std::string type;
    aiString path;
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
    void Draw(ShaderProgram &shaderProgram);
private:
    unsigned int VAO, VBO, EBO;
    void setupMesh();
};

#endif

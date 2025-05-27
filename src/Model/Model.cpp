#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include "Model.h"
#include <glad/glad.h>
#include <iostream>
#include "../Utils/ImageLoader/stb_image.h"
#include "../Utils/Logger.h"

void Model::Draw(ShaderProgram &shader) {
    for (auto &mesh: meshes) {
        mesh.Draw(shader);
    }
}

void Model::loadModel(std::string path, unsigned int &numAnimations) {
    auto *importer = new Assimp::Importer();
    const aiScene *scene = importer->ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << importer->GetErrorString() << std::endl;
        return;
    }
    numAnimations = scene->mNumAnimations;
    Logger::info("ASSIMP", "mNumAnimations " + std::to_string(scene->mNumAnimations));
    Logger::info("ASSIMP", "mNumMeshes " + std::to_string(scene->mNumMeshes));
    directory = path.substr(0, path.find_last_of('\\'));

    processNode(scene->mRootNode, scene);
    delete importer;
}

void Model::loadModel(std::string path) {
    auto *importer = new Assimp::Importer();
    const aiScene *scene = importer->ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << importer->GetErrorString() << std::endl;
        return;
    }
    Logger::info("ASSIMP", "mNumAnimations " + std::to_string(scene->mNumAnimations));
    Logger::info("ASSIMP", "mNumMeshes " + std::to_string(scene->mNumMeshes));
    directory = path.substr(0, path.find_last_of('\\'));

    processNode(scene->mRootNode, scene);
    delete importer;
}

void Model::processNode(aiNode *node, const aiScene *scene) {
    Logger::info("MODEL", "Node " + std::string(node->mName.C_Str()));
    Logger::info("MODEL", "mNumMeshes " + std::to_string(node->mNumMeshes));
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    Logger::info("MESH", "mNumVertices " + std::to_string(mesh->mNumVertices));
    Logger::info("MESH", "mNumBones " + std::to_string(mesh->mNumBones));
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;

        setVertexBoneDataToDefault(vertex);

        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;
        if (mesh->mNormals) {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.normal = vector;
        } else {
            vertex.normal = glm::vec3(0.f);
        }

        if (mesh->mTangents) {
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.tangent = vector;
        } else {
            vertex.tangent = glm::vec3(0.f);
        }

        if (mesh->mBitangents) {
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.bitangent = vector;
        } else {
            vertex.bitangent = glm::vec3(0.f);
        }

        if (mesh->mTextureCoords[0]) {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoords = vec;
        } else {
            vertex.texCoords = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    if (mesh->mMaterialIndex >= 0) {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material,
                                                                aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        std::vector<Texture> specularMaps = loadMaterialTextures(material,
                                                                 aiTextureType_SPECULAR, "texture_specular");
        if (specularMaps.size() > 0)
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        std::vector<Texture> normalMaps = loadMaterialTextures(material,
                                                               aiTextureType_NORMALS, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        std::vector<Texture> roughnessMaps = loadMaterialTextures(material, aiTextureType_GLTF_METALLIC_ROUGHNESS,
                                                                  "texture_rm");
        textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());
        std::vector<Texture> emissionMaps = loadMaterialTextures(material,
                                                                 aiTextureType_EMISSIVE, "texture_emission");
        if (emissionMaps.size() > 0) {
            std::cout << emissionMaps[0].path.C_Str() << std::endl;
        }
        textures.insert(textures.end(), emissionMaps.begin(), emissionMaps.end());
    }

    extractBoneWeightForVertices(vertices, mesh);

    return Mesh(vertices, indices, textures);
}

unsigned int textureFromFile(const char *path, const std::string &directory) {
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << filename << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName) {
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        Texture texture;
        texture.id = textureFromFile(str.C_Str(), directory);
        texture.type = typeName;
        texture.path = str;
        textures.push_back(texture);
    }
    return textures;
}

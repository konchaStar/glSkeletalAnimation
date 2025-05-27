#ifndef ANIMATION_MODEL_H
#define ANIMATION_MODEL_H
#define GLM_ENABLE_EXPERIMENTAL

#include "Mesh.h"
#include "glm/glm.hpp"
#include "../Utils/AssimpHelper/assimp_glm_helpers.h"
#include <map>
#include <vector>
#include <iostream>

struct BoneInfo {
    int id;
    glm::mat4 offset;
};

class Model {
public:
    Model(const char *path, unsigned int &numAnimations) {
        loadModel(path, numAnimations);
    }

    Model(const char *path) {
        loadModel(path);
    }

    void Draw(ShaderProgram &shader);

    auto &GetBoneInfoMap() { return m_BoneInfoMap; }

    int &GetBoneCount() { return m_BoneCounter; }

    ~Model() {

    }

private:
    std::map<std::string, BoneInfo> m_BoneInfoMap;
    int m_BoneCounter = 0;
    std::vector<Mesh> meshes;
    std::string directory;

    void setVertexBoneDataToDefault(Vertex &vertex) {
        for (int i = 0; i < 4; i++) {
            vertex.m_BoneIDs[i] = -1;
            vertex.m_Weights[i] = 0.0f;
        }
    }

    void setVertexBoneData(Vertex &vertex, int boneID, float weight) {
        for (int i = 0; i < 4; ++i) {
            if (vertex.m_BoneIDs[i] < 0) {
                vertex.m_Weights[i] = weight;
                vertex.m_BoneIDs[i] = boneID;
                break;
            }
        }
    }

    void extractBoneWeightForVertices(std::vector<Vertex> &vertices, aiMesh *mesh) {
        for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
            int boneID = -1;
            std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
            if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end()) {
                BoneInfo newBoneInfo;
                newBoneInfo.id = m_BoneCounter;
                newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(
                        mesh->mBones[boneIndex]->mOffsetMatrix);
                m_BoneInfoMap[boneName] = newBoneInfo;
                boneID = m_BoneCounter;
                m_BoneCounter++;
            } else {
                boneID = m_BoneInfoMap[boneName].id;
            }
            assert(boneID != -1);
            auto weights = mesh->mBones[boneIndex]->mWeights;
            int numWeights = mesh->mBones[boneIndex]->mNumWeights;

            for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex) {
                int vertexId = weights[weightIndex].mVertexId;
                float weight = weights[weightIndex].mWeight;
                assert(vertexId <= vertices.size());
                setVertexBoneData(vertices[vertexId], boneID, weight);
            }
        }

    }

    void loadModel(std::string path, unsigned int &numAnimations);
    void loadModel(std::string path);

    void processNode(aiNode *node, const aiScene *scene);

    Mesh processMesh(aiMesh *mesh, const aiScene *scene);

    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                                              std::string typeName);
};

#endif

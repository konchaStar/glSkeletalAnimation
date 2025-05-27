#ifndef ANIMATION_ANIMATION_H
#define ANIMATION_ANIMATION_H


#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include "Bone.h"
#include "../Model/Model.h"
#include "assimp/postprocess.h"
#include "assimp/Importer.hpp"
#include "../Utils/Logger.h"

struct AssimpNodeData {
    glm::mat4 transformation;
    std::string name;
    int childrenCount;
    std::vector<AssimpNodeData> children;
};

class Animation {
private:
    float m_Duration;
    int m_TicksPerSecond;
    std::map<std::string, Bone> m_Bones;
    AssimpNodeData m_RootNode;
    std::map<std::string, BoneInfo> m_BoneInfoMap;
public:
    Animation() = default;

    Animation(const std::string &animationPath, Model *model, int index, std::string& name) {
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
        assert(scene && scene->mRootNode);
        if (index >= scene->mNumAnimations) {
            Logger::error("ANIMATION", "index");
        }
        auto animation = scene->mAnimations[index];
        name = animation->mName.C_Str();
        scene->mAnimations[index]->mName.C_Str();
        m_Duration = animation->mDuration;
        m_TicksPerSecond = animation->mTicksPerSecond;
        ReadHierarchyData(m_RootNode, scene->mRootNode);
        ReadMissingBones(animation, *model);
    }

    ~Animation() {
    }

    Bone *FindBone(const std::string &name) {
        if (m_Bones.find(name) == m_Bones.end()) return nullptr;
        else return &m_Bones.at(name);
    }


    inline float GetTicksPerSecond() { return m_TicksPerSecond; }

    inline float GetDuration() { return m_Duration; }

    inline const AssimpNodeData &GetRootNode() { return m_RootNode; }

    inline const std::map<std::string, BoneInfo> &GetBoneIDMap() {
        return m_BoneInfoMap;
    }

private:
    void ReadMissingBones(const aiAnimation *animation, Model &model) {
        int size = animation->mNumChannels;
        Logger::info("ANIMATION", "mNumChannels " + std::to_string(size));

        auto &boneInfoMap = model.GetBoneInfoMap();
        int &boneCount = model.GetBoneCount();
        for (int i = 0; i < size; i++) {
            auto channel = animation->mChannels[i];
            std::string boneName = channel->mNodeName.data;

            if (boneInfoMap.find(boneName) != boneInfoMap.end()) {
//                boneInfoMap[boneName].id = boneCount;
//                boneCount++;
                m_Bones.insert(std::make_pair(channel->mNodeName.data, Bone(channel->mNodeName.data,
                                                                            boneInfoMap[channel->mNodeName.data].id,
                                                                            channel)));
            } else {

            }
        }
        Logger::info("ANIMATION", "parsedBones " + std::to_string(m_Bones.size()));
        m_BoneInfoMap = boneInfoMap;
    }

    void ReadHierarchyData(AssimpNodeData &dest, const aiNode *src) {
        assert(src);

        dest.name = src->mName.data;
        dest.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
        dest.childrenCount = src->mNumChildren;

        for (int i = 0; i < src->mNumChildren; i++) {
            AssimpNodeData newData;
            ReadHierarchyData(newData, src->mChildren[i]);
            dest.children.push_back(newData);
        }
    }
};

#endif

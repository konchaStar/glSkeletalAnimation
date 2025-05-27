#ifndef ANIMATION_ANIMATOR_H
#define ANIMATION_ANIMATOR_H

#include <glm/matrix.hpp>
#include <thread>
#include "Animation.h"

using namespace std;

class Animator {
public:
    Animator(Animation *animation) {
        m_CurrentTime = 0.0;
        m_CurrentAnimation = animation;

        m_FinalBoneMeshMatrices.reserve(250);
        m_FinalBoneMatrices.reserve(250);

        for (int i = 0; i < 250; i++) {
            m_FinalBoneMeshMatrices.push_back(glm::mat4(1.0f));
            m_FinalBoneMatrices.push_back(glm::mat4(1.f));
        }
    }

    void UpdateAnimation(float dt, float speed, bool loop) {
        m_DeltaTime = dt * speed;
        if (m_CurrentAnimation) {
            m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * m_DeltaTime;
            m_CurrentTime = loop ? fmod(m_CurrentTime, m_CurrentAnimation->GetDuration()) : min(m_CurrentTime, m_CurrentAnimation->GetDuration() - 0.1f);
            glm::mat4 identity = glm::mat4(1.f);
            CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), identity);
        }
    }

    void PlayAnimation(Animation *pAnimation) {
        m_CurrentAnimation = pAnimation;
        m_CurrentTime = 0.0f;
    }

    void CalculateBoneTransform(const AssimpNodeData *node, glm::mat4 parentTransform) {
        std::string nodeName = node->name;
        glm::mat4 nodeTransform = node->transformation;

        Bone *Bone = m_CurrentAnimation->FindBone(nodeName);

        if (Bone) {
            Bone->Update(m_CurrentTime);
            nodeTransform = Bone->GetLocalTransform();
        }


        glm::mat4 globalTransformation = parentTransform * nodeTransform;

        auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
        if (boneInfoMap.find(nodeName) != boneInfoMap.end()) {
            int index = boneInfoMap[nodeName].id;
            glm::mat4 offset = boneInfoMap[nodeName].offset;
            m_FinalBoneMatrices[index] = globalTransformation;
            m_FinalBoneMeshMatrices[index] = globalTransformation * offset;
        }
        for (int i = 0; i < node->childrenCount; i++) {
            CalculateBoneTransform(&node->children[i], globalTransformation);
        }

    }

    std::vector<glm::mat4> getFinalBoneMeshMatrices() {
        return m_FinalBoneMeshMatrices;
    }

    std::vector<glm::mat4> getFinalBoneMatrices() {
        return m_FinalBoneMatrices;
    }

    float getCurrentTime() {
        return m_CurrentTime;
    }

    void setCurrentTime(float currentTime) {
        m_CurrentTime = currentTime;
    }
private:
    std::vector<glm::mat4> m_FinalBoneMeshMatrices;
    std::vector<glm::mat4> m_FinalBoneMatrices;
    Animation *m_CurrentAnimation;
    float m_CurrentTime;
    float m_DeltaTime;
};

#endif

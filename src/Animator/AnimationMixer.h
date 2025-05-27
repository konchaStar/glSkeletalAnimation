#ifndef ANIMATION_ANIMATIONMIXER_H
#define ANIMATION_ANIMATIONMIXER_H

#include "Animator.h"
#include "Animator.h"
#include <math.h>

class AnimationMixer {
private:
    Animator *animator1;
    Animator *animator2;
    float weight;
public:
    AnimationMixer(Animation *animation1, Animation *animation2, float weight = 0.5) {
        this->animator1 = new Animator(animation1);
        this->animator2 = new Animator(animation2);
        this->weight = max(min(weight, 1.f), 0.f);
    }

    void updateAnimations(float dt, float speed) {
        this->animator1->UpdateAnimation(dt, speed, true);
        this->animator2->UpdateAnimation(dt, speed, true);
    }

    void setAnimations(Animation* animation1, Animation* animation2) {
        this->animator1->PlayAnimation(animation1);
        this->animator2->PlayAnimation(animation2);
    }

    std::vector<glm::mat4> getFinalBoneMatrices() {
        return getFinalBoneMatrices(this->weight);
    }

    std::vector<glm::mat4> getFinalBoneMatrices(float weight) {
        weight = max(min(weight, 1.f), 0.f);
        std::vector<glm::mat4> bones1 = animator1->getFinalBoneMeshMatrices();
        std::vector<glm::mat4> bones2 = animator2->getFinalBoneMeshMatrices();
        std::vector<glm::mat4> finalBones;
        for(int i = 0; i < bones1.size(); i++) {
            finalBones.push_back(bones1[i] * (1 - weight) + bones2[i] * weight);
        }
        return finalBones;
    }

    ~AnimationMixer() {
        delete this->animator1;
        delete this->animator2;
    }
};

#endif

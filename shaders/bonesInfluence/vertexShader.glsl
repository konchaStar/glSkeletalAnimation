#version 460 core
layout (location = 0) in vec3 pos;
layout (location = 2) in vec2 texCoord;
layout (location = 5) in ivec4 boneIds;
layout (location = 6) in vec4 weights;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

const int MAX_BONES = 250;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];
uniform int boneId;

out vec2 fTexCoord;
out float fWeight;

void main() {
    vec4 totalPosition = vec4(0.f);
    fWeight = 0.f;
    for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
        if (boneIds[i] == -1)
            continue;
        if (boneIds[i] >= MAX_BONES) {
            totalPosition = vec4(pos, 1.0f);
            break;
        }
        if(boneIds[i] == boneId) {
            fWeight = weights[i];
        }
        vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(pos, 1.0f);
        totalPosition += localPosition * weights[i];
    }
    totalPosition /= dot(weights, vec4(1.0));
    gl_Position = proj * view * model * totalPosition;
    fTexCoord = texCoord;
}
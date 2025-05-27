#version 460 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;
layout (location = 5) in ivec4 boneIds;
layout (location = 6) in vec4 weights;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

const int MAX_BONES = 250;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

out vec2 fTexCoord;
out vec3 fNormal;
out vec3 fTangent;
out vec3 fBitangent;
out vec3 fPos;

void main() {
    vec4 totalPosition = vec4(0.f);
    vec3 totalNormal = vec3(0.f);
    vec3 totalTangent = vec3(0.f);
    vec3 totalBitangent = vec3(0.f);
    for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
        if (boneIds[i] == -1)
        continue;
        if (boneIds[i] >= MAX_BONES) {
            totalPosition = vec4(pos, 1.0f);
            totalNormal = normal;
            totalTangent = tangent;
            totalBitangent = bitangent;
            break;
        }
        vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(pos, 1.0f);
        vec3 localNormal = (finalBonesMatrices[boneIds[i]] * vec4(normal, 0.f)).xyz;
        vec3 localTangent = (finalBonesMatrices[boneIds[i]] * vec4(tangent, 0.f)).xyz;
        vec3 localBitangent = (finalBonesMatrices[boneIds[i]] * vec4(bitangent, 0.f)).xyz;
        totalTangent += localTangent * weights[i];
        totalBitangent += localBitangent * weights[i];
        totalNormal += localNormal * weights[i];
        totalPosition += localPosition * weights[i];
    }
    totalPosition /= dot(weights, vec4(1.0));
    fNormal = normalize((model * vec4(totalNormal, 0.f)).xyz);
    fTangent = normalize((model * vec4(totalTangent, 0.f)).xyz);
    fBitangent = normalize((model * vec4(totalBitangent, 0.f)).xyz);
    gl_Position = proj * view * model * totalPosition;
    fTexCoord = texCoord;
    fPos = (model * totalPosition).xyz;
}
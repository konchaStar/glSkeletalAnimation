#version 460 core

out vec4 FragColor;
in vec2 fTexCoord;
in vec3 fNormal;
in vec3 fTangent;
in vec3 fBitangent;
in vec3 fPos;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
//const vec3 lPos = vec3(5, 5, 5);
uniform vec3 lPos[4];
uniform vec3 lColor[4];
const int toonColorSteps = 3;
const float toonScaleFactor = 1.f / toonColorSteps;

void main() {
    vec4 diffuse = texture(texture_diffuse1, fTexCoord);
    diffuse = pow(diffuse, vec4(2.2f));
    vec3 tNormal = normalize(texture(texture_normal1, fTexCoord).xyz * 2 - vec3(1.f));
    vec3 N = normalize(fTangent * tNormal.x + fBitangent * tNormal.y + fNormal * tNormal.z);
    vec4 color = vec4(0.f);
    for(int i = 0; i < 4; i++) {
        vec3 L = normalize(fPos - lPos[i]);
        float intensity = max(0.f, dot(-L, N));
        intensity = ceil(intensity * toonColorSteps) * toonScaleFactor;
        color += diffuse * intensity * vec4(lColor[i], 1.f);
    }

    FragColor = pow(color, vec4(1 / 2.2f));
}
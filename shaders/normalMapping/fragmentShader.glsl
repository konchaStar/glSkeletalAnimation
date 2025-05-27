#version 460 core

out vec4 FragColor;
in vec2 fTexCoord;
in vec3 fNormal;
in vec3 fTangent;
in vec3 fBitangent;
in vec3 fPos;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
const vec3 lPos = vec3(5, 5, 5);

void main() {
    vec4 diffuse = texture(texture_diffuse1, fTexCoord);
    vec4 ambient = 0.03 * diffuse;
    vec3 tNormal = normalize(texture(texture_normal1, fTexCoord).xyz * 2 - vec3(1.f));
    vec3 N = normalize(fTangent * tNormal.x + fBitangent * tNormal.y + fNormal * tNormal.z);
    vec3 L = normalize(fPos - lPos);
    float intensity = max(0.f, dot(-L, N));
    FragColor = pow(pow(diffuse, vec4(2.2f)) * intensity, vec4(1 / 2.2f)) + ambient;
}
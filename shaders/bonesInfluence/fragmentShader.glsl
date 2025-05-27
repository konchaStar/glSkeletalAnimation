#version 460 core

out vec4 FragColor;
in vec2 fTexCoord;
in float fWeight;

uniform sampler2D heatMap;
const vec3 colors[] = { vec3(0, 0, 1), vec3(0, 1, 1), vec3(0, 1, 0), vec3(1, 1, 0), vec3(1, 0, 0) };

void main() {
    float w = fWeight * 4;
    int i = int(floor(w));
    int j = int(ceil(w));
    FragColor = vec4(mix(colors[i], colors[j], w - i), 1.f);

}
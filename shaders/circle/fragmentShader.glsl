#version 460 core

uniform int selected;

out vec4 FragColor;

void main() {
    if (selected == 0) {
        FragColor = vec4(1.f, 0.5f, 0.f, 0.f);
    } else {
        FragColor = vec4(0.f, 1.f, 0.f, 0.f);
    }
}
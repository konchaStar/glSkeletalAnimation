#version 460 core
out vec4 FragColor;

in vec2 TexCoords;
in vec2 FragPos;

uniform sampler2D screenTexture;
uniform float time;

const float scan = 0.5f;

void main()
{
    vec3 color = texture(screenTexture, TexCoords).rgb;

    float apply = abs(sin(FragPos.y * 200.f + time * 10) * 0.5f * scan);

    FragColor = vec4(mix(color, vec3(0), apply), 1.f);
}
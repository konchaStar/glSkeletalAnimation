#version 460 core
#define PI 3.1415926535897932384626433832795

out vec4 FragColor;
in vec2 fTexCoord;
in vec3 fNormal;
in vec3 fTangent;
in vec3 fBitangent;
in vec3 fPos;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_specular2;
uniform sampler2D texture_rm1;
uniform sampler2D texture_emission1;

uniform vec3 camera;
//const vec3 lPos[] = { vec3(5, 5, 5), vec3(-5, 5, 5), vec3(5, 5, -5), vec3(-5, 5, -5) };
//const vec3 lColor[] = { vec3(1.f, 0.5f, 0.5f), vec3(0.5f, 1.f, 0.5f), vec3(0.5f, 0.5f, 1.f), vec3(1.f, 0.5f, 1.f) };

uniform vec3 lPos[4];
uniform vec3 lColor[4];

vec3 fresnelSchlick(float VdotH, vec3 F0) {
    float t = 1 - VdotH;
    float t2 = t * t;
    float t5 = t2 * t2 * t;
    return F0 + (vec3(1.f) - F0) * t5;
}

float distribution(float NdotH, float roughness) {
    float a2 = roughness * roughness;
    float k = NdotH * NdotH * (a2 - 1) + 1;
    return min(1e8f, a2 / (PI * k * k));
}

float visibility(float NdotV, float NdotL, float roughness) {
    float a2 = roughness * roughness;
    float v = NdotL * sqrt(NdotV * NdotV * (1 - a2) + a2);
    float l = NdotV * sqrt(NdotL * NdotL * (1 - a2) + a2);
    return min(1e8f, 0.5f / (v + l));
}

vec4 fromLinear(vec4 linearRGB) {
    bvec3 cutoff = lessThanEqual(linearRGB.rgb, vec3(0.0031308));
    vec3 higher = vec3(1.055) * pow(linearRGB.rgb, vec3(1.0 / 2.4)) - vec3(0.055);
    vec3 lower = linearRGB.rgb * vec3(12.92);

    return vec4(mix(higher, lower, cutoff), linearRGB.a);
}

vec4 toLinear(vec4 sRGB) {
    bvec3 cutoff = lessThanEqual(sRGB.rgb, vec3(0.04045));
    vec3 higher = pow((sRGB.rgb + vec3(0.055)) / vec3(1.055), vec3(2.4));
    vec3 lower = sRGB.rgb / vec3(12.92);

    return vec4(mix(higher, lower, cutoff), sRGB.a);
}

const mat3 ACESInputMat = mat3(
0.59719, 0.07600, 0.02840,
0.35458, 0.90834, 0.13383,
0.04823, 0.01566, 0.83777
);

const mat3 ACESOutputMat = mat3(
1.60475, -0.10208, -0.00327,
-0.53108, 1.10813, -0.07276,
-0.07367, -0.00605, 1.07602
);

vec3 RRTAndODTFit(vec3 color) {
    vec3 a = color * (color + 0.0245786) - 0.000090537;
    vec3 b = color * (0.983729 * color + 0.4329510) + 0.238081;
    return a / b;
}

vec3 toneMapACES_Hill(vec3 color) {
    color = ACESInputMat * color;

    color = RRTAndODTFit(color);

    color = ACESOutputMat * color;

    color = clamp(color, 0.0, 1.0);

    return color;
}

void main() {
    vec3 baseColor = toLinear(texture(texture_diffuse1, fTexCoord)).xyz;
    vec3 specular = toLinear(texture(texture_specular2, fTexCoord)).xyz;
    vec3 rm = texture(texture_rm1, fTexCoord).xyz;
    vec3 emission = toLinear(texture(texture_emission1, fTexCoord)).xyz;

    vec3 tN = normalize(texture(texture_normal1, fTexCoord).xyz * 2 - vec3(1.f));
    vec3 N = normalize(fTangent * tN.x + fBitangent * tN.y + fNormal * tN.z);
    vec3 V = normalize(camera - fPos);

    float NdotV = max(dot(N, V), 0);

    vec3 F0 = mix(specular, baseColor, rm.b);
    float r2 = rm.g * rm.g;

    vec3 albedo = (1 - rm.b) * baseColor;
    vec3 diffuse = albedo / PI;

    vec3 color = 0.03 * baseColor + emission * 10;

    for (int i = 0; i < 4; i++) {

        vec3 L = normalize(lPos[i] - fPos);

        float NdotL = max(dot(N, L), 0);

        vec3 H = normalize(V + L);

        float NdotH = max(dot(N, H), 0);
        float VdotH = max(dot(V, H), 0);

        float distrib = distribution(NdotH, r2);
        float visib = visibility(NdotV, NdotL, r2);
        vec3 reflectance = fresnelSchlick(VdotH, F0);

        vec3 spec = reflectance * distrib * visib;
        float dist = distance(fPos, lPos[i]);
        vec3 irradiance = 500.f * lColor[i] * NdotL / (dist * dist);
        color += ((vec3(1.f) - reflectance) * diffuse + spec) * irradiance;
    }

    FragColor = fromLinear(vec4(toneMapACES_Hill(color * 0.5), 1.f));
}
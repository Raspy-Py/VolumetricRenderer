// type: fragment
#version 450

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform GlobalShaderData
{
    mat4 WorldToLocal;
    vec3 CameraPosition;
} gsd;
layout(binding = 2) uniform sampler2D texSampler;

vec2 IntersectAABB(vec3 rayOrigin, vec3 rayDir, vec3 boxMin, vec3 boxMax)
{
    vec3 tMin = (boxMin - rayOrigin) / rayDir;
    vec3 tMax = (boxMax - rayOrigin) / rayDir;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
    return vec2(tNear, tFar);
}

void main()
{
    vec3 cameraInBoxLocal = vec3(gsd.WorldToLocal * vec4(gsd.CameraPosition, 1));
    vec3 fragmentInBoxLocal = vec3(gsd.WorldToLocal * vec4(fragPosition, 1));
    vec3 rayDirection = normalize(fragmentInBoxLocal - cameraInBoxLocal);
    vec2 intersection = IntersectAABB(cameraInBoxLocal, rayDirection, vec3(-1,-1,-1), vec3(1,1,1));

    float density = 0.3;
    float beerLambert = 1 - exp(density * min(intersection.x - intersection.y, 0));
    vec4 color = vec4(0.3, 0.9, 0.2, 1.0);
    outColor = color * beerLambert;

}
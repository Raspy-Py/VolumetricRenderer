// type: fragment
#version 450

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform GlobalShaderData
{
    mat4 WorldToLocal;
    vec3 CameraPosition;
    mat4 MediaScroll;
} gsd;

layout(binding = 2) uniform sampler3D texSampler;

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

const float density = 1;
const int maxSteps = 128;
const vec3 boxMin = vec3(-1,-1,-1);
const vec3 boxMax = vec3( 1, 1, 1);

void main()
{
    vec3 cameraInBoxLocal = vec3(gsd.WorldToLocal * vec4(gsd.CameraPosition, 1));
    vec3 fragmentInBoxLocal = vec3(gsd.WorldToLocal * vec4(fragPosition, 1));
    vec3 rayDirection = normalize(fragmentInBoxLocal - cameraInBoxLocal);
    vec2 intersection = IntersectAABB(cameraInBoxLocal, rayDirection, boxMin, boxMax);

    // Points of ray-box intersection
    float stepSize = (1.0f / maxSteps) * 4;
    vec3 Pin = cameraInBoxLocal + rayDirection * intersection.x;
    vec3 Pout = cameraInBoxLocal + rayDirection * intersection.y;
    vec3 stepVec = stepSize * rayDirection;
    int actualSteps = min(maxSteps, int(distance(Pin, Pout) / stepSize));

    // Normilize points in range [0, 1]
    Pin -= boxMin;
    Pout -= boxMin;
    vec3 boxRange = abs(boxMax-boxMin);
    Pin /= boxRange;
    Pout /= boxRange;
    stepVec /= boxRange;
    vec3 accumDist = vec3(0);

    for (int i = 0; i < actualSteps; ++i)
    {
        //float distToCentre = distance(vec3(0.5, 0.5, 0.5), Pin) * 2;
        //if (distToCentre > 2) continue;

        ///float scale = max(0, 1 - pow(distToCentre, 4));
        float scale = 0.2;
        //vec4 volume = texture(texSampler, Pin*0.5 + vec3(0, 0, gsd.FrameTime)* 0.5);
        //float sample1 = texture(texSampler, Pin*0.99 + vec3(gsd.MediaScroll[0].x,gsd.MediaScroll[1].x,gsd.MediaScroll[2].x) * 0.01).x;
        float sample1 = texture(texSampler, Pin).x;
        float sample2 = texture(texSampler, Pin*0.8 + vec3(gsd.MediaScroll[0].y,gsd.MediaScroll[1].y,gsd.MediaScroll[2].y) * 0.2).y;
        float sample3 = texture(texSampler, Pin*0.75 + vec3(gsd.MediaScroll[0].z,gsd.MediaScroll[1].z,gsd.MediaScroll[2].z) * 0.25).z;
        float sample4 = texture(texSampler, Pin*0.7 + vec3(gsd.MediaScroll[0].w,gsd.MediaScroll[1].w,gsd.MediaScroll[2].w) * 0.3).w;
        //vec4 volume = texture(texSampler, Pin*0.2);
        float currentSample = (sample1 * sample2 ) * (sample3 + sample4) * scale;
        //currentSample = sample1 > 0.2 && sample2 > 0.3 ? currentSample : 0;
        accumDist += currentSample;
        Pin += stepVec;
    }
    accumDist *= stepSize;

    // Beer-Lambert
    vec3 color = vec3(1) - exp(vec3(density) * min(-accumDist, vec3(0,0,0)));
    outColor = vec4(color , 1.0);
}
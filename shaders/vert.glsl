// type: vertex
#version 450

layout(binding = 0) uniform ObjectShaderData
{
    mat4 Model;
    mat4 View;
    mat4 Projection;
} osd;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec2 fragTexCoord;

void main()
{
    gl_Position = osd.Projection * osd.View * osd.Model * vec4(inPosition, 1.0);
    fragPosition = vec3(osd.Model * vec4(inPosition, 1));
    fragTexCoord = inTexCoord;
}

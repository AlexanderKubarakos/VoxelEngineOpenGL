#version 460 core
layout (location = 0) in int aPos;

layout (std430, binding = 0) buffer Data {
    ivec4 d[];
} d_Data;

out vec3 fColor;
out vec3 normal;

uniform mat4 MVP;

float WorldScaler = 1;

vec3 normals[6] = {
    vec3(0,1,0),
    vec3(0,-1,0),
    vec3(0,0,1),
    vec3(0,0,-1),
    vec3(1,0,0),
    vec3(-1,0,0)
};

void main()
{
    int x = aPos & 31;
    int y = (aPos >> 5) & 31;
    int z = (aPos >> 10) & 31;
    vec3 augmentedPos = vec3(x * WorldScaler, y * WorldScaler, z * WorldScaler) + d_Data.d[gl_DrawID].xyz * 16 * WorldScaler;
    gl_Position = MVP * vec4(augmentedPos, 1.0);

    normal = normals[d_Data.d[gl_DrawID].w];

    fColor = vec3(1,1,1);
}
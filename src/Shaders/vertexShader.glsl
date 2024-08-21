#version 460 core
layout (location = 0) in int aPos;

layout (std430, binding = 0) buffer Data {
    ivec4 d[];
} d_Data;

out vec3 vertPos;
out vec3 color;
out vec3 normal;

uniform mat4 MVP;
uniform mat4 MV;

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
    int x = aPos & 63;
    int y = (aPos >> 6) & 63;
    int z = (aPos >> 12) & 63;
    vec3 augmentedPos = vec3(x * WorldScaler, y * WorldScaler, z * WorldScaler) + d_Data.d[gl_DrawID].xyz * 32 * WorldScaler;
    gl_Position = MVP * vec4(augmentedPos, 1.0);
    vec4 viewSpace = MV * vec4(augmentedPos, 1.0);

    vertPos = vec3(viewSpace) / viewSpace.w;
    normal = normals[d_Data.d[gl_DrawID].w];
    color = vec3(1,1,1);
}
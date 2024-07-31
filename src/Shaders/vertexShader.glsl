#version 460 core
layout (location = 0) in int aPos;

layout (std430, binding = 0) buffer Data {
    ivec4 d[];
} d_Data;

out vec3 fColor;
out vec3 normal;

uniform mat4 MVP;

void main()
{
    int x = aPos & 31;
    int y = (aPos >> 5) & 31;
    int z = (aPos >> 10) & 31;
    vec3 augmentedPos = vec3(x, y, z) + d_Data.d[gl_DrawID].xyz * 16;
    gl_Position = MVP * vec4(augmentedPos, 1.0);

    switch (d_Data.d[gl_DrawID].w)
    {
    case 0:
        normal = vec3(0,1,0);
        break;
     case 1:
        normal = vec3(0,-1,0);
        break;
    case 2:
        normal = vec3(0,0,1);
        break;
    case 3:
        normal = vec3(0,0,-1);
        break;
    case 4:
        normal = vec3(1,0,0);
        break;
    case 5:
        normal = vec3(-1,0,0);
    }

    fColor = vec3(1,1,1);
}
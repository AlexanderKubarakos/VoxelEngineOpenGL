#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (std430, binding = 0) buffer Data {
    vec4 d[];
} d_Data;

out vec3 fColor;

uniform mat4 MVP;


void main()
{
    vec3 augmentedPos = aPos + + d_Data.d[gl_DrawID].xyz * 16;
    gl_Position = MVP * vec4(augmentedPos, 1.0);
    fColor = aColor;
    vec3 temp = aNormal;
}
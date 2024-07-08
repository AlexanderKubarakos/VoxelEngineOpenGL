#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 MVP;
uniform vec3 ChunkPosition;

void main()
{
    vec3 augmentedPos = aPos + ChunkPosition * 16;
    gl_Position = MVP * vec4(augmentedPos, 1.0);
}
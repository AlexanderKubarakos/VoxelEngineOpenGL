#version 330 core
out vec4 FragColor;

in vec3 fColor;
in vec3 normal;

void main()
{
    FragColor = vec4(fColor, 1.0);

    if (normal.x > 0.9 && normal.x < 1.1)
    {
        FragColor = vec4(0,0,0,1);
    }
} 
#include "VBO.h"

#include <glad/glad.h>

VBO::VBO()
{
    glGenBuffers(1, &mID);
}

VBO::~VBO()
{
    glDeleteBuffers(1, &mID);
}

void VBO::Bind()
{   
    glBindBuffer(GL_ARRAY_BUFFER, mID);
}

void VBO::SetBufferData(const std::vector<float>& tVertices)
{
    Bind();
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(tVertices.size() * sizeof(float)), tVertices.data(), GL_STATIC_DRAW);
}

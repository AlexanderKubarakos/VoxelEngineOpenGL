#include "VAO.h"


VAO::VAO()
{
    glGenVertexArrays(1, &mID);
}

VAO::~VAO()
{
    glDeleteVertexArrays(1, &mID);
}

void VAO::Bind()
{   
    glBindVertexArray(mID);
}

void VAO::AddAttribute(unsigned int tIndex, int tSize, GLenum tType, GLboolean tNormalized, GLsizei tStride, unsigned int tOffsetPointer)
{
    Bind();
    glVertexAttribPointer(tIndex, tSize, tType, tNormalized, tStride, reinterpret_cast<void*>(tOffsetPointer));
    glEnableVertexAttribArray(0);  
}

#include "VAO.h"

#include <cassert>
#include <glad/glad.h>

#ifdef _DEBUG
#define BindCheck() \
	int i; \
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &i); \
	assert(i == static_cast<int>(mID));
#else
#define BindCheck() 
#endif

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

void VAO::AddAttribute(unsigned int tIndex, int tSize, GLenum tType, GLboolean tNormalized)
{
    BindCheck()
    glEnableVertexAttribArray(tIndex);
    glVertexAttribFormat(tIndex, tSize, tType, tNormalized, 0);
    glVertexAttribBinding(tIndex, tIndex);
}
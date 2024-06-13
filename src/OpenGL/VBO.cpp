#include "VBO.h"

#include <cassert>
#include <glad/glad.h>

#ifdef _DEBUG
	#define BindCheck() \
	int i; \
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &i); \
	assert(i == static_cast<int>(mID));
#else
	#define BindCheck() 
#endif

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

void VBO::BindBuffer(unsigned int tBindIndex, unsigned int tOffset, int tStride)
{
    glBindVertexBuffer(tBindIndex, mID, tOffset, tStride);
}

void VBO::SetBufferData(const std::vector<float>& tVertices)
{
    BindCheck()
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(tVertices.size() * sizeof(float)), tVertices.data(), GL_STATIC_DRAW);
}


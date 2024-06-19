#include "VBO.h"

VBO::VBO()
{
    glCreateBuffers(1, &m_Id);
}

VBO::~VBO()
{
    glDeleteBuffers(1, &m_Id);
}

void VBO::SetBufferData(const std::vector<float>& t_Vertices)
{
	glNamedBufferData(m_Id, static_cast<GLsizeiptr>(t_Vertices.size() * sizeof(float)), t_Vertices.data(), GL_STATIC_DRAW);
}


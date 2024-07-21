#include "Buffer.h"

Buffer::Buffer()
{
    glCreateBuffers(1, &m_Id);
}

Buffer::~Buffer()
{
    glDeleteBuffers(1, &m_Id);
}

void Buffer::SetBufferDataFloat(const float* t_Data, int t_Length) const
{
    glNamedBufferData(m_Id, static_cast<GLsizeiptr>(t_Length * sizeof(float)), t_Data, GL_STATIC_DRAW);
}

void Buffer::SetBufferDataFloat(const std::vector<float>& t_Data) const
{
	glNamedBufferData(m_Id, static_cast<GLsizeiptr>(t_Data.size() * sizeof(float)), t_Data.data(), GL_STATIC_DRAW);
}

void Buffer::SetBufferDataInt(const int* t_Data, int t_Length) const
{
    glNamedBufferData(m_Id, static_cast<GLsizeiptr>(t_Length * sizeof(int)), t_Data, GL_STATIC_DRAW);
}

void Buffer::SetBufferDataInt(const std::vector<int>& t_Data) const
{
    glNamedBufferData(m_Id, static_cast<GLsizeiptr>(t_Data.size() * sizeof(int)), t_Data.data(), GL_STATIC_DRAW);
}
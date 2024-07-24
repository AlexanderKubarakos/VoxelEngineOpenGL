#include "Buffer.h"

Buffer::Buffer()
{
    glCreateBuffers(1, &m_Id);
}

Buffer::~Buffer()
{
    glDeleteBuffers(1, &m_Id);
}

void Buffer::BindBuffer(GLbitfield t_BindPointFlag)
{
    glBindBuffer(t_BindPointFlag, m_Id);
}

void Buffer::SetupBufferStorage(size_t size, void* data, GLbitfield flags) const
{
	glNamedBufferStorage(m_Id, static_cast<int>(size), data, flags);
}

void* Buffer::MapBufferRange(size_t offset, size_t length, GLbitfield flags) const
{
    return glMapNamedBufferRange(m_Id, static_cast<int>(offset), static_cast<int>(length), flags);
}

#include "VAO.h"

#include <glad/glad.h>

VAO::VAO()
{
    glCreateVertexArrays(1, &m_Id);
}

VAO::~VAO()
{
    glDeleteVertexArrays(1, &m_Id);
}

void VAO::Bind()
{   
    glBindVertexArray(m_Id);
}

void VAO::AddAttribute(unsigned int t_AttributeIndex, unsigned int t_BindingIndex, int t_Size, GLenum t_Type, GLboolean t_Normalized, int t_RelativeOffset)
{
    glEnableVertexArrayAttrib(m_Id, t_AttributeIndex);
    glVertexArrayAttribFormat(m_Id, t_AttributeIndex, t_Size, t_Type, t_Normalized, t_RelativeOffset);
    glVertexArrayAttribBinding(m_Id, t_AttributeIndex, t_BindingIndex);
}

void VAO::BindVertexBuffer(const Buffer& t_Buffer, unsigned int t_BindingIndex, unsigned int t_Offset, int t_Stride)
{
    glVertexArrayVertexBuffer(m_Id, t_BindingIndex, t_Buffer.GetID(), t_Offset, t_Stride);
}

void VAO::BindElementBuffer(const Buffer& t_Buffer)
{
    glVertexArrayElementBuffer(m_Id, t_Buffer.GetID());
}

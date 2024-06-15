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

void VAO::AddAttribute(unsigned int t_AttributeIndex, unsigned int t_BindingIndex, int t_Size, GLenum t_Type, GLboolean t_Normalized)
{
    glEnableVertexArrayAttrib(m_Id, t_AttributeIndex);
    glVertexArrayAttribFormat(m_Id, t_AttributeIndex, t_Size, t_Type, t_Normalized, 0);
    glVertexArrayAttribBinding(m_Id, t_AttributeIndex, t_BindingIndex);
}

void VAO::BindVertexBuffer(VBO& t_VBO, unsigned int t_BindingIndex, unsigned int t_Offset, int t_Stride)
{
    glVertexArrayVertexBuffer(m_Id, t_BindingIndex, t_VBO.GetID(), t_Offset, t_Stride);
}

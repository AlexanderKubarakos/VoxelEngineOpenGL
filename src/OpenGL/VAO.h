#pragma once

#include <glad/glad.h>

#include "VBO.h"

class VAO
{
public:
    VAO();
    ~VAO();

    VAO(const VAO& other) = delete;
    VAO(VAO&& other) noexcept = delete;
    VAO& operator=(const VAO& other) = delete;
    VAO& operator=(VAO&& other) noexcept = delete;

    void Bind();
    void AddAttribute(unsigned int t_AttributeIndex, unsigned int t_BindingIndex, int t_Size, GLenum t_Type, GLboolean t_Normalized);
    void BindVertexBuffer(VBO& t_VBO, unsigned int t_BindIndex, unsigned int t_Offset, int t_Stride);
    unsigned int GetID() const { return m_Id; }
private:

    friend VBO;
    unsigned int m_Id{ 0 };
};

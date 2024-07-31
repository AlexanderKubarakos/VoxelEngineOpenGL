#pragma once

#include <glad/glad.h>

#include "Buffer.hpp"

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
    void AddAttributeFloat(unsigned int t_AttributeIndex, unsigned int t_BindingIndex, int t_Size, GLenum t_Type, GLboolean t_Normalized, int t_RelativeOffset);
    void AddAttributeInt(unsigned int t_AttributeIndex, unsigned int t_BindingIndex, int t_Size, GLenum t_Type, int t_RelativeOffset);
    void BindVertexBuffer(const Buffer& t_Buffer, unsigned int t_BindingIndex, unsigned int t_Offset, int t_Stride);
    void BindElementBuffer(const Buffer& t_Buffer);
    unsigned int GetID() const { return m_Id; }
private:
    unsigned int m_Id{ 0 };
};

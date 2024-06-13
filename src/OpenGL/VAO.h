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
    void AddAttribute(unsigned int tAttributeIndex, unsigned int tBindingIndex, int tSize, GLenum tType, GLboolean tNormalized);
private:

    friend VBO;
    unsigned int mID{ 0 };
};

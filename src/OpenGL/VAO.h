#pragma once

#include <glad/glad.h>

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
    void AddAttribute(unsigned int tIndex, int tSize, GLenum tType, GLboolean tNormalized, GLsizei tStride, unsigned int tOffsetPointer);
private:
    unsigned int mID{ 0 };
};

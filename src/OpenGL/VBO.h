#pragma once

#include <vector>

class VBO
{
public:
    VBO();
    ~VBO();

    VBO(const VBO& other) = delete;
    VBO(VBO&& other) noexcept = delete;
    VBO& operator=(const VBO& other) = delete;
    VBO& operator=(VBO&& other) noexcept = delete;

    void Bind();
    void SetBufferData(const std::vector<float>& tVertices);
private:
    unsigned int mID{ 0 };
};
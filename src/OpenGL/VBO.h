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

    void SetBufferData(const std::vector<float>& t_Vertices);
    unsigned int GetID() const { return m_Id; }
private:
    unsigned int m_Id{ 0 };
};
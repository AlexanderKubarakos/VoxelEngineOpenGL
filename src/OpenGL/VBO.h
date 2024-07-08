#pragma once

#include <vector>
#include <glad/glad.h>

class VBO
{
public:
    VBO();
    ~VBO();

    VBO(const VBO& t_Other) = delete;
    VBO(VBO&& t_Other) noexcept
	    : m_Id(t_Other.m_Id)
    {
        t_Other.m_Id = 0;
    }

    VBO& operator=(const VBO& t_Other) = delete;
    VBO& operator=(VBO&& t_Other) noexcept
    {
	    if (this == &t_Other)
		    return *this;
	    m_Id = t_Other.m_Id;
        t_Other.m_Id = 0;
	    return *this;
    }

    void SetBufferData(const std::vector<float>& t_Vertices);
    unsigned int GetID() const { return m_Id; }
private:
    unsigned int m_Id{ 0 };
};
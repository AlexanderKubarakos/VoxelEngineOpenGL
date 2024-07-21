#pragma once

#include <vector>
#include <glad/glad.h>

class Buffer
{
public:
    Buffer();
    ~Buffer();

    Buffer(const Buffer& t_Other) = delete;
    Buffer(Buffer&& t_Other) noexcept
	    : m_Id(t_Other.m_Id)
    {
        t_Other.m_Id = 0;
    }

    Buffer& operator=(const Buffer& t_Other) = delete;
    Buffer& operator=(Buffer&& t_Other) noexcept
    {
	    if (this == &t_Other)
		    return *this;
	    m_Id = t_Other.m_Id;
        t_Other.m_Id = 0;
	    return *this;
    }

    void SetBufferDataFloat(const std::vector<float>& t_Data) const;
    void SetBufferDataFloat(const float* t_Data, int t_Length) const;
    void SetBufferDataInt(const std::vector<int>& t_Data) const;
    void SetBufferDataInt(const int* t_Data, int t_Length) const;
    unsigned int GetID() const { return m_Id; }
private:
    unsigned int m_Id{ 0 };
};
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

    template<typename T>
    void SetBufferData(const std::vector<T>& t_Data, GLbitfield t_UsageFlags = GL_STATIC_DRAW) const
    {
        glNamedBufferData(m_Id, static_cast<GLsizeiptr>(t_Data.size() * sizeof(T)), t_Data.data(), t_UsageFlags);
    }
    template<typename T>
    void SetBufferData(const T* t_Data, size_t t_Length, GLbitfield t_UsageFlags = GL_STATIC_DRAW) const
    {
        glNamedBufferData(m_Id, static_cast<GLsizeiptr>(t_Length * sizeof(T)), t_Data, t_UsageFlags);
    }

    void BindBuffer(GLbitfield t_BindPointFlag);

    void SetupBufferStorage(size_t size, void* data, GLbitfield flags) const;
    void* MapBufferRange(size_t offset, size_t length, GLbitfield flags) const;
    unsigned int GetID() const { return m_Id; }
private:
    unsigned int m_Id{ 0 };
};
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

    // Well what's different with *BufferData* and *BufferStorage*, BufferData will send the data to the gpu and the gpu will create a new spot of memory for it
    // orphaning the old memory. Buffer Storage signs a contract with the GPU so that the memory you allocate on the gpu will stay there and can be changed
    // this is quicker however you can not change it's size
    // TLDR: prefer buffer storage over data
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
    void BindBufferBase(GLbitfield t_BindPointFlag, GLint t_Target);

    void SetupBufferStorage(size_t size, void* data, GLbitfield flags) const;
    void* MapBufferRange(size_t offset, size_t length, GLbitfield flags) const;
    unsigned int GetID() const { return m_Id; }
private:
    unsigned int m_Id{ 0 };
};
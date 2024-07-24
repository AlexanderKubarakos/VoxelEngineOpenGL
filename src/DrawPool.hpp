#pragma once
#include <deque>

#include "Vertex.hpp"
#include "OpenGL/Buffer.h"
#include "OpenGL/VAO.h"



class DrawPool
{
public:
	DrawPool(size_t t_BucketQuantity, size_t t_BucketSize);
	~DrawPool();

	DrawPool(const DrawPool& t_Other) = delete;
	DrawPool(DrawPool&& t_Other) noexcept = delete;
	DrawPool& operator=(const DrawPool& t_Other) = delete;
	DrawPool& operator=(DrawPool&& t_Other) noexcept = delete;

	typedef size_t* BucketID;
	BucketID AllocateBucket(int t_Size);
	void FillBucket(BucketID t_Id, const std::vector<Vertex>& t_Data);
	void Render();
private:
	struct DAIC
	{
		DAIC() = default;
		DAIC(GLuint t_IndicesCount, GLuint t_InstanceCount, GLuint t_FirstIndex, GLint t_BaseVertex,
			BucketID t_BucketID)
			: m_IndicesCount(t_IndicesCount),
			m_InstanceCount(t_InstanceCount),
			m_FirstIndex(t_FirstIndex),
			m_BaseVertex(t_BaseVertex),
			m_BaseInstance(0),
			m_BucketID(t_BucketID)
		{
		}

		GLuint m_IndicesCount; // how many indices to draw
		GLuint m_InstanceCount; // How many instances to draw
		GLuint m_FirstIndex; // First index to draw (start of index list)
		GLint m_BaseVertex; // First vertex (start of vertex buffer ie. start*)
		GLuint m_BaseInstance; // Not important

		BucketID m_BucketID;
	};

	size_t  m_BucketQuantity; // How many buckets
	size_t  m_BucketSize; // Size of each bucket

	std::deque<Vertex*> m_EmptyBuckets;
	Vertex* m_Start;

	VAO m_VAO;
	Buffer m_VertexBuffer;
	Buffer m_IndicesBuffer;
	Buffer m_IndirectCallBuffer;

	std::vector<DAIC> m_IndirectCallList;
	std::vector<GLuint> m_IndicesList;

	void Reserve(size_t t_BucketQuantity, size_t t_BucketSize);
	void UpdateIndirectCalls(); // Update indirect call buffer
	void GenerateIndices(); // Update index buffer, every 4 vertices we need to generate 6 indices
};

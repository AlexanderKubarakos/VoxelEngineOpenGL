#pragma once
#include <deque>
#include <array>

#include "glm/glm.hpp"
#include "Vertex.hpp"
#include "OpenGL/Buffer.hpp"
#include "OpenGL/VAO.hpp"
#include "Utilities.hpp"

#include "OpenGL/Shader.hpp"

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
	// Allocate memory in the pool for a mesh of vertex count = t_Size
	BucketID AllocateBucket(int t_Size);
	// Fill bucket t_Id with data for t_Data, will throw error if trying to over fill bucket
	void FillBucket(BucketID t_Id, const std::vector<Vertex>& t_Data, Utilities::DIRECTION t_MeshDirection, glm::ivec4& t_ExtraData);
	// Free a bucket, add it back to the queue to be filled, and delete its draw call
	void FreeBucket(BucketID t_Id);
	// Render all meshes in pool
	void Render(const glm::mat4& t_MVP, const glm::mat4& t_MV);
	// Show Debug Data
	void Debug();
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
			m_BucketID(t_BucketID),
			m_Direction(Utilities::DIRECTION::UP)
		{
		}

		GLuint m_IndicesCount; // how many indices to draw
		GLuint m_InstanceCount; // How many instances to draw
		GLuint m_FirstIndex; // First index to draw (start of index list)
		GLint m_BaseVertex; // First vertex relative to start, ie. 25 vertices in
		GLuint m_BaseInstance; // Not important

		// This pointer stores the index that this DAIC is kept at in the m_IndirectCallList, this pointer allows us to
		// update it's position in the vector, then update the value of the pointer, so that any point in the program a
		// "reference" can be kept to this DAIC in particular, even if it shifts in memory (like it can in a vector)
		BucketID m_BucketID;
		Utilities::DIRECTION m_Direction;
	};

	size_t m_BucketQuantity; // How many buckets
	size_t m_BucketSize; // Size of each bucket

	size_t m_DrawCallLength;

	std::deque<Vertex*> m_EmptyBuckets;
	Vertex* m_Start;

	VAO m_VAO;
	Buffer m_VertexBuffer;
	Buffer m_IndicesBuffer;
	Buffer m_IndirectCallBuffer;
	Buffer m_ExtraChunkDataBuffer;

	Shader m_Shader;
	GLint m_MVPUniformLocation;

	std::vector<DAIC> m_IndirectCallList;
	std::vector<glm::ivec4> m_ExtraChunkDataList;

	void Reserve(size_t t_BucketQuantity, size_t t_BucketSize); // Allocates memory in back end for pool
	void UpdateDrawCalls(const glm::mat4& t_MVP); // Resorts draw call (DAIC) and re-uploads all data to GPU so that draw calls are correct
	void GenerateIndices(); // Update index buffer, every 4 vertices we need to generate 6 indices

	// Debug Data
	std::array<bool, 6> m_SideOcclusionOverride;
};

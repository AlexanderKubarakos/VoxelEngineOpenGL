#include "DrawPool.hpp"

#include <iostream>

// This draw pool expects lists of vertices, and will automatically create the correct indices for every 4 vertices to make a quad
// aka this draw pool expects quads not triangles

static glm::vec3* ptr;

DrawPool::DrawPool(const size_t t_BucketQuantity, const size_t t_BucketSize)
{
	Reserve(t_BucketQuantity, t_BucketSize);
	GenerateIndices();
}

DrawPool::~DrawPool()
{
	for (DAIC& daic : m_IndirectCallList)
	{
		delete daic.m_BucketID;
	}
}

// t_Size is vertex count
DrawPool::BucketID DrawPool::AllocateBucket(const int t_Size)
{
	if (t_Size < 1 || t_Size > m_BucketSize || m_EmptyBuckets.empty())
		return nullptr;

	const size_t start = m_EmptyBuckets.back() - m_Start;
	m_IndirectCallList.emplace_back(t_Size/4 * 6, 1, 0, static_cast<GLint>(start), new size_t(m_IndirectCallList.size()));
	m_EmptyBuckets.pop_back();

	return m_IndirectCallList.back().m_BucketID;
}

void DrawPool::FillBucket(BucketID t_Id, const std::vector<Vertex>& t_Data, glm::vec4& t_ExtraData)
{
	if (t_Data.size() > m_BucketSize)
	{
		std::cerr << "Error: Trying to over fill bucket. Bucket Length: " << m_BucketSize << " and Data Length: " << t_Data.size() << '\n';
	}
		
	// find start of the bucket for t_Id, m.BaseVertex is relative to m_Start
	Vertex* start = m_Start + m_IndirectCallList[*t_Id].m_BaseVertex;
	memcpy(start, t_Data.data(), t_Data.size() * sizeof(Vertex));

	//TODO: make sure these stay in sync
	m_ExtraChunkDataList[*t_Id] = t_ExtraData;

	UpdateIndirectCalls();
}

void DrawPool::FreeBucket(BucketID t_Id)
{
	if (t_Id == nullptr)
		return;

	// Push to the back of the queue where this bucket starts
	m_EmptyBuckets.push_front(m_Start + m_IndirectCallList[*t_Id].m_BaseVertex);

	std::swap(m_IndirectCallList[*t_Id], m_IndirectCallList.back());
	m_IndirectCallList.pop_back();
	*m_IndirectCallList[*t_Id].m_BucketID = *t_Id;
	delete t_Id; // Free the memory for the old DAIC pointer

	// TODO: maybe remove update indirect call here? just in case we want to delete a lot of buckets in same frame
	UpdateIndirectCalls();
}

// Bucket Size is vertex count
void DrawPool::Reserve(const size_t t_BucketQuantity, const size_t t_BucketSize)
{
	m_BucketQuantity = t_BucketQuantity;
	m_BucketSize = t_BucketSize;

	if (m_BucketSize % 4 != 0)
	{
		std::cerr << "Error: Pool bucket size is not a factor of 4\n";
	}

	// Force list to length so that we can index it
	// Might be clearer as a std::array
	m_ExtraChunkDataList.resize(t_BucketQuantity);

	// Setup VAO attributes
	m_VAO.AddAttribute(0, 0, 3, GL_FLOAT, GL_FALSE, 0); // Position Attribute
	m_VAO.AddAttribute(1, 0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3); // Normal Attribute
	m_VAO.AddAttribute(2, 0, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 6); // Color Attribute

	// Bind the buffer to the vao
	m_VAO.BindVertexBuffer(m_VertexBuffer, 0, 0, sizeof(Vertex));

	// Byte Size of Buffer, how many bytes is the pool
	size_t mapSize = t_BucketQuantity * t_BucketSize * sizeof(Vertex);
	// GL access flags for GL buffer, client can write to buffer, client buffer exists till destroyed, client and server are kept
	GLbitfield accessFlags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

	// Set up the vertex buffer to be persistent storage, buffers exist on the server
	m_VertexBuffer.SetupBufferStorage(mapSize, nullptr, accessFlags);

	// Map (allocate memory on client) memory that will be coherent (kept up to date) with server
	m_Start = static_cast<Vertex*>(m_VertexBuffer.MapBufferRange(0, mapSize, accessFlags));

	// Fill the queue of empty buckets with every bucket, each is a pointer to the start of the bucket, with t_BucketSize of memory
	for (size_t i = 0; i < m_BucketQuantity; i++)
	{
		// Add all bucket pointers to queue
		m_EmptyBuckets.push_front(m_Start + i * t_BucketSize);
	}
}

void DrawPool::UpdateIndirectCalls()
{
	m_IndirectCallBuffer.SetBufferData<DAIC>(m_IndirectCallList, GL_DYNAMIC_DRAW);
	m_ExtraChunkDataBuffer.SetBufferData<glm::vec4>(m_ExtraChunkDataList, GL_DYNAMIC_DRAW);
}

void DrawPool::GenerateIndices()
{
	std::vector<GLuint> indices;
	// Generates all the indices that are needed, every 4 vertices need 6 indices
	for (int j = 0; j < m_BucketSize/4; j++) {
		indices.push_back(j * 4 + 0);  //Triangle 1
		indices.push_back(j * 4 + 1);
		indices.push_back(j * 4 + 3);
		indices.push_back(j * 4 + 0);  //Triangle 2
		indices.push_back(j * 4 + 2);
		indices.push_back(j * 4 + 3);
	}

	m_IndicesBuffer.SetBufferData<GLuint>(indices);
}

void DrawPool::Render()
{
	m_VAO.Bind();
	m_IndicesBuffer.BindBuffer(GL_ELEMENT_ARRAY_BUFFER);
	m_IndirectCallBuffer.BindBuffer(GL_DRAW_INDIRECT_BUFFER);
	m_ExtraChunkDataBuffer.BindBufferBase(GL_SHADER_STORAGE_BUFFER, 0);

	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(m_IndirectCallList.size()), sizeof(DAIC));
}

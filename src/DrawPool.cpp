#include "DrawPool.hpp"

#include <algorithm>
#include <iostream>
#include "LinearAlgebra.hpp"

#include "imgui.h"

DrawPool::DrawPool(const size_t t_BucketQuantity, const size_t t_BucketSize) : m_Shader("src/Shaders/vertexShader.glsl", "src/Shaders/fragmentShader.glsl")
{
	Reserve(t_BucketQuantity, t_BucketSize);
	GenerateIndices();

	m_MVPUniformLocation = m_Shader.getUniformLocation("MVP");
	std::fill(m_SideOcclusionOverride.begin(), m_SideOcclusionOverride.end(), true);
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
	if (t_Size > m_BucketSize || m_EmptyBuckets.empty())
	{
		std::cerr << "Error: While trying to allocate bucket, out of buckets, or requesting no space in bucket. Buckets Left: " << m_EmptyBuckets.size() << '\n';
		return nullptr;
	}
		
	// TODO: handle empty buckets, dont waste a draw call
	// Perhaps have if the chunk mesh is empty dont even try to allocate bucket
	if (t_Size < 1)
		std::cout << "Trying to allocate an empty bucket, ok but like don't do that...\n";

	const size_t start = m_EmptyBuckets.back() - m_Start;
	m_IndirectCallList.emplace_back(t_Size/4 * 6, 1, 0, static_cast<GLint>(start), new size_t(m_IndirectCallList.size()));
	m_EmptyBuckets.pop_back();

	return m_IndirectCallList.back().m_BucketID;
}

void DrawPool::FillBucket(BucketID t_Id, const std::vector<Vertex>& t_Data, Utilities::DIRECTION t_MeshDirection, glm::ivec4& t_ExtraData)
{
	if (t_Data.size() > m_BucketSize)
	{
		std::cerr << "Error: Trying to over fill bucket. Bucket Length: " << m_BucketSize << " and Data Length: " << t_Data.size() << '\n';
	}
		
	// find start of the bucket for t_Id, m.BaseVertex is relative to m_Start
	Vertex* start = m_Start + m_IndirectCallList[*t_Id].m_BaseVertex;
	memcpy(start, t_Data.data(), t_Data.size() * sizeof(Vertex));

	t_ExtraData.w = t_MeshDirection;
	m_ExtraChunkDataList[*t_Id] = t_ExtraData;
	m_IndirectCallList[*t_Id].m_Direction = t_MeshDirection;
}

void DrawPool::FreeBucket(BucketID t_Id)
{
	if (t_Id == nullptr)
		return;
	// Push to the back of the queue where this bucket starts
	m_EmptyBuckets.push_front(m_Start + m_IndirectCallList[*t_Id].m_BaseVertex);

	// put the draw call we wish to remove to the back
	std::swap(m_IndirectCallList[*t_Id], m_IndirectCallList.back());
	std::swap(m_ExtraChunkDataList[*t_Id], m_ExtraChunkDataList.back());
	m_IndirectCallList.pop_back();
	m_ExtraChunkDataList.pop_back();
	*m_IndirectCallList[*t_Id].m_BucketID = *t_Id;
	delete t_Id; // Free the memory for the old DAIC pointer
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
	m_VAO.AddAttributeInt(0, 0, 1, GL_INT, 0); // Position Attribute

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

	m_VAO.Bind();
	m_IndicesBuffer.BindBuffer(GL_ELEMENT_ARRAY_BUFFER);
	m_IndirectCallBuffer.BindBuffer(GL_DRAW_INDIRECT_BUFFER);
	m_ExtraChunkDataBuffer.BindBufferBase(GL_SHADER_STORAGE_BUFFER, 0);
}

void DrawPool::UpdateDrawCalls(const glm::mat4& t_MVP)
{
	auto frustum = LinAlg::frustumExtraction(t_MVP);
	// Sort draw calls
	auto func = [&](DAIC& daic)->bool
		{
			if (!m_SideOcclusionOverride[daic.m_Direction])
				return false;
			
			return LinAlg::isChunkInFrustum(frustum, m_ExtraChunkDataList[*daic.m_BucketID]);
		};

	int M = 0;
	int J = m_IndirectCallList.size() - 1; //Backside Approach

	//Execute Sorting Pass
	while (M <= J) {
		while (func(m_IndirectCallList[M]) && M < J) M++;
		while (!func(m_IndirectCallList[J]) && M < J) J--;
		*m_IndirectCallList[M].m_BucketID = J;
		*m_IndirectCallList[J].m_BucketID = M;
		std::swap(m_ExtraChunkDataList[M], m_ExtraChunkDataList[J]);
		std::swap(m_IndirectCallList[M++], m_IndirectCallList[J--]);
	}

	// Tell GPU to only run draw calls for the DAICs on left of the partition. All others won't be drawn
	m_DrawCallLength = M;
	m_IndirectCallBuffer.SetBufferData<DAIC>(m_IndirectCallList, GL_DYNAMIC_DRAW);
	m_ExtraChunkDataBuffer.SetBufferData<glm::ivec4>(m_ExtraChunkDataList, GL_DYNAMIC_DRAW);
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

void DrawPool::Render(const glm::mat4& t_MVP, const glm::mat4& t_MV)
{
	UpdateDrawCalls(t_MVP);

	m_Shader.Use();
	m_Shader.SetMatrix4f(m_MVPUniformLocation, t_MVP);
	m_Shader.SetMatrix4f(m_Shader.getUniformLocation("MV"), t_MV);
	m_VAO.Bind();
	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(m_DrawCallLength), sizeof(DAIC));
}

void DrawPool::Debug()
{
	if (ImGui::CollapsingHeader("Draw Pool", ImGuiTreeNodeFlags_None))
	{
		ImGui::Text("Bucket Quantity: %i", m_BucketQuantity);
		ImGui::Text("Bucket Size (Vertices): %i", m_BucketSize);
		ImGui::Text("Indirect Draw Call count: (%i/%i)", m_DrawCallLength, m_IndirectCallList.size());
		ImGui::Text("Occlusion Override");
		ImGui::Checkbox("Up", &m_SideOcclusionOverride[Utilities::DIRECTION::UP]);
		ImGui::Checkbox("Down", &m_SideOcclusionOverride[Utilities::DIRECTION::DOWN]);
		ImGui::Checkbox("North", &m_SideOcclusionOverride[Utilities::DIRECTION::NORTH]);
		ImGui::Checkbox("South", &m_SideOcclusionOverride[Utilities::DIRECTION::SOUTH]);
		ImGui::Checkbox("East", &m_SideOcclusionOverride[Utilities::DIRECTION::EAST]);
		ImGui::Checkbox("West", &m_SideOcclusionOverride[Utilities::DIRECTION::WEST]);
	}
}

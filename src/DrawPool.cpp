#include "DrawPool.hpp"

#include <algorithm>
#include <iostream>
#include "LinearAlgebra.hpp"

#include "imgui.h"

DrawPool::DrawPool(const size_t t_BucketQuantity, const size_t t_BucketSize) : m_Shader("src/Shaders/vertexShader.glsl", "src/Shaders/fragmentShader.glsl"), m_BackFaceCulling{true}, m_CullingMaster{true}
{
	Reserve(t_BucketQuantity, t_BucketSize);

	m_MVPUniformLocation = m_Shader.getUniformLocation("MVP");
	m_MVUniformLocation = m_Shader.getUniformLocation("MV");
	std::fill(m_SideOcclusionOverride.begin(), m_SideOcclusionOverride.end(), true);
}

DrawPool::~DrawPool()
{
	for (DAIC& daic : m_IndirectCallList)
	{
		delete daic.m_BucketID;
	}
}

// t_Size is vertex count, face count now
DrawPool::BucketID DrawPool::AllocateBucket(const int t_Size)
{
	ZoneScoped;
	if (t_Size > m_BucketSize || m_EmptyBuckets.empty())
	{
		ERROR_PRINT("Error: While trying to allocate bucket, out of buckets, or requesting no (0) space in bucket. Buckets Left: " << m_EmptyBuckets.size())
		return nullptr;
	}

	if (t_Size < 1)
	{
		ERROR_PRINT("Trying to allocate an empty bucket, not allocating a bucket, DAIC or BucketID")
		return nullptr;
	}

	const size_t start = m_EmptyBuckets.back() - m_Start;
	m_IndirectCallList.emplace_back(t_Size * 6, 1, static_cast<GLint>(start), new size_t(m_IndirectCallList.size()));
	m_IndirectCallList.back().m_OpenGLFirstVertex = static_cast<GLuint>(m_BucketSize * 6 * (start / m_BucketSize));
	m_EmptyBuckets.pop_back();

	return m_IndirectCallList.back().m_BucketID;
}

void DrawPool::FillBucket(BucketID t_Id, const std::vector<FaceVertex>& t_Data, Utilities::DIRECTION t_MeshDirection, glm::ivec4& t_ExtraData)
{
	ZoneScoped;
	if (t_Data.size() > m_BucketSize)
	{
		ERROR_PRINT("Error: Trying to over fill bucket. Bucket Length: " << m_BucketSize << " and Data Length: " << t_Data.size())
		return;
	}

	if (t_Id == nullptr)
	{
		ERROR_PRINT("Warning: Trying to fill a bucket with a nullptr ID, perhaps intentional")
		return;
	}

	// find start of the bucket for t_Id, m.BaseVertex is relative to m_Start
	FaceVertex* start = m_Start + m_IndirectCallList[*t_Id].m_VertexPoolPosition;
	memcpy(start, t_Data.data(), t_Data.size() * sizeof(FaceVertex));

	t_ExtraData.w = t_MeshDirection;
	m_ExtraChunkDataList[*t_Id] = t_ExtraData;
	m_IndirectCallList[*t_Id].m_Direction = t_MeshDirection;
	m_IndirectCallList[*t_Id].m_VertexCount = static_cast<GLuint>(t_Data.size() * 6);
}

void DrawPool::FreeBucket(BucketID t_Id)
{
	ZoneScoped;
	if (t_Id == nullptr)
		return;
	// Push to the back of the queue where this bucket starts
	m_EmptyBuckets.push_front(m_Start + m_IndirectCallList[*t_Id].m_VertexPoolPosition);

	// put the draw call we wish to remove to the back
	std::swap(m_IndirectCallList[*t_Id], m_IndirectCallList.back());
	std::swap(m_ExtraChunkDataList[*t_Id], m_ExtraChunkDataList[m_IndirectCallList.size()-1]);
	*m_IndirectCallList[*t_Id].m_BucketID = *t_Id;
	m_IndirectCallList.pop_back();
	delete t_Id; // Free the memory for the old DAIC pointer
}

void DrawPool::FreeBucket(std::array<DrawPool::BucketID, 6>& t_Buckets)
{
	ZoneScoped;
	for (auto& id : t_Buckets)
	{
		FreeBucket(id);
		id = nullptr;
	}
}

// Bucket Size is vertex count
void DrawPool::Reserve(const size_t t_BucketQuantity, const size_t t_BucketSize)
{
	ZoneScoped;
	m_BucketQuantity = t_BucketQuantity;
	m_BucketSize = t_BucketSize;

	if (m_BucketSize % 4 != 0)
	{
		std::cerr << "Error: Pool bucket size is not a factor of 4\n";
	}

	// Force list to length so that we can index it
	// Might be clearer as a std::array
	m_ExtraChunkDataList.resize(t_BucketQuantity);

	// Byte Size of Buffer, how many bytes is the pool
	size_t mapSize = t_BucketQuantity * t_BucketSize * sizeof(FaceVertex);
	// GL access flags for GL buffer, client can write to buffer, client buffer exists till destroyed, client and server are kept
	GLbitfield accessFlags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

	// Set up the vertex buffer to be persistent storage, buffers exist on the server
	m_VertexBuffer.SetupBufferStorage(mapSize, nullptr, accessFlags);

	// Map (allocate memory on client) memory that will be coherent (kept up to date) with server
	m_Start = static_cast<FaceVertex*>(m_VertexBuffer.MapBufferRange(0, mapSize, accessFlags));
	assert((mapSize / 1024.0f / 1024.0f / 1024.0f) < 2.0f);

	if (m_Start == nullptr)
	{
		ERROR_PRINT("FATAL ERROR: Failed to allocate memory on GPU, trying to allocate " << mapSize/1024/1024 << "MB\n")
	}

	// Fill the queue of empty buckets with every bucket, each is a pointer to the start of the bucket, with t_BucketSize of memory
	for (size_t i = 0; i < m_BucketQuantity; i++)
	{
		// Add all bucket pointers to queue
		m_EmptyBuckets.push_front(m_Start + i * t_BucketSize);
	}

	m_VAO.Bind();
	m_IndirectCallBuffer.BindBuffer(GL_DRAW_INDIRECT_BUFFER);
	m_ExtraChunkDataBuffer.BindBufferBase(GL_SHADER_STORAGE_BUFFER, 0);
	m_VertexBuffer.BindBufferBase(GL_SHADER_STORAGE_BUFFER, 1);
}

void DrawPool::UpdateDrawCalls(const glm::mat4& t_MVP, const Camera& camera)
{
	ZoneScoped;
	if (!m_CullingMaster)
		return;
	auto frustum = LinAlg::frustumExtraction(t_MVP);
	// Sort draw calls

	auto shouldRender = [&](DAIC& daic)->bool
		{
			if (!m_SideOcclusionOverride[daic.m_Direction])
				return false;

			// Cameras position, looking vector
			glm::ivec4 chunkPos = m_ExtraChunkDataList[*daic.m_BucketID];

			if (!LinAlg::isChunkInFrustum(frustum, chunkPos))
				return false;

			glm::vec3 chunkPosFloat = chunkPos * 32 + 16;

			static glm::vec3 normals[6] = {
				glm::vec3(0,1,0),
				glm::vec3(0,-1,0),
				glm::vec3(0,0,1),
				glm::vec3(0,0,-1),
				glm::vec3(1,0,0),
				glm::vec3(-1,0,0)
			};

			glm::vec3 normal = normals[chunkPos.w];
			if (m_BackFaceCulling && glm::dot(normal, (chunkPosFloat - 16.0f * normal) - camera.CameraPos()) > 0)
				return false;

			return true;
		};

	int M = 0;
	int J = static_cast<int>(m_IndirectCallList.size() - 1);

	//Execute Sorting Pass
	while (M <= J) {
		while (shouldRender(m_IndirectCallList[M]) && M < J) M++;
		while (!shouldRender(m_IndirectCallList[J]) && M < J) J--;
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

void DrawPool::Render(const glm::mat4& t_MVP, const glm::mat4& t_MV, const Camera& camera)
{
	ZoneScoped;
	UpdateDrawCalls(t_MVP, camera);
	m_Shader.Use();
	m_Shader.SetMatrix4f(m_MVPUniformLocation, t_MVP);
	m_Shader.SetMatrix4f(m_MVUniformLocation, t_MV);
	m_VAO.Bind();
	glMultiDrawArraysIndirect(GL_TRIANGLES, nullptr, static_cast<GLsizei>(m_DrawCallLength), sizeof(DAIC));
}

void DrawPool::Debug()
{
	ZoneScoped;
	if (ImGui::CollapsingHeader("Draw Pool", ImGuiTreeNodeFlags_None))
	{
		ImGui::Text("Bucket Quantity: %i", m_BucketQuantity);
		ImGui::Text("Bucket Size (Faces): %i", m_BucketSize);
		ImGui::Text("DrawPool Size (KB): %i", m_BucketQuantity * m_BucketSize * sizeof(FaceVertex) / 1024);
		ImGui::Text("DrawPool Used Size (KB): %i", (m_BucketQuantity - m_EmptyBuckets.size()) * m_BucketSize * sizeof(FaceVertex) / 1024);
		ImGui::Text("Indirect Draw Call count: (%i/%i)", m_DrawCallLength, m_IndirectCallList.size());
		ImGui::Text("Culling Settings");
		ImGui::Checkbox("Any Culling", &m_CullingMaster);
		ImGui::Checkbox("Back Face Culling", &m_BackFaceCulling);
		ImGui::Checkbox("Up", &m_SideOcclusionOverride[Utilities::DIRECTION::UP]);
		ImGui::Checkbox("Down", &m_SideOcclusionOverride[Utilities::DIRECTION::DOWN]);
		ImGui::Checkbox("North", &m_SideOcclusionOverride[Utilities::DIRECTION::NORTH]);
		ImGui::Checkbox("South", &m_SideOcclusionOverride[Utilities::DIRECTION::SOUTH]);
		ImGui::Checkbox("East", &m_SideOcclusionOverride[Utilities::DIRECTION::EAST]);
		ImGui::Checkbox("West", &m_SideOcclusionOverride[Utilities::DIRECTION::WEST]);
	}
}

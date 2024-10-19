#pragma once

#include "Chunk.hpp"
#include "OpenGL/camera.hpp"

#include <vector>
#include <deque>
#include <queue>
#include <unordered_map>
#include <shared_mutex>

#include "AtomicQueue.h"
#include "ChunkMap.hpp"
struct MeshData
{
	MeshData() = default;
	MeshData(const std::array<std::vector<FaceVertex>, 6>& t_FaceData, const glm::ivec3& t_Chunk) : m_FaceData(t_FaceData), m_Chunk(t_Chunk) {}
	std::array<std::vector<FaceVertex>, 6> m_FaceData;
	glm::ivec3 m_Chunk;
};
// Manages a list of chunks, allowing removal and adding
// and all rendering functionality too
class ChunkManager
{
public:
	ChunkManager();
	~ChunkManager();

	// Load/Unload around player
	void LoadUnloadAroundPlayer(const Camera& camera);
	// Render all chunks in the draw pool
	void RenderChunks(const Camera& t_Camera, const glm::mat4& t_Projection);

	// Create debug info for ImGui
	void ShowDebugInfo();

	ChunkManager(const ChunkManager& t_Other) = delete;
	ChunkManager(ChunkManager&& t_Other) noexcept = delete;
	ChunkManager& operator=(const ChunkManager& t_Other) = delete;
	ChunkManager& operator=(ChunkManager&& t_Other) noexcept = delete;

private:
	void ProcessChunks(const glm::vec3& t_PlayerPosition);
	DrawPool m_DrawPool;
	ChunkMap m_Chunks;
	bool m_LoadUnloadChunks;
	int m_ViewDistance;

	// Threading
	std::shared_mutex m_Mutex;
	AtomicQueue<glm::ivec3> m_ChunksToRemove;
	AtomicQueue<std::shared_ptr<Chunk>> m_ChunksToAdd;
	AtomicQueue<glm::ivec3> m_ChunksToMesh;
	AtomicQueue<MeshData> m_MeshDataToProcesses;

	void ThreadedUnloadAndLoad(const Camera& camera);
	std::thread m_Thread, m_ThreadMeshing;
	void ThreadedMeshing();
	void MeshUp(int8_t* t_NeighborData, std::vector<FaceVertex>& t_Data, const glm::ivec3& t_ToMesh);
	void MeshDown(int8_t* t_NeighborData, std::vector<FaceVertex>& t_Data, const glm::ivec3& t_ToMesh);
	void MeshSouth(int8_t* t_NeighborData, std::vector<FaceVertex>& t_Data, const glm::ivec3& t_ToMesh);
	void MeshNorth(int8_t* t_NeighborData, std::vector<FaceVertex>& t_Data, const glm::ivec3& t_ToMesh);
	void MeshEast(int8_t* t_NeighborData, std::vector<FaceVertex>& t_Data, const glm::ivec3& t_ToMesh);
	void MeshWest(int8_t* t_NeighborData, std::vector<FaceVertex>& t_Data, const glm::ivec3& t_ToMesh);
};
#pragma once

#include "Chunk.hpp"
#include "OpenGL/camera.hpp"

#include <vector>
#include <deque>
#include <queue>
#include <unordered_map>

#include "ChunkMap.hpp"

// Manages a list of chunks, allowing removal and adding
// and all rendering functionality too
class ChunkManager
{
public:
	ChunkManager();
	~ChunkManager();

	// Add a Chunk
	void AddChunk(const glm::ivec3& t_ChunkPosition);
	// Remove a Chunk
	void RemoveChunk(const glm::ivec3& t_ChunkPosition);
	// Load/Unload around player
	void LoadUnloadAroundPlayer(const Camera& t_Camera);
	// Mesh all chunks that are in the queue
	void MeshChunks();
	// Render all chunks in the draw pool
	void RenderChunks(const Camera& t_Camera, const glm::mat4& t_Projection);

	// Create debug info for ImGui
	void ShowDebugInfo();

	ChunkManager(const ChunkManager& t_Other) = delete;
	ChunkManager(ChunkManager&& t_Other) noexcept = delete;
	ChunkManager& operator=(const ChunkManager& t_Other) = delete;
	ChunkManager& operator=(ChunkManager&& t_Other) noexcept = delete;

private:
	ChunkMap::iterator RemoveChunk(ChunkMap::iterator& t_Iterator);
	// Get a chunk from the list
	ChunkMap::iterator GetChunk(const glm::ivec3& t_ChunkPosition);
	// Mesh a chunk
	void MeshChunk(const glm::ivec3& t_ToMesh);

	DrawPool m_DrawPool;
	ChunkMap m_Chunks;
	std::deque<glm::ivec3> m_MeshingQueue;
	bool m_LoadUnloadChunks;
	int m_ViewDistance;

	// Threading
	std::thread m_ChunkLoadingUnloadingThread;
	std::mutex m_ChunkMapMutex;
	std::queue<std::array<DrawPool::BucketID, 6>> m_UnloadQueue;
	void ThreadedUnloadAndLoad(const Camera& t_Camera, std::mutex& t_Mutex, ChunkMap& t_ChunkMap,
		std::queue<std::array<DrawPool::BucketID, 6>>& t_UnloadQueue);
};
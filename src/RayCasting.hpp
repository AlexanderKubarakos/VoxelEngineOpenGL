#pragma once

#include "ChunkMap.hpp"
#include "Opengl/camera.hpp"
#include "Utilities.hpp"

#include "glm/glm.hpp"

namespace RayCasting
{
	class Hit
	{
	public:
		bool hit{ false };
		glm::ivec3 m_Chunk{};
		glm::ivec3 m_Voxel{};

		explicit operator bool() const
		{
			return hit;
		}
	};
	// Casts a ray through the world [t_Map] from Camera [t_Camera]
	Hit CastRay(const ChunkMap& t_Map, const Camera& t_Camera)
	{
		Hit hit;
		constexpr float maxLength = 3000.0f;

		// Ray direction
		glm::vec3 rayDir = glm::normalize(t_Camera.CameraFront());
		// Ray Start
		glm::vec3 rayStart = t_Camera.GetAtomicCameraPos();

		int stepX = rayDir.x > 0 ? 1 : -1;
		int stepY = rayDir.y > 0 ? 1 : -1;
		int stepZ = rayDir.z > 0 ? 1 : -1;

		int voxelX = static_cast<int>(rayStart.x);
		int voxelY = static_cast<int>(rayStart.y);
		int voxelZ = static_cast<int>(rayStart.z);

		float tDeltaX = stepX / rayDir.x;
		float tDeltaY = stepY / rayDir.y;
		float tDeltaZ = stepZ / rayDir.z;

		float tMaxX = (rayDir.x > 0 ? (std::ceil(rayStart.x) - rayStart.x) : (rayStart.x - std::floor(rayStart.x))) / abs(rayDir.x);
		float tMaxY = (rayDir.y > 0 ? (std::ceil(rayStart.y) - rayStart.y) : (rayStart.y - std::floor(rayStart.y))) / abs(rayDir.y);
		float tMaxZ = (rayDir.z > 0 ? (std::ceil(rayStart.z) - rayStart.z) : (rayStart.z - std::floor(rayStart.z))) / abs(rayDir.z);

		float distance = 0.0f;

		glm::uvec3 currentChunkPosition = t_Map.begin()->first;
		auto* currentChunkData = t_Map.begin()->second->m_BlockData;

		// Step till ray hits or max length is reached
		while (distance < maxLength)
		{
			if (tMaxX < tMaxY)
			{
				if (tMaxX < tMaxZ)
				{
					voxelX += stepX;
					tMaxX += tDeltaX;
					distance += tDeltaX;
				}
				else
				{
					voxelZ += stepZ;
					tMaxZ += tDeltaZ;
					distance += tDeltaZ;
				}
			}
			else
			{
				if (tMaxY < tMaxZ)
				{
					voxelY += stepY;
					tMaxY += tDeltaY;
					distance += tDeltaY;
				}
				else
				{
					voxelZ += stepZ;
					tMaxZ += tDeltaZ;
					distance += tDeltaZ;
				}
			}

			// If the ray has moved out of the world in some direction
			if (voxelX < 0 || voxelY < 0 || voxelZ < 0)
			{
				return hit;
			}

			// If the ray has moved to a different chunk than last iteration
			if (currentChunkPosition.x != voxelX / 32 ||
				currentChunkPosition.y != voxelY / 32 ||
				currentChunkPosition.z != voxelZ / 32)
			{
				// Moved over chunk boarder therefore find the new chunk
				currentChunkPosition.x = voxelX / 32;
				currentChunkPosition.y = voxelY / 32;
				currentChunkPosition.z = voxelZ / 32;

				// Find the new chunk
				auto chunk = t_Map.find(glm::ivec3(voxelX / 32, voxelY / 32, voxelZ / 32));
				if (chunk == t_Map.end())
				{
					return hit;
				}

				currentChunkData = chunk->second->m_BlockData;
			}

			// [x & 31] is optimized [x % 32], leaving up to compiler implement shifting for multiplication
			// if the current voxel is not air [0] we hit something
			if (currentChunkData[(voxelX & 31) + (voxelY & 31) * 32 + (voxelZ & 31) * 32 * 32] != 0)
			{
				hit.hit = true;
				hit.m_Chunk = glm::ivec3(voxelX / 32, voxelY / 32, voxelZ / 32);
				hit.m_Voxel = glm::ivec3(voxelX & 31, voxelY & 31, voxelZ & 31);
				return hit;
			}
		}

		return hit;
	}
}

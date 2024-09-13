#pragma once
#include <unordered_map>

#include "Chunk.hpp"

#include "glm/glm.hpp"

struct VecFunc
{
    size_t operator()(const glm::ivec3& k)const
    {
        return std::hash<int>()(k.x) ^ (std::hash<int>()(k.y) << 1) ^ (std::hash<int>()(k.z) << 2);
    }

    bool operator()(const glm::ivec3& a, const glm::ivec3& b)const
    {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    }
};

typedef std::unordered_map <glm::ivec3, Chunk, VecFunc, VecFunc> ChunkMap;
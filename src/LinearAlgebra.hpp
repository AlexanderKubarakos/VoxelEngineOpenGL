#pragma once
#include "glm/glm.hpp"
#include <array>

namespace LinAlg
{
	struct Plane
	{
		glm::vec4 normal;
	};

	struct Frustum
	{
		std::array<Plane, 6> planes;
	};

	float signedDistance(const Plane& t_Plane, const glm::vec3& t_Point);
	bool isChunkInFrustum(const Frustum& t_Frustum, const glm::vec3& t_Pos);
	Frustum frustumExtraction(const glm::mat4& t_PV);
}

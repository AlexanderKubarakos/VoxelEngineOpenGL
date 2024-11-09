#include "LinearAlgebra.hpp"

#include "glm/gtc/matrix_access.hpp"

float LinAlg::signedDistance(const Plane& t_Plane, const glm::vec3& t_Point)
{
	return glm::dot(t_Plane.normal, glm::vec4(t_Point, 1));
}

bool LinAlg::isChunkInFrustum(const Frustum& t_Frustum, const glm::vec3& t_Pos)
{
	float radius = 96.0f;
	glm::vec3 centerSphere = t_Pos * 32.0f + 8.0f;

	// chuck if every point (j) is outside the same plane (i)
	for (int i = 0; i < 6; i++)
	{
		if (!(-signedDistance(t_Frustum.planes[i], centerSphere) < radius))
			return false;
	}
	return true;
}

LinAlg::Frustum LinAlg::frustumExtraction(const glm::mat4& t_PV)
{
	Frustum frustum{};

	// Left clipping plane
	frustum.planes[0].normal = glm::row(t_PV, 3) + glm::row(t_PV, 0);
	// Right clipping plane
	frustum.planes[1].normal = glm::row(t_PV, 3) - glm::row(t_PV, 0);
	// Top clipping plane
	frustum.planes[2].normal = glm::row(t_PV, 3) + glm::row(t_PV, 1);
	// Bottom clipping plane
	frustum.planes[3].normal = glm::row(t_PV, 3) - glm::row(t_PV, 1);
	// Near clipping plane
	frustum.planes[4].normal = glm::row(t_PV, 3) + glm::row(t_PV, 2);
	// Far clipping plane
	frustum.planes[5].normal = glm::row(t_PV, 3) - glm::row(t_PV, 2);

	return frustum;
}
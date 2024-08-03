#include "LinearAlgebra.hpp"

#include "glm/gtc/matrix_access.hpp"

float LinAlg::signedDistance(const Plane& t_Plane, const glm::vec3& t_Point)
{
	return glm::dot(t_Plane.normal, glm::vec4(t_Point, 1));
}

bool LinAlg::isChunkInFrustum(const Frustum& t_Frustum, const glm::vec3& t_Pos)
{
	std::array<glm::vec3, 8> points =
	{
		{
			{t_Pos.x, t_Pos.y, t_Pos.z},
			{t_Pos.x + 16, t_Pos.y, t_Pos.z},
			{t_Pos.x, t_Pos.y + 16, t_Pos.z},
			{t_Pos.x + 16, t_Pos.y + 16, t_Pos.z},
			{t_Pos.x, t_Pos.y, t_Pos.z + 16},
			{t_Pos.x + 16, t_Pos.y, t_Pos.z + 16},
			{t_Pos.x, t_Pos.y + 16, t_Pos.z + 16},
			{t_Pos.x + 16, t_Pos.y + 16, t_Pos.z + 16},
		},
	};

	// chuck if every point (j) is outside the same plane (i)
	for (int i = 0; i < 6; i++)
	{
		int out = 0;
		for (int j = 0; j < 8; j++)
		{
			out += signedDistance(t_Frustum.planes[i], points[j]) < 0 ? 1 : 0;
		}
		if (out == 8)
			return false;
	}

	return true;
}

LinAlg::Frustum LinAlg::frustumExtraction(const glm::mat4& t_PV)
{
	Frustum frustum;

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
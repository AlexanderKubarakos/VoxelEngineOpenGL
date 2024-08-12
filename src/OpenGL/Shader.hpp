#pragma once

#include <string>

#include <fstream>

#include <glm/fwd.hpp>

#include "glad/glad.h"

class Shader
{
public:
	Shader(const char* t_VertexPath, const char* t_FragmentPath);

	GLint getUniformLocation(const std::string& t_Uniform) const;
	void SetBool(GLint uniform, bool value) const;
	void SetInt(GLint uniform, int value) const;
	void SetFloat(GLint uniform, float value) const;
	void SetVec3(GLint uniform, glm::vec3& value) const;
	void SetMatrix4f(GLint uniform, const glm::mat4& matrix) const;

	void Use() const;
private:
	unsigned int m_Id;
};

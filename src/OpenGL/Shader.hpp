#pragma once

#include <string>

#include <fstream>

#include <glm/fwd.hpp>

class Shader
{
public:
	Shader(const char* t_VertexPath, const char* t_FragmentPath);

	void SetBool(const std::string& uniform, bool value) const;
	void SetInt(const std::string& uniform, int value) const;
	void SetFloat(const std::string& uniform, float value) const;
	void SetVec3(const std::string& uniform, glm::vec3& value);
	void SetMatrix4f(const std::string& uniform, glm::mat4& matrix);

	void Use() const;
private:
	unsigned int m_Id;
};

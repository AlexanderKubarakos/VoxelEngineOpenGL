#pragma once



#include <string>

#include <fstream>

#include <glm/fwd.hpp>

class Shader
{
public:
	unsigned int mID;

	Shader(const char* vertexPath, const char* fragmentPath);

	void SetBool(const std::string& uniform, bool value) const;
	void SetInt(const std::string& uniform, int value) const;
	void SetFloat(const std::string& uniform, float value) const;
	void SetMatrix4f(const std::string& uniform, glm::mat4& matrix);

	private:
	void Use() const;
};

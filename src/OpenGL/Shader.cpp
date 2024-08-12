#include "Shader.hpp"

#include <glad/glad.h>
#include <iostream>
#include <sstream>

#include "glm/gtc/type_ptr.hpp"

Shader::Shader(const char* t_VertexPath, const char* t_FragmentPath)
{
	// Crash info if shader doesn't compile
	int  success;
	char infoLog[512];

	// 1. retrieve the vertex/fragment source code from filePath
	std::string vertexCode;
	std::string fragmentCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	// ensure ifstream objects can throw exceptions:
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		// open files
		vShaderFile.open(t_VertexPath);
		fShaderFile.open(t_FragmentPath);
		std::stringstream vShaderStream, fShaderStream;
		// read file's buffer contents into streams
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		// close file handlers
		vShaderFile.close();
		fShaderFile.close();
		// convert stream into string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
	catch (std::ifstream::failure const& e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << '\n';
		(void)e.what();
	}
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();

	// create new vertex shader, like all openGL objects it's refers to by an m_Id
	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	// Import shader code to the vertexShader object, then compile it
	glShaderSource(vertexShader, 1, &vShaderCode, nullptr);
	glCompileShader(vertexShader);

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

	// print debug on failure
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << '\n';
	}

	// Create new fragmentShader and compile
	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fShaderCode, nullptr);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << '\n';
	}

	// A shader program amalgamates shaders and is the thing we use for render calls, aka manages our shaders
	m_Id = glCreateProgram();

	// Attach our shaders, linking them allows them to run on the current shader, ie. vertex shader
	glAttachShader(m_Id, vertexShader);
	glAttachShader(m_Id, fragmentShader);
	glLinkProgram(m_Id);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// Print error on failure
	glGetProgramiv(m_Id, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(m_Id, 512, nullptr, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINK_FAILED\n" << infoLog << '\n';
	}
}

void Shader::Use() const
{
	glUseProgram(m_Id);
}

GLint Shader::getUniformLocation(const std::string& t_Uniform) const
{
	return glGetUniformLocation(m_Id, t_Uniform.c_str());
}

void Shader::SetBool(GLint t_Uniform, bool value) const
{
	glUniform1i(t_Uniform, (int)value);
}

void Shader::SetInt(GLint t_Uniform, int value) const
{
	glUniform1i(t_Uniform, value);
}

void Shader::SetFloat(GLint t_Uniform, float value) const
{
	glUniform1f(t_Uniform, value);
}

void Shader::SetVec3(GLint t_Uniform, glm::vec3& value) const
{
	glUniform3f(t_Uniform, value.x, value.y, value.z);
}

void Shader::SetMatrix4f(GLint t_Uniform, const glm::mat4& matrix) const
{
	glUniformMatrix4fv(t_Uniform, 1, GL_FALSE, glm::value_ptr(matrix));
}
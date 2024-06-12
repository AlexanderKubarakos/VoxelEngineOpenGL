#include "Shader.h"

#include <glad/glad.h>
#include <iostream>
#include <sstream>

#include "glm/gtc/type_ptr.hpp"

Shader::Shader(const char* tVertexPath, const char* tFragmentPath)
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
		vShaderFile.open(tVertexPath);
		fShaderFile.open(tFragmentPath);
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

	// create new vertex shader, like all openGL objects it's refers to by an mID
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
	mID = glCreateProgram();

	// Attach our shaders, linking them allows them to run on the current shader, ie. vertex shader
	glAttachShader(mID, vertexShader);
	glAttachShader(mID, fragmentShader);
	glLinkProgram(mID);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// Print error on failure
	glGetProgramiv(mID, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(mID, 512, nullptr, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINK_FAILED\n" << infoLog << '\n';
	}
}

void Shader::Use() const
{
	glUseProgram(mID);
}

void Shader::SetBool(const std::string& uniform, bool value) const
{
	glUniform1i(glGetUniformLocation(mID, uniform.c_str()), (int)value);
}
void Shader::SetInt(const std::string& uniform, int value) const
{
	glUniform1i(glGetUniformLocation(mID, uniform.c_str()), value);
}
void Shader::SetFloat(const std::string& uniform, float value) const
{
	glUniform1f(glGetUniformLocation(mID, uniform.c_str()), value);
}
void Shader::SetMatrix4f(const std::string& uniform, glm::mat4& matrix)
{
	glUniformMatrix4fv(glGetUniformLocation(mID, uniform.c_str()), 1, GL_FALSE, glm::value_ptr(matrix));
}
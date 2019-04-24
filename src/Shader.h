#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

#include"glad/glad.h"
#include"GLFW/glfw3.h"

#include "xy_calc.h"
#include "xy_ext.h"

class Shader
{
public:
	//程序ID
	GLuint Program;
	//构造器读取并构建着色器
	Shader() {};
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath);
	//使用程序
	void Use();
	void Delete();
};
void ShaderVersion();

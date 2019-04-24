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
	//����ID
	GLuint Program;
	//��������ȡ��������ɫ��
	Shader() {};
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath);
	//ʹ�ó���
	void Use();
	void Delete();
};
void ShaderVersion();

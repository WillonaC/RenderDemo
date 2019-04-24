#include "Shader.h"

Shader::Shader(const GLchar* vertexPath, const GLchar* fragmentPath)
{
	//1.从文件路径中获取顶点着色器和片段着色器
	std::string vertexCode;
	std::string fragmentCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	//保证ifstream对象可以抛出异常：
	vShaderFile.exceptions(std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::badbit);
	try
	{
		//打开文件
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, fShaderStream;
		//读取文件的缓冲内容到流中
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		//关闭文件
		vShaderFile.close();
		fShaderFile.close();
		//转换流至GLchar数组
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}
	const GLchar* vShaderCode = vertexCode.c_str();
	const GLchar* fShaderCode = fragmentCode.c_str();

	//2.编译着色器
	GLuint vertex = glCreateShader(GL_VERTEX_SHADER);	//顶点着色器对象
	glShaderSource(vertex, 1, &vShaderCode, NULL);	//把着色器源码附加到着色器对象上
													//参数1：要编译的着色器对象；参数2：传递的源码字符串数量；参数3：源码；参数4：先设置为NULL
	glCompileShader(vertex);

	//检测着色器编译是否成功
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}


	GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);	//片段着色器对象
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);

	//检测着色器编译是否成功
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	//着色器程序
	this->Program = glCreateProgram();
	//把之前编译的着色器附加到程序对象上
	glAttachShader(this->Program, vertex);
	glAttachShader(this->Program, fragment);
	//链接
	glLinkProgram(this->Program);

	//检测链接程序是否成功
	glGetProgramiv(this->Program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(this->Program, 512, NULL, infoLog);
		std::cout << "ERROR::PROGRAMME::LINKING_FAILED\n" << infoLog << std::endl;
	}

	//链接之后，删除着色器对象
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

void Shader::Use()
{
	glUseProgram(this->Program);
}

void Shader::Delete()
{
	glDeleteProgram(this->Program);
}

void ShaderVersion()
{
	const GLubyte *renderer = glGetString(GL_RENDERER);
	const GLubyte *vendor = glGetString(GL_VENDOR);
	const GLubyte *version = glGetString(GL_VERSION);
	const GLubyte *glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
	GLint major, minor;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	std::cout << "GL Vendor    :" << vendor << std::endl;
	std::cout << "GL Renderer  : " << renderer << std::endl;
	std::cout << "GL Version (string)  : " << version << std::endl;
	std::cout << "GL Version (integer) : " << major << "." << minor << std::endl;
	std::cout << "GLSL Version : " << glslVersion << std::endl;
}
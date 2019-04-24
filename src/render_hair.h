#ifndef __RENDERHAIR__
#define __RENDERHAIR__
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>
#include <iomanip>

#include"glad/glad.h"
#include"GLFW/glfw3.h"

#include "tiny_obj_loader.h"
#include "xy_calc.h"
#include "xy_ext.h"
#include "xy_opengl.h"
#include "Shader.h"
#include "stb/stb_image.h"
#include "tool.h"

#define PI 3.14159265358979323846

class RenderHair
{
public:
	RenderHair() {};
	void LoadObjFile();
	void LoadCameraPos();
	void LoadCameraRot();
	void loadImages();
	void recordCameras();
	void render_hair();
private:
	//// OpenGL GLFW 全局变量
	GLFWwindow* window;
	int	WindowWidth = 800;
	int WindowHeight = 800;

	//// Obj Model 全局变量
	char* ObjFilePath = "E:/RenderDemo/asset/img/model_hair/";
	std::string objFileName = "E:/RenderDemo/asset/model/model_hair/";
	std::string modelName = "head.obj";
	std::string indFileName = "hair.ind";
	std::string shaderFilePath = "E:/RenderDemo/shader/";

	//// reference views
	const int inter = 1;
	unsigned int NumCameraPos = 1250 / inter;
	std::vector<std::vector<float>> CameraPos = std::vector<std::vector<float>>(NumCameraPos, std::vector<float>(3));
	std::vector<std::vector<float>> Angles = std::vector<std::vector<float>>(NumCameraPos, std::vector<float>(3));
	std::vector<std::vector<float>> RotMat = std::vector<std::vector<float>>(NumCameraPos, std::vector<float>(9));
	//float CameraPos[NumCameraPos][3];	// camera positions
	//float Angles[NumCameraPos][3];		// rotation angles of x/y/z axis
	//float RotMat[NumCameraPos][9];		// rot = rotz * roty * rotx
	float *camposes, *camrots;

	//// light & camera
	/// ref cameras' RT in 3dmax, vertex in Maya
	/// so convert vertex from Maya to 3dmax, including visual camera
	xy::vec3 camAimRender = xy::vec3(0.0, 13.5, -0.0);//(0.0, 0.0, 13.5);	//in 3dmax  (0.0, 13.5, -0.0)in maya
													  //-14.780400 -2.785890 13.500000 //view16 3dmax cam1.031.png
	xy::vec3 tmp = xy::vec3(-14.780400, 13.500000, 2.785890);
	xy::vec3 camPos = tmp - camAimRender;

	//// model rotation & translation
	float ModelRotate_X = 0.0f;
	float ModelRotate_Y = 0.0f;
	float ModelRotate_Z = 0.0f;
	float ModelTranslate_X = 0;
	float ModelTranslate_Y = 0;
	float ModelTranslate_Z = 0;
	float objScale = 2.54;

	////// accelerate
	int gridHeight = 25, gridWidth = 50;
	int windowSize = 5;
	std::vector<std::vector<int>> grid_cams = std::vector<std::vector<int>>(gridHeight * gridWidth);
	std::vector<std::vector<int>> grid_refs = std::vector<std::vector<int>>(gridHeight * gridWidth);
	//std::vector<int> grid_cams[gridHeight * gridWidth];
	//std::vector<int> grid_refs[gridHeight * gridWidth];
	int max_num = 0;
	xy::vec3 cam_center = xy::vec3{ 0,0,0 };
	int *gridRefCams;

	//// Shader 全局变量
	Shader ShaderProgram;//顶点着色器对象
	GLuint alphaTexture;
	ObjAsset obj_asset;
	ObjGAsset objG_asset;
	HairAsset hair_asset;
	HairGAsset hairG_asset;

	//// time fps
	GLfloat deltaTime = 0.0f;	//当前帧与上一帧的时间差
	GLfloat lastFrame = 0.0f;	//上一帧的时间
};
#endif
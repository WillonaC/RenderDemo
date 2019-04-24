#ifndef __RENDERMESH__
#define __RENDERMESH__
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>
#include <iomanip>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "xy_calc.h"
#include "xy_ext.h"
#include "xy_opengl.h"
#include "Shader.h"
#include "tiny_obj_loader.h"
#include "stb/stb_image.h"
#include "tool.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#define PI 3.14159265358979323846

class RenderMesh
{
public:
	RenderMesh() {};
	void LoadObjFile();
	void LoadCameraPos();
	void LoadCameraRot();
	void LoadCameraIntr();
	void loadImages();
	void render_mesh();
private:
	//// OpenGL GLFW 全局变量
	GLFWwindow* window;
	int	WindowWidth = 400 * 2;
	int WindowHeight = 600 * 2;

	//// Obj Model 全局变量
	char* ObjFilePath = "F:/[A]project/RenderDemo/asset/img/model_mesh/";
	std::string objFileName = "F:/[A]project/RenderDemo/asset/model/model_mesh/";
	std::string modelName = "2_agi_mask.obj";
	std::string shaderFilePath = "F:/[A]project/RenderDemo/shader/";

	//// reference views
	const int inter = 1;
	const unsigned int NumCameraPos = 56 / inter;
	std::vector<std::vector<float>> CameraPos = std::vector<std::vector<float>>(NumCameraPos * inter, std::vector<float>(3));
	std::vector<std::vector<float>> RotMat = std::vector<std::vector<float>>(NumCameraPos * inter, std::vector<float>(9));
	std::vector<std::vector<float>> Intr = std::vector<std::vector<float>>(NumCameraPos * inter, std::vector<float>(9));
	//float CameraPos[NumCameraPos * inter][3];	// camera positions
	//float RotMat[NumCameraPos * inter][9];		// rot = rotz * roty * rotx
	//float Intr[NumCameraPos * inter][9];
	//float *camposes = new float[NumCameraPos * 3], *camrots = new float[NumCameraPos * 9], *camintrs = new float[NumCameraPos * 9];
	float *camposes, *camrots, *camintrs;

	//// light & camera
	xy::vec3 ttt = xy::vec3(-95.847135, -135.543380, -46.748149);
	xy::vec3 objCenter;

	//// model rotation & translation
	float ModelRotate_X = 0.0f;
	float ModelRotate_Y = 0.0f;
	float ModelRotate_Z = 0.0f;
	float ModelTranslate_X = 0.0;
	float ModelTranslate_Y = 0.0;
	float ModelTranslate_Z = 0.0;

	//// Shader 全局变量
	Shader ShaderProgram;//顶点着色器对象  
	GLuint rgbTexture, alphaTexture;
	ObjAsset obj_asset;
	ObjGAsset objG_asset;

	//// time fps
	GLfloat deltaTime = 0.0f;	//当前帧与上一帧的时间差
	GLfloat lastFrame = 0.0f;	//上一帧的时间
};

#endif
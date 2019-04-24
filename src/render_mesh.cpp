#include "render_mesh.h"

void RenderMesh::LoadObjFile()
{

	obj_asset = MakeObjAsset(objFileName + modelName, objFileName);
	objG_asset = MakeObjGAsset(obj_asset);
}

void RenderMesh::LoadCameraPos()
{
	std::ifstream inf;
	std::ostringstream camPosfilepath;
	camPosfilepath << ObjFilePath << "cameraPosition.txt";
	inf.open(camPosfilepath.str());
	if (!inf)
	{
		std::cout << "cannot open cameraPosition file" << std::endl;
		return;
	}
	std::string str;
	std::string pattern = " ";
	int ind = 0;
	while (getline(inf, str))
	{
		std::vector<std::string> result = Tool::split(str, pattern);

		for (int i = 0; i < result.size(); i++)
		{
			CameraPos[ind][i] = atof(result[i].c_str());
		}
		ind++;
		if (ind == NumCameraPos*inter)
			break;
	}

	camposes = new float[NumCameraPos * 3];
	for (int i = 0; i < NumCameraPos; i++)
	{
		camposes[i * 3] = CameraPos[i * inter][0];
		camposes[i * 3 + 1] = CameraPos[i * inter][1];
		camposes[i * 3 + 2] = CameraPos[i * inter][2];
	}
}

void RenderMesh::LoadCameraRot()
{
	std::ifstream inf;
	std::ostringstream camRotfilepath;
	camRotfilepath << ObjFilePath << "cameraRotation.txt";
	inf.open(camRotfilepath.str());
	if (!inf)
	{
		std::cout << "cannot open cameraRotation file" << std::endl;
		return;
	}
	std::string str;
	std::string pattern = " ";
	int ind = 0;
	while (getline(inf, str))
	{
		std::vector<std::string> result = Tool::split(str, pattern);

		for (int i = 0; i < result.size(); i++)
		{
			RotMat[ind][i] = atof(result[i].c_str());
		}
		ind++;
		if (ind == NumCameraPos * inter)
			break;
	}

	camrots = new float[NumCameraPos * 9];
	for (int i = 0; i < NumCameraPos; i++)
		for (int j = 0; j < 9; j++)
			camrots[i * 9 + j] = RotMat[i*inter][j];
}

void RenderMesh::LoadCameraIntr()
{
	std::ifstream inf;
	std::ostringstream camIntrfilepath;
	camIntrfilepath << ObjFilePath << "intri.txt";
	inf.open(camIntrfilepath.str());
	if (!inf)
	{
		std::cout << "cannot open cameraIntrinsic file" << std::endl;
		return;
	}
	std::string str;
	std::string pattern = " ";
	int ind = 0;
	while (getline(inf, str))
	{
		std::vector<std::string> result = Tool::split(str, pattern);
		for (int i = 0; i < result.size(); i++)
		{
			Intr[ind][i] = atof(result[i].c_str());
		}
		ind++;
		if (ind == NumCameraPos * inter)
			break;
	}

	camintrs = new float[NumCameraPos * 9];
	for (int i = 0; i < NumCameraPos; i++)
		for (int j = 0; j < 9; j++)
			camintrs[i * 9 + j] = Intr[i*inter][j];
}

void RenderMesh::loadImages()
{
	std::ostringstream tmp;
	tmp << ObjFilePath << "(1).jpg";
	int width_, height_, num_channels_;
	stbi_set_flip_vertically_on_load(false);
	unsigned char *data = stbi_load(tmp.str().c_str(), &width_, &height_, &num_channels_, 0);
	glGenTextures(1, &rgbTexture);
	glBindTexture(GL_TEXTURE_2D_ARRAY, rgbTexture);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, width_, height_, NumCameraPos, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glGenTextures(1, &alphaTexture);
	glBindTexture(GL_TEXTURE_2D_ARRAY, alphaTexture);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, width_, height_, NumCameraPos, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
	for (int i = 0; i < NumCameraPos; i++)
	{
		std::ostringstream ImgRGBAPath, ImgAlphaPath;
		ImgRGBAPath << ObjFilePath << "(" << i + 1 << ").jpg";
		ImgAlphaPath << ObjFilePath << "(" << i + 1 << ")_alpha.png";

		//opencv
		int width_rgb, height_rgb, num_channels_rgb;
		stbi_set_flip_vertically_on_load(false);
		unsigned char *RGBImg = stbi_load(ImgRGBAPath.str().c_str(), &width_rgb, &height_rgb, &num_channels_rgb, 0);

		int width_alpha, height_alpha, num_channels_alpha;
		stbi_set_flip_vertically_on_load(false);
		unsigned char *AlphaImg = stbi_load(ImgAlphaPath.str().c_str(), &width_alpha, &height_alpha, &num_channels_alpha, 0);

		glBindTexture(GL_TEXTURE_2D_ARRAY, rgbTexture);
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width_rgb, height_rgb, 1, GL_RGB, GL_UNSIGNED_BYTE, RGBImg);
		glBindTexture(GL_TEXTURE_2D_ARRAY, alphaTexture);
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width_alpha, height_alpha, 1, GL_RED, GL_UNSIGNED_BYTE, AlphaImg);
	}
}

struct MeshRenderParams {
	float sigma;
};

void ImguiInit(GLFWwindow *wptr)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui_ImplGlfw_InitForOpenGL(wptr, false);
	ImGui_ImplOpenGL3_Init();
	ImGui::StyleColorsDark();
}

void ImguiOverlay(MeshRenderParams &xyrparams)

{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("Tweak");
	ImGui::Text("(%.2fms,%.0ffps)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::SliderFloat("sigma", &xyrparams.sigma, 0.f, 0.4f);

	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImguiExit()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void RenderMesh::render_mesh()
{
	std::cout << "Render Mesh" << std::endl;
	//初始化glfw窗口
	window = MakeGlfw45Window(WindowWidth, WindowHeight, "Render Mesh", false, false);
	
	// print GLSL version
	ShaderVersion();

	LoadCameraPos();
	LoadCameraRot();
	LoadCameraIntr();
	loadImages();
	LoadObjFile();

	AABB scene_bounds{};

	for (int kth_vert = 0; kth_vert < obj_asset.attrib.vertices.size() / 3; ++kth_vert) {
		auto x = obj_asset.attrib.vertices[kth_vert * 3 + 0];
		auto y = obj_asset.attrib.vertices[kth_vert * 3 + 1];
		auto z = obj_asset.attrib.vertices[kth_vert * 3 + 2];
		scene_bounds.Extend(xy::vec3{ x, y, z });
	}

	std::cout << "model: " << std::endl;
	std::cout << "bound: " << scene_bounds.inf << "\t" << scene_bounds.sup << std::endl;
	objCenter = scene_bounds.Center();
	ArcballCamera camera = ArcballCamera(scene_bounds, { 0,0,-1 }, { 0,1,0 },
		//ArcballCamera camera = ArcballCamera(scene_bounds, ttt, { 0,1,0 },
		xy::DegreeToRadian(15.2037f),
		static_cast<float>(WindowWidth) / WindowHeight);
	//WanderCamera camera = WanderCamera(ttt,objCenter,xy::vec3(0.0,1.0,0.0), 
	//	xy::DegreeToRadian(15.2037f), static_cast<float>(WindowWidth) / WindowHeight,
	//	0.1f,10000.0f);

	// GL Functions
	//glEnable(GL_CULL_FACE); // 开启剔除操作效果
	//glCullFace(GL_BACK); //背面剔除，不做渲染，节省计算量
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	//glShadeModel(GL_SMOOTH);	//GL_SMOOTH: interpolation with the colors of vertice
	//GL_FLAT: fill with the color of the last vertex
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_BLEND);
	///alpha
	//glBlendFunc(GL_ONE, GL_ONE);
	//glBlendEquation(GL_MAX);
	///color
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_MAX);

	///// Shader Program
	ShaderProgram = Shader((shaderFilePath+"shader_mesh.vs").data(), (shaderFilePath+"shader_mesh.frag").data());

	GLuint ssbo_pos;
	glGenBuffers(1, &ssbo_pos);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_pos);
	//camposes = new float[NumCameraPos * 3];
	glBufferData(GL_SHADER_STORAGE_BUFFER, NumCameraPos * 3 * sizeof(float), (void*)camposes, GL_STATIC_READ);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssbo_pos);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

	GLuint ssbo_rot;
	glGenBuffers(1, &ssbo_rot);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_rot);
	glBufferData(GL_SHADER_STORAGE_BUFFER, NumCameraPos * 9 * sizeof(float), camrots, GL_STATIC_READ);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ssbo_rot);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

	GLuint ssbo_intr;
	glGenBuffers(1, &ssbo_intr);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_intr);
	glBufferData(GL_SHADER_STORAGE_BUFFER, NumCameraPos * 9 * sizeof(float), camintrs, GL_STATIC_READ);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, ssbo_intr);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

	auto input = XYInput(window);
	MeshRenderParams param;
	param.sigma = 0.1;
	ImguiInit(window);

	int flag = 0;
	glViewport(0, 0, WindowWidth, WindowHeight);
	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		glClearDepth(1.0f);
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		if (flag % 1000 == 0)
			std::cout << "Frame rate: " << 1 / deltaTime << " fps" << std::endl;

		input.PollEvents();
		if (!ImGui::GetIO().WantCaptureMouse) {
			camera.HandleInput(input);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glClearColor(55. / 255, 55. / 255, 110. / 255, 1.0);
		//glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		ShaderProgram.Use();

		glUniform1i(glGetUniformLocation(ShaderProgram.Program, "RGBATexArray"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, rgbTexture);

		glUniform1i(glGetUniformLocation(ShaderProgram.Program, "AlphaTexArray"), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D_ARRAY, alphaTexture);

		auto ModelRotateX = xy::AngleAxisToQuat(ModelRotate_X, { 1.f,.0f,.0f });
		auto ModelRotateY = xy::AngleAxisToQuat(ModelRotate_Y, { 0.f,1.f,.0f });
		auto ModelRotateZ = xy::AngleAxisToQuat(ModelRotate_Z, { 0.f,.0f,1.f });
		auto Rotate = ModelRotateZ * ModelRotateY * ModelRotateX;
		auto ModelRotate = xy::QuatToMat4(Rotate);
		//auto ModelTranslate = xy::Translation({ ModelTranslate_X - objCenter.x, ModelTranslate_Y - objCenter.y, ModelTranslate_Z - objCenter.z });
		auto ModelTranslate = xy::Translation({ ModelTranslate_X, ModelTranslate_Y, ModelTranslate_Z });
		auto ModelScale = xy::Scale({ 1.0f, 1.0f, 1.0f });
		xy::mat4 ModelM = ModelTranslate * ModelRotate * ModelScale;
		glUniformMatrix4fv(glGetUniformLocation(ShaderProgram.Program, "ModelM"), 1, GL_FALSE, ModelM.data);

		xy::mat4 CameraM = camera.View();;
		glUniformMatrix4fv(glGetUniformLocation(ShaderProgram.Program, "CameraM"), 1, GL_FALSE, CameraM.data);

		xy::mat4 ProjM = camera.Proj();
		glUniformMatrix4fv(glGetUniformLocation(ShaderProgram.Program, "ProjM"), 1, GL_FALSE, ProjM.data);

		glUniform3f(glGetUniformLocation(ShaderProgram.Program, "cameraPosition"), camera.Pos().x, camera.Pos().y, camera.Pos().z);
		glUniform1i(glGetUniformLocation(ShaderProgram.Program, "refCamNum"), NumCameraPos);
		glUniform1f(glGetUniformLocation(ShaderProgram.Program, "sigma"), param.sigma);

		// draw
		for (int kth_part = 0; kth_part < objG_asset.parts.size(); ++kth_part) {

			auto mtl_idx = objG_asset.part2mtl[kth_part];
			const auto &mtl = objG_asset.mtls[mtl_idx];
			const auto &part = objG_asset.parts[kth_part];

			part.DrawElements(GL_TRIANGLES, { 0, 1, 2 });
		}

		ImguiOverlay(param);

		flag++;
		//交换缓冲
		glfwSwapBuffers(window);
	}

	ImguiExit();
	DelObjGAsset(objG_asset);
	glfwTerminate();
}
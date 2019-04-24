#include "render_hair.h"

void RenderHair::LoadObjFile()
{
	obj_asset = MakeObjAsset(objFileName + modelName, objFileName);

	for (int kth_vert = 0; kth_vert < obj_asset.attrib.vertices.size() / 3; ++kth_vert) {
		obj_asset.attrib.vertices[kth_vert * 3 + 0] = -obj_asset.attrib.vertices[kth_vert * 3 + 0];
		obj_asset.attrib.vertices[kth_vert * 3 + 2] = -obj_asset.attrib.vertices[kth_vert * 3 + 2];
	}

	objG_asset = MakeObjGAsset(obj_asset);

	hair_asset = MakeHairAsset(objFileName + indFileName);

	for (int kth_vert = 0; kth_vert < hair_asset.positions.size(); ++kth_vert) {
		hair_asset.positions[kth_vert].x = -hair_asset.positions[kth_vert].x;
		hair_asset.positions[kth_vert].z = -hair_asset.positions[kth_vert].z;
	}

	hairG_asset = MakeHairGAsset(hair_asset);
}

void RenderHair::LoadCameraPos()
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
		if (ind%inter != 0)
		{
			ind++;
			continue;
		}
		std::vector<std::string> result = Tool::split(str, pattern);
		for (int i = 0; i < result.size(); i++)
		{
			CameraPos[ind/inter][i] = atof(result[i].c_str());
		}
		ind++;
		if (ind == NumCameraPos * inter)
			break;
	}

	camposes = new float[NumCameraPos * 3];
	for (int i = 0; i < NumCameraPos; i++)
	{
		camposes[i * 3] = CameraPos[i][0];
		camposes[i * 3 + 1] = CameraPos[i][1];
		camposes[i * 3 + 2] = CameraPos[i][2];
	}
}

void RenderHair::LoadCameraRot()
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
		if (ind%inter != 0)
		{
			ind++;
			continue;
		}
		std::vector<std::string> result = Tool::split(str, pattern);
		for (int i = 0; i < result.size(); i++)
		{
			RotMat[ind/inter][i] = atof(result[i].c_str());
		}
		ind++;
		if (ind == NumCameraPos*inter)
			break;
	}

	camrots = new float[NumCameraPos * 9];
	for (int i = 0; i < NumCameraPos; i++)
		for (int j = 0; j < 9; j++)
			camrots[i * 9 + j] = RotMat[i][j];
}

void RenderHair::loadImages()
{
	glGenTextures(1, &alphaTexture);
	glBindTexture(GL_TEXTURE_2D_ARRAY, alphaTexture);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, 800, 800, NumCameraPos, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	for (int i = 0; i < NumCameraPos; i++)
	{
		std::ostringstream ImgRGBAPath;
		ImgRGBAPath << ObjFilePath << "cam" << i / (50/inter) + 1 << "." << std::setw(3) << std::setfill('0') << i % (50/inter) * (2 * inter) + 1 << ".png";
		int width, height, num_channels;
		stbi_set_flip_vertically_on_load(false);
		unsigned char *data = stbi_load(ImgRGBAPath.str().c_str(), &width, &height, &num_channels, 0);
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glUniform1i(glGetUniformLocation(ShaderProgram.Program, "RGBATexArray"), 0);
		stbi_image_free(data);
	}
}

////// accelerate
void RenderHair::recordCameras()
{
	int halfsie = windowSize / 2;
	for (int i = 0; i < NumCameraPos; i++)
	{
		cam_center += xy::vec3{ CameraPos[i][0],CameraPos[i][1],CameraPos[i][2] };
	}
	cam_center /= NumCameraPos;

	for (int i = 0; i < NumCameraPos; i++)
	{
		xy::vec3 pos = xy::vec3{ CameraPos[i][0],CameraPos[i][1],CameraPos[i][2] };
		xy::vec3 pos_norm = xy::Normalize(pos - cam_center);
		float theta1, theta2;
		theta1 = acos(pos_norm[2]);
		theta2 = acos(pos_norm[0] / sin(theta1));
		if (pos_norm[1] < 0)
			theta2 = 2 * PI - theta2;
		int gridH = floor(theta1 / (PI / gridHeight));
		int gridW = floor(theta2 / (2 * PI / gridWidth));
		int gridn = (gridH + gridHeight) % gridHeight * gridWidth + (gridW + gridWidth) % gridWidth;
		grid_cams[gridn].push_back(i);
	}
	for (int i = 0; i < gridHeight; i++)
	{
		for (int j = 0; j < gridWidth; j++)
		{
			int curGrid = i*gridWidth + j;
			//std::cout << i << " " << j << " " << curGrid << std::endl;
			for (int p = i - halfsie; p <= i + halfsie; p++)
			{
				for (int q = j - halfsie; q <= j + halfsie; q++)
				{
					int gridn = (p + gridHeight) % gridHeight * gridWidth + (q + gridWidth) % gridWidth;
					for (int k = 0; k < grid_cams[gridn].size(); k++)
						grid_refs[curGrid].push_back(grid_cams[gridn][k]);
				}
			}
			if (grid_refs[curGrid].size() > max_num)
				max_num = grid_refs[curGrid].size();
		}
	}

	gridRefCams = new int[gridHeight * gridWidth * max_num];
	for (int i = 0; i < gridHeight * gridWidth; i++)
	{
		for (int j = 0; j < grid_refs[i].size(); j++)
		{
			gridRefCams[i * max_num + j] = grid_refs[i][j];
		}
		for (int j = grid_refs[i].size(); j < max_num; j++)
		{
			gridRefCams[i * max_num + j] = -1;
		}
	}
}

void RenderHair::render_hair()
{
	std::cout << "Render Hair" << std::endl;
	//初始化glfw窗口
	window = MakeGlfw45Window(WindowWidth, WindowHeight, "Render Hair", false, false);
	
	// print GLSL version
	ShaderVersion();

	// load cameras' RT & load images and model
	LoadCameraPos();
	LoadCameraRot();
	loadImages();
	recordCameras();
	std::cout << "max_num: " << max_num << std::endl;
	LoadObjFile();

	AABB scene_bounds{};

	for (int kth_vert = 0; kth_vert < obj_asset.attrib.vertices.size() / 3; ++kth_vert) {
		auto x = obj_asset.attrib.vertices[kth_vert * 3 + 0];
		auto y = obj_asset.attrib.vertices[kth_vert * 3 + 1];
		auto z = obj_asset.attrib.vertices[kth_vert * 3 + 2];
		scene_bounds.Extend(xy::vec3{ x, y, z });
	}
	for (int kth_vert = 0; kth_vert < hair_asset.positions.size(); ++kth_vert) {
		auto x = hair_asset.positions[kth_vert];
		scene_bounds.Extend(x);
	}

	std::cout << "model: " << std::endl;
	std::cout << "bound: " << scene_bounds.inf << "\t" << scene_bounds.sup << std::endl;
	auto diff = camAimRender - scene_bounds.Center();
	scene_bounds.inf += diff;
	scene_bounds.sup += diff;
	std::cout << "center: " << scene_bounds.Center() << std::endl;

	ArcballCamera camera = ArcballCamera(scene_bounds, { 0,0,-1 }, { 0,1,0 },
		xy::DegreeToRadian(15.2037f),
		static_cast<float>(WindowWidth) / WindowHeight);
	//WanderCamera camera = WanderCamera(camPos,camAimRender,xy::vec3(0.0,1.0,0.0),
	//	xy::DegreeToRadian(15.2037f), static_cast<float>(WindowWidth) / WindowHeight,
	//	0.1f,10000.0f);

	//// GL Functions
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);
	/// alpha
	//glBlendFunc(GL_ONE, GL_ONE);
	//glBlendEquation(GL_MAX);
	/// color
	glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ONE);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_MAX);

	GLuint ssbo_pos;
	glGenBuffers(1, &ssbo_pos);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_pos);
	glBufferData(GL_SHADER_STORAGE_BUFFER, NumCameraPos * 3 * sizeof(float), camposes, GL_STATIC_READ);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssbo_pos);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	GLuint ssbo_rot;
	glGenBuffers(1, &ssbo_rot);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_rot);
	glBufferData(GL_SHADER_STORAGE_BUFFER, NumCameraPos * 9 * sizeof(float), camrots, GL_STATIC_READ);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ssbo_rot);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	////// accelerate
	GLuint ssbo_refs;
	glGenBuffers(1, &ssbo_refs);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_refs);
	glBufferData(GL_SHADER_STORAGE_BUFFER, gridWidth * gridHeight * max_num * sizeof(int), gridRefCams, GL_STATIC_READ);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, ssbo_refs);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	///// Shader Program
	ShaderProgram = Shader((shaderFilePath+"shader_hair.vs").data(), (shaderFilePath+"shader_hair.frag").data());
	
	auto input = XYInput(window);
	int flag = 0;
	glViewport(0, 0, WindowWidth, WindowHeight);
	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		glClearDepth(1.0f);
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		if(flag%1000==0)
			std::cout << "Frame rate: " << 1 / deltaTime << " fps" << std::endl;

		input.PollEvents();
		camera.HandleInput(input);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);

		ShaderProgram.Use();

		auto ModelRotateX = xy::AngleAxisToQuat(ModelRotate_X, {1.f,.0f,.0f});
		auto ModelRotateY = xy::AngleAxisToQuat(ModelRotate_Y, { 0.f,1.f,.0f });
		auto ModelRotateZ = xy::AngleAxisToQuat(ModelRotate_Z, { 0.f,.0f,1.f });
		auto Rotate = ModelRotateZ * ModelRotateY * ModelRotateX;
		auto ModelRotate = xy::QuatToMat4(Rotate);
		auto ModelTranslate = xy::Translation({ ModelTranslate_X, ModelTranslate_Y, ModelTranslate_Z });
		auto ModelScale = xy::Scale({ 1.0f, 1.0f, 1.0f });
		xy::mat4 ModelM = ModelTranslate * ModelRotate * ModelScale;
		glUniformMatrix4fv(glGetUniformLocation(ShaderProgram.Program, "ModelM"), 1, GL_FALSE, ModelM.data);
		
		xy::mat4 CameraM = camera.View();
		glUniformMatrix4fv(glGetUniformLocation(ShaderProgram.Program, "CameraM"), 1, GL_FALSE, CameraM.data);

		xy::mat4 ProjM = camera.Proj();
		glUniformMatrix4fv(glGetUniformLocation(ShaderProgram.Program, "ProjM"), 1, GL_FALSE, ProjM.data);
		
		glUniform3f(glGetUniformLocation(ShaderProgram.Program, "cameraPosition"), camera.Pos().x,camera.Pos().y,camera.Pos().z);
		glUniform1i(glGetUniformLocation(ShaderProgram.Program, "refCamNum"), NumCameraPos);

		////// accelerate
		glUniform1i(glGetUniformLocation(ShaderProgram.Program, "gridWidth"), gridWidth);
		glUniform1i(glGetUniformLocation(ShaderProgram.Program, "gridHeight"), gridHeight);
		glUniform1i(glGetUniformLocation(ShaderProgram.Program, "max_num"), max_num);
		glUniform3f(glGetUniformLocation(ShaderProgram.Program, "cam_center"), cam_center[0], cam_center[1], cam_center[2]);

		for (int kth_part = 0; kth_part < objG_asset.parts.size(); ++kth_part) {

			auto mtl_idx = objG_asset.part2mtl[kth_part];
			const auto &mtl = objG_asset.mtls[mtl_idx];
			const auto &part = objG_asset.parts[kth_part];

			part.DrawElements(GL_TRIANGLES, { 0, 1, 2 });
		}

		hairG_asset.DrawElements({ 0 });

		flag++;
		//交换缓冲
		glfwSwapBuffers(window);
	}

	glfwTerminate();
}
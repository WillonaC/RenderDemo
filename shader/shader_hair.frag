#version 440
#extension GL_EXT_gpu_shader4 : enable

layout(std430, binding = 4) buffer camPosData
{
	float pos_SSBO[];
};

layout(std430, binding = 5) buffer camRotData
{
	float rot_SSBO[];
};

layout(std430, binding = 6) buffer refstData
{
	int refs_SSBO[];
};

uniform mat4 ModelM;
uniform mat4 ProjM;
uniform mat4 CameraM;
uniform vec3 cameraPosition;
uniform int refCamNum;
uniform sampler2DArray RGBATexArray;

uniform int gridHeight, gridWidth;
uniform int max_num;
uniform vec3 cam_center;

mat3 refRot; // 转置过的
vec3 refPos;
mat3 intri = mat3(vec3(1333.3333, 0, 400), vec3(0, 1333.3333, 400), vec3(0, 0, 1));// 转置过的

in vec3 vertexPos;
out vec4 FragColor;

int imgCols = 800, imgRows = 800;
int	WindowWidth = 800, WindowHeight = 800;
float objScale = 2.54;
float sigma = 0.03;
const int referenceNum = 4;

float PI = 3.14159265358979323846;

struct Refs
{
	float angle;
	int id;
	float weight;
};
Refs refs[referenceNum];
Refs refs_NEW[referenceNum];

void main()
{
	vec3 cameraPosition_NEW = vec3(cameraPosition.x, -cameraPosition.z, cameraPosition.y);
	vec4 point_3d = vec4(vertexPos[0], -vertexPos[2], vertexPos[1], 1.0);
/*
	// find nearest cams
	vec3 curCamPos_norm = normalize(cameraPosition_NEW - vec3(point_3d));
	for (int i = 0; i < referenceNum; i++)
	{
		refPos = vec3(pos_SSBO[i * 3], pos_SSBO[i * 3 + 1], pos_SSBO[i * 3 + 2]);
		vec3 refCamPos_norm = normalize(refPos - vec3(point_3d));
		refs[i].id = i;
		refs[i].angle = dot(curCamPos_norm, refCamPos_norm);
		refs[i].weight = exp(-(1 - pow(refs[i].angle, 2)) / 2 / pow(sigma, 2));
	}
	for (int i = referenceNum; i < refCamNum; i++)
	{
		refPos = vec3(pos_SSBO[i * 3], pos_SSBO[i * 3 + 1], pos_SSBO[i * 3 + 2]);
		vec3 refCamPos_norm = normalize(refPos - vec3(point_3d));
		float tmp_angle = dot(curCamPos_norm, refCamPos_norm);
		int min_id = -1;
		float min_angle = 999;
		for (int j = 0; j < referenceNum; j++)
		{
			if (refs[j].angle < min_angle)
			{ 
				min_angle = refs[j].angle;
				min_id = j;
			}
		}
		if (tmp_angle > min_angle)
		{
			refs[min_id].id = i;
			refs[min_id].angle = tmp_angle;
			refs[min_id].weight = exp(-(1 - pow(tmp_angle, 2)) / 2 / pow(sigma, 2));
		}
	}

	// compute alpha
	float dis_sum = 0;
	int flag = 0;
	vec4 result = vec4(0.0, 0.0, 0.0, 0.0);
	for (int i = 0; i < referenceNum; i++)
	{
		if (flag == 4)
			break;
		refPos = vec3(pos_SSBO[refs[i].id * 3], pos_SSBO[refs[i].id * 3 + 1], pos_SSBO[refs[i].id * 3 + 2]);
		refRot = mat3(vec3(rot_SSBO[refs[i].id * 9], rot_SSBO[refs[i].id * 9 + 1], rot_SSBO[refs[i].id * 9 + 2]), vec3(rot_SSBO[refs[i].id * 9 + 3], rot_SSBO[refs[i].id * 9 + 4], rot_SSBO[refs[i].id * 9 + 5]), vec3(rot_SSBO[refs[i].id * 9 + 6], rot_SSBO[refs[i].id * 9 + 7], rot_SSBO[refs[i].id * 9 + 8]));
		vec3 point_2d = transpose(intri) * refRot * (vec3(point_3d) - refPos);
		point_2d /= point_2d[2];
		if (round(point_2d[0]) >= 0 && round(point_2d[0]) <= imgCols && round(point_2d[1]) >= 0 && round(point_2d[1]) <= imgRows)
		{
			flag++;
			result += refs[i].weight * texture(RGBATexArray, vec3(1.0 - point_2d[0] / 800, point_2d[1] / 800, refs[i].id));
			dis_sum += refs[i].weight;
		}
	}

	if (dis_sum != 0)
	{
		result /= dis_sum;
	}
*/
	// find nearest cams NEW
	vec3 pos_norm = normalize(cameraPosition_NEW - cam_center);
	float theta1, theta2;
	theta1 = acos(pos_norm[2]);
	theta2 = acos(pos_norm[0] / sin(theta1));
	if (pos_norm[1] < 0)
		theta2 = 2 * PI - theta2;
	int gridH = int(floor(theta1 / (PI / gridHeight)));
	int gridW = int(floor(theta2 / (2 * PI / gridWidth)));
	int gridn = (gridH + gridHeight) % gridHeight * gridWidth + (gridW + gridWidth) % gridWidth;

	vec3 curCamPos_norm_NEW = normalize(cameraPosition_NEW - vec3(point_3d));
	int referenceNum_NEW = referenceNum;
	if (referenceNum > max_num)
		referenceNum_NEW = max_num;
	for (int i = 0; i < referenceNum; i++)
		refs_NEW[i].id = -1;
	for (int i = 0; i < referenceNum_NEW; i++)
	{
		if (refs_SSBO[gridn * max_num + i] != -1)
		{
			int refID = refs_SSBO[gridn * max_num + i];
			refPos = vec3(pos_SSBO[refID * 3], pos_SSBO[refID * 3 + 1], pos_SSBO[refID * 3 + 2]);
			vec3 refCamPos_norm = normalize(refPos - vec3(point_3d));
			refs_NEW[i].id = refID;
			refs_NEW[i].angle = dot(curCamPos_norm_NEW, refCamPos_norm);
			refs_NEW[i].weight = exp(-(1 - pow(refs_NEW[i].angle, 2)) / 2 / pow(sigma, 2));
		}
		else
			break;
	}
	for (int i = referenceNum; i < max_num; i++)
	{
		if (refs_SSBO[gridn * max_num + i] != -1)
		{
			int refID = refs_SSBO[gridn * max_num + i];
			refPos = vec3(pos_SSBO[refID * 3], pos_SSBO[refID * 3 + 1], pos_SSBO[refID * 3 + 2]);
			vec3 refCamPos_norm = normalize(refPos - vec3(point_3d));
			float tmp_angle = dot(curCamPos_norm_NEW, refCamPos_norm);
			int min_id = -1;
			float min_angle = 999;
			for (int j = 0; j < referenceNum; j++)
			{
				if (refs_NEW[j].angle < min_angle)
				{
					min_angle = refs_NEW[j].angle;
					min_id = j;
				}
			}
			if (tmp_angle > min_angle)
			{
				refs_NEW[min_id].id = refID;
				refs_NEW[min_id].angle = tmp_angle;
				refs_NEW[min_id].weight = exp(-(1 - pow(tmp_angle, 2)) / 2 / pow(sigma, 2));
			}
		}
		else
			break;
	}

	// compute alpha NEW
	float dis_sum_NEW = 0;
	vec4 result_NEW = vec4(0.0, 0.0, 0.0, 0.0);
	for (int i = 0; i < referenceNum; i++)
	{
		if (refs_NEW[i].id == -1)
			continue;
		refPos = vec3(pos_SSBO[refs_NEW[i].id * 3], pos_SSBO[refs_NEW[i].id * 3 + 1], pos_SSBO[refs_NEW[i].id * 3 + 2]);
		refRot = mat3(vec3(rot_SSBO[refs_NEW[i].id * 9], rot_SSBO[refs_NEW[i].id * 9 + 1], rot_SSBO[refs_NEW[i].id * 9 + 2]), vec3(rot_SSBO[refs_NEW[i].id * 9 + 3], rot_SSBO[refs_NEW[i].id * 9 + 4], rot_SSBO[refs_NEW[i].id * 9 + 5]), vec3(rot_SSBO[refs_NEW[i].id * 9 + 6], rot_SSBO[refs_NEW[i].id * 9 + 7], rot_SSBO[refs_NEW[i].id * 9 + 8]));
		vec3 point_2d = transpose(intri) * refRot * (vec3(point_3d) - refPos);
		point_2d /= point_2d[2];
		if (round(point_2d[0]) >= 0 && round(point_2d[0]) <= imgCols && round(point_2d[1]) >= 0 && round(point_2d[1]) <= imgRows)
		{
			result_NEW += refs_NEW[i].weight * texture(RGBATexArray, vec3(1.0 - point_2d[0] / 800, point_2d[1] / 800, refs_NEW[i].id));
			dis_sum_NEW += refs_NEW[i].weight;
		}
	}

	if (dis_sum_NEW != 0)
	{
		result_NEW /= dis_sum_NEW;
	}
	
	
	//FragColor = result;
	FragColor = result_NEW;
}
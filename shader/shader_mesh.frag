#version 440

layout(std430, binding = 4) buffer camPosData
{
	float pos_SSBO[];
};

layout(std430, binding = 5) buffer camRotData
{
	float rot_SSBO[];
};

layout(std430, binding = 6) buffer camIntrData
{
	float intr_SSBO[];
};

uniform mat4 ModelM;
uniform mat4 ProjM;
uniform mat4 CameraM;
uniform vec3 cameraPosition;
uniform int refCamNum;
uniform float sigma;

//mat3 intri = mat3(vec3(914.2857, 0, 400), vec3(0, 914.2857, 400), vec3(0, 0, 1));// 转置过的
//mat3 intri = mat3(vec3(23771.853378, 0.000000, 2994.948895291689), vec3(0, 23771.853378, 2025.138577503879), vec3(0, 0, 1));// 转置过的
//mat3 intri = mat3(vec3(2.248990 * 10000, 0,2000), vec3(0 , 2.248990 * 10000, 3000), vec3(0, 0, 1));// 转置过的

in vec3 vertexPos;
out vec4 FragColor;

int imgCols = 4000, imgRows = 6000;
//float sigma = 0.3;
const int referenceNum = 4;
uniform sampler2DArray RGBATexArray;
uniform sampler2DArray AlphaTexArray;

struct Refs
{
	float angle;
	int id;
	float weight;
};
Refs refs[referenceNum];

void main()
{
	//vec4 point_3d = vec4(vertexPos[0], -vertexPos[2], vertexPos[1], 1.0);
	vec4 point_3d = vec4(vertexPos[0], vertexPos[1], vertexPos[2], 1.0);
	//vec4 point_3d = vec4(-0.0502200000000000,	0.175360000000000, - 6.44270000000000, 1.0);
	

	vec3 point_2d;
	
	// find nearest cams
	vec3 curCamPos_norm = normalize(cameraPosition - vec3(point_3d));
	vec3 refPos;
	mat3 refRot, intri;
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

	//for (int i = 0; i < 4; i++)
	//	refs[i].id = 0;

	// compute alpha
	float dis_sum = 0;
	vec3 result = vec3(0.0, 0.0, 0.0);
	float alphaValue = 0;
	for (int i = 0; i < referenceNum; i++)
	{
		refPos = vec3(pos_SSBO[refs[i].id * 3], pos_SSBO[refs[i].id * 3 + 1], pos_SSBO[refs[i].id * 3 + 2]);
		refRot = mat3(vec3(rot_SSBO[refs[i].id * 9], rot_SSBO[refs[i].id * 9 + 1], rot_SSBO[refs[i].id * 9 + 2]), vec3(rot_SSBO[refs[i].id * 9 + 3], rot_SSBO[refs[i].id * 9 + 4], rot_SSBO[refs[i].id * 9 + 5]),
			vec3(rot_SSBO[refs[i].id * 9 + 6], rot_SSBO[refs[i].id * 9 + 7], rot_SSBO[refs[i].id * 9 + 8]));
		intri = mat3(vec3(intr_SSBO[refs[i].id * 9], intr_SSBO[refs[i].id * 9 + 1], intr_SSBO[refs[i].id * 9 + 2]), vec3(intr_SSBO[refs[i].id * 9 + 3], intr_SSBO[refs[i].id * 9 + 4], intr_SSBO[refs[i].id * 9 + 5]),
			vec3(intr_SSBO[refs[i].id * 9 + 6], intr_SSBO[refs[i].id * 9 + 7], intr_SSBO[refs[i].id * 9 + 8]));
		//intri = mat3(vec3(2.248990 * 10000, 0, 2000), vec3(0, 2.248990 * 10000, 3000), vec3(0, 0, 1));// 转置过的

		//refPos = vec3(-95.847135, -135.54338, -46.748);

		//refRot = mat3(vec3(-0.797518, 0.239369, - 0.553775),
		//vec3(0.603259, 0.326444, - 0.727677),
		//vec3(0.006594, - 0.914405, - 0.404746));
		/*
		refRot = mat3(vec3(0.797518376558218, - 0.239368553776787, - 0.553775346611560),
		vec3(-0.603258619690491, - 0.326443956358331 ,- 0.727676700964270),
		vec3(-0.00659369555470319,	0.914405292427778, - 0.404746197460831));

		refRot = mat3(vec3(0.239368553776787,	0.797518376558218 ,- 0.553775346611560),
		vec3(0.326443956358331 ,- 0.603258619690491 ,- 0.727676700964270),
		vec3(-0.914405292427778 ,- 0.00659369555470319, - 0.404746197460831));

		refRot = mat3(vec3(-0.239368553776787,	0.797518376558218 ,- 0.553775346611560),
		vec3(-0.326443956358331 ,- 0.603258619690491 ,- 0.727676700964270),
		vec3(0.914405292427778, - 0.00659369555470319 ,- 0.404746197460831));
		 */
			
		/*refPos = vec3(2.426848, -2.692385, -15.89230);

		refRot = mat3(vec3(-0.9628464, -0.1275705, -0.2380179),
			vec3(0.08654348, -0.9806653, 0.1755157),
			vec3(-0.2558065, 0.1483957, 0.9552705));//raw*/
		  
		/*refRot = mat3(-vec3(0.1927, 0.8591, -0.4741),
			-vec3(0.9580, -0.2693, -0.0987),
			-vec3(0.2125, 0.4352, 0.8749));//new*/
				
		/*vec3 loc = refPos;
		vec3 look = normalize(vec3(refRot[0][2], refRot[1][2], refRot[2][2]));
		vec3 up = vec3(refRot[0][1], refRot[1][1], refRot[2][1]);

		vec3 eye = loc;
		vec3 spot = loc + look;
		up = -up;

		vec3 f = normalize(spot - eye);
		vec3 s = normalize(cross(f, up));
		vec3 u = normalize(cross(s, f));
		mat4 v = transpose(mat4(vec4(s.x, s.y, s.z ,-dot(s, eye)),vec4(u.x, u.y, u.z, -dot(u, eye)), vec4(-f.x, -f.y, -f.z, dot(f, eye)),vec4( 0, 0 ,0 ,1)));


		vec4 ttt = v*point_3d;
		point_2d = transpose(intri)*ttt.xyz;
		point_2d /= point_2d[2];*/

		//refPos = -transpose(refRot)*refPos;
		//refRot = transpose(refRot);

		point_2d = transpose(intri) * (refRot) * (vec3(point_3d) - refPos);
		point_2d /= point_2d[2];
		if (round(point_2d[0]) >= 0 && round(point_2d[0]) <= imgCols && round(point_2d[1]) >= 0 && round(point_2d[1]) <= imgRows)
		{
			float u = point_2d[0] / imgCols;
			float v = point_2d[1] / imgRows;

			result += refs[i].weight * vec3(texture(RGBATexArray, vec3(u, v, refs[i].id)));
			alphaValue += refs[i].weight * texture(AlphaTexArray, vec3(u, v, refs[i].id))[0];
			dis_sum += refs[i].weight;

			//result += vec3(texture(RGBATexArray, vec3(u, v, refs[i].id)));
			//dis_sum += 1;
		}
	}

	if (dis_sum != 0)
	{
		result /= dis_sum;
		alphaValue /= dis_sum;
	}

	//FragColor = vec4(vec3(texture(RGBATexArray, vec3(0.5,0.5, 0)).xyz),1.0);
	FragColor = vec4(result, alphaValue);
	//FragColor = vec4(result, 1);
	//FragColor = vec4(alphaValue, alphaValue, alphaValue, alphaValue);
	//if(FragColor.x==0&& FragColor.y==0&& FragColor.z==0)
		//FragColor = vec4(1, 0, 0, 1);
	//if(refs[0].id==0&&refs[1].id==0&&refs[2].id==0&&refs[3].id==0)
		//FragColor = vec4(1, 1, 0, 1);
		//FragColor = vec4((vertexPos.x + 15) / 27, (vertexPos.y + 15) / 25, (vertexPos.z + 10) / 50, 1.0);
		//FragColor = vec4((vertexPos.x+0.5)/1.5,(vertexPos.y+2.6)/2.6,(vertexPos.z+9)/3,1.0);

	//FragColor = vec4(point_2d[0] / imgCols, point_2d[1] / imgRows,1.0, 1.0);
	//FragColor = vec4(vec3(texture(RGBATexArray, vec3(0.5,0.5, 0)).xyz),1.0);
	//if(abs(point_2d[0]-3544)<1 && abs(point_2d[1]-2946)<1)
		//FragColor = vec4(1, 1, 0, 1);
}
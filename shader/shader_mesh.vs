#version 440

uniform mat4 ModelM;
uniform mat4 ProjM;
uniform mat4 CameraM;

layout(location = 0) in vec3 Vertex;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TexCoord;
out vec3 vertexPos;

void main()
{
	gl_Position = ProjM * CameraM * ModelM * vec4(Vertex, 1.0);
	vertexPos = Vertex;
}
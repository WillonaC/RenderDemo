#version 440

uniform mat4 ModelM;
uniform mat4 ProjM;
uniform mat4 CameraM;

in vec3 Vertex;

out vec3 vertexPos;

void main()
{
	gl_Position = ProjM * CameraM * ModelM * vec4(Vertex, 1.0);
	vertexPos = Vertex;
}
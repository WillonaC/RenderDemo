#include "render_hair.h"
#include "render_mesh.h"

int main()
{	
	RenderHair renderHair = RenderHair();
	renderHair.render_hair();
	RenderMesh renderMesh = RenderMesh();
	renderMesh.render_mesh();
}
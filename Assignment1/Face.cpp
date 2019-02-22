#include "Face.h"

Face::Face()
{
}

Face::Face(int numVertices, float * vpositions)
{
	this->vpositions = vpositions;
	this->numVertices = numVertices;
}

Face::Face(int numVertices, float * vpositions, float * vnormals, float * vtexcoord)
{
	this->vpositions = vpositions;
	this->vnormals = vnormals;
	this->vtexcoord = vtexcoord;
	this->numVertices = numVertices;
}

Face::~Face()
{
}

int Face::findIntersectingVertex(glm::vec3 ray_world)
{
	return -1; //no intersection
}

void Face::getPositionsAndNormalsFromObjl(std::vector<objl::Vertex> vertices, 
					float* &vpositions, 
					float* &vnormals, 
					float* &vtexcoord)
{
	vpositions = new float[vertices.size() * 3];
	vnormals = new float[vertices.size() * 3];
	vtexcoord = new float[vertices.size() * 2];

	for (int i = 0; i < vertices.size(); i++) {
		vpositions[i * 3] = vertices[i].Position.X;
		vpositions[i * 3 + 1] = vertices[i].Position.Y;
		vpositions[i * 3 + 2] = vertices[i].Position.Z;

		vnormals[i * 3] = vertices[i].Normal.X;
		vnormals[i * 3 + 1] = vertices[i].Normal.Y;
		vnormals[i * 3 + 2] = vertices[i].Normal.Z;

		vtexcoord[i * 2] = vertices[i].TextureCoordinate.X;
		vtexcoord[i * 2 + 1] = vertices[i].TextureCoordinate.Y;
	}

	//delete[] array;
	//array = nullptr;

}

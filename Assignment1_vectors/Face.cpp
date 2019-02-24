#include "Face.h"

Face::Face()
{
}

Face::Face(int numVertices, std::vector<float> vpositions)
{
	this->vpositions = vpositions;
	this->numVertices = numVertices;
}

Face::Face(int numVertices, std::vector<float> vpositions, std::vector<float> vnormals, std::vector<float> vtexcoord)
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
	std::vector<float> &vpositions,
	std::vector<float> &vnormals,
	std::vector<float> &vtexcoord)
{
	for (int i = 0; i < vertices.size(); i++) {
		vpositions.push_back(vertices[i].Position.X);
		vpositions.push_back(vertices[i].Position.Y);
		vpositions.push_back(vertices[i].Position.Z);

		vnormals.push_back(vertices[i].Normal.X);
		vnormals.push_back(vertices[i].Normal.Y);
		vnormals.push_back(vertices[i].Normal.Z);

		vtexcoord.push_back(vertices[i].TextureCoordinate.X);
		vtexcoord.push_back(vertices[i].TextureCoordinate.Y);
	}
}

void Face::initialiseDeltaBlendshape(std::vector<float> neutralPositions)
{
	if (this->deltaBlendshape.size() == 0)
	{
		for (int i = 0; i < this->numVertices * 3; i++)
		{
			this->deltaBlendshape.push_back(this->vpositions[i] - neutralPositions[i]);
		}
	}
	else
	{
		throw "Delta blendshapes already initialised";
	}
}

#pragma once

#include <glm/glm.hpp>
#include <string>
#include "..\Dependencies\OBJ_Loader.h"

class Face
{
public:
	Face();
	Face(int numVertices, float * vpositions);
	Face(int numVertices, float * vpositions, float * vnormals, float * vtexcoord);
	
	~Face();


	std::string name;
	int numVertices = 0;
	float * vpositions;   // 3-floats per vertex
	float * vnormals;	  // 3-floats per vertex
	float * vtexcoord;    // 2-floats per vertex
	std::vector<unsigned int> indices;

	int findIntersectingVertex(glm::vec3 ray_world);  // returns the vertex nearest to ray or -1 if no intersection

	static void getPositionsAndNormalsFromObjl(std::vector<objl::Vertex> vertices, float* &vpositions, float* &vnormals, float* &vtexcoord);
};


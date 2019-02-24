#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "..\Dependencies\OBJ_Loader.h"

class Face
{
public:
	Face();
	Face(int numVertices, std::vector<float> vpositions);
	Face(int numVertices, std::vector<float>  vpositions, std::vector<float>  vnormals, std::vector<float>  vtexcoord);
	
	~Face();


	std::string name;
	int numVertices = 0;
	std::vector<float> vpositions;   // 3-floats per vertex
	std::vector<float> vnormals;	  // 3-floats per vertex
	std::vector<float> vtexcoord;    // 2-floats per vertex
	std::vector<float> deltaBlendshape;    // 3-floats per vertex
	std::vector<unsigned int> indices;

	int findIntersectingVertex(glm::vec3 ray_world);  // returns the vertex nearest to ray or -1 if no intersection

	static void getPositionsAndNormalsFromObjl(std::vector<objl::Vertex> vertices, std::vector<float> &vpositions, std::vector<float> &vnormals, std::vector<float> &vtexcoord);

	void initialiseDeltaBlendshape(std::vector<float> neutralPositions);
};


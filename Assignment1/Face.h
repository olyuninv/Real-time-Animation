#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include "..\Dependencies\OBJ_Loader.h"
#include "Geometry.h"

namespace Assignment1
{
	class Face
	{
	public:
		Face();
		Face(int numVertices, float * vpositions);
		Face(int numVertices, float * vpositions, float * vnormals, float * vtexcoord);

		~Face();


		std::string name;
		int numVertices = 0;
		float * vpositions = nullptr;   // 3-floats per vertex
		float * vnormals = nullptr;;	  // 3-floats per vertex
		float * vtexcoord = nullptr;;    // 2-floats per vertex
		float * deltaBlendshape = nullptr;;    // 3-floats per vertex
		float * deltaNormal = nullptr;;    // 3-floats per vertex

		std::vector<unsigned int> indices;

		// Current position

		glm::vec3 initialScaleVector = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::mat4 globalTransform = glm::mat4(1.0);

		bool findIntersectingVertex(glm::vec3 ray_world, glm::vec3 cameraPos, float farclip, int &intersectingVertex, glm::vec3 &pointWorld);  // returns the vertex nearest to ray or -1 if no intersection

		static void getPositionsAndNormalsFromObjl(std::vector<objl::Vertex> vertices, float* &vpositions, float* &vnormals, float* &vtexcoord);

		void calculateDeltaBlendshape(float* &neutralPositions);

		void calculateDeltaNormals(float* &neutralNormals);
	};

}
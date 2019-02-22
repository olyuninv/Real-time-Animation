#include "Blendshape.h"

namespace Assignment1
{
	blendshape::blendshape()
	{
	}

	blendshape::~blendshape()
	{
	}

	float * blendshape::calculateFace(Face neutralFace, int numBlendshapes, Face * blendshapes, float * weights)
	{
		float* newPositions = new float[neutralFace.numVertices * 3]{ 0.0f };

		for (int i = 0; i < neutralFace.numVertices * 3; i++)
		{
			for (int j = 0; j < numBlendshapes; j++)
			{
				newPositions[i] += weights[j] * blendshapes[j].deltaBlendshape[i];
			}

			newPositions[i] += neutralFace.vpositions[i];
		}

		return newPositions;			//this needs to be destroyed before calling this function again
	}

	float * blendshape::recalculateNormals(int numVertices, float * customPositions)
	{
		float* newNormals = new float[numVertices * 3]{ 0.0f };

		int numTriangles = numVertices / 3;

		for (int i = 0; i < numTriangles; i++)
		{
			glm::vec3 v1 = glm::vec3(customPositions[i * 9 + 3] - customPositions[i * 9 + 0], customPositions[i * 9 + 4] - customPositions[i * 9 + 1], customPositions[i * 9 + 5] - customPositions[i * 9 + 2]);
			glm::vec3 v2 = glm::vec3(customPositions[i * 9 + 6] - customPositions[i * 9 + 0], customPositions[i * 9 + 7] - customPositions[i * 9 + 1], customPositions[i * 9 + 8] - customPositions[i * 9 + 2]);

			glm::vec3 normal = glm::cross(v1, v2);

			normal = glm::normalize(normal);

			for (int j = 0; j < 3; j++)
			{
				newNormals[i * 9 + j * 3 + 0] = normal.x;
				newNormals[i * 9 + j * 3 + 1] = normal.y;
				newNormals[i * 9 + j * 3 + 2] = normal.z;
			}
		}

		return newNormals;
	}
}
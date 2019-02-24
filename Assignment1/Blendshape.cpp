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
		float newWeightsLength = blendshape::calculateWeightsLength(numBlendshapes, weights);

		float * adjustedWeights = new float[numBlendshapes];
		for (int j = 0; j < numBlendshapes; j++)
		{
			adjustedWeights[j] = weights[j] / newWeightsLength;
		}

		float neutralWeight = 1.0f / newWeightsLength;

		for (int i = 0; i < neutralFace.numVertices * 3; i++)
		{
			for (int j = 0; j < numBlendshapes; j++)
			{
				newPositions[i] += adjustedWeights[j] * blendshapes[j].deltaBlendshape[i];
			}

			newPositions[i] += neutralFace.vpositions[i] * neutralWeight;
		}

		delete[] adjustedWeights;
		adjustedWeights = nullptr;
		return newPositions;			//this needs to be destroyed before calling this function again
	}

	float * blendshape::recalculateNormals(std::vector<unsigned int> indices, int numVertices, float * customPositions)
	{
		float* newNormals = new float[numVertices * 3]{ 0.0f };

		//int numTriangles = indices.size() / 3;

		for (int i = 0; i < indices.size(); i += 3)
		{
			// first triangle index = i
			// second triangle index = i + 1
			// third triangle index = i + 2
			
			glm::vec3 v1 = glm::vec3(customPositions[indices[i * 3 + 1] + 0] - customPositions[indices[i * 3] + 0],
								customPositions[indices[i * 3 + 1] + 1] - customPositions[indices[i * 3] + 1],
								customPositions[indices[i * 3 + 1] + 2] - customPositions[indices[i * 3]  + 2]);
			glm::vec3 v2 = glm::vec3(customPositions[indices[i * 3 + 2] + 0] - customPositions[indices[i * 3]  + 0],
								customPositions[indices[i * 3 + 2]  + 1] - customPositions[indices[i * 3] + 1],
								customPositions[indices[i * 3 + 2]  + 2] - customPositions[indices[i * 3] + 2]);

			glm::vec3 normal = glm::cross(v1, v2);

			normal = glm::normalize(normal);
			//glm::vec3 curr_normal = glm::vec3(newNormals[indices[i * 3]], newNormals[indices[i * 3] + 1], newNormals[indices[i * 3] + 2]);

			//glm::vec3 newNormal = glm::normalize(curr_normal + normal);

			for (int j = 0; j < 3; j++)
			{
				newNormals[indices[i * 3 + j] + 0] += normal.x;
				newNormals[indices[i * 3 + j] + 1] += normal.y;
				newNormals[indices[i * 3 + j] + 2] += normal.z;
			}
		}

		return newNormals;
	}

	float blendshape::calculateWeightsLength(int numWeights, float * weights)
	{
		float squaredWeights = 1.0f;  // this is the weight of the neutral face that we are starting from

		for (int i = 0; i < numWeights; i++)
		{
			squaredWeights += pow(weights[0], 2);
		}

		return sqrt(squaredWeights);
	}
}
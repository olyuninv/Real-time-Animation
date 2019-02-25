#include "Blendshape.h"

namespace Assignment1
{
	blendshape::blendshape()
	{
	}

	blendshape::~blendshape()
	{
	}

	void blendshape::calculateFace(Face neutralFace, int numBlendshapes, Face * blendshapes, float * weights, float* &customPositions, float* &customNormals)
	{
		float newWeightsLength = blendshape::calculateWeightsLength(numBlendshapes, weights);

		float * adjustedWeights = new float[numBlendshapes];
		for (int j = 0; j < numBlendshapes; j++)
		{
			adjustedWeights[j] = weights[j] / newWeightsLength;
		}

		//float neutralWeight = 1.0f / newWeightsLength;

		// set to 0
		for (int i = 0; i < neutralFace.numVertices * 3; i++)
		{
			customPositions[i] = 0.0f;
			customNormals[i] = 0.0f;
		}

		for (int i = 0; i < neutralFace.numVertices * 3; i++)
		{
			for (int j = 0; j < numBlendshapes; j++)
			{
				customPositions[i] += adjustedWeights[j] * blendshapes[j].deltaBlendshape[i]; 
				customNormals[i] += adjustedWeights[j] * blendshapes[j].deltaNormal[i];
			}

			customPositions[i] += neutralFace.vpositions[i];// *neutralWeight;
			customNormals[i] += neutralFace.vnormals[i];// *neutralWeight;
		}

		//// normalise the normals
		//for (int i = 0; i < neutralFace.numVertices * 3; i+= 3 )
		//{
		//	float normalLength = sqrt(customNormals[i] * customNormals[i] + customNormals[i + 1] * customNormals[i + 1] + customNormals[i + 2] * customNormals[i + 2]);
		//	customNormals[i] = customNormals[i] / normalLength;
		//	customNormals[i + 1] = customNormals[i + 1] / normalLength;
		//	customNormals[i + 2] = customNormals[i + 2] / normalLength;
		//}

		delete[] adjustedWeights;
		adjustedWeights = nullptr;
	}

	void blendshape::recalculateNormals(std::vector<unsigned int> indices, int numVertices, float * customPositions, float* &customNormals)
	{
		// set to 0
		for (int i = 0; i < numVertices * 3; i++)
		{
			customNormals[i] = 0.0f;
		}

		for (int i = 0; i < indices.size(); i += 3)
		{
			// first point index = indices[i]
			// second point index = indices[i + 1]
			// third point index = indices[i + 2]
			// start of 3 floats for Point 1 = indices[i] * 3
			// start of 3 floats for Point 2 = indices[i + 1] * 3
			// start of 3 floats for Point 3 = indices[i + 2] * 3

			// Point 1 x = customPositions[indices[i] * 3]
			// Point 1 y = customPositions[indices[i] * 3 + 1]
			// Point 1 z = customPositions[indices[i] * 3 + 2]
			// Point 2 x = customPositions[indices[i + 1] * 3]
			// Point 2 y = customPositions[indices[i + 1] * 3 + 1]
			// Point 2 z = customPositions[indices[i + 1] * 3 + 2]
			// Point 3 x = customPositions[indices[i + 2] * 3]
			// Point 3 y = customPositions[indices[i + 2] * 3 + 1]
			// Point 3 z = customPositions[indices[i + 2] * 3 + 2]			

			glm::vec3 v1 = glm::vec3(customPositions[indices[i + 1] * 3] - customPositions[indices[i] * 3],
				customPositions[indices[i + 1] * 3 + 1] - customPositions[indices[i] * 3 + 1],
				customPositions[indices[i + 1] * 3 + 2] - customPositions[indices[i] * 3 + 2]);
			glm::vec3 v2 = glm::vec3(customPositions[indices[i + 2] * 3] - customPositions[indices[i] * 3],
				customPositions[indices[i + 2] * 3 + 1] - customPositions[indices[i] * 3 + 1],
				customPositions[indices[i + 2] * 3 + 2] - customPositions[indices[i] * 3 + 2]);

			glm::vec3 normal = glm::cross(v1, v2);

			normal = glm::normalize(normal);
			//glm::vec3 curr_normal = glm::vec3(newNormals[indices[i * 3]], newNormals[indices[i * 3] + 1], newNormals[indices[i * 3] + 2]);

			//glm::vec3 newNormal = glm::normalize(curr_normal + normal);

			for (int j = 0; j < 3; j++)
			{
				customNormals[indices[i + j] * 3 + 0] += normal.x;
				customNormals[indices[i + j] * 3 + 1] += normal.y;
				customNormals[indices[i + j] * 3 + 2] += normal.z;
			}
		}
	}

	float blendshape::calculateWeightsLength(int numWeights, float * weights)
	{
		float squaredWeights = 1.0f;  // this is the weight of the neutral face that we are starting from

		for (int i = 0; i < numWeights; i++)
		{
			squaredWeights += pow(weights[i], 2);
		}

		return sqrt(squaredWeights);
	}
}
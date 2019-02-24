#include "Blendshape.h"

namespace Assignment1
{
	blendshape::blendshape()
	{
	}

	blendshape::~blendshape()
	{
	}

	void blendshape::calculateFace(Face neutralFace, int numBlendshapes, Face * blendshapes, float * weights, float* &customPositions) //, float* &customNormals)
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
		}

		for (int i = 0; i < neutralFace.numVertices * 3; i++)
		{
			for (int j = 0; j < numBlendshapes; j++)
			{
				customPositions[i] += adjustedWeights[j] * blendshapes[j].deltaBlendshape[i];
			}

			customPositions[i] += neutralFace.vpositions[i];// *neutralWeight;
		}

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

			float P1x = customPositions[indices[i] * 3];
			float P1y = customPositions[indices[i] * 3 + 1];
			float P1z = customPositions[indices[i] * 3 + 2];
			float P2x = customPositions[indices[i + 1] * 3];
			float P2y = customPositions[indices[i + 1] * 3 + 1];
			float P2z = customPositions[indices[i + 1] * 3 + 2];
			float P3x = customPositions[indices[i + 2] * 3];
			float P3y = customPositions[indices[i + 2] * 3 + 1];
			float P3z = customPositions[indices[i + 2] * 3 + 2];

			// U = P2 - P1
			float Ux = P2x - P1x;
			float Uy = P2y - P1y;
			float Uz = P2z - P1z;
			//V = P3 - P1
			float Vx = P3x - P1x;
			float Vy = P3y - P1y;
			float Vz = P3z - P1z;

			float Nx = Uy * Vz - Uz * Vy;
			float Ny = Uz * Vx - Ux * Vz;
			float Nz = Ux * Vy - Uy * Vx;
			float length = sqrt(Nx*Nx + Ny * Ny + Nz * Nz);

			Nx = Nx / length;
			Ny = Ny / length;
			Nz = Nz / length;

			for (int j = 0; j < 3; j++)
			{
				customNormals[indices[i + j] * 3] += Nx;
				customNormals[indices[i + j] * 3 + 1] += Ny;
				customNormals[indices[i + j] * 3 + 2] += Nz;
				float lengthFinal = (customNormals[indices[i + j] * 3] * customNormals[indices[i + j] * 3] +
					customNormals[indices[i + j] * 3 + 1] * customNormals[indices[i + j] * 3 + 1] +
					customNormals[indices[i + j] * 3 + 2] * customNormals[indices[i + j] * 3 + 2]);

				customNormals[indices[i + j] * 3] = customNormals[indices[i + j] * 3] / lengthFinal;
				customNormals[indices[i + j] * 3 + 1] = customNormals[indices[i + j] * 3 + 1] / lengthFinal;
				customNormals[indices[i + j] * 3 + 2] = customNormals[indices[i + j] * 3 + 2] / lengthFinal;
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
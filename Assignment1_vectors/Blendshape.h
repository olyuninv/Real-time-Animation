#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "Face.h"

namespace Assignment1_vectors
{
	class blendshape
	{

	public:
		blendshape();

		~blendshape();

		static void calculateFace(Face neutralFace, int numBlendshapes, Face * blendshapes, std::vector<float> weights, std::vector<float> &customPositions);

		static void recalculateNormals(std::vector<unsigned int> indices, int numVertices, std::vector<float> customPositions, std::vector<float> &customNormals);

		static float calculateWeightsLength(int numWeights, std::vector<float> weights);
	};
}

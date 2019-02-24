#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "Face.h"

namespace Assignment1
{
	class blendshape
	{

	public:
		blendshape();

		~blendshape();

		static void calculateFace(Face neutralFace, int numBlendshapes, Face * blendshapes, float * weights, float* &customPositions);

		static void recalculateNormals(std::vector<unsigned int> indices, int numVertices, float * customPositions, float* &customNormals);

		static float calculateWeightsLength(int numWeights, float * weights);
	};
}

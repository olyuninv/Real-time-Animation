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

		static float * calculateFace(Face neutralFace, int numBlandshapes, Face * blendshapes, float * weights);

		static float * recalculateNormals(std::vector<unsigned int> indices, int numVertices, float * customPositions);

		static float calculateWeightsLength(int numWeights, float * weights);
	};
}

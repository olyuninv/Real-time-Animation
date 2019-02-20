#pragma once

#include <vector>
#include "..\Dependencies\OBJ_Loader.h"

namespace Assignment1
{
	class blendshape
	{

	public:
		blendshape();

		~blendshape();

		static float* createBlendshape(std::vector<objl::Vertex>);
	};
}

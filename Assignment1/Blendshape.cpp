#include "Blendshape.h"

namespace Assignment1
{
	blendshape::blendshape()
	{
	}

	blendshape::~blendshape()
	{
	}

	float* blendshape::createBlendshape(std::vector<objl::Vertex> vertices)
	{		
		float *blendshape1 = new float [vertices.size() * 3];

		for (int i = 0; i < vertices.size(); i++) {
			blendshape1[i * 3] = vertices[i].Position.X;
			blendshape1[i * 3 + 1] = vertices[i].Position.Y;
			blendshape1[i * 3 + 2] = vertices[i].Position.Z;
		}

		//delete[] array;
		//array = nullptr;

		return blendshape1;
	}
}
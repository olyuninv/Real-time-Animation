#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

namespace Assignment1
{
	class Geometry
	{
	public:

		static float signedVolume(vec3 a, vec3 b, vec3 c, vec3 d)
		{
			return (1.0f / 6.0f)*dot(cross(b - a, c - a), d - a);
		}

		static bool sameSignVolumes(float a, float b)
		{
			if (a >= 0 && b >= 0 || a < 0 && b < 0)
				return true;

			return false;
		}


		static bool sameSignVolumes(float a, float b, float c)
		{
			if (a >= 0 && b >= 0 && c >= 0 || a < 0 && b < 0 && c < 0)
				return true;

			return false;
		}

		static int closestPoint(vec3 intersection, vec3 p1, vec3 p2, vec3 p3)
		{
			float len1 = length(intersection - p1);
			float len2 = length(intersection - p2);
			float len3 = length(intersection - p3);

			if (len1 <= len2 && len1 <= len3)
			{
				return 1;
			}
			else if (len2 <= len3)
			{
				return 2;
			}
			else
			{
				return 3;
			}
		}
	};
}
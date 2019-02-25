#include "Face.h"

using namespace glm;

namespace Assignment1
{
	Face::Face()
	{
	}

	Face::Face(int numVertices, float * vpositions)
	{
		this->vpositions = vpositions;
		this->numVertices = numVertices;
	}

	Face::Face(int numVertices, float * vpositions, float * vnormals, float * vtexcoord)
	{
		this->vpositions = vpositions;
		this->vnormals = vnormals;
		this->vtexcoord = vtexcoord;
		this->numVertices = numVertices;
	}

	Face::~Face()
	{
	}

	bool Face::findIntersectingVertex(glm::vec3 ray_world, glm::vec3 cameraPos, float farclip, int &intersectingIndex, glm::vec3 &pointWorld)
	{
		// Find closest intersection point
		bool foundIntersection = false;
		int selectedVertexIndex = -1;
		pointWorld = vec3(0.0f);

		float zIntersection = farclip;
		vec3 intersectionPoint;

		// Find the other end of the ray (at far clip plane)
		auto farpoint = cameraPos + ray_world * farclip;

		for (int i = 0; i < this->indices.size(); i += 3) {

			// Get the points in World co-ord
			vec4 p1 = this->globalTransform *
				vec4(this->vpositions[this->indices[i] * 3],
					this->vpositions[this->indices[i] * 3 + 1],
					this->vpositions[this->indices[i] * 3 + 2],
					1.0f);
			vec4 p2 = this->globalTransform *
				vec4(this->vpositions[this->indices[i + 1] * 3],
					this->vpositions[this->indices[i + 1] * 3 + 1],
					this->vpositions[this->indices[i + 1] * 3 + 2],
					1.0f);
			vec4 p3 = this->globalTransform *
				vec4(this->vpositions[this->indices[i + 2] * 3],
					this->vpositions[this->indices[i + 2] * 3 + 1],
					this->vpositions[this->indices[i + 2] * 3 + 2], 
					1.0f);

			float volume1 = Geometry::signedVolume(cameraPos, vec3(p1), vec3(p2), vec3(p3));
			float volume2 = Geometry::signedVolume(farpoint, vec3(p1), vec3(p2), vec3(p3));

			if (!Geometry::sameSignVolumes(volume1, volume2))
			{
				float volume3 = Geometry::signedVolume(cameraPos, farpoint, vec3(p1), vec3(p2));
				float volume4 = Geometry::signedVolume(cameraPos, farpoint, vec3(p2), vec3(p3));
				float volume5 = Geometry::signedVolume(cameraPos, farpoint, vec3(p3), vec3(p1));

				if (Geometry::sameSignVolumes(volume3, volume4, volume5))
				{
					// Find intersection point

					// Normal to the triangle
					auto N = cross(vec3(p2) - vec3(p1), vec3(p3) - vec3(p1));

					// distance to intersection on the line:
					//auto t = -dot(cameraPos, N - vec3(p1)) / dot(cameraPos, farpoint - cameraPos); //ray_wor);
					auto t = -(dot(N, cameraPos) - dot(N, vec3(p1))) / dot(N, ray_world);

					if (t < zIntersection)
					{
						// Found a closer intersection point
						zIntersection = t;
						intersectionPoint = cameraPos + t * ray_world;

						// Find the closest vertex
						int indexOfClosestPoint = Geometry::closestPoint(intersectionPoint, vec3(p1), vec3(p2), vec3(p3));

						// Set selected vertex - this may be overwritten by a closer point later on
						foundIntersection = true;

						selectedVertexIndex = this->indices[i + indexOfClosestPoint - 1];
						
						if (indexOfClosestPoint == 1)
						{
							pointWorld.x = p1.x;
							pointWorld.y = p1.y;
							pointWorld.z = p1.z;
						}
						else if (indexOfClosestPoint = 2)
						{
							pointWorld.x = p2.x;
							pointWorld.y = p2.y;
							pointWorld.z = p2.z;
						}
						else
						{
							pointWorld.x = p3.x;
							pointWorld.y = p3.y;
							pointWorld.z = p3.z;
						}
					}
				}
			}
		}

		if (foundIntersection)
		{
			intersectingIndex = selectedVertexIndex;
			return true; // intersection found
		}

		return false; //no intersection
	}

	void Face::getPositionsAndNormalsFromObjl(std::vector<objl::Vertex> vertices,
		float* &vpositions,
		float* &vnormals,
		float* &vtexcoord)
	{
		for (int i = 0; i < vertices.size(); i++) {
			vpositions[i * 3] = vertices[i].Position.X;
			vpositions[i * 3 + 1] = vertices[i].Position.Y;
			vpositions[i * 3 + 2] = vertices[i].Position.Z;

			vnormals[i * 3] = vertices[i].Normal.X;
			vnormals[i * 3 + 1] = vertices[i].Normal.Y;
			vnormals[i * 3 + 2] = vertices[i].Normal.Z;

			vtexcoord[i * 2] = vertices[i].TextureCoordinate.X;
			vtexcoord[i * 2 + 1] = vertices[i].TextureCoordinate.Y;
		}
	}

	void Face::calculateDeltaBlendshape(float* &neutralPositions)
	{
		this->deltaBlendshape = (float *)std::malloc(this->numVertices * 3 * sizeof(float));

		for (int i = 0; i < this->numVertices * 3; i++)
		{
			deltaBlendshape[i] = this->vpositions[i] - neutralPositions[i];
		}
	}

	void Face::calculateDeltaNormals(float* &neutralNormals)
	{
		this->deltaNormal = (float *)std::malloc(this->numVertices * 3 * sizeof(float));

		for (int i = 0; i < this->numVertices * 3; i++)
		{
			deltaNormal[i] = this->vnormals[i] - neutralNormals[i];
		}
	}
}
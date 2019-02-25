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

	int Face::findIntersectingVertex(glm::vec3 ray_world, int &intersectingVertex)
	{
		for (int i = 0; i < this->indices.size(); i += 3) {

			// Get the points in World co-ord
			vec4 p1 = this->globalTransform *
				vec4(mesh.Vertices[mesh.Indices[i]].Position.X,
					mesh.Vertices[mesh.Indices[i]].Position.Y,
					mesh.Vertices[mesh.Indices[i]].Position.Z, 1.0f);
			vec4 p2 = this->globalTransform *
				vec4(mesh.Vertices[mesh.Indices[i + 1]].Position.X,
					mesh.Vertices[mesh.Indices[i + 1]].Position.Y,
					mesh.Vertices[mesh.Indices[i + 1]].Position.Z, 1.0f);
			vec4 p3 = this->globalTransform *
				vec4(mesh.Vertices[mesh.Indices[i + 2]].Position.X,
					mesh.Vertices[mesh.Indices[i + 2]].Position.Y,
					mesh.Vertices[mesh.Indices[i + 2]].Position.Z, 1.0f);

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
					auto t = -(dot(N, cameraPos) - dot(N, vec3(p1))) / dot(N, ray_wor);

					if (t < zIntersection)
					{
						// Found a closer intersection point
						zIntersection = t;
						intersectionPoint = cameraPos + t * ray_wor;

						// Find the closest vertex
						int indexOfClosestPoint = Geometry::closestPoint(intersectionPoint, vec3(p1), vec3(p2), vec3(p3));

						// Set selected vertex - this may be overwritten by a closer point later on
						foundIntersection = true;

						vertexSelected = true;
						selectedSceneObject = sceneIndex;
						selectedObjectMesh = meshIndex;
						selectedVertexIndex = mesh.Indices[i + indexOfClosestPoint - 1];

						auto selectedVertexLocal = sceneObjects[selectedSceneObject].Meshes[selectedObjectMesh].Vertices[selectedVertexIndex].Position;
						tw_posX = selectedVertexLocal.X;
						tw_posY = selectedVertexLocal.Y;
						tw_posZ = selectedVertexLocal.Z;

						if (indexOfClosestPoint == 1)
						{
							tw_posX_world = p1.x;
							tw_posY_world = p1.y;
							tw_posZ_world = p1.z;
						}
						else if (indexOfClosestPoint = 2)
						{
							tw_posX_world = p2.x;
							tw_posY_world = p2.y;
							tw_posZ_world = p2.z;
						}
						else
						{
							tw_posX_world = p3.x;
							tw_posY_world = p3.y;
							tw_posZ_world = p3.z;
						}
					}
				}
			}
		}

		return -1; //no intersection
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
}
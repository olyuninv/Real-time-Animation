#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1 
#define GLFW_DLL

#include <sys/stat.h>

#include <stdio.h>
#include <cstdlib>
#include <stdlib.h>
#include <string>
#include <set>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>

#include <GL/glew.h>

#include <AntTweakBar.h>
#include <GLFW/glfw.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "opengl_utils.h"
#include "CGobject.h"

#include "dirent.h"

#include "..\Dependencies\OBJ_Loader.h"
#include "Blendshape.h"
#include "Face.h"
#include "Geometry.h"

// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#define MAX_OBJECTS 50
#define MAX_MARKERS 50
#define NUM_BLENDSHAPES 24
#define ANIM_LINES 250

using namespace glm;
using namespace std;
using namespace Assignment1;

// func declarations
void get_files_in_directory(
	std::set<std::string> &out, //list of file names within directory
	const std::string &directory //absolute path to the directory
);

bool read_obj(const std::string& filename,
	std::vector<glm::vec3> &positions,
	std::vector<glm::vec2> &tex_coords,
	std::vector<glm::vec3> &normals,
	std::vector<std::vector<int>> &indices);

size_t split(const std::string &txt, std::vector<std::string> &strs, char ch);

// variables
TwBar *bar;         // Pointer to a tweak bar

bool usingLowRes = false;
bool playAnimation = false;
int animationStage = 0;
double animationStartTime = 0;

// settings
const unsigned int SCR_WIDTH = 1920; // 1200; // 1920;
const unsigned int SCR_HEIGHT = 1100; // 650; //1100;

double time = 0, dt;// Current time and enlapsed time
double speed = 0.3; // Model rotation speed

glm::mat4 projection;
glm::mat4 view;

//CGObject sceneObjects[MAX_OBJECTS];
int numObjects = 0;

Face blendshapes[NUM_BLENDSHAPES];
int numBlendshapes = 0;

vector<CGObject> constraintObjects;

opengl_utils glutils;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

// camera movement
GLfloat rotate_angle = 0.0f;
bool firstMouse = true;
float myyaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float mypitch = 0.0f;
float lastX = SCR_WIDTH / 2.0; //800.0f / 2.0;
float lastY = SCR_HEIGHT / 2.0; //600.0 / 2.0;
float fov = 45.0f;
float nearclip = 0.1f;
float farclip = 100.0f;

// camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 10.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

GLuint VAOs[MAX_OBJECTS];
int numVAOs = 0;

GLuint faceVAO_positions;

int n_vbovertices = 0;
int n_ibovertices = 0;

//lighting position
glm::vec3 lightPos(1.0f, 2.0f, 4.0f);

bool lbutton_down = false;

// Selected vertex
bool vertexSelected = false;
int selectedVertexIndex = -1;

// use screenObjects for storing markers
bool markerSelected = false;  // if green marker clicked
int selectedMarkerIndex = -1;

//TW_only
float tw_posX = 0.0f;
float tw_posY = 0.0f;
float tw_posZ = 0.0f;
float tw_posX_world = 0.0f;
float tw_posY_world = 0.0f;
float tw_posZ_world = 0.0f;
bool allowDrag = false;


Face neutralFace;
Face customFace;

vector<int> constraintVertexIndices;
float* weights = (float *)std::malloc(NUM_BLENDSHAPES * sizeof(float));
float animation_weights[ANIM_LINES][24]; //[250][24];

void resetWeights(float *weights)
{
	for (int i = 0; i < NUM_BLENDSHAPES; i++)
	{
		weights[i] = 0.0f;
	}
}

void TW_CALL PlayAnimationCallback(void *clientData)
{
	playAnimation = true;
	animationStage = 0;

	float *selectedWeights = static_cast<float *>(clientData);

	resetWeights(selectedWeights);
}

void TW_CALL StopAnimationCallback(void *clientData)
{
	playAnimation = false;
	animationStage = 0;

	float *selectedWeights = static_cast<float *>(clientData);

	resetWeights(selectedWeights);
}

void TW_CALL SetConstraintCallback(void *clientData)
{
	vector<CGObject> *selectedObject = (static_cast<vector<CGObject> *>(clientData));

	// only set constraint when vertex picker selected
	if (markerSelected && selectedMarkerIndex == 0)
	{
		constraintVertexIndices.push_back(selectedVertexIndex);

		// create new sceneObject - marker		
		CGObject sphereObject = CGObject();
		sphereObject.Meshes = (*selectedObject)[0].Meshes;
		sphereObject.initialTranslateVector = vec3(0.0, 0.0, 0.0); // this is reset at each draw		
		sphereObject.initialScaleVector = vec3(0.2f / customFace.initialScaleVector.x,
			0.2f / customFace.initialScaleVector.y,
			0.2f / customFace.initialScaleVector.z);

		sphereObject.color = glm::vec3(0.0, 0.0, 1.0);
		sphereObject.startVBO = (*selectedObject)[0].startVBO;
		sphereObject.startIBO = (*selectedObject)[0].startIBO;

		sphereObject.VAOs = std::vector<GLuint>();
		sphereObject.VAOs.push_back((*selectedObject)[0].VAOs[0]);

		selectedObject->push_back(sphereObject);
		selectedMarkerIndex = numObjects;

		numObjects++;
	}

}

void TW_CALL RemoveConstraintCallback(void *clientData)
{
	vector<CGObject> *selectedObject = (static_cast<vector<CGObject> *>(clientData));

	if (markerSelected && selectedMarkerIndex != 0)
	{
		// only remove constraint when constraint selected
		constraintVertexIndices.erase(constraintVertexIndices.begin() + selectedMarkerIndex);
		(*selectedObject).erase((*selectedObject).begin() + selectedMarkerIndex);
		numObjects--;
	}

}

void TW_CALL ResetCallback(void *clientData)
{
	float *selectedWeights = static_cast<float *>(clientData);

	resetWeights(selectedWeights);
}

// Callback function called by GLFW when window size changes
void GLFWCALL WindowSizeCB(int width, int height)
{
	// Send the new window size to AntTweakBar
	glViewport(0, 0, width, height);

	TwWindowSize(width, height);
}

void GLFWCALL mouse_pos_callback(int mouseX, int mouseY)
{
	if (!TwEventMousePosGLFW(mouseX, mouseY))
	{
		if (lbutton_down && allowDrag)
		{
			// drag marker
			// set to intersection with z-plane
			float x = (2.0f * mouseX) / SCR_WIDTH - 1.0f;
			float y = 1.0f - (2.0f * mouseY) / SCR_HEIGHT;
			float z = 1.0f;

			vec3 ray_nds = vec3(x, y, z);

			vec4 ray_clip = vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);

			vec4 ray_eye = inverse(projection) * ray_clip;
			ray_eye = vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

			vec4 temp = (inverse(view) * ray_eye);

			vec3 ray_wor = vec3(temp.x, temp.y, temp.z);
			// don't forget to normalise the vector at some point
			ray_wor = glm::normalize(ray_wor);

			glm::vec3 N = glm::vec3(0.0, 0.0, 1.0);
			glm::vec4 p1 = customFace.globalTransform * glm::vec4(0.0, 0.0, constraintObjects[0].position.z, 0.0);

			auto t = -(dot(N, cameraPos) - dot(N, vec3(p1))) / dot(N, ray_wor);
			glm::vec3 intersectionPoint = cameraPos + t * ray_wor;

			constraintObjects[0].position = intersectionPoint;

		}
	}
}

void GLFWCALL mouse_button_callback(int button, int action)
{
	if (!TwEventMouseButtonGLFW(button, action))   // Send event to AntTweakBar
	{
		if (!playAnimation)
		{
			if (button == GLFW_MOUSE_BUTTON_LEFT)
			{
				if (GLFW_PRESS == action)
				{
					lbutton_down = true;
				}
				else if (GLFW_RELEASE == action)
				{
					lbutton_down = false;
					allowDrag = false;
				}

				if (lbutton_down)
				{
					int mouse_x, mouse_y;

					//getting cursor position
					glfwGetMousePos(&mouse_x, &mouse_y);

					bool foundIntersection = false;
					bool foundIntersectionWithMarker = false;

					float x = (2.0f * mouse_x) / SCR_WIDTH - 1.0f;
					float y = 1.0f - (2.0f * mouse_y) / SCR_HEIGHT;
					float z = 1.0f;

					vec3 ray_nds = vec3(x, y, z);

					vec4 ray_clip = vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);

					vec4 ray_eye = inverse(projection) * ray_clip;
					ray_eye = vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

					vec4 temp = (inverse(view) * ray_eye);

					vec3 ray_wor = vec3(temp.x, temp.y, temp.z);
					ray_wor = glm::normalize(ray_wor);

					// Find the other end of the ray (at far clip plane)
					auto farpoint = cameraPos + ray_wor * farclip;

					// Find closest intersection point
					float zIntersection = farclip;
					vec3 intersectionPoint;

					objl::Mesh mesh;

					// LOOK for intersection with marker

					for (int sceneIndex = 0; sceneIndex < numObjects; sceneIndex++)
					{
						for (int meshIndex = 0; meshIndex < constraintObjects[sceneIndex].Meshes.size(); meshIndex++)
						{
							// Find intersection with the fragment
							mesh = constraintObjects[sceneIndex].Meshes[meshIndex];

							for (int i = 0; i < mesh.Indices.size(); i += 3) {
								// Get the points in World co-ord

								vec4 p1 = constraintObjects[sceneIndex].globalTransform *
									vec4(mesh.Vertices[mesh.Indices[i]].Position.X,
										mesh.Vertices[mesh.Indices[i]].Position.Y,
										mesh.Vertices[mesh.Indices[i]].Position.Z, 1.0f);
								vec4 p2 = constraintObjects[sceneIndex].globalTransform *
									vec4(mesh.Vertices[mesh.Indices[i + 1]].Position.X,
										mesh.Vertices[mesh.Indices[i + 1]].Position.Y,
										mesh.Vertices[mesh.Indices[i + 1]].Position.Z, 1.0f);
								vec4 p3 = constraintObjects[sceneIndex].globalTransform*
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
										// Intersection found

										// Normal to the triangle
										auto N = cross(vec3(p2) - vec3(p1), vec3(p3) - vec3(p1));

										// distance to intersection on the line:
										//auto t = -dot(cameraPos, N - vec3(p1)) / dot(cameraPos, farpoint - cameraPos); //ray_wor);
										auto t = -(dot(N, cameraPos) - dot(N, vec3(p1))) / dot(N, ray_wor);

										if (t < zIntersection)
										{
											// Found a closer intersection point
											zIntersection = t;

											// no need to calculate exact vertex
											foundIntersection = true;
											markerSelected = true;

											selectedMarkerIndex = sceneIndex;  // this will be the marker that we move			

											if (sceneIndex == 0)
											{
												// selected Marker - allow to drag
												allowDrag = true;
											}
										}
									}
								}
							}
						}
					}

					if (!foundIntersection)
					{
						//clear marker selection
						selectedMarkerIndex = -1;
					}

					// LOOK for intersection with face
					if (!foundIntersection)
					{
						// Look for intersection with the face
						int intersectingVertexIndex = -1;
						glm::vec3 pointWorld;
						foundIntersection = customFace.findIntersectingVertex(ray_wor, cameraPos, farclip, intersectingVertexIndex, pointWorld);

						if (foundIntersection)
						{
							vertexSelected = true;
							selectedVertexIndex = intersectingVertexIndex;
							selectedMarkerIndex = 0;

							tw_posX = customFace.vpositions[selectedVertexIndex * 3];
							tw_posY = customFace.vpositions[selectedVertexIndex * 3 + 1];
							tw_posZ = customFace.vpositions[selectedVertexIndex * 3 + 2];

							tw_posX_world = pointWorld.x;
							tw_posY_world = pointWorld.y;
							tw_posZ_world = pointWorld.z;

						}
					}

					if (!foundIntersection)
					{
						vertexSelected = false;
						selectedVertexIndex = -1;
						tw_posX = 0.0;
						tw_posY = 0.0;
						tw_posZ = 0.0;

						tw_posX_world = 0.0;
						tw_posY_world = 0.0;
						tw_posZ_world = 0.0;
					}

					//if (lbutton_down && allowDrag)
					//{
					//	// drag marker
					//	// set to intersection with z-plane
					//	int s = 3;
					//	//constraintObjects[0].position.z
					//	//constraintObjects[0].position = 

					//}
				}
			}
		}

	}
}

void GLFWCALL key_callback(int button, int action)
{
	if (!TwEventKeyGLFW(button, action))   // Send event to AntTweakBar
	{
		if (256 == button && action == GLFW_PRESS)
			glfwCloseWindow();

		float cameraSpeed = 2.5 * dt;
		if (87 == button && action == GLFW_PRESS)
			cameraPos += cameraSpeed * cameraFront;
		if (83 == button && action == GLFW_PRESS)
			cameraPos -= cameraSpeed * cameraFront;
		if (65 == button && action == GLFW_PRESS)
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		if (68 == button && action == GLFW_PRESS)
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}
}

void addToObjectBuffer(Assignment1::CGObject *cg_object)
{
	int VBOindex = cg_object->startVBO;
	int IBOindex = cg_object->startIBO;

	for (int i = 0; i < cg_object->Meshes.size(); i++) {

		//TODO: Remove call to gl to glutils
		glutils.addVBOBufferSubData(VBOindex, cg_object->Meshes[i].Vertices.size(), &cg_object->Meshes[i].Vertices[0].Position.X);
		glutils.linkCurrentBuffertoShader(cg_object->VAOs[i], VBOindex, IBOindex);
		VBOindex += cg_object->Meshes[i].Vertices.size();
		IBOindex += cg_object->Meshes[i].Indices.size();
	}
}

void addToIndexBuffer(Assignment1::CGObject *cg_object)
{
	int IBOindex = cg_object->startIBO;
	for (auto const& mesh : cg_object->Meshes) {
		glutils.addIBOBufferSubData(IBOindex, mesh.Indices.size(), &mesh.Indices[0]);
		IBOindex += mesh.Indices.size();
	}
}

//---------------------------------------------------------------------------------------------------------//

std::vector<objl::Mesh> loadMeshes(const string objFileLocation)
{
	objl::Loader obj_loader;

	bool result = obj_loader.LoadFile(objFileLocation);
	if (result && obj_loader.LoadedMeshes.size() > 0)
	{
		return obj_loader.LoadedMeshes;
	}
	else
		throw new exception("Could not load mesh");
}

Assignment1::CGObject loadObjObject(vector<objl::Mesh> meshes, bool addToBuffers, bool subjectToGravity, vec3 initTransformVector, vec3 initScaleVector, vec3 color, float coef, CGObject* parent)
{
	Assignment1::CGObject object = Assignment1::CGObject();
	object.Meshes = meshes;
	object.subjectToGravity = subjectToGravity;
	object.initialTranslateVector = initTransformVector;
	object.position = initTransformVector;
	object.initialScaleVector = initScaleVector;
	object.color = color;
	object.coef = coef;
	object.Parent = parent;
	object.startVBO = n_vbovertices;
	object.startIBO = n_ibovertices;
	object.VAOs = std::vector<GLuint>();

	if (addToBuffers)
	{
		for (auto const& mesh : meshes) {
			glutils.generateVertexArray(&(VAOs[numVAOs]));
			GLuint tmpVAO = VAOs[numVAOs];
			object.VAOs.push_back(tmpVAO);
			n_vbovertices += mesh.Vertices.size();
			n_ibovertices += mesh.Indices.size();
			numVAOs++;
		}
	}

	return object;
}

Face createFace(string faceName, objl::Mesh mesh)
{
	int numVertices = mesh.Vertices.size();

	float* v_positions = (float *)std::malloc(numVertices * 3 * sizeof(float));
	float* v_normals = (float *)std::malloc(numVertices * 3 * sizeof(float));
	float* v_texcoords = (float *)std::malloc(numVertices * 2 * sizeof(float));

	// !creates 3 new arrays
	Face::getPositionsAndNormalsFromObjl(mesh.Vertices,
		v_positions,
		v_normals,
		v_texcoords);

	Face newFace = Face(numVertices,
		v_positions,
		v_normals,
		v_texcoords);

	newFace.name = faceName;
	newFace.indices = mesh.Indices;

	return newFace;
}

void createFaces()
{
	string dirName;
	string neutralFileName;

	if (usingLowRes)
	{
		dirName = "../Assignment1/meshes/Low-res Blendshape Model";
		neutralFileName = "Mery_jaw_open.obj";
	}
	else
	{
		dirName = "../Assignment1/meshes/High-res Blendshape Model";
		neutralFileName = "neutral.obj";
	}

	// Add neutral face
	string neutralFileNameFull = dirName + "/" + neutralFileName;
	vector<objl::Mesh> neutralMesh = loadMeshes(neutralFileNameFull);

	std::vector<objl::Mesh> new_meshNeutral;
	CGObject::recalculateVerticesAndIndexes(neutralMesh, new_meshNeutral);

	int numberOfVertices = new_meshNeutral[0].Vertices.size();
	int numberOfIndices = new_meshNeutral[0].Indices.size();

	neutralFace = createFace(neutralFileName, new_meshNeutral[0]);

	// Load blendshapes
	set<string> fileList;
	get_files_in_directory(fileList, dirName);

	for (auto const& filename : fileList) {

		if (numBlendshapes == NUM_BLENDSHAPES)
			break;

		if (filename != neutralFileName)
		{
			const string fullfileName = dirName + "/" + filename;
			vector<objl::Mesh> faceMeshes = loadMeshes(fullfileName);

			std::vector<objl::Mesh> new_meshBlendshape;
			CGObject::recalculateVerticesAndIndexes(faceMeshes, new_meshBlendshape);

			if (numberOfVertices == new_meshBlendshape[0].Vertices.size())
			{
				Face face1 = createFace(filename, new_meshBlendshape[0]);

				face1.calculateDeltaBlendshape(neutralFace.vpositions);
				face1.calculateDeltaNormals(neutralFace.vnormals);
				blendshapes[numBlendshapes] = face1;
				numBlendshapes++;
			}
		}
	}

	float* customPositions = (float *)std::malloc(numberOfVertices * 3 * sizeof(float));
	float* customNormals = (float *)std::malloc(numberOfVertices * 3 * sizeof(float));
	blendshape::calculateFace(neutralFace, NUM_BLENDSHAPES, blendshapes, weights, customPositions, customNormals);

	//blendshape::recalculateNormals(neutralFace.indices, numberOfVertices, customPositions, customNormals);

	// Generate the only face to be displayed - from neutral for now
	customFace = Face(numberOfVertices,
		customPositions,
		customNormals,
		//neutralFace.vnormals,
		neutralFace.vtexcoord);
	customFace.name = "Custom";
	customFace.indices = neutralFace.indices;
	customFace.initialScaleVector = glm::vec3(0.2f, 0.2f, 0.2f);

	glGenVertexArrays(1, &faceVAO_positions);

	// The VBO containing the face positions
	glBindVertexArray(faceVAO_positions);

	glGenBuffers(1, &glutils.faceVBO_positions);
	glBindBuffer(GL_ARRAY_BUFFER, glutils.faceVBO_positions);
	glBufferData(GL_ARRAY_BUFFER,
		numberOfVertices * 3 * sizeof(float),
		customFace.vpositions,
		GL_STREAM_DRAW);

	glGenBuffers(1, &glutils.faceVBO_normals);
	glBindBuffer(GL_ARRAY_BUFFER, glutils.faceVBO_normals);
	glBufferData(GL_ARRAY_BUFFER,
		numberOfVertices * 3 * sizeof(float),
		customFace.vnormals,
		GL_STREAM_DRAW);

	glGenBuffers(1, &glutils.faceVBO_texcoord);
	glBindBuffer(GL_ARRAY_BUFFER, glutils.faceVBO_texcoord);
	glBufferData(GL_ARRAY_BUFFER,
		numberOfVertices * 2 * sizeof(float),
		customFace.vtexcoord,
		GL_STATIC_DRAW);

	glGenBuffers(1, &glutils.faceIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glutils.faceIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		numberOfIndices * sizeof(unsigned int),
		&customFace.indices[0],
		GL_STATIC_DRAW);

	glutils.linkFaceBuffertoShader(faceVAO_positions);
}

void createObjects()
{
	// Shader Attribute locations
	glutils.getAttributeLocations();

	createFaces();

	//CGObject testObject = loadObjObject(testMeshes, true, true, vec3(0.0f, -3.0f, 0.0f), vec3(0.2f, 0.2f, 0.2f), vec3(1.0f, 1.0f, 1.0f), 0.65f, NULL);
	//sceneObjects[numObjects] = testObject;
	//numObjects++;

	/*const char* boyFileName = "../Assignment1/meshes/Head/male head.obj";
	vector<objl::Mesh> meshes = loadMeshes(boyFileName);
	CGObject boyObject = loadObjObject(meshes, true, true, vec3(0.0f, 0.0f, 0.0f), vec3(0.15f, 0.15f, 0.15f), vec3(1.0f, 1.0f, 1.0f), 0.65f, NULL);
	sceneObjects[numObjects] = boyObject;
	numObjects++;*/

	/*const char* cubeFileName = "../Assignment1/meshes/Cube/cube.obj";
	vector<objl::Mesh> cubeMeshes = loadMeshes(cubeFileName);
	CGObject cubeObject = loadObjObject(cubeMeshes, true, true, vec3(5.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f), 0.65f, NULL);
	sceneObjects[numObjects] = cubeObject;
	numObjects++; */

	n_vbovertices = 0;
	n_ibovertices = 0;

	const char* sphereFileName = "../Assignment1/meshes/Sphere/sphere.obj";
	vector<objl::Mesh> sphereMeshes = loadMeshes(sphereFileName);
	CGObject sphereObject = loadObjObject(sphereMeshes, true, true, vec3(0.0f, 0.0f, 0.0f), vec3(0.4f, 0.4f, 0.4f), vec3(0.0f, 1.0f, 0.0f), 0.65f, NULL);
	sphereObject.initialScaleVector = vec3(0.2f / customFace.initialScaleVector.x,
		0.2f / customFace.initialScaleVector.y,
		0.2f / customFace.initialScaleVector.z);

	constraintObjects.push_back(sphereObject);

	//selectedMarkerIndex = numObjects;
	numObjects++;
	constraintVertexIndices.push_back(-1); //hack - so that constraints and sceneobjects have the same index

	glutils.createVBO(n_vbovertices);

	glutils.createIBO(n_ibovertices);

	//addToObjectBuffer(&testObject);
	//addToObjectBuffer(&boyObject);
	/*addToObjectBuffer(&cubeObject);*/
	addToObjectBuffer(&sphereObject);

	//addToIndexBuffer(&testObject);
	//addToIndexBuffer(&boyObject);
	//addToIndexBuffer(&cubeObject);
	addToIndexBuffer(&sphereObject);

	glutils.linkCurrentBuffertoShader(sphereObject.VAOs[0], sphereObject.startVBO, sphereObject.startIBO);
}

void readAnimationFile(string file)
{

	ifstream myReadFile;
	int lineNumber = 0;  //
	int weightIndex = 0;

	myReadFile.open(file);

	char output[100];
	if (myReadFile.is_open()) {
		while (!myReadFile.eof()) {

			myReadFile >> output;

			float d2 = strtod(output, NULL);
			animation_weights[lineNumber][weightIndex] = d2;
			weightIndex++;

			if (weightIndex == 24)
			{
				lineNumber++;
				weightIndex = 0;
			}
		}
	}
	myReadFile.close();
}

void init()
{
	glEnable(GL_DEPTH_TEST);

	glutils = opengl_utils();

	glutils.createShaders();

	glutils.setupUniformVariables();

	readAnimationFile("../Assignment1/meshes/blendshape_animation.txt");

	createObjects();
}

void drawFace(glm::mat4 projection, glm::mat4 view)
{
	glPushMatrix();
	glLoadIdentity();

	// DRAW FACE

	//if (newWeightsLength != prev_weights_length)
	//{		
	free(customFace.vpositions);
	free(customFace.vnormals);

	float* customPositions = (float *)std::malloc(neutralFace.numVertices * 3 * sizeof(float));
	float* customNormals = (float *)std::malloc(neutralFace.numVertices * 3 * sizeof(float));

	blendshape::calculateFace(neutralFace, NUM_BLENDSHAPES, blendshapes, weights, customPositions, customNormals);
	//blendshape::recalculateNormals(neutralFace.indices, neutralFace.numVertices, customFace.vpositions, customFace.vnormals);

	//// Generate the only face to be displayed - from neutral for now
	////tidy up
	//delete[] customFace.vpositions;
	//delete[] customFace.vnormals;

	////re-assign pointer
	customFace.vpositions = customPositions;
	customFace.vnormals = customNormals;

	glBindVertexArray(faceVAO_positions);

	glGenBuffers(1, &glutils.faceVBO_positions);
	glBindBuffer(GL_ARRAY_BUFFER, glutils.faceVBO_positions);
	glBufferData(GL_ARRAY_BUFFER,
		customFace.numVertices * 3 * sizeof(float),
		customFace.vpositions,
		GL_STREAM_DRAW);

	glGenBuffers(1, &glutils.faceVBO_normals);
	glBindBuffer(GL_ARRAY_BUFFER, glutils.faceVBO_normals);
	glBufferData(GL_ARRAY_BUFFER,
		customFace.numVertices * 3 * sizeof(float),
		customFace.vnormals,
		GL_STREAM_DRAW);

	//prev_weights_length = newWeightsLength;
//}

	glutils.linkFaceBuffertoShader(faceVAO_positions);

	glm::mat4 faceTransform = glm::mat4(1);
	faceTransform = glm::translate(faceTransform, vec3(0.0f, -3.0f, 0.0f));
	faceTransform = glm::scale(faceTransform, customFace.initialScaleVector);

	glutils.updateUniformVariables(faceTransform, view, projection);
	customFace.globalTransform = faceTransform; // keep current state	

	glUniform3f(glutils.objectColorLoc, 1.0f, 0.5f, 0.0f);
	glDrawElements(GL_TRIANGLES, customFace.indices.size(), GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(glutils.loc1);
	glDisableVertexAttribArray(glutils.loc2);
	glDisableVertexAttribArray(glutils.loc3);

	glPopMatrix();
}

void display()
{
	if (usingLowRes)
	{
		lightPos.z = 1.0;

	}

	// render
	glClearColor(0.78f, 0.84f, 0.49f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Rotate model
	dt = glfwGetTime() - time;
	if (dt < 0) dt = 0;
	time += dt;

	if (playAnimation)
	{
		// update weights
		double deltaTime = time - animationStartTime;

		if (deltaTime > 4)
		{
			for (int k = 0; k < NUM_BLENDSHAPES; k++)
			{
				weights[k] = animation_weights[animationStage][k];
			}

			animationStage++;
		}

		if (animationStage >= ANIM_LINES)
		{
			// reached end of animation
			animationStage = 0;
			playAnimation = false;
			resetWeights(weights);
		}
	}

	// Update projection 
	projection = glm::perspective(glm::radians(fov), (float)(SCR_WIDTH) / (float)(SCR_HEIGHT), nearclip, farclip);
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

	glm::mat4 local1(1.0f);
	local1 = glm::translate(local1, cameraPos);
	glm::mat4 global1 = local1;

	glutils.updateUniformVariables(global1, view, projection);

	// activate shader
	glUseProgram(glutils.PhongProgramID);

	// Enable OpenGL transparency and light (could have been done once at init)
	glEnable(GL_SMOOTH);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHT0);    // use default light diffuse and position
	//glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
	//glEnable(GL_COLOR_MATERIAL);
	//glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
	//glEnable(GL_LINE_SMOOTH);
	//glLineWidth(3.0);
	//glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glUniform3f(glutils.lightColorLoc, 1.0f, 1.0f, 1.0f);
		
	glUniform3f(glutils.lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
	
	glUniform3f(glutils.viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);

	drawFace(projection, view);

	// DRAW CONSTRAINTS
	if (!playAnimation)
	{
		glPushMatrix();
		glLoadIdentity();

		// draw selected vertex = always tied to selected vertex index
		if (vertexSelected)
		{
			mat4 globalCGObjectTransform;

			if (!allowDrag)
			{
				constraintObjects[0].position = vec3(customFace.vpositions[selectedVertexIndex * 3],
					customFace.vpositions[selectedVertexIndex * 3 + 1],
					customFace.vpositions[selectedVertexIndex * 3 + 2]);
				globalCGObjectTransform = customFace.globalTransform * constraintObjects[0].createTransform();  //sphere	
			}			
			else
			{
				mat4 localTransform = mat4(1.0);
				localTransform = glm::translate(localTransform, constraintObjects[0].position);
				/*localTransform = glm::scale(localTransform, glm::vec3(0.2f / customFace.initialScaleVector.x,
					0.2f / customFace.initialScaleVector.y,
					0.2f / customFace.initialScaleVector.z));
				*/
				localTransform = glm::scale(localTransform, vec3(0.2f, 0.2f, 0.2f));
				globalCGObjectTransform =  localTransform; //customFace.globalTransform *

			}
			
			glutils.updateUniformVariables(globalCGObjectTransform);
			constraintObjects[0].globalTransform = globalCGObjectTransform; // keep current state		

			glUniform3f(glutils.objectColorLoc, constraintObjects[0].color.r, constraintObjects[0].color.g, constraintObjects[0].color.b);
			constraintObjects[0].Draw(glutils);
		}

		// DRAW constraints - even if no vertex selected
		for (int i = 1; i < numObjects; i++)
		{
			constraintObjects[i].position = vec3(customFace.vpositions[constraintVertexIndices[i] * 3],
				customFace.vpositions[constraintVertexIndices[i] * 3 + 1],
				customFace.vpositions[constraintVertexIndices[i] * 3 + 2]);
			mat4 globalCGObjectTransform = customFace.globalTransform * constraintObjects[i].createTransform();  //sphere


			glutils.updateUniformVariables(globalCGObjectTransform);
			constraintObjects[i].globalTransform = globalCGObjectTransform; // keep current state		

			glUniform3f(glutils.objectColorLoc, constraintObjects[i].color.r, constraintObjects[i].color.g, constraintObjects[i].color.b);
			constraintObjects[i].Draw(glutils);
		}

		/*glDisableVertexAttribArray(glutils.loc1);
		glDisableVertexAttribArray(glutils.loc2);
		glDisableVertexAttribArray(glutils.loc3);*/


		glPopMatrix();
	}

	// Draw tweak bars
	TwDraw();

	// Present frame buffer
	glfwSwapBuffers();
}

int main(void)
{
	GLFWvidmode mode;   // GLFW video mode

	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	// Create a window
	glfwGetDesktopMode(&mode);
	if (!glfwOpenWindow(SCR_WIDTH, SCR_HEIGHT, mode.RedBits, mode.GreenBits, mode.BlueBits,
		0, 16, 0, GLFW_WINDOW /* or GLFW_FULLSCREEN */))
	{
		// A fatal error occured    
		fprintf(stderr, "Cannot open GLFW window\n");
		glfwTerminate();
		return 1;
	}

	glfwEnable(GLFW_MOUSE_CURSOR);
	glfwEnable(GLFW_KEY_REPEAT);
	glfwSetWindowTitle("Facial Animation");

	// Initialize AntTweakBar
	TwInit(TW_OPENGL_CORE, NULL);

	// Create a tweak bar
	bar = TwNewBar("Blendshapes");

	// Change the font size, and add a global message to the Help bar.
	TwDefine(" Blendshapes size=' 320 450 ' valueswidth=100 GLOBAL fontSize=3 help='Change parameters to control blendshapes' ");

	TwAddButton(bar, "Reset", ResetCallback, weights, " label='Reset' ");

	TwAddButton(bar, "PlayAnimation", PlayAnimationCallback, weights, " label='Play Animation' ");
	TwAddButton(bar, "StopAnimation", StopAnimationCallback, weights, " label='Stop Animation' ");

	TwAddSeparator(bar, "", NULL);

	// Add 'time' to 'bar': it is a read-only (RO) variable of type TW_TYPE_DOUBLE, with 1 precision digit
	TwAddVarRO(bar, "Vertex selected", TW_TYPE_BOOLCPP, &vertexSelected, " label='Vertex selected' group = 'Vertex Picking' precision=1 help='Indicates if vertex is selected.' ");

	//TwAddVarRO(bar, "selected Scene Object", TW_TYPE_INT32, &selectedSceneObject, " label='Selected scene object' precision=0 help='Index of the selected object.' ");

	//TwAddVarRO(bar, "selected Mesh index", TW_TYPE_INT32, &selectedObjectMesh, " label='Selected mesh index' precision=0 help='Index of the selected mesh.' ");

	TwAddVarRO(bar, "selected Vertex index", TW_TYPE_INT32, &selectedVertexIndex, " label='Selected vertex index' group = 'Vertex Picking' precision=0 help='Index of the selected vertex.' ");

	TwAddVarRO(bar, "selected - posX", TW_TYPE_FLOAT, &tw_posX,
		" label='PosX - local' group = 'Vertex Picking' precision=2 help='local X-coord of the selected vertex.' ");
	TwAddVarRO(bar, "selected - posY", TW_TYPE_FLOAT, &tw_posY,
		" label='PosY - local' group = 'Vertex Picking' precision=2 help='local Y-coord of the selected vertex.' ");
	TwAddVarRO(bar, "selected - posZ", TW_TYPE_FLOAT, &tw_posZ,
		" label='PosZ - local' group = 'Vertex Picking' precision=2 help='local Z-coord of the selected vertex.' ");


	TwAddVarRO(bar, "selected - posX - world", TW_TYPE_FLOAT, &tw_posX_world,
		" label='PosX - world' group = 'Vertex Picking' precision=2 help='World X-coord of the selected vertex.' ");
	TwAddVarRO(bar, "selected - posY - world", TW_TYPE_FLOAT, &tw_posY_world,
		" label='PosY - world' group = 'Vertex Picking' precision=2 help='World Y-coord of the selected vertex.' ");
	TwAddVarRO(bar, "selected - posZ - world", TW_TYPE_FLOAT, &tw_posZ_world,
		" label='PosZ - world' group = 'Vertex Picking' precision=2 help='World Z-coord of the selected vertex.' ");

	TwAddButton(bar, "SetConstraint", SetConstraintCallback, &constraintObjects, " label='Set Constraint' group = 'Vertex Picking' ");
	TwAddButton(bar, "RemoveConstraint", RemoveConstraintCallback, &constraintObjects, " label='Remove Constraint' group = 'Vertex Picking' ");

	TwAddSeparator(bar, "", NULL);

	// initialise weights
	for (int i = 0; i < NUM_BLENDSHAPES; i++)
	{
		weights[i] = 0.0f;
	}


	// Load blendshapes
	string dirName;
	string neutralFileName;

	if (usingLowRes)
	{
		dirName = "../Assignment1/meshes/Low-res Blendshape Model";
		neutralFileName = "Mery_jaw_open.obj";
	}
	else
	{
		dirName = "../Assignment1/meshes/High-res Blendshape Model";
		neutralFileName = "neutral.obj";
	}

	set<string> fileList;
	get_files_in_directory(fileList, dirName);

	int i = 0;
	for (auto const& filename : fileList) {

		if (i == NUM_BLENDSHAPES)
			break;

		if (filename != neutralFileName)
		{
			string name = filename.substr(0, filename.size() - 4);
			string desc = " precision=2 step=0.05 min=0.0 max=1.0 group = 'Facial Expressions' label='" + name + "'";
			TwAddVarRW(bar, name.c_str(), TW_TYPE_FLOAT, &weights[i], desc.c_str());
			i++;
		}
	}

	// Add 'time' to 'bar': it is a read-only (RO) variable of type TW_TYPE_DOUBLE, with 1 precision digit
	//TwAddVarCB(bar, "JawOpen", TW_TYPE_FLOAT, SetCallbackLocalJawOpen, GetCallbackLocalJawOpen, &weights, " group='Facial Expressions' step=0.05 label='Jaw Open' precision=2 ");

	// Set GLFW event callbacks
	// - Redirect window size changes to the callback function WindowSizeCB
	glfwSetWindowSizeCallback(WindowSizeCB);
	// - Directly redirect GLFW mouse button events to AntTweakBar
	glfwSetMouseButtonCallback(mouse_button_callback);
	// - Directly redirect GLFW mouse position events to AntTweakBar
	glfwSetMousePosCallback(mouse_pos_callback); //	(GLFWmouseposfun)TwEventMousePosGLFW)
	// - Directly redirect GLFW mouse wheel events to AntTweakBar
	glfwSetMouseWheelCallback((GLFWmousewheelfun)TwEventMouseWheelGLFW);
	// - Directly redirect GLFW key events to AntTweakBar
	glfwSetKeyCallback(key_callback);
	// - Directly redirect GLFW char events to AntTweakBar
	glfwSetCharCallback((GLFWcharfun)TwEventCharGLFW);

	// Initialize time
	time = glfwGetTime();

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	init();

	// Main loop (repeated while window is not closed and [ESC] is not pressed)
	while (glfwGetWindowParam(GLFW_OPENED) && !glfwGetKey(GLFW_KEY_ESC))
	{
		display();
	}

	// delete blendshapes
	std::free(neutralFace.vpositions);
	std::free(neutralFace.vnormals);
	std::free(neutralFace.vtexcoord);
	std::free(customFace.vpositions);
	std::free(customFace.vnormals);

	for (auto face : blendshapes) {
		std::free(face.vpositions);
		std::free(face.vnormals);
		std::free(face.vtexcoord);
		std::free(face.deltaBlendshape);
	}

	std::free(weights);

	// optional: de-allocate all resources once they've outlived their purpose:	
	glutils.deleteVertexArrays();
	glutils.deletePrograms();
	glutils.deleteBuffers();

	TwTerminate();
	glfwTerminate();
	return 0;
}

void get_files_in_directory(
	std::set<std::string> &out, //list of file names within directory
	const std::string &directory //absolute path to the directory
)
{
#ifdef _WIN32
	HANDLE dir;
	WIN32_FIND_DATA file_data;

	if ((dir = FindFirstFile((directory + "/*").c_str(), &file_data)) == INVALID_HANDLE_VALUE)
		return; /* No files found */

	do {
		const std::string file_name = file_data.cFileName;
		//const std::string full_file_name = directory + "/" + file_name;
		const bool is_directory = (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

		if (file_name[0] == '.')
			continue;

		if (is_directory)
			continue;

		out.insert(file_name);
	} while (FindNextFile(dir, &file_data));

	FindClose(dir);
#else
	DIR *dir;
	class dirent *ent;
	class stat st;

	dir = opendir(directory.c_str());
	while ((ent = readdir(dir)) != NULL) {
		const std::string file_name = ent->d_name;
		const std::string full_file_name = directory + "/" + file_name;

		if (file_name[0] == '.')
			continue;

		if (stat(full_file_name.c_str(), &st) == -1)
			continue;

		const bool is_directory = (st.st_mode & S_IFDIR) != 0;

		if (is_directory)
			continue;

		out.insert(file_name);
	}
	closedir(dir);
#endif
}

bool read_obj(const std::string& filename,
	std::vector<vec3> &positions,
	std::vector<vec2> &tex_coords,
	std::vector<vec3> &normals,
	std::vector<std::vector<int>> &indices)
{
	char   s[200];
	float  x, y, z;
	//std::vector<vec3> positions; //vertex positions
	//std::vector<vec2> tex_coords;   //texture coordinates
	//std::vector<vec3> normals;  //normals 
	//std::vector<std::vector<int>> indices; //connectivity

	// open file (in ASCII mode)
	FILE* in = fopen(filename.c_str(), "r");
	if (!in) return false;


	// clear line once
	memset(&s, 0, 200);


	// parse line by line (currently only supports vertex positions & faces
	while (in && !feof(in) && fgets(s, 200, in))
	{
		// comment
		if (s[0] == '#' || isspace(s[0])) continue;

		// vertex
		else if (strncmp(s, "v ", 2) == 0)
		{
			if (sscanf(s, "v %f %f %f", &x, &y, &z))
			{
				positions.push_back(vec3(x, y, z));
			}
		}
		// normal
		else if (strncmp(s, "vn ", 3) == 0)
		{
			if (sscanf(s, "vn %f %f %f", &x, &y, &z))
			{
				normals.push_back(vec3(x, y, z));
			}
		}

		// texture coordinate
		else if (strncmp(s, "vt ", 3) == 0)
		{
			if (sscanf(s, "vt %f %f", &x, &y))
			{
				tex_coords.push_back(vec2(x, y));
			}
		}

		// face
		else if (strncmp(s, "f ", 2) == 0)
		{
			int component(0), nV(0);
			bool endOfVertex(false);
			char *p0, *p1(s + 1);

			std::vector<int> polygon;

			// skip white-spaces
			while (*p1 == ' ') ++p1;

			while (p1)
			{
				p0 = p1;

				// overwrite next separator

				// skip '/', '\n', ' ', '\0', '\r' <-- don't forget Windows
				while (*p1 != '/' && *p1 != '\r' && *p1 != '\n' && *p1 != ' ' && *p1 != '\0') ++p1;

				// detect end of vertex
				if (*p1 != '/')
				{
					endOfVertex = true;
				}

				// replace separator by '\0'
				if (*p1 != '\0')
				{
					*p1 = '\0';
					p1++; // point to next token
				}

				// detect end of line and break
				if (*p1 == '\0' || *p1 == '\n')
				{
					p1 = 0;
				}

				// read next vertex component
				if (*p0 != '\0')
				{
					switch (component)
					{
					case 0: // vertex
					{
						polygon.push_back(atoi(p0) - 1);
						break;
					}
					case 1: // texture coord
					{
						//add code for saving texture coordinate index
						break;
					}
					case 2: // normal
					  //add code for saving normal coordinate index
						break;
					}
				}

				++component;

				if (endOfVertex)
				{
					component = 0;
					nV++;
					endOfVertex = false;
				}
			}

			indices.push_back(polygon);

		}
		// clear line
		memset(&s, 0, 200);
	}

	fclose(in);
	return true;
}

size_t split(const std::string &txt, std::vector<std::string> &strs, char ch)
{
	size_t pos = txt.find(ch);
	size_t initialPos = 0;
	strs.clear();

	// Decompose statement
	while (pos != std::string::npos) {
		strs.push_back(txt.substr(initialPos, pos - initialPos));
		initialPos = pos + 1;

		pos = txt.find(ch, initialPos);
	}

	// Add the last one
	strs.push_back(txt.substr(initialPos, min(pos, txt.size()) - initialPos + 1));

	return strs.size();
}
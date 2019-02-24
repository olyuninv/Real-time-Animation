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

// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#define MAX_OBJECTS 30
#define MAX_MARKERS 50
#define NUM_BLENDSHAPES 2


using namespace glm;
using namespace std;
using namespace Assignment1;

void get_files_in_directory(
	std::set<std::string> &out, //list of file names within directory
	const std::string &directory //absolute path to the directory
);

bool read_obj(const std::string& filename,
	std::vector<glm::vec3> &positions,
	std::vector<glm::vec2> &tex_coords,
	std::vector<glm::vec3> &normals,
	std::vector<std::vector<int>> &indices);

TwBar *bar;         // Pointer to a tweak bar

bool usingLowRes = false;

// settings
const unsigned int SCR_WIDTH = 1920; // 1200; // 1920;
const unsigned int SCR_HEIGHT = 1100; // 650; //1100;

double time = 0, dt;// Current time and enlapsed time
double turn = 0;    // Model turn counter
double speed = 0.3; // Model rotation speed

glm::mat4 projection;
glm::mat4 view;

CGObject sceneObjects[MAX_OBJECTS];
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
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -6.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 3.0f, 0.0f);

GLuint VAOs[MAX_OBJECTS];
int numVAOs = 0;

GLuint faceVAO_positions;

int n_vbovertices = 0;
int n_ibovertices = 0;

//lighting position
glm::vec3 lightPos(10.0f, 10.0f, 3.0f);

// Selected vertex
bool vertexSelected = false;
bool markerSelected = false;
int sphereObjectIndex = -1;

int selectedSceneObject = 0;
int selectedObjectMesh = 0;
int selectedVertexIndex = 0;

//TW_only
float tw_posX = 0.0f;
float tw_posY = 0.0f;
float tw_posZ = 0.0f;
float tw_posX_world = 0.0f;
float tw_posY_world = 0.0f;
float tw_posZ_world = 0.0f;

float signedVolume(vec3 a, vec3 b, vec3 c, vec3 d);
bool sameSignVolumes(float vol1, float vol2);
bool sameSignVolumes(float vol1, float vol2, float vol3);
int closestPoint(vec3 intersection, vec3 p1, vec3 p2, vec3 p3);

Face neutralFace;
Face customFace;

float prev_weights_length = -1.0f;
float* weights = (float *)std::malloc(NUM_BLENDSHAPES * sizeof(float));

void TW_CALL ResetCallback(void *clientData)
{
	float *selectedWeights = static_cast<float *>(clientData);

	for (int i = 0; i < NUM_BLENDSHAPES; i++)
	{
		selectedWeights[i] = 0.0f;
	}	
}

//void TW_CALL SetCallbackLocalJawOpen(const void *value, void *clientData)
//{
//	float *selectedWeights = (static_cast<float *>(clientData));
//	selectedWeights[0] = *(const float *)value;
//}
//
//void TW_CALL GetCallbackLocalJawOpen(void *value, void *clientData)
//{
//	float *selectedWeights = (static_cast<float *>(clientData));
//	*static_cast<float *>(value) = selectedWeights[0];
//
//	/*CGObject *selectedObject = &(static_cast<CGObject *>(clientData)[1]);
//	*static_cast<float *>(value) = selectedObject->eulerAngles.x;*/
//}


// Callback function called by GLFW when window size changes
void GLFWCALL WindowSizeCB(int width, int height)
{
	// Send the new window size to AntTweakBar
	glViewport(0, 0, width, height);

	TwWindowSize(width, height);
}


void GLFWCALL mouse_button_callback(int button, int action)
{
	if (!TwEventMouseButtonGLFW(button, action))   // Send event to AntTweakBar
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		{
			int mouse_x, mouse_y;
			bool foundIntersection = false;

			//getting cursor position
			glfwGetMousePos(&mouse_x, &mouse_y);

			float x = (2.0f * mouse_x) / SCR_WIDTH - 1.0f;
			float y = 1.0f - (2.0f * mouse_y) / SCR_HEIGHT;
			float z = 1.0f;

			vec3 ray_nds = vec3(x, y, z);

			vec4 ray_clip = vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);

			vec4 ray_eye = inverse(projection) * ray_clip;
			ray_eye = vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

			vec4 temp = (inverse(view) * ray_eye);

			vec3 ray_wor = vec3(temp.x, temp.y, temp.z);
			// don't forget to normalise the vector at some point
			ray_wor = glm::normalize(ray_wor);

			// Find the other end of the ray (at far clip plane)
			auto farpoint = cameraPos + ray_wor * farclip;

			// Find closest intersection point
			bool intersectionFound = false;
			float zIntersection = farclip;
			vec3 intersectionPoint;

			objl::Mesh mesh;

			for (int sceneIndex = 0; sceneIndex < numObjects; sceneIndex++)
			{
				for (int meshIndex = 0; meshIndex < sceneObjects[sceneIndex].Meshes.size(); meshIndex++)
				{
					// Find intersection with the fragment

					mesh = sceneObjects[sceneIndex].Meshes[meshIndex];

					for (int i = 0; i < mesh.Indices.size(); i += 3) {
						// Get the points in World co-ord

						vec4 p1 = sceneObjects[sceneIndex].globalTransform *
							vec4(mesh.Vertices[mesh.Indices[i]].Position.X,
								mesh.Vertices[mesh.Indices[i]].Position.Y,
								mesh.Vertices[mesh.Indices[i]].Position.Z, 1.0f);
						vec4 p2 = sceneObjects[sceneIndex].globalTransform *
							vec4(mesh.Vertices[mesh.Indices[i + 1]].Position.X,
								mesh.Vertices[mesh.Indices[i + 1]].Position.Y,
								mesh.Vertices[mesh.Indices[i + 1]].Position.Z, 1.0f);
						vec4 p3 = sceneObjects[sceneIndex].globalTransform*
							vec4(mesh.Vertices[mesh.Indices[i + 2]].Position.X,
								mesh.Vertices[mesh.Indices[i + 2]].Position.Y,
								mesh.Vertices[mesh.Indices[i + 2]].Position.Z, 1.0f);

						float volume1 = signedVolume(cameraPos, vec3(p1), vec3(p2), vec3(p3));
						float volume2 = signedVolume(farpoint, vec3(p1), vec3(p2), vec3(p3));

						if (!sameSignVolumes(volume1, volume2))
						{
							float volume3 = signedVolume(cameraPos, farpoint, vec3(p1), vec3(p2));
							float volume4 = signedVolume(cameraPos, farpoint, vec3(p2), vec3(p3));
							float volume5 = signedVolume(cameraPos, farpoint, vec3(p3), vec3(p1));

							if (sameSignVolumes(volume3, volume4, volume5))
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
									int indexOfClosestPoint = closestPoint(intersectionPoint, vec3(p1), vec3(p2), vec3(p3));

									// Set selected vertex - this may be overwritten by a closer point later on
									foundIntersection = true;

									if (selectedSceneObject != sphereObjectIndex)
									{
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
					}
				}
			}
			if (!foundIntersection)
			{
				vertexSelected = false;
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

float signedVolume(vec3 a, vec3 b, vec3 c, vec3 d)
{
	return (1.0f / 6.0f)*dot(cross(b - a, c - a), d - a);
}

bool sameSignVolumes(float a, float b)
{
	if (a >= 0 && b >= 0 || a < 0 && b < 0)
		return true;

	return false;
}


bool sameSignVolumes(float a, float b, float c)
{
	if (a >= 0 && b >= 0 && c >= 0 || a < 0 && b < 0 && c < 0)
		return true;

	return false;
}

int closestPoint(vec3 intersection, vec3 p1, vec3 p2, vec3 p3)
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

void createObjects()
{
	// Shader Attribute locations
	glutils.getAttributeLocations();

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
				blendshapes[numBlendshapes] = face1;
				numBlendshapes++;
			}
		}
	}

	float* customPositions = (float *)std::malloc(numberOfVertices * 3 * sizeof(float));
	blendshape::calculateFace(neutralFace, NUM_BLENDSHAPES, blendshapes, weights, customPositions);

	float* customNormals = (float *)std::malloc(numberOfVertices * 3 * sizeof(float)); 
	blendshape::recalculateNormals(neutralFace.indices, new_meshNeutral[0].Vertices.size(), customPositions, customNormals);

	// Generate the only face to be displayed - from neutral for now
	customFace = Face(numberOfVertices,
		customPositions,
		customNormals,
	//	neutralFace.vnormals,
		neutralFace.vtexcoord);
		
	customFace.name = "Custom";
	customFace.indices = new_meshNeutral[0].Indices;

	glGenVertexArrays(1, &faceVAO_positions);

	// The VBO containing the face positions
	glBindVertexArray(faceVAO_positions);

	glGenBuffers(1, &glutils.faceVBO_positions);
	glBindBuffer(GL_ARRAY_BUFFER, glutils.faceVBO_positions);
	glBufferData(GL_ARRAY_BUFFER,
		numberOfVertices * 3 * sizeof(float),
		neutralFace.vpositions,
		GL_STREAM_DRAW);

	glGenBuffers(1, &glutils.faceVBO_normals);
	glBindBuffer(GL_ARRAY_BUFFER, glutils.faceVBO_normals);
	glBufferData(GL_ARRAY_BUFFER,
		numberOfVertices * 3 * sizeof(float),
		neutralFace.vnormals,
		GL_STREAM_DRAW);

	glGenBuffers(1, &glutils.faceVBO_texcoord);
	glBindBuffer(GL_ARRAY_BUFFER, glutils.faceVBO_texcoord);
	glBufferData(GL_ARRAY_BUFFER,
		numberOfVertices * 2 * sizeof(float),
		neutralFace.vtexcoord,
		GL_STATIC_DRAW);

	glGenBuffers(1, &glutils.faceIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glutils.faceIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		numberOfIndices * 3 * sizeof(unsigned int),
		&neutralFace.indices[0],
		GL_STATIC_DRAW);

	glutils.linkFaceBuffertoShader(faceVAO_positions);

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
	numObjects++;

	const char* sphereFileName = "../Assignment1/meshes/Sphere/sphere.obj";
	vector<objl::Mesh> sphereMeshes = loadMeshes(sphereFileName);
	CGObject sphereObject = loadObjObject(sphereMeshes, true, true, vec3(0.0f, 0.0f, 0.0f), vec3(0.4f, 0.4f, 0.4f), vec3(0.0f, 1.0f, 0.0f), 0.65f, NULL);
	sceneObjects[numObjects] = sphereObject;
	sphereObjectIndex = numObjects;
	numObjects++;*/

	/*glutils.createVBO(n_vbovertices);

	glutils.createIBO(n_ibovertices);*/

	/*for (int i = 0; i < fileList.size(); i++) {
		addToObjectBuffer(&sceneObjects[i]);
	}*/

	//addToObjectBuffer(&testObject);
	//addToObjectBuffer(&boyObject);
	/*addToObjectBuffer(&cubeObject);
	addToObjectBuffer(&sphereObject);*/


	/*for (int i = 0; i < fileList.size(); i++) {
		addToIndexBuffer(&sceneObjects[i]);
	}*/

	//addToIndexBuffer(&testObject);
	//addToIndexBuffer(&boyObject);
	//addToIndexBuffer(&cubeObject);
	//addToIndexBuffer(&sphereObject);
}

void init()
{
	glEnable(GL_DEPTH_TEST);

	glutils = opengl_utils();

	glutils.createShaders();

	glutils.setupUniformVariables();

	createObjects();
}

void display()
{
	// render
	glClearColor(0.78f, 0.84f, 0.49f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Rotate model
	dt = glfwGetTime() - time;
	if (dt < 0) dt = 0;
	time += dt;
	turn += speed * dt;

	//glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
	glLoadIdentity();

	// activate shader
	glUseProgram(glutils.PhongProgramID);

	// Enable OpenGL transparency and light (could have been done once at init)
	glEnable(GL_SMOOTH);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_DEPTH_TEST);
	//glEnable(GL_LIGHT0);    // use default light diffuse and position
	//glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
	//glEnable(GL_COLOR_MATERIAL);
	//glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
	//glEnable(GL_LINE_SMOOTH);
	//glLineWidth(3.0);
	//glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Update projection 
	projection = glm::perspective(glm::radians(fov), (float)(SCR_WIDTH) / (float)(SCR_HEIGHT), nearclip, farclip);
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

	glm::mat4 local1(1.0f);
	local1 = glm::translate(local1, cameraPos);
	glm::mat4 global1 = local1;

	glutils.updateUniformVariables(global1, view, projection);

	glUniform3f(glutils.lightColorLoc, 1.0f, 1.0f, 1.0f);
	glUniform3f(glutils.lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
	glUniform3f(glutils.viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);

	//// DRAW objects
	//for (int i = 0; i < numObjects - 1; i++)     // TODO : need to fix this hardcoding
	//{
	//	mat4 globalCGObjectTransform = sceneObjects[i].createTransform();
	//	glutils.updateUniformVariables(globalCGObjectTransform);
	//	sceneObjects[i].globalTransform = globalCGObjectTransform; // keep current state		

	//	glUniform3f(glutils.objectColorLoc, sceneObjects[i].color.r, sceneObjects[i].color.g, sceneObjects[i].color.b);
	//	sceneObjects[i].Draw(glutils);
	//}

	/*glDisableVertexAttribArray(glutils.loc1);
	glDisableVertexAttribArray(glutils.loc2);
	glDisableVertexAttribArray(glutils.loc3);*/

	// DRAW FACE
	
	//if (newWeightsLength != prev_weights_length)
	//{		
		blendshape::calculateFace(neutralFace, NUM_BLENDSHAPES, blendshapes, weights, customFace.vpositions);
		blendshape::recalculateNormals(neutralFace.indices, neutralFace.numVertices, customFace.vpositions, customFace.vnormals);
				
		//// Generate the only face to be displayed - from neutral for now
		////tidy up
		//delete[] customFace.vpositions;
		//delete[] customFace.vnormals;

		////re-assign pointer
		//customFace.vpositions = customPositions;
		//customFace.vnormals = customNormals;

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
	//faceTransform = glm::rotate(faceTransform, 2.0f, vec3(0.0f, 1.0f, 0.0f));
	glutils.updateUniformVariables(glm::scale(faceTransform, vec3(0.2f, 0.2f, 0.2f)), view, projection);
	glUniform3f(glutils.objectColorLoc, 1.0f, 1.0f, 1.0f);
	glDrawElements(GL_TRIANGLES, customFace.indices.size(), GL_UNSIGNED_INT, 0);

	/*glDisableVertexAttribArray(glutils.loc1);
	glDisableVertexAttribArray(glutils.loc2);
	glDisableVertexAttribArray(glutils.loc3);*/

	glPopMatrix();

	// DRAW CONSTRAINTS
	glPushMatrix();
	glLoadIdentity();

	if (vertexSelected && selectedSceneObject != sphereObjectIndex)
	{
		sceneObjects[sphereObjectIndex].position = vec3(sceneObjects[selectedSceneObject].Meshes[selectedObjectMesh].Vertices[selectedVertexIndex].Position.X,
			sceneObjects[selectedSceneObject].Meshes[selectedObjectMesh].Vertices[selectedVertexIndex].Position.Y,
			sceneObjects[selectedSceneObject].Meshes[selectedObjectMesh].Vertices[selectedVertexIndex].Position.Z);
		sceneObjects[sphereObjectIndex].Parent = &sceneObjects[selectedSceneObject];

		// perform scaling opposite to parent
		sceneObjects[sphereObjectIndex].initialScaleVector = vec3(0.4f / sceneObjects[selectedSceneObject].initialScaleVector.x,
			0.4f / sceneObjects[selectedSceneObject].initialScaleVector.y,
			0.4f / sceneObjects[selectedSceneObject].initialScaleVector.z);

		mat4 globalCGObjectTransform = sceneObjects[sphereObjectIndex].createTransform();  //sphere

		glutils.updateUniformVariables(globalCGObjectTransform);
		//sceneObjects[sphereObjectIndex].globalTransform = globalCGObjectTransform; // keep current state		

		glUniform3f(glutils.objectColorLoc, sceneObjects[sphereObjectIndex].color.r, sceneObjects[sphereObjectIndex].color.g, sceneObjects[sphereObjectIndex].color.b);
		sceneObjects[sphereObjectIndex].Draw(glutils);
	}

	glPopMatrix();
	
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
	
	// Add 'time' to 'bar': it is a read-only (RO) variable of type TW_TYPE_DOUBLE, with 1 precision digit
	TwAddVarRO(bar, "time", TW_TYPE_BOOLCPP, &vertexSelected, " label='Vertex selected' precision=1 help='Indicates if vertex is selected.' ");

	//TwAddVarRO(bar, "selected Scene Object", TW_TYPE_INT32, &selectedSceneObject, " label='Selected scene object' precision=0 help='Index of the selected object.' ");

	//TwAddVarRO(bar, "selected Mesh index", TW_TYPE_INT32, &selectedObjectMesh, " label='Selected mesh index' precision=0 help='Index of the selected mesh.' ");

	/*TwAddVarRO(bar, "selected Vertex index", TW_TYPE_INT32, &selectedVertexIndex, " label='Selected vertex index' precision=0 help='Index of the selected vertex.' ");

	TwAddVarRO(bar, "selected - posX", TW_TYPE_FLOAT, &tw_posX,
		" label='PosX - local' precision=2 help='local X-coord of the selected vertex.' ");
	TwAddVarRO(bar, "selected - posY", TW_TYPE_FLOAT, &tw_posY,
		" label='PosY - local' precision=2 help='local Y-coord of the selected vertex.' ");
	TwAddVarRO(bar, "selected - posZ", TW_TYPE_FLOAT, &tw_posZ,
		" label='PosZ - local' precision=2 help='local Z-coord of the selected vertex.' ");


	TwAddVarRO(bar, "selected - posX - world", TW_TYPE_FLOAT, &tw_posX_world,
		" label='PosX - world' precision=2 help='World X-coord of the selected vertex.' ");
	TwAddVarRO(bar, "selected - posY - world", TW_TYPE_FLOAT, &tw_posY_world,
		" label='PosY - world' precision=2 help='World Y-coord of the selected vertex.' ");
	TwAddVarRO(bar, "selected - posZ - world", TW_TYPE_FLOAT, &tw_posZ_world,
		" label='PosZ - world' precision=2 help='World Z-coord of the selected vertex.' ");*/

	TwAddSeparator(bar, "", NULL);

	// initialise weights
	for (int i = 0; i < NUM_BLENDSHAPES; i++)
	{
		weights[i] = 0.0f;
	}

	TwAddButton(bar, "Reset", ResetCallback, weights, " label='Reset' ");

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
	glfwSetMousePosCallback((GLFWmouseposfun)TwEventMousePosGLFW);
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
	delete[] customFace.vpositions;
	delete[] customFace.vnormals;

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
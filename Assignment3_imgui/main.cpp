#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

#include <stdio.h>
#include <cstdlib>
#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <GL/glew.h>   
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <glm/gtc/type_ptr.hpp>

#include "opengl_utils.h"
#include "CGobject.h"
#include "Interpolate.h"

#include "..\Dependencies\OBJ_Loader.h"

// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#define MAX_OBJECTS 50
#define MAX_IK_TRIES 100 // TIMES THROUGH THE CCD LOOP
#define IK_POS_THRESH 0.01f // THRESHOLD FOR SUCCESS
#define MAX_KEYFRAMES_GOAL 8 

using namespace glm;
using namespace std;
using namespace Assignment3_imgui;

// GLFW 
GLFWwindow* window;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_pos_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// Function declarations
glm::vec3 calculateEndPoint(glm::vec3 startPoint, float length, float angleZ, float angleY);

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1100;

opengl_utils glutils;

bool lbutton_down = false;

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

// camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 15.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float initialCylinderLength = 2.0f;

bool pause = true;
bool useNormalMap = false;
bool useSpecularMap = false;

GLuint VAOs[MAX_OBJECTS];
int numVAOs = 0;

GLuint textures[4];

unsigned int textureIDcubemap;
unsigned int textureIDlotus;
unsigned int textureIDlotusBump;

unsigned int n_vbovertices = 0;
unsigned int n_ibovertices = 0;

CGObject sceneObjects[MAX_OBJECTS];
int numObjects = 0;


// ImGUI definitions
bool show_demo_window = true;
bool show_another_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
bool withinReach = false;

float keyframeGoalPositions[MAX_KEYFRAMES_GOAL][3];
Curve* curve;

enum IKMethod
{
	CCD,
	Jacobian
};

enum AnimType
{
	track,
	onclick
};

enum AnimSpeed
{
	no_speed,
	equal,
	easeIn_easeOut
};

int IKmethod = IKMethod::CCD;
int animType = AnimType::onclick;
bool animStarted = false;
float animStartTime = 0.0f;
int animCurrFrame = 0;
AnimSpeed animationSpeed = AnimSpeed::no_speed;
float timePerLineSegment = 3.0f;

int numJoints = 2;
float goal[3] = { 8.0f,0.0f,0.0f };
float increment = 0.1f;

// do not hardcode indexes
int sphereIndex = 1;
int indexOfFirstJoint = 3;


static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}




//lighting position
glm::vec3 lightPos(3.0f, 1.0f, 3.0f);

enum class textureInterpolation
{
	nearest = 1,
	linear = 2,
	nearest_mipmap_nearest_interpolation = 3, //GL_NEAREST_MIPMAP_NEAREST: takes the nearest mipmap to match the pixel size and uses nearest neighbor interpolation for texture sampling.
	nearest_mipmap_linear_interpolation = 4, //GL_LINEAR_MIPMAP_NEAREST : takes the nearest mipmap level and samples using linear interpolation.
	interpolate_mipmap_nearest_interpolation = 5, //GL_NEAREST_MIPMAP_LINEAR : linearly interpolates between the two mipmaps that most closely match the size of a pixel and samples via nearest neighbor interpolation.
	interpolate_mipmap_linear_interpolation = 6, //GL_LINEAR_MIPMAP_LINEAR : linearly interpolates between the two closest mipmaps and samples the texture via linear interpolation.* /	

};

void addToObjectBuffer(CGObject *cg_object)
{
	int VBOindex = cg_object->startVBO;
	int IBOindex = cg_object->startIBO;

	for (int i = 0; i < cg_object->Meshes.size(); i++) {

		glutils.addVBOBufferSubData(VBOindex, cg_object->Meshes[i].Vertices.size(), &cg_object->Meshes[i].Vertices[0].Position.X);
		//glutils.linkCurrentBuffertoShader(cg_object->VAOs[i], VBOindex, IBOindex);
		VBOindex += cg_object->Meshes[i].Vertices.size();
		IBOindex += cg_object->Meshes[i].Indices.size();
	}
}

void addToTangentBuffer(CGObject *cg_object)
{
	int VBOindex = cg_object->startVBO;
	int IBOindex = cg_object->startIBO;

	for (int i = 0; i < cg_object->Meshes.size(); i++) {

		//TODO: Remove call to gl to glutils
		glutils.addTBOBufferSubData(VBOindex, cg_object->tangentMeshes[i].tangents.size(), &cg_object->tangentMeshes[i].tangents[0].x);
		//glutils.linkCurrentBuffertoShader(cg_object->VAOs[i], VBOindex, IBOindex);
		VBOindex += cg_object->Meshes[i].Vertices.size();
		IBOindex += cg_object->Meshes[i].Indices.size();
	}
}

void addToBitangentBuffer(CGObject *cg_object)
{
	int VBOindex = cg_object->startVBO;
	int IBOindex = cg_object->startIBO;

	for (int i = 0; i < cg_object->Meshes.size(); i++) {

		//TODO: Remove call to gl to glutils
		glutils.addBTBOBufferSubData(VBOindex, cg_object->tangentMeshes[i].bitangents.size(), &cg_object->tangentMeshes[i].bitangents[0].x);
		//glutils.linkCurrentBuffertoShader(cg_object->VAOs[i], VBOindex, IBOindex);
		VBOindex += cg_object->Meshes[i].Vertices.size();
		IBOindex += cg_object->Meshes[i].Indices.size();
	}
}

void addToIndexBuffer(CGObject *cg_object)
{
	int IBOindex = cg_object->startIBO;
	for (auto const& mesh : cg_object->Meshes) {
		glutils.addIBOBufferSubData(IBOindex, mesh.Indices.size(), &mesh.Indices[0]);
		IBOindex += mesh.Indices.size();
	}
}

//---------------------------------------------------------------------------------------------------------//

std::vector<objl::Mesh> loadMeshes(const char* objFileLocation)
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

CGObject loadObjObject(vector<objl::Mesh> meshes, vector<TangentMesh> tangentMeshes, bool addToBuffers, bool subjectToGravity, vec3 initTransformVector, vec3 initScaleVector, vec3 color, float coef, CGObject* parent)
{
	CGObject object = CGObject();
	object.Meshes = meshes;
	object.tangentMeshes = tangentMeshes;
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

CGObject createTorso(int &numberOfObjects)
{
	const char* cylinderFileName = "../Assignment3_imgui/meshes/Cylinder/cylinder.obj";
	vector<objl::Mesh> cylinderMeshes = loadMeshes(cylinderFileName);

	std::vector<objl::Mesh> new_meshesCylinder;
	std::vector<TangentMesh> new_tangentMeshesCylinder;

	//recalculate meshes
	CGObject::recalculateVerticesAndIndexes(cylinderMeshes, new_meshesCylinder, new_tangentMeshesCylinder);

	CGObject torso = loadObjObject(new_meshesCylinder, new_tangentMeshesCylinder, true, true, vec3(-2.0f, -2.0f, 0.0f), vec3(7.0f, 1.0f, 7.0f), vec3(244 / 255.0f, 164 / 255.0f, 96 / 255.0f), 0.65f, NULL);
	sceneObjects[numberOfObjects] = torso;
	numberOfObjects++;

	return torso;
}

CGObject createCubeMap(int &numberOfObjects)
{
	// CUBEMAP
	const char* cubeFileName = "../Assignment3_imgui/meshes/Cube/Cube.obj";
	vector<objl::Mesh> cubeMesh = loadMeshes(cubeFileName);

	std::vector<objl::Mesh> dummy_cubeMeshes = std::vector<objl::Mesh>();
	std::vector<TangentMesh> dummy_cubeTangents = std::vector<TangentMesh>();
	CGObject::recalculateVerticesAndIndexes(cubeMesh, dummy_cubeMeshes, dummy_cubeTangents);

	CGObject cubeObject = loadObjObject(dummy_cubeMeshes, dummy_cubeTangents, true, true, vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f), 0.65f, NULL);
	sceneObjects[numberOfObjects] = cubeObject;
	numberOfObjects++;

	return cubeObject;
}

void createObjects()
{
	// Shader Attribute locations
	glutils.getAttributeLocations();

	CGObject cubeObject = createCubeMap(numObjects);

	const char* sphereFileName = "../Assignment3_imgui/meshes/Sphere/sphere.obj";
	vector<objl::Mesh> sphereMeshes = loadMeshes(sphereFileName);

	std::vector<objl::Mesh> new_meshesSphere;
	std::vector<TangentMesh> new_tangentMeshesSphere;

	//recalculate meshes
	CGObject::recalculateVerticesAndIndexes(sphereMeshes, new_meshesSphere, new_tangentMeshesSphere);

	CGObject sphereObject = loadObjObject(new_meshesSphere, new_tangentMeshesSphere, true, true, vec3(5.0f, 0.0f, 0.0f), vec3(0.4f, 0.4f, 0.4f), vec3(0.0f, 1.0f, 0.0f), 0.65f, NULL);
	sphereObject.initialScaleVector = vec3(0.5f, 0.5f, 0.5f);
	sceneObjects[numObjects] = sphereObject;
	numObjects++;

	const char* cylinderFileName = "../Assignment3_imgui/meshes/Cylinder/cylinder_sm.obj"; //
	vector<objl::Mesh> cylinderMeshes = loadMeshes(cylinderFileName);

	std::vector<objl::Mesh> new_meshesCylinder;
	std::vector<TangentMesh> new_tangentMeshesCylinder;

	//recalculate meshes
	CGObject::recalculateVerticesAndIndexes(cylinderMeshes, new_meshesCylinder, new_tangentMeshesCylinder);

	CGObject torso = loadObjObject(new_meshesCylinder, new_tangentMeshesCylinder, true, true, vec3(-0.4f, -4.5f, 0.0f), vec3(7.0f, 2.5f, 7.0f), vec3(244 / 255.0f, 164 / 255.0f, 96 / 255.0f), 0.65f, NULL);
	sceneObjects[numObjects] = torso;
	numObjects++;

	CGObject topArm = loadObjObject(new_meshesCylinder, new_tangentMeshesCylinder, false, true, vec3(0.0f, 0.0f, 0.0f), vec3(1.5f, 1.0f, 1.5f), vec3(1.0f, 1.0f, 0.0f), 0.65f, NULL);
	topArm.setInitialRotation(vec3(0.0f, 0.0f, -2.5f));
	topArm.startVBO = torso.startVBO;  //reusing model
	topArm.startIBO = torso.startIBO;  //reusing model
	topArm.VAOs.push_back(torso.VAOs[0]);  //reusing model
	sceneObjects[numObjects] = topArm;
	numObjects++;

	initialCylinderLength = (topArm.Meshes[0].Vertices[1].Position.Y - topArm.Meshes[0].Vertices[0].Position.Y);

	CGObject bottomArm = loadObjObject(new_meshesCylinder, new_tangentMeshesCylinder, false, true,
		//calculateEndPoint (topArm.position, topArm.initialScaleVector.y * initialCylinderLength, topArm.eulerAngles.z, topArm.eulerAngles.x),
		topArm.rotationMatrix * vec4(0.0, topArm.initialScaleVector.y * initialCylinderLength, 0.0, 1.0),
		vec3(1.5f, 0.8f, 1.5f),
		vec3(1.0f, 1.0f, 0.0f),
		0.65f,
		NULL);

	bottomArm.setInitialRotation(vec3(0.0f, 0.0f, -2.0f));
	bottomArm.startVBO = torso.startVBO;  //reusing model
	bottomArm.startIBO = torso.startIBO;  //reusing model
	bottomArm.VAOs.push_back(torso.VAOs[0]);  //reusing model
	sceneObjects[numObjects] = bottomArm;
	numObjects++;

	CGObject palm = loadObjObject(new_meshesCylinder, new_tangentMeshesCylinder, false, true,
		//calculateEndPoint (topArm.position, topArm.initialScaleVector.y * initialCylinderLength, topArm.eulerAngles.z, topArm.eulerAngles.x),
		bottomArm.rotationMatrix * vec4(0.0, bottomArm.initialScaleVector.y * initialCylinderLength, 0.0, 1.0),
		vec3(1.5f, 0.3f, 1.5f),
		vec3(1.0f, 1.0f, 0.0f),
		0.65f,
		NULL);

	palm.setInitialRotation(vec3(0.0f, 0.0f, -1.5f));
	palm.startVBO = torso.startVBO;  //reusing model
	palm.startIBO = torso.startIBO;  //reusing model
	palm.VAOs.push_back(torso.VAOs[0]);  //reusing model
	sceneObjects[numObjects] = palm;
	numObjects++;

	CGObject fingers = loadObjObject(new_meshesCylinder, new_tangentMeshesCylinder, false, true,
		//calculateEndPoint (topArm.position, topArm.initialScaleVector.y * initialCylinderLength, topArm.eulerAngles.z, topArm.eulerAngles.x),
		palm.rotationMatrix * vec4(0.0, palm.initialScaleVector.y * initialCylinderLength, 0.0, 1.0),
		vec3(1.5f, 0.15f, 1.5f),
		vec3(1.0f, 1.0f, 0.0f),
		0.65f,
		NULL);

	fingers.setInitialRotation(vec3(0.0f, 0.0f, 0.0f));
	fingers.startVBO = torso.startVBO;  //reusing model
	fingers.startIBO = torso.startIBO;  //reusing model
	fingers.VAOs.push_back(torso.VAOs[0]);  //reusing model
	sceneObjects[numObjects] = fingers;
	numObjects++;

	CGObject snake = loadObjObject(new_meshesCylinder, new_tangentMeshesCylinder, false, true,
		//calculateEndPoint (topArm.position, topArm.initialScaleVector.y * initialCylinderLength, topArm.eulerAngles.z, topArm.eulerAngles.x),
		fingers.rotationMatrix * vec4(0.0, fingers.initialScaleVector.y * initialCylinderLength, 0.0, 1.0),
		vec3(1.5f, 0.15f, 1.5f),
		vec3(1.0f, 1.0f, 0.0f),
		0.65f,
		NULL);

	snake.setInitialRotation(vec3(0.0f, 0.0f, 0.0f));
	snake.startVBO = torso.startVBO;  //reusing model
	snake.startIBO = torso.startIBO;  //reusing model
	snake.VAOs.push_back(torso.VAOs[0]);  //reusing model
	sceneObjects[numObjects] = snake;
	numObjects++;

	glutils.createVBO(n_vbovertices);

	glutils.createIBO(n_ibovertices);

	addToObjectBuffer(&cubeObject);
	addToObjectBuffer(&sphereObject);
	addToObjectBuffer(&torso);
	addToIndexBuffer(&cubeObject);
	addToIndexBuffer(&sphereObject);
	addToIndexBuffer(&torso);

	glutils.createTBO(n_vbovertices);
	addToTangentBuffer(&cubeObject);
	addToTangentBuffer(&sphereObject);
	addToTangentBuffer(&torso);

	glutils.createBTBO(n_vbovertices);
	addToBitangentBuffer(&cubeObject);
	addToBitangentBuffer(&sphereObject);
	addToBitangentBuffer(&torso);
}

void loadCube()
{
	vector<std::string> faces = vector<std::string>();

	faces.push_back("../Assignment3_imgui/meshes/Yokohama/posx.jpg");
	faces.push_back("../Assignment3_imgui/meshes/Yokohama/negx.jpg");
	faces.push_back("../Assignment3_imgui/meshes/Yokohama/posy.jpg");
	faces.push_back("../Assignment3_imgui/meshes/Yokohama/negy.jpg");
	faces.push_back("../Assignment3_imgui/meshes/Yokohama/posz.jpg");
	faces.push_back("../Assignment3_imgui/meshes/Yokohama/negz.jpg");

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
}

void readKeyframePositionsFile(string file)
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
			keyframeGoalPositions[lineNumber][weightIndex] = d2;
			weightIndex++;

			if (weightIndex == 3)
			{
				lineNumber++;
				weightIndex = 0;
			}
		}
	}
	myReadFile.close();
}

void populateCurve()
{
	curve = new Curve();
	curve->set_steps(100); // generate 100 interpolate points between the last 4 way points

	for (int i = 0; i < MAX_KEYFRAMES_GOAL; i++)
	{
		curve->add_way_point(glm::vec3(keyframeGoalPositions[i][0], keyframeGoalPositions[i][1], keyframeGoalPositions[i][2]));
	}

	std::cout << "nodes: " << curve->node_count() << std::endl;
	std::cout << "total length: " << curve->total_length() << std::endl;
	/*for (int i = 0; i < curve->node_count(); ++i) {
		std::cout << "node #" << i << ": " << curve->node(i).data << " (length so far: " << curve->length_from_starting_point(i) << ")" << std::endl;
	}*/
}

void init()
{
	readKeyframePositionsFile("../Assignment3_imgui/ScriptedGoal.txt");
	populateCurve();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);// you enable blending function
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glutils = opengl_utils();

	glutils.createShaders();

	glutils.setupUniformVariables();

	// Setup cubemap texture
	glGenTextures(4, textures);  //1, &textureIDcubemap);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textures[0]); //textureIDcubemap);

	loadCube();

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glUniform1i(glutils.cubeLocation2, 0);   // cubemap
	glUniform1i(glutils.cubeLocation3, 0);   // cubemap

	glActiveTexture(GL_TEXTURE1);
	//glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[1]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// load and generate the texture
	int width, height, nrChannels;
	unsigned char *data = stbi_load("../Assignment3_imgui/meshes/Chess_Board/Chess_Board.jpg", &width, &height, &nrChannels, NULL);

	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(data);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
		stbi_image_free(data);
	}

	glUniform1i(glutils.texture3, 1);

	if (useNormalMap)
	{
		// Setup lotus texture - bump
		glActiveTexture(GL_TEXTURE2);
		//glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, textures[2]);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// load and generate the texture
		int width1, height1, nrChannels1;
		unsigned char *data1 = stbi_load("../Assignment3_imgui/meshes/Chess_Board/Chess_Board.jpg", &width1, &height1, &nrChannels1, 4);

		if (data1)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width1, height1, 0, GL_RGB, GL_UNSIGNED_BYTE, data1);
			glGenerateMipmap(GL_TEXTURE_2D);
			stbi_image_free(data1);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
			stbi_image_free(data1);
		}
		glUniform1i(glutils.normalMap3, 2);
	}

	if (useSpecularMap)
	{
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, textures[3]);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// load and generate the texture
		int width2, height2, nrChannels2;
		unsigned char *data2 = stbi_load("../Assignment3_imgui/meshes/simplePlane/600px-Chess_Board.png", &width2, &height2, &nrChannels2, 3);

		if (data2)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width2, height2, 0, GL_RGB, GL_UNSIGNED_BYTE, data2);
			glGenerateMipmap(GL_TEXTURE_2D);
			stbi_image_free(data2);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
			stbi_image_free(data2);
		}
		glUniform1i(glutils.specularMap3, 3);
	}

	createObjects();
}

glm::vec3 calculateEndPoint(glm::vec3 startPoint, float length, float angleZ, float angleY)
{
	return glm::vec3(startPoint.x + length * -sin(angleZ) * cos(angleY),
		startPoint.y + length * cos(angleZ),
		startPoint.z + length * sin(angleZ) * sin(angleY));

	//vec3(topArm.initialScaleVector.y * initialCylinderLength * -sin(topArm.eulerAngles.z),
	//	topArm.initialScaleVector.y * initialCylinderLength * cos(topArm.eulerAngles.z),
	//	0.0f),
}

float sq_distance(vec3 point1, vec3 point2)
{
	return pow(point2.x - point1.x, 2) + pow(point2.y - point1.y, 2) + pow(point2.z - point1.z, 2);
}

vec3 getEulerAngles(mat4 rotationMatrix)
{
	float theta1, theta2;
	float phi1, phi2;
	float psi1, psi2;

	if (abs(rotationMatrix[0][2]) != 1)
	{
		theta1 = -asin(rotationMatrix[0][2]);
		theta2 = 3.14 - theta1;  // py - theta 1
		psi1 = atan2f(rotationMatrix[1][2] / cos(theta1), rotationMatrix[2][2] / cos(theta1));
		psi2 = atan2f(rotationMatrix[1][2] / cos(theta2), rotationMatrix[2][2] / cos(theta2));
		phi1 = atan2f(rotationMatrix[0][1] / cos(theta1), rotationMatrix[0][0] / cos(theta1));
		phi2 = atan2f(rotationMatrix[0][1] / cos(theta2), rotationMatrix[0][0] / cos(theta2));
	}
	else
	{
		phi1 = phi2 = 0;
		if (rotationMatrix[0][2] == -1)
		{
			theta1 = theta2 = 3.14 / 2;
			psi1 = psi2 = phi1 + atan2f(rotationMatrix[1][0], rotationMatrix[2][0]);
		}
		else
		{
			theta1 = theta2 = -3.14 / 2;
			psi1 = psi2 = -phi1 + atan2f(-rotationMatrix[1][0], -rotationMatrix[2][0]);
		}
	}

	return vec3(psi1, theta1, phi1);
}

bool computeCCDLink(glm::vec3 goalVec)
{
	bool goalWithinThreshold = false;

	int link = numJoints - 1;  // TODO: Add NUM_JOINTS UI
	int tries = 0;

	glm::vec3 currRoot, currEnd, lastJointPosition, targetVector, currVector, normal;
	float cosAngle, turnAngle, turnDegrees, lastJointLength;
	quat quaternion;
	int indexOfCurrLink, indexOfLastJoint;

	indexOfLastJoint = indexOfFirstJoint + numJoints - 1;

	do
	{
		indexOfCurrLink = indexOfFirstJoint + link;

		currRoot = sceneObjects[indexOfCurrLink].position;

		// calculate from position of last joint 
		//lastJointPosition = sceneObjects[indexOfFirstJoint + numJoints - 1].position;  // calculate end from last joint
		//lastJointLength = sceneObjects[indexOfFirstJoint + numJoints - 1].initialScaleVector.y * initialCylinderLength;

		currEnd = sceneObjects[indexOfLastJoint].position + vec3(sceneObjects[indexOfLastJoint].rotationMatrix *
			vec4(0.0, sceneObjects[indexOfLastJoint].initialScaleVector.y * initialCylinderLength, 0.0, 1.0));

		if (sq_distance(goalVec, currEnd) < IK_POS_THRESH)
		{
			// we are close
			goalWithinThreshold = true;
		}
		else
		{
			// continue search
			currVector = currEnd - currRoot;
			targetVector = goalVec - currRoot;

			currVector = glm::normalize(currVector);
			targetVector = glm::normalize(targetVector);

			cosAngle = dot(targetVector, currVector);
			if (abs(cosAngle) < 0.99999)
			{
				normal = cross(currVector, targetVector);
				normal = glm::normalize(normal);
				turnAngle = acos(cosAngle);

				quaternion.x = normal.x * sin(turnAngle / 2);
				quaternion.y = normal.y * sin(turnAngle / 2);
				quaternion.z = normal.z * sin(turnAngle / 2);
				quaternion.w = cos(turnAngle / 2);

				sceneObjects[indexOfCurrLink].rotationMatrix = glm::toMat4(quaternion) * sceneObjects[indexOfCurrLink].rotationMatrix;
				//vec3 newEulerAngles = getEulerAngles(newRotationMatrix);
				//sceneObjects[indexOfCurrLink].setInitialRotation(newEulerAngles); // calculate depending on the previous object	

				// update position and angles for all links
				for (int k = indexOfCurrLink + 1; k < numObjects; k++)
				{
					if (k != indexOfFirstJoint) // ignore first joint
					{
						sceneObjects[k].position = sceneObjects[k - 1].position +
							vec3(sceneObjects[k - 1].rotationMatrix * vec4(0.0, sceneObjects[k - 1].initialScaleVector.y * initialCylinderLength, 0.0, 1.0));
					}

					//mat4 newRotationTemp = sceneObjects[k].rotationMatrix * glm::toMat4(quaternion);
					//vec3 newEulerAnglesTemp = getEulerAngles(newRotationTemp);
					//sceneObjects[k].setInitialRotation(newEulerAnglesTemp); // calculate depending on the previous object					
					sceneObjects[k].rotationMatrix = glm::toMat4(quaternion) * sceneObjects[k].rotationMatrix;
				}
			}

			if (--link < 0)
			{
				// move back to last link
				link = numJoints - 1;
			}
		}

	} while (++tries < 1000 && goalWithinThreshold == false);  //MAX_IK_TRIES

	return goalWithinThreshold;
}

bool computeJacobian()
{
	return false;
}

void displayScene(glm::mat4 projection, glm::mat4 view)
{
	glUseProgram(glutils.ShaderWithTextureID);

	glm::mat4 local1(1.0f);
	local1 = glm::translate(local1, cameraPos);
	glm::mat4 global1 = local1;

	glutils.updateUniformVariablesReflectance(global1, view, projection);

	glUniform3f(glutils.viewPosLoc3, cameraPos.x, -cameraPos.y, cameraPos.z);
	glUniform3f(glutils.lightPosLoc3, lightPos.x, lightPos.y, lightPos.z);
	glUniform1i(glutils.useNormalMapUniform3, useNormalMap);
	glUniform1i(glutils.useSpecularMapUniform3, useSpecularMap);

	// DRAW objects
	for (int i = 1; i < numObjects - ((numObjects - 3) - numJoints); i++)
	{
		// update position based on previous joint
		if (i > indexOfFirstJoint)
		{
			sceneObjects[i].position = sceneObjects[i - 1].position +
				vec3(sceneObjects[i - 1].rotationMatrix * vec4(0.0, sceneObjects[i - 1].initialScaleVector.y * initialCylinderLength, 0.0, 1.0));

		}

		mat4 globalCGObjectTransform = sceneObjects[i].createTransform(true);
		glutils.updateUniformVariablesReflectance(globalCGObjectTransform, view);
		sceneObjects[i].globalTransform = globalCGObjectTransform; // keep current state		

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textures[0]);
		glUniform1i(glutils.cubeLocation3, 0);

		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textures[1]);
		glUniform1i(glutils.texture3, 1);

		if (useNormalMap)
		{
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, textures[2]);
			glUniform1i(glutils.normalMap3, 2);
		}

		if (useSpecularMap)
		{
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, textures[3]);
			glUniform1i(glutils.specularMap3, 3);
		}

		glUniform3f(glutils.objectColorLoc3, sceneObjects[i].color.r, sceneObjects[i].color.g, sceneObjects[i].color.b);
		sceneObjects[i].Draw(glutils, glutils.ShaderWithTextureID);

		glDisable(GL_TEXTURE_2D);
	}
}

void displayCubeMap(glm::mat4 projection, glm::mat4 view)
{
	// First Draw cube map - sceneObjects[0]
	glDepthMask(GL_FALSE);
	glUseProgram(glutils.CubeMapID);

	glm::mat4 viewCube = glm::mat4(glm::mat3(view));
	glutils.updateUniformVariablesCubeMap(viewCube, projection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textures[0]); //textureIDcubemap);

	glBindVertexArray(VAOs[0]);

	glutils.bindVertexAttribute(glutils.loc4, 3, sceneObjects[0].startVBO, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glutils.IBO);

	glDrawElements(GL_TRIANGLES, sceneObjects[0].Meshes[0].Indices.size(), GL_UNSIGNED_INT, (void*)(sceneObjects[0].startIBO * sizeof(unsigned int)));

	glDepthMask(GL_TRUE);

}

void drawImgui(ImGuiIO& io)
{
	//glfwMakeContextCurrent(window);
	io.WantCaptureMouse = true;
	io.WantCaptureKeyboard = true;
	//io.MousePos = ImVec2(0.0, 0.0);
	//io.MouseDragThreshold = 20.0;
	//io.MouseDrawCursor = true;
	//io.WantSetMousePos = true;
	//io.MouseDrawCursor = true;

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	//// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	//if (show_demo_window)
	//	ImGui::ShowDemoWindow(&show_demo_window);

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		//static int counter = 0;

		ImGui::Begin("Inverse Kinematics");

		const char* items[] = { "CCD", "Jacobian" };
		ImGui::Combo("IK method", &IKmethod, items, IM_ARRAYSIZE(items));

		const char* items2[] = { "Track", "On Click"};
		ImGui::Combo("Animation method", &animType, items2, IM_ARRAYSIZE(items2));
		
		if (ImGui::Button("Show Animation"))
		{
			animStarted = true;	
			animStartTime = glfwGetTime();
			animCurrFrame = 0;
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Text("Please set the next location of the goal:");
		ImGui::DragFloat3("Goal", goal, increment, -10.0f, 10.0f, "%.1f");

		ImGui::Text("Select number of joints:");
		ImGui::SliderInt("Number of joints", &numJoints, 2, 5);

		ImGui::Text("Change the length of arm parts:");
		ImGui::DragFloat("Top arm", &sceneObjects[indexOfFirstJoint].initialScaleVector.y, 0.1f, 0.5f, 5.0f, "%.1f");
		ImGui::DragFloat("Bottom arm ", &sceneObjects[indexOfFirstJoint + 1].initialScaleVector.y, 0.1f, 0.5f, 5.0f, "%.1f");

		if (withinReach)
		{
			ImGui::Text("Object within reach");
		}
		else
		{
			ImGui::Text("Object outside of reach");
		}

		//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	// Rendering
	int display_w, display_h;
	glfwMakeContextCurrent(window);
	glfwGetFramebufferSize(window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui::Render();
}

void display(ImGuiIO& io)
{
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	// inpuT
	processInput(window);

	drawImgui(io);

	// render
	glClearColor(0.78f, 0.84f, 0.49f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Update projection 
	glm::mat4 projection = glm::perspective(glm::radians(fov), (float)(SCR_WIDTH) / (float)(SCR_HEIGHT), 0.1f, 100.0f);
	glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	if (animStarted)
	{
		view = glm::lookAt(glm::vec3(0.0f, 0.0f, 15.0f), glm::vec3(0.0f, 0.0f, 15.0f) + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	// DRAW CUBEMAP
	displayCubeMap(projection, view);

	// DRAW SCENE

	if (animStarted)
	{	
		if (animationSpeed == no_speed)
		{
			// check for end of animation
			if (animCurrFrame < curve->node_count())
			{
				sceneObjects[sphereIndex].position.x = curve->node(animCurrFrame).x;
				sceneObjects[sphereIndex].position.y = curve->node(animCurrFrame).y;
				sceneObjects[sphereIndex].position.z = curve->node(animCurrFrame).z;
				animCurrFrame++;
			}
			else
			{
				animStarted = false;
				animStartTime = 0.0f;
				animCurrFrame = 0;
			}
		}
		else
		{
			float dt = currentFrame - animStartTime;
			//TODO - easeIn - easeOut
		}
	}
	else
	{
		// Update target position from imGui
		sceneObjects[sphereIndex].position.x = goal[0];
		sceneObjects[sphereIndex].position.y = goal[1];
		sceneObjects[sphereIndex].position.z = goal[2];
	}

	displayScene(projection, view);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// Update Arm positions
	if (IKmethod == IKMethod::CCD)
	{
		if (animStarted || animType == AnimType::track)
		{
			withinReach = computeCCDLink(sceneObjects[sphereIndex].position);
		}
		else
		{
			// assume animation on-click - only update on button click
			if (!pause)
			{
				withinReach = computeCCDLink(sceneObjects[sphereIndex].position);
				pause = true;
			}
		}
	}
	else
	{
		// Assume Jacobian
		computeJacobian();
	}

	// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
	glfwSwapBuffers(window);
	glfwPollEvents();

}


int main(void) {
	// Initialise GLFW
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Surface Mapping", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window.\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	//detect key inputs
	//glfwSetKeyCallback(window, keycallback);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, mouse_pos_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSwapInterval(1); // Enable vsync

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
	//io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	const char* glsl_version = "#version 330";
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Read 'misc/fonts/README.txt' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != NULL);

	//ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	init();

	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0)
	{
		display(io);
	}

	// optional: de-allocate all resources once they've outlived their purpose:	
	glutils.deleteVertexArrays();
	glutils.deletePrograms();
	glutils.deleteBuffers();
	delete curve;

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	float cameraSpeed = 2.5 * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		rotate_angle += 1.0f;
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		pause = !pause;
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
		useNormalMap = !useNormalMap;
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
		useSpecularMap = !useSpecularMap;
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
	{
		cameraPos = glm::vec3(0.0f, 0.0f, 15.0f);
		cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
		cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
		firstMouse = true;
	}
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
	{
		cameraPos = glm::vec3(0.0f, 15.0f, 0.0f);
		cameraFront = glm::vec3(0.0f, -1.0f, 0.0f);
		cameraUp = glm::vec3(0.0f, 0.0f, 1.0f);
		firstMouse = true;
	}
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
	{
		cameraPos = glm::vec3(15.0f, 0.0f, 0.0f);
		cameraFront = glm::vec3(-1.0f, 0.0f, 0.0f);
		cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
		firstMouse = true;
	}
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		goal[1] += increment;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		goal[1] -= increment;
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		goal[0] += increment;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		goal[0] -= increment;
	}
	if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
	{
		goal[2] += increment;
	}
	if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
	{
		goal[2] -= increment;
	}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
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
			firstMouse = true;
		}
	}
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (lbutton_down)
	{

		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
		lastX = xpos;
		lastY = ypos;

		float sensitivity = 0.1f; // change this value to your liking
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		myyaw += xoffset;
		mypitch += yoffset;

		// make sure that when pitch is out of bounds, screen doesn't get flipped
		if (mypitch > 89.0f)
			mypitch = 89.0f;
		if (mypitch < -89.0f)
			mypitch = -89.0f;

		glm::vec3 front;
		front.x = cos(glm::radians(myyaw)) * cos(glm::radians(mypitch));
		front.y = sin(glm::radians(mypitch));
		front.z = sin(glm::radians(myyaw)) * cos(glm::radians(mypitch));
		cameraFront = glm::normalize(front);
	}
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (fov >= 1.0f && fov <= 45.0f)
		fov -= yoffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 45.0f)
		fov = 45.0f;
}

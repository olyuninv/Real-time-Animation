#define _CRT_SECURE_NO_WARNINGS

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

#include <AntTweakBar.h>
#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "opengl_utils.h"
#include "CGobject.h"

#include "..\Dependencies\OBJ_Loader.h"

// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#define MAX_OBJECTS 50

using namespace glm;
using namespace std;
using namespace Assignment2_glfw3;

TwBar *bar;         // Pointer to a tweak bar

// GLFW 
GLFWwindow* window;
//void framebuffer_size_callback(GLFWwindow* window, int width, int height);
//void mouse_callback(GLFWwindow* window, double xpos, double ypos);
//void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
//void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1100;

opengl_utils glutils; 

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

// camera movement
GLfloat rotate_angle = 0.0f;
bool firstMouse = true;
float cameraYaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float cameraPitch = 0.0f;
double lastX = SCR_WIDTH / 2.0; //800.0f / 2.0;
double lastY = SCR_HEIGHT / 2.0; //600.0 / 2.0;
float fov = 45.0f;

// camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 10.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool pause = true;

//lighting position
glm::vec3 lightPos(-5.0f, -5.0f, -5.0f);

GLuint VAOs[MAX_OBJECTS];
int numVAOs = 0;

int n_vbovertices = 0;
int n_ibovertices = 0;

CGObject sceneObjects[MAX_OBJECTS];
int numObjects = 0;


float local_roll = 0.0f;
float pitch = 0.0f;
float yaw = 0.0f;

void TW_CALL SetCallbackLocalRoll(const void *value, void *clientData)
{
	CGObject *selectedObject = &(static_cast<CGObject *>(clientData)[0]);
	selectedObject->initialRotateAngle.x = *(const float *)value;
}

void TW_CALL GetCallbackLocalRoll(void *value, void *clientData)
{
	CGObject *selectedObject = &(static_cast<CGObject *>(clientData)[0]);
	*static_cast<float *>(value) = selectedObject->initialRotateAngle.x;
}

void TW_CALL SetCallbackLocalPitch(const void *value, void *clientData)
{
	CGObject *selectedObject = &(static_cast<CGObject *>(clientData)[0]);
	selectedObject->initialRotateAngle.z = *(const float *)value;
}

void TW_CALL GetCallbackLocalPitch(void *value, void *clientData)
{
	CGObject *selectedObject = &(static_cast<CGObject *>(clientData)[0]);
	*static_cast<float *>(value) = selectedObject->initialRotateAngle.z;
}

void TW_CALL SetCallbackLocalYaw(const void *value, void *clientData)
{
	CGObject *selectedObject = &(static_cast<CGObject *>(clientData)[0]);
	selectedObject->initialRotateAngle.y = *(const float *)value;
}

void TW_CALL GetCallbackLocalYaw(void *value, void *clientData)
{
	CGObject *selectedObject = &(static_cast<CGObject *>(clientData)[0]);
	*static_cast<float *>(value) = selectedObject->initialRotateAngle.y;
}

// Callback function called by GLFW when window size changes
void WindowSizeCB(GLFWwindow* window, int width, int height)
{
	// Send the new window size to AntTweakBar
	glViewport(0, 0, width, height);

	TwWindowSize(width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (!TwEventMouseButtonGLFW3(window, button, action, mods))   // Send event to AntTweakBar
	{


	}
}

void mouse_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (!TwEventCursorPosGLFW3(window, xpos, ypos))   // Send event to AntTweakBar
	{
		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		double xoffset = xpos - lastX;
		double yoffset = lastY - ypos; //lastY - ypos; // reversed since y-coordinates go from bottom to top
		lastX = xpos;
		lastY = ypos;

		double sensitivity = 0.1f; // change this value to your liking
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		cameraYaw += xoffset;
		cameraPitch += yoffset;

		// make sure that when pitch is out of bounds, screen doesn't get flipped
		if (cameraPitch > 89.0f)
			cameraPitch = 89.0f;
		if (cameraPitch < -89.0f)
			cameraPitch = -89.0f;

		glm::vec3 front;
		front.x = cos(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
		front.y = sin(glm::radians(cameraPitch));
		front.z = sin(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
		cameraFront = glm::normalize(front);
	}
}

void key_callback(GLFWwindow* window, int button, int scancode, int action, int mods)
{
	if (!TwEventKeyGLFW3(window, button, scancode, action, mods))   // Send event to AntTweakBar
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
		if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
			pause = !pause;
	}
}

void addToObjectBuffer(CGObject *cg_object)
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

CGObject loadObjObject(vector<objl::Mesh> meshes, bool addToBuffers, bool subjectToGravity, vec3 initTransformVector, vec3 initScaleVector, vec3 color, float coef, CGObject* parent)
{
	CGObject object = CGObject();
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
		for (auto const& mesh :meshes) {					
			glutils.generateVertexArray(&(VAOs[numVAOs]));			
			object.VAOs.push_back(VAOs[numVAOs]);
			n_vbovertices += mesh.Vertices.size();
			n_ibovertices += mesh.Indices.size();
			numVAOs++;
		}
	}
	
	return object;
}

void createObjects()
{
	// Shader Attribute locations
	glutils.getAttributeLocationsPhong();

	const char* cubeFileName = "../Assignment2/meshes/small_airplane/planeUV.obj";
	vector<objl::Mesh> cubeMeshes = loadMeshes(cubeFileName);   // returns 2
	CGObject cubeObject = loadObjObject(cubeMeshes, true, true, vec3(0.0f, 0.0f, 0.0f), vec3(0.3f, 0.3f, 0.3f), vec3(1.0f, 0.5f, 0.0f), 0.65f, NULL); 
																																					  
	sceneObjects[numObjects] = cubeObject;
	numObjects++;

	glutils.createVBO(n_vbovertices);

	glutils.createIBO(n_ibovertices);

	addToObjectBuffer(&cubeObject);
	addToIndexBuffer(&cubeObject);
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
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;	
	lastFrame = currentFrame;

	// inpuT
//	processInput(window);

	// render
	glClearColor(0.78f, 0.84f, 0.49f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();

	glLoadIdentity();

	// activate shader
	glUseProgram(glutils.BlinnPhongID);

	// Update projection 
	glm::mat4 projection = glm::perspective(glm::radians(fov), (float)(SCR_WIDTH) / (float)(SCR_HEIGHT), 0.1f, 100.0f);
	glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

	glm::mat4 local1(1.0f);
	local1 = glm::translate(local1, cameraPos);
	glm::mat4 global1 = local1;

	glutils.updateUniformVariablesBlinnPhong(global1, view, projection);
	glUniform3f(glutils.lightColorLoc2, 1.0f, 1.0f, 1.0f);
	glUniform3f(glutils.lightPosLoc2, lightPos.x, lightPos.y, lightPos.z);
	glUniform3f(glutils.viewPosLoc2, cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform1f(glutils.ambientCoef2, 0.1f);
	glUniform1f(glutils.diffuseCoef2, 1.0f);
	glUniform1f(glutils.specularCoef2, 0.5f);
	glUniform1i(glutils.shininess2, 32);

	// DRAW 1st object
	for (int i = 0; i < numObjects; i++)     // TODO : need to fix this hardcoding
	{
		if (i == 1)
		{
			// second object
			glUniform1f(glutils.specularCoef2, 0.8f);
			glUniform1i(glutils.shininess2, 128);
		}

		mat4 globalCGObjectTransform = sceneObjects[i].createTransform();
		glutils.updateUniformVariablesBlinnPhong(globalCGObjectTransform);
		//sceneObjects[i].globalTransform = globalCGObjectTransform; // keep current state		

		glUniform3f(glutils.objectColorLoc2, sceneObjects[i].color.r, sceneObjects[i].color.g, sceneObjects[i].color.b);
		sceneObjects[i].Draw(glutils, false);
	}
	
	glPopMatrix();
	
	// disable VAO
	for (auto const& vao : VAOs) {
		glDisableVertexAttribArray(vao);
	}

	// Draw tweak bars
	TwDraw();

	// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
	glfwSwapBuffers(window);
	glfwPollEvents();
}

int main(void) {
	
	GLFWvidmode mode;   // GLFW video mode

	// Initialise GLFW
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
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Geometrical Transformations", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window.\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Initialize AntTweakBar
	TwInit(TW_OPENGL, NULL);

	// Create a tweak bar
	bar = TwNewBar("TweakBar");

	// Change the font size, and add a global message to the Help bar.
	TwDefine(" GLOBAL fontSize=3 help='This example illustrates the definition of custom structure type as well as many other features.' ");

	// Add 'time' to 'bar': it is a read-only (RO) variable of type TW_TYPE_DOUBLE, with 1 precision digit
	TwAddVarCB(bar, "Roll", TW_TYPE_FLOAT, SetCallbackLocalRoll, GetCallbackLocalRoll, &sceneObjects, " group='Plane rotation' label='Roll' precision=1 keyincr=r keyDecr=R help='Change roll of the plane' ");

	// Add 'time' to 'bar': it is a read-only (RO) variable of type TW_TYPE_DOUBLE, with 1 precision digit
	TwAddVarCB(bar, "Pitch", TW_TYPE_FLOAT, SetCallbackLocalPitch, GetCallbackLocalPitch, &sceneObjects, " group='Plane rotation' label='Pitch' min=0 max=360 precision=1 keyincr=p keyDecr=P help='Change pitch of the plane' ");

	// Add 'time' to 'bar': it is a read-only (RO) variable of type TW_TYPE_DOUBLE, with 1 precision digit
	TwAddVarCB(bar, "Yaw", TW_TYPE_FLOAT, SetCallbackLocalYaw, GetCallbackLocalYaw, &sceneObjects, " group='Plane rotation' label='Yaw' min=0 max=360 precision=1 keyincr=y keyDecr=Y help='Change yaw of the plane' ");

	//detect key inputs
	//glfwSetKeyCallback(window, keycallback);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, WindowSizeCB);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, mouse_position_callback);
	//glfwSetScrollCallback(window, scroll_callback);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	init();

	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0)
	{
		display();
	}

	// optional: de-allocate all resources once they've outlived their purpose:	
	glutils.deleteVertexArrays();
	glutils.deletePrograms();
	glutils.deleteBuffers();
	
	glfwTerminate();
	return 0;
}

//void processInput(GLFWwindow *window)
//{
//	/*if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//		glfwSetWindowShouldClose(window, true);
//
//	float cameraSpeed = 2.5 * deltaTime;
//	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
//		cameraPos += cameraSpeed * cameraFront;
//	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
//		cameraPos -= cameraSpeed * cameraFront;
//	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
//		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
//	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
//		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
//	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
//		pause = !pause;*/
//}
//
//// glfw: whenever the window size changed (by OS or user resize) this callback function executes
//// ---------------------------------------------------------------------------------------------
//void framebuffer_size_callback(GLFWwindow* window, int width, int height)
//{
//	// make sure the viewport matches the new window dimensions; note that width and 
//	// height will be significantly larger than specified on retina displays.
//	glViewport(0, 0, width, height);
//}
//
//// glfw: whenever the mouse moves, this callback is called
//// -------------------------------------------------------
//void mouse_callback(GLFWwindow* window, double xpos, double ypos)
//{
//	if (firstMouse)
//	{
//		lastX = xpos;
//		lastY = ypos;
//		firstMouse = false;
//	}
//
//	float xoffset = xpos - lastX;
//	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
//	lastX = xpos;
//	lastY = ypos;
//
//	float sensitivity = 0.1f; // change this value to your liking
//	xoffset *= sensitivity;
//	yoffset *= sensitivity;
//
//	myyaw += xoffset;
//	mypitch += yoffset;
//
//	// make sure that when pitch is out of bounds, screen doesn't get flipped
//	if (mypitch > 89.0f)
//		mypitch = 89.0f;
//	if (mypitch < -89.0f)
//		mypitch = -89.0f;
//
//	glm::vec3 front;
//	front.x = cos(glm::radians(myyaw)) * cos(glm::radians(mypitch));
//	front.y = sin(glm::radians(mypitch));
//	front.z = sin(glm::radians(myyaw)) * cos(glm::radians(mypitch));
//	cameraFront = glm::normalize(front);
//}
//
//// glfw: whenever the mouse scroll wheel scrolls, this callback is called
//// ----------------------------------------------------------------------
//void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
//{
//	if (fov >= 1.0f && fov <= 45.0f)
//		fov -= yoffset;
//	if (fov <= 1.0f)
//		fov = 1.0f;
//	if (fov >= 45.0f)
//		fov = 45.0f;
//}

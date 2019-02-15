#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1 
#define GLFW_DLL

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

#include <GL/glew.h>

#include <AntTweakBar.h>
#include <GLFW/glfw.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "opengl_utils.h"
#include "CGobject.h"

#include "..\Dependencies\OBJ_Loader.h"

// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#define MAX_OBJECTS 30

using namespace glm;
using namespace std;
using namespace Assignment2;

TwBar *bar;         // Pointer to a tweak bar

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

opengl_utils glutils; 

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

// camera movement
bool firstMouse = true;
float cameraYaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float cameraPitch = 0.0f;
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

int n_vbovertices = 0;
int n_ibovertices = 0;

GLfloat pointVertex[6];

//lighting position
glm::vec3 lightPos(-5.0f, -5.0f, -5.0f);

float local_roll = 0.0f;
float pitch = 0.0f;
float yaw = 0.0f;

void TW_CALL SetCallbackLocalRoll(const void *value, void *clientData)
{
	CGObject *selectedObject = & (static_cast<CGObject *>(clientData)[0]);
	selectedObject->eulerAngles.x = *(const float *)value;
}

void TW_CALL GetCallbackLocalRoll(void *value, void *clientData)
{
	 CGObject *selectedObject = &(static_cast<CGObject *>(clientData)[0]);
	 *static_cast<float *>(value) = selectedObject->eulerAngles.x;
}

void TW_CALL SetCallbackLocalPitch(const void *value, void *clientData)
{
	CGObject *selectedObject = &(static_cast<CGObject *>(clientData)[0]);
	selectedObject->eulerAngles.z = *(const float *)value;
}

void TW_CALL GetCallbackLocalPitch(void *value, void *clientData)
{
	CGObject *selectedObject = &(static_cast<CGObject *>(clientData)[0]);
	*static_cast<float *>(value) = selectedObject->eulerAngles.z;
}

void TW_CALL SetCallbackLocalYaw(const void *value, void *clientData)
{
	CGObject *selectedObject = &(static_cast<CGObject *>(clientData)[0]);
	selectedObject->eulerAngles.y = *(const float *)value;
}

void TW_CALL GetCallbackLocalYaw(void *value, void *clientData)
{
	CGObject *selectedObject = &(static_cast<CGObject *>(clientData)[0]);
	*static_cast<float *>(value) = selectedObject->eulerAngles.y;
}

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
		
		
	}
}

void GLFWCALL mouse_position_callback(int xpos, int ypos)
{
	if (!TwEventMousePosGLFW(xpos, ypos))   // Send event to AntTweakBar
	{
		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos; //lastY - ypos; // reversed since y-coordinates go from bottom to top
		lastX = xpos;
		lastY = ypos;

		float sensitivity = 0.1f; // change this value to your liking
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
		//cameraFront = glm::normalize(front);
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

void addToObjectBuffer(Assignment2::CGObject *cg_object)
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
			GLuint tmpVAO = VAOs[numVAOs];
			object.VAOs.push_back(tmpVAO);
			n_vbovertices += mesh.Vertices.size();
			n_ibovertices += mesh.Indices.size();
			numVAOs++;
		}
	}
	
	return object;
}

void drawCoordinateLines()
{
	glColor3f(1.0, 0.0, 0.0); // red x
	glUniform3f(glutils.objectColorLoc2, 1.0, 0.0, 0.0);
	glBegin(GL_LINES);
	// x aix

	glVertex3f(-4.0, 0.0f, 0.0f);
	glVertex3f(4.0, 0.0f, 0.0f);

	// arrow
	glVertex3f(4.0, 0.0f, 0.0f);
	glVertex3f(3.8, 0.2f, 0.0f);

	glVertex3f(4.0, 0.0f, 0.0f);
	glVertex3f(3.8, -0.2f, 0.0f);
	glEnd();
	glFlush();

	// y 
	glColor3f(0.0, 1.0, 0.0); // green y
	glUniform3f(glutils.objectColorLoc2, 0.0, 1.0, 0.0);

	glBegin(GL_LINES);
	glVertex3f(0.0, -4.0f, 0.0f);
	glVertex3f(0.0, 4.0f, 0.0f);

	// arrow
	glVertex3f(0.0, 4.0f, 0.0f);
	glVertex3f(0.2, 3.8f, 0.0f);

	glVertex3f(0.0, 4.0f, 0.0f);
	glVertex3f(-0.2, 3.8f, 0.0f);
	glEnd();
	glFlush();

	// z 
	glColor3f(0.0, 0.0, 1.0); // blue z
	glUniform3f(glutils.objectColorLoc2, 0.0, 0.0, 1.0);

	glBegin(GL_LINES);
	glVertex3f(0.0, 0.0f, -4.0f);
	glVertex3f(0.0, 0.0f, 4.0f);

	// arrow
	glVertex3f(0.0, 0.0f, 4.0f);
	glVertex3f(0.0, 0.2f, 3.8f);

	glVertex3f(0.0, 0.0f, 4.0f);
	glVertex3f(0.0, -0.2f, 3.8f);
	glEnd();
	glFlush();
}

void createObjects()
{
	// Shader Attribute locations
	glutils.getAttributeLocations();
	
	const char* cubeFileName = "../Assignment2/meshes/small_airplane/planeUV_centered.obj";
	vector<objl::Mesh> cubeMeshes = loadMeshes(cubeFileName);   // returns 2
	CGObject cubeObject = loadObjObject(cubeMeshes, true, true, vec3(2.0f, 0.0f, 0.0f), vec3(0.3f, 0.3f, 0.3f), vec3(1.0f, 0.5f, 0.0f), 0.65f, NULL); //choco - vec3(0.4f, 0.2f, 0.0f), 0.65f, NULL);
	cubeObject.initialRotateAngleEuler.z = cubeObject.eulerAngles.x = 0.3f;
	cubeObject.initialRotateAngleEuler.z = cubeObject.eulerAngles.y = 0.4f;
	cubeObject.initialRotateAngleEuler.z = cubeObject.eulerAngles.z = 0.5f;
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
	// render
	glClearColor(0.78f, 0.84f, 0.49f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Rotate model
	dt = glfwGetTime() - time;
	if (dt < 0) dt = 0;
	time += dt;
	turn += speed*dt;
	
	// activate shader
	glUseProgram(glutils.SimpleShaderID);

	int pass, numPass;

	// Enable OpenGL transparency and light (could have been done once at init)
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	
	
	
	numPass = 2;

	// Update projection 
	projection = glm::perspective(glm::radians(fov), (float)(SCR_WIDTH) / (float)(SCR_HEIGHT), nearclip, farclip);
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

	glm::mat4 local1(1.0f);
	//local1 = glm::translate(local1, cameraPos);
	glm::mat4 global1 = local1;

	glPushMatrix();
	glLoadIdentity();

	glutils.updateUniformVariablesSimple(global1,
		glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp),
		projection);

	drawCoordinateLines();

	glPopMatrix();

	glPushMatrix();
	glLoadIdentity();

	// activate shader
	glUseProgram(glutils.PhongProgramID);


	glutils.updateUniformVariables(global1, view, projection);	

	glUniform3f(glutils.lightColorLoc, 1.0f, 1.0f, 1.0f);
	glUniform3f(glutils.lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
	glUniform3f(glutils.viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform1f(glutils.ambientCoef, 0.1f);
	glUniform1f(glutils.diffuseCoef, 1.0f);
	glUniform1f(glutils.specularCoef, 0.5f);
	glUniform1i(glutils.shininess, 32);

	// DRAW objects
	for (int i = 0; i < numObjects; i++)     // TODO : need to fix this hardcoding
	{
		mat4 globalCGObjectTransform = sceneObjects[i].createTransform();
		glutils.updateUniformVariables(globalCGObjectTransform);
		sceneObjects[i].globalTransform = globalCGObjectTransform; // keep current state		

		glUniform3f(glutils.objectColorLoc, sceneObjects[i].color.r, sceneObjects[i].color.g, sceneObjects[i].color.b);
		sceneObjects[i].Draw(glutils);
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
	glfwEnable(GLFW_STICKY_KEYS);
	glfwEnable(GLFW_KEY_REPEAT);
	glfwSetWindowTitle("Geometrical Transformations");

	// Initialize AntTweakBar
	TwInit(TW_OPENGL, NULL);
	
	// Create a tweak bar
	bar = TwNewBar("TweakBar");

	// Change the font size, and add a global message to the Help bar.
	TwDefine(" GLOBAL fontSize=3 help='This example illustrates the definition of custom structure type as well as many other features.' ");
	
	// Add 'time' to 'bar': it is a read-only (RO) variable of type TW_TYPE_DOUBLE, with 1 precision digit
	TwAddVarCB(bar, "Roll", TW_TYPE_FLOAT, SetCallbackLocalRoll, GetCallbackLocalRoll, &sceneObjects, " group='Plane rotation' step=0.1 label='Roll' precision=2 keyincr=r keyDecr=R help='Change roll of the plane' ");

	// Add 'time' to 'bar': it is a read-only (RO) variable of type TW_TYPE_DOUBLE, with 1 precision digit
	TwAddVarCB(bar, "Pitch", TW_TYPE_FLOAT, SetCallbackLocalPitch, GetCallbackLocalPitch, &sceneObjects, " group='Plane rotation' step=0.1 label='Pitch' precision=2 keyincr=p keyDecr=P help='Change pitch of the plane' ");

	// Add 'time' to 'bar': it is a read-only (RO) variable of type TW_TYPE_DOUBLE, with 1 precision digit
	TwAddVarCB(bar, "Yaw", TW_TYPE_FLOAT, SetCallbackLocalYaw, GetCallbackLocalYaw, &sceneObjects, " group='Plane rotation' step=0.1 label='Yaw' precision=2 keyincr=y keyDecr=Y help='Change yaw of the plane' ");
		
	//TwAddVarRO(bar, "selected - posX", TW_TYPE_FLOAT, &tw_posX, 
//			" label='PosX - local' precision=2 help='local X-coord of the selected vertex.' ");

	// Set GLFW event callbacks
	// - Redirect window size changes to the callback function WindowSizeCB
	glfwSetWindowSizeCallback(WindowSizeCB);
	// - Directly redirect GLFW mouse button events to AntTweakBar
	glfwSetMouseButtonCallback(mouse_button_callback);
	// - Directly redirect GLFW mouse position events to AntTweakBar
	glfwSetMousePosCallback(mouse_position_callback);   
	// - Directly redirect GLFW mouse wheel events to AntTweakBar
	glfwSetMouseWheelCallback((GLFWmousewheelfun)TwEventMouseWheelGLFW);
	// - Directly redirect GLFW key events to AntTweakBar
	glfwSetKeyCallback(key_callback); //  
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

	// optional: de-allocate all resources once they've outlived their purpose:	
	glutils.deleteVertexArrays();
	glutils.deletePrograms();
	glutils.deleteBuffers();
	
	TwTerminate();
	glfwTerminate();
	return 0;
}


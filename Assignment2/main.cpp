#define _CRT_SECURE_NO_WARNINGS
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
const unsigned int SCR_WIDTH = 1200;//1920;
const unsigned int SCR_HEIGHT = 650;//1100;

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

int n_vbovertices = 0;
int n_ibovertices = 0;

GLfloat pointVertex[6];

//lighting position
glm::vec3 lightPos(10.0f, 10.0f, 3.0f);

float local_roll = 0.0f;
float pitch = 0.0f;
float yaw = 0.0f;

void TW_CALL SetCallbackLocalRoll(const void *value, void *clientData)
{
	local_roll = *(const float *)value;
}

void TW_CALL GetCallbackLocalRoll(void *value, void *clientData)
{
	*(float *)value = local_roll;
}

// Callback function called by GLFW when window size changes
void GLFWCALL WindowSizeCB(int width, int height)
{
	// Send the new window size to AntTweakBar
	TwWindowSize(width, height);
}

void GLFWCALL mouse_button_callback(int button, int action)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		
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

void createObjects()
{
	// Shader Attribute locations
	glutils.getAttributeLocations();
	
	const char* cubeFileName = "../Assignment2/meshes/Cube/cube.obj";
	vector<objl::Mesh> cubeMeshes = loadMeshes(cubeFileName);   // returns 2
	CGObject cubeObject = loadObjObject(cubeMeshes, true, true, vec3(5.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f), 0.65f, NULL); //choco - vec3(0.4f, 0.2f, 0.0f), 0.65f, NULL);
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
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotated(360.0*turn, 0.4, 1, 0.2);

	// activate shader
	glUseProgram(glutils.PhongProgramID);

	int pass, numPass;

	// Enable OpenGL transparency and light (could have been done once at init)
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHT0);    // use default light diffuse and position
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(3.0);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	numPass = 2;

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
	glfwEnable(GLFW_KEY_REPEAT);
	glfwSetWindowTitle("Facial Animation");

	// Initialize AntTweakBar
	TwInit(TW_OPENGL, NULL);
	
	// Create a tweak bar
	bar = TwNewBar("TweakBar");

	// Change the font size, and add a global message to the Help bar.
	TwDefine(" GLOBAL fontSize=3 help='This example illustrates the definition of custom structure type as well as many other features.' ");
	
	// Add 'time' to 'bar': it is a read-only (RO) variable of type TW_TYPE_DOUBLE, with 1 precision digit
	TwAddVarCB(bar, "Roll", TW_TYPE_FLOAT, SetCallbackLocalRoll, GetCallbackLocalRoll, NULL, " label='Roll' precision=1 help='Roll of the local object.' ");
		
	//TwAddVarRO(bar, "selected - posX", TW_TYPE_FLOAT, &tw_posX, 
//			" label='PosX - local' precision=2 help='local X-coord of the selected vertex.' ");

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
	glfwSetKeyCallback((GLFWkeyfun)TwEventKeyGLFW);
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


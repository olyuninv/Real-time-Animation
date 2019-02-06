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
using namespace Assignment1;

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

int n_vbovertices = 0;
int n_ibovertices = 0;

GLfloat pointVertex[6];

//lighting position
glm::vec3 lightPos(10.0f, 10.0f, 3.0f);

// Selected vertex
bool vertexSelected = false;
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

		for (int sceneIndex = 0; sceneIndex < numObjects; sceneIndex ++)
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
						vec4 (mesh.Vertices[mesh.Indices[i + 1]].Position.X,
							mesh.Vertices[mesh.Indices[i + 1]].Position.Y,
							mesh.Vertices[mesh.Indices[i + 1]].Position.Z, 1.0f);
					vec4 p3 = sceneObjects[sceneIndex].globalTransform*
						vec4 (mesh.Vertices[mesh.Indices[i + 2]].Position.X,
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
		if (!foundIntersection)
		{
			vertexSelected = false;
		}
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
	if (a >= 0 && b >= 0 && c >=0 || a < 0 && b < 0 && c < 0)
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
	
	const char* boyFileName = "../Assignment1/meshes/Head/male head.obj"; 
	vector<objl::Mesh> meshes = loadMeshes(boyFileName);   // returns 2
	CGObject boyObject = loadObjObject(meshes, true, true, vec3(0.0f, 0.0f, 0.0f), vec3(0.15f, 0.15f, 0.15f), vec3(1.0f, 1.0f, 1.0f), 0.65f, NULL); //choco - vec3(0.4f, 0.2f, 0.0f), 0.65f, NULL);
	sceneObjects[numObjects] = boyObject;
	numObjects++;
	
	const char* cubeFileName = "../Assignment1/meshes/Cube/cube.obj";
	vector<objl::Mesh> cubeMeshes = loadMeshes(cubeFileName);   // returns 2
	CGObject cubeObject = loadObjObject(cubeMeshes, true, true, vec3(5.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f), 0.65f, NULL); //choco - vec3(0.4f, 0.2f, 0.0f), 0.65f, NULL);
	sceneObjects[numObjects] = cubeObject;
	numObjects++;

	glutils.createVBO(n_vbovertices);

	glutils.createIBO(n_ibovertices);
	
	addToObjectBuffer(&boyObject);
	addToObjectBuffer(&cubeObject);
	addToIndexBuffer(&boyObject);
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

	if (vertexSelected)
	{
		//glUseProgram(glutils.CircleID);
		
		/*pointVertex[0] = (GLfloat)sceneObjects[selectedSceneObject].Meshes[selectedObjectMesh].Vertices[selectedVertexIndex].Position.X;
		pointVertex[1] = (GLfloat)sceneObjects[selectedSceneObject].Meshes[selectedObjectMesh].Vertices[selectedVertexIndex].Position.Y;
		pointVertex[2] = (GLfloat)sceneObjects[selectedSceneObject].Meshes[selectedObjectMesh].Vertices[selectedVertexIndex].Position.Z;
		pointVertex[3] = 1.0f;
		pointVertex[4] = 0.0f;
		pointVertex[5] = 0.0f;*/

		//glEnable(GL_POINT_SMOOTH);
		//glEnableClientState(GL_VERTEX_ARRAY);
		//glEnableClientState(GL_COLOR_ARRAY);
		//glPointSize(50);
		//glColor3f(1, 0, 0);
		//glVertexPointer(3, GL_FLOAT, 6, &pointVertex[0]);
		//glColorPointer(3, GL_FLOAT, 6, &pointVertex[3]);
		//glDrawArrays(GL_POINTS, 0, 1);
		//glDisableClientState(GL_VERTEX_ARRAY);
		//glDisableClientState(GL_COLOR_ARRAY);
		//glDisable(GL_POINT_SMOOTH);

		/*glUseProgram(glutils.CircleID);

		glBegin(GL_POINTS);
		glVertex3f(sceneObjects[0].Meshes[selectedObjectMesh].Vertices[selectedVertexIndex].Position.X, 
			sceneObjects[0].Meshes[selectedObjectMesh].Vertices[selectedVertexIndex].Position.Y, 
			sceneObjects[0].Meshes[selectedObjectMesh].Vertices[selectedVertexIndex].Position.Z);

		glEnd();*/

		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_NOTEQUAL, 0);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_POINT_SMOOTH);
		glPointSize(20.0);

		//glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(&(projection[0][0]));
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(&view[0][0]); 

		glBegin(GL_POINTS);
		glUniform3f(glutils.objectColorLoc, 1.0f, 0.0f, 0.0f);
		//glutils.updateUniformVariables(sceneObjects[selectedSceneObject].globalTransform);

		auto cursorPoint = sceneObjects[selectedSceneObject].globalTransform *
			vec4(sceneObjects[selectedSceneObject].Meshes[selectedObjectMesh].Vertices[selectedVertexIndex].Position.X,
				sceneObjects[selectedSceneObject].Meshes[selectedObjectMesh].Vertices[selectedVertexIndex].Position.Y,
				sceneObjects[selectedSceneObject].Meshes[selectedObjectMesh].Vertices[selectedVertexIndex].Position.Z, 1.0);
		
		glVertex3f(cursorPoint.x - 5.0f, cursorPoint.y, cursorPoint.z + 0.02f);
		
		glEnd();
		glDisable(GL_POINT_SMOOTH);
		glBlendFunc(GL_NONE, GL_NONE);
		glDisable(GL_BLEND);
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
	TwInit(TW_OPENGL, NULL);
	
	// Create a tweak bar
	bar = TwNewBar("TweakBar");

	// Change the font size, and add a global message to the Help bar.
	TwDefine(" GLOBAL fontSize=3 help='This example illustrates the definition of custom structure type as well as many other features.' ");
	
	// Add 'time' to 'bar': it is a read-only (RO) variable of type TW_TYPE_DOUBLE, with 1 precision digit
	TwAddVarRO(bar, "time", TW_TYPE_BOOLCPP, &vertexSelected, " label='Vertex selected' precision=1 help='Indicates if vertex is selected.' ");

	TwAddVarRO(bar, "selected Scene Object", TW_TYPE_INT32, &selectedSceneObject, " label='Selected scene object' precision=0 help='Index of the selected object.' ");

	TwAddVarRO(bar, "selected Mesh index", TW_TYPE_INT32, &selectedObjectMesh, " label='Selected mesh index' precision=0 help='Index of the selected mesh.' ");
	
	TwAddVarRO(bar, "selected Vertex index", TW_TYPE_INT32, &selectedVertexIndex, " label='Selected vertex index' precision=0 help='Index of the selected vertex.' ");

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
		" label='PosZ - world' precision=2 help='World Z-coord of the selected vertex.' ");

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


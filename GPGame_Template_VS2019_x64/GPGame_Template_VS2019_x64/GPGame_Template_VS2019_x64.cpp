// Simplified Renderer application for GP course
// Features:
// Reduced OpenGL version from 4.5 to 3.3 to allow it to render in older laptops.
// Added Shapes library for rendering cubes, spheres and vectors.
// Added examples of matrix multiplication on Update.
// Added resize screen and keyboard callbacks.
// Added FPS camera functionality
// Update 2019/01 updated libraries and created project for VS2017 including directory dependant links to libraries.
// Update 2020/01 updated libraries for x64 and for VS2020, also adding MAC compiled Libraries.

// Suggestions or extra help please do email me at S.Padilla@hw.ac.uk

// Standard C++ libraries
#include <iostream>
#include <vector>
#include <list>
using namespace std;

// Helper graphic libraries
#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include "graphics.h"
#include "shapes.h"

// MAIN FUNCTIONS
float getDistanceBetweenCenters(Shapes shape1, Shapes shape2);
float getX(Shapes shape);
float getY(Shapes shape);
float getZ(Shapes shape);
void startup();
void updateCamera();
void updateSceneElements();
void renderScene();

// CALLBACK FUNCTIONS
void onResizeCallback(GLFWwindow* window, int w, int h);
void onKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void onMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void onMouseMoveCallback(GLFWwindow* window, double x, double y);
void onMouseWheelCallback(GLFWwindow* window, double xoffset, double yoffset);

// VARIABLES
bool        quit = false;
float       deltaTime = 0.0f;    // Keep track of time per frame.
float       lastTime = 0.0f;    // variable to keep overall time.
bool        keyStatus[1024];    // Hold key status.
bool		mouseEnabled = true; // keep track of mouse toggle.

// MAIN GRAPHICS OBJECT
Graphics    myGraphics;        // Runing all the graphics in this object

// DEMO OBJECTS
Cube        myCube;
Sphere      mySphere;
Arrow       arrowX;
Arrow       arrowY;
Arrow       arrowZ;
Cube        myFloor;
Line        myLine;
Cylinder    myCylinder;
list<Shapes> allShapes = { myCube, mySphere, myCylinder };

// Some global variable to do the animation.
float t = 0.001f;            // Global variable for animation



float cx = 2.0f;
float cy = 0.5f;
float cz = 0.0f;

int main()
{
	int errorGraphics = myGraphics.Init();			// Launch window and graphics context
	if (errorGraphics) return 0;					// Close if something went wrong...

	startup();										// Setup all necessary information for startup (aka. load texture, shaders, models, etc).



	// MAIN LOOP run until the window is closed
	while (!quit) {

		// Update the camera transform based on interactive inputs or by following a predifined path.
		updateCamera();

		// Update position, orientations and any other relevant visual state of any dynamic elements in the scene.
		updateSceneElements();

		// Render a still frame into an off-screen frame buffer known as the backbuffer.
		renderScene();

		// Swap the back buffer with the front buffer, making the most recently rendered image visible on-screen.
		glfwSwapBuffers(myGraphics.window);        // swap buffers (avoid flickering and tearing)

	}


	myGraphics.endProgram();            // Close and clean everything up...

   // cout << "\nPress any key to continue...\n";
   // cin.ignore(); cin.get(); // delay closing console to read debugging errors.
	cout << "here\n";
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			cout << myCube.w_matrix[j][i] << " ";
		}
		cout << "\n";
	}
	float cubex = getX(myCube);
	float cubey = getY(myCube);
	float cubez = getZ(myCube);
	float spherex = getX(mySphere);
	float spherey = getY(mySphere);
	float spherez = getZ(mySphere);

	float cubesphere = getDistanceBetweenCenters(myCube, mySphere);
	cout << "\n" << cubesphere;
	float cubesphere1 = getDistanceBetweenCenters(myCube, myCylinder);
	cout << "\n" << cubesphere1;
	float cubesphere2 = getDistanceBetweenCenters(myCylinder, mySphere);
	cout << "\n" << cubesphere2;
	return 0;
	return 0;
}

float getX(Shapes shape) {
	float x_location = shape.w_matrix[3][0];
	return x_location;
}
float getY(Shapes shape) {
	float y_location = shape.w_matrix[3][1];
	return y_location;
}
float getZ(Shapes shape) {
	float z_location = shape.w_matrix[3][2];
	return z_location;
}
float getDistanceBetweenCenters(Shapes shape1, Shapes shape2) {
	float shape1x = getX(shape1);
	float shape1y = getY(shape1);
	float shape1z = getZ(shape1); 
	float shape2x = getX(shape2);
	float shape2y = getY(shape2);
	float shape2z = getZ(shape2);
	float distancex = shape1x - shape2x;
	float distancey= shape1y - shape2y;
	float distancez = shape1z - shape2z;
	float xycomponent = sqrt((distancex * distancex) + (distancey * distancey));
	float finalDistance = sqrt((xycomponent * xycomponent) + (distancez * distancez));
	return finalDistance;
}

bool isColliding(Shapes shape1, Shapes shape2) 
{
	float distBetween = getDistanceBetweenCenters(shape1, shape2);
	if (distBetween < 1)
	{
		return true;
	}
	else {
		return false;
	}

}

void startup() {
	// Keep track of the running time
	GLfloat currentTime = (GLfloat)glfwGetTime();    // retrieve timelapse
	deltaTime = currentTime;                        // start delta time
	lastTime = currentTime;                            // Save for next frame calculations.

	// Callback graphics and key update functions - declared in main to avoid scoping complexity.
	// More information here : https://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowSizeCallback(myGraphics.window, onResizeCallback);            // Set callback for resize
	glfwSetKeyCallback(myGraphics.window, onKeyCallback);                    // Set Callback for keys
	glfwSetMouseButtonCallback(myGraphics.window, onMouseButtonCallback);    // Set callback for mouse click
	glfwSetCursorPosCallback(myGraphics.window, onMouseMoveCallback);        // Set callback for mouse move
	glfwSetScrollCallback(myGraphics.window, onMouseWheelCallback);            // Set callback for mouse wheel.

	// Calculate proj_matrix for the first time.
	myGraphics.aspect = (float)myGraphics.windowWidth / (float)myGraphics.windowHeight;
	myGraphics.proj_matrix = glm::perspective(glm::radians(50.0f), myGraphics.aspect, 0.1f, 1000.0f);

	// Load Geometry examples
	myCube.Load();

	mySphere.Load();
	mySphere.fillColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);    // You can change the shape fill colour, line colour or linewidth

	arrowX.Load(); arrowY.Load(); arrowZ.Load();
	arrowX.fillColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); arrowX.lineColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	arrowY.fillColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f); arrowY.lineColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	arrowZ.fillColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); arrowZ.lineColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

	myFloor.Load();
	myFloor.fillColor = glm::vec4(130.0f / 255.0f, 96.0f / 255.0f, 61.0f / 255.0f, 1.0f);    // Sand Colour
	myFloor.lineColor = glm::vec4(130.0f / 255.0f, 96.0f / 255.0f, 61.0f / 255.0f, 1.0f);    // Sand again

	myCylinder.Load();
	myCylinder.fillColor = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
	myCylinder.lineColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	myLine.Load();
	myLine.fillColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	myLine.lineColor = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	myLine.lineWidth = 5.0f;

	// Optimised Graphics
	myGraphics.SetOptimisations();        // Cull and depth testing

}

void updateCamera() {

	// calculate movement for FPS camera
	GLfloat xoffset = myGraphics.mouseX - myGraphics.cameraLastX;
	GLfloat yoffset = myGraphics.cameraLastY - myGraphics.mouseY;    // Reversed mouse movement
	myGraphics.cameraLastX = (GLfloat)myGraphics.mouseX;
	myGraphics.cameraLastY = (GLfloat)myGraphics.mouseY;

	GLfloat sensitivity = 0.05f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	myGraphics.cameraYaw += xoffset;
	myGraphics.cameraPitch += yoffset;

	// check for pitch out of bounds otherwise screen gets flipped
	if (myGraphics.cameraPitch > 89.0f) myGraphics.cameraPitch = 89.0f;
	if (myGraphics.cameraPitch < -89.0f) myGraphics.cameraPitch = -89.0f;

	// Calculating FPS camera movement (See 'Additional Reading: Yaw and Pitch to Vector Calculations' in VISION)
	glm::vec3 front;
	front.x = cos(glm::radians(myGraphics.cameraYaw)) * cos(glm::radians(myGraphics.cameraPitch));
	front.y = sin(glm::radians(myGraphics.cameraPitch));
	front.z = sin(glm::radians(myGraphics.cameraYaw)) * cos(glm::radians(myGraphics.cameraPitch));

	myGraphics.cameraFront = glm::normalize(front);

	// Update movement using the keys
	GLfloat cameraSpeed = 1.0f * deltaTime;
	if (keyStatus[GLFW_KEY_W]) myGraphics.cameraPosition += cameraSpeed * myGraphics.cameraFront;
	if (keyStatus[GLFW_KEY_S]) myGraphics.cameraPosition -= cameraSpeed * myGraphics.cameraFront;
	if (keyStatus[GLFW_KEY_A]) myGraphics.cameraPosition -= glm::normalize(glm::cross(myGraphics.cameraFront, myGraphics.cameraUp)) * cameraSpeed;
	if (keyStatus[GLFW_KEY_D]) myGraphics.cameraPosition += glm::normalize(glm::cross(myGraphics.cameraFront, myGraphics.cameraUp)) * cameraSpeed;

	// IMPORTANT PART
	// Calculate my view matrix using the lookAt helper function
	if (mouseEnabled) {
		myGraphics.viewMatrix = glm::lookAt(myGraphics.cameraPosition,			// eye
			myGraphics.cameraPosition + myGraphics.cameraFront,					// centre
			myGraphics.cameraUp);												// up
	}
}

void updateSceneElements() {

	glfwPollEvents();                                // poll callbacks

	// Calculate frame time/period -- used for all (physics, animation, logic, etc).
	GLfloat currentTime = (GLfloat)glfwGetTime();    // retrieve timelapse
	deltaTime = currentTime - lastTime;                // Calculate delta time
	lastTime = currentTime;                            // Save for next frame calculations.

	// Do not forget your ( T * R * S ) http://www.opengl-tutorial.org/beginners-tutorials/tutorial-3-matrices/

	// Calculate Cube position
	myCube.w_matrix = 
		glm::translate(glm::vec3(cx, cy, cz)) *
		glm::mat4(1.0f);
	myCube.mv_matrix = myGraphics.viewMatrix * myCube.w_matrix;
	myCube.proj_matrix = myGraphics.proj_matrix;

	// calculate Sphere movement
	mySphere.w_matrix = glm::translate(glm::vec3(-2.0f, 0.5f, 0.0f)) *
		glm::rotate(-t, glm::vec3(0.0f, 1.0f, 0.0f)) *
		glm::rotate(-t, glm::vec3(1.0f, 0.0f, 0.0f)) *
		glm::mat4(1.0f);
	mySphere.mv_matrix = myGraphics.viewMatrix * mySphere.w_matrix;
	mySphere.proj_matrix = myGraphics.proj_matrix;


	//Calculate Arrows translations (note: arrow model points up)
	glm::mat4 mv_matrix_x =
		glm::translate(glm::vec3(0.0f, 0.0f, 0.0f)) *
		glm::rotate(glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
		glm::scale(glm::vec3(0.2f, 0.5f, 0.2f)) *
		glm::mat4(1.0f);
	arrowX.mv_matrix = myGraphics.viewMatrix * mv_matrix_x;
	arrowX.proj_matrix = myGraphics.proj_matrix;

	glm::mat4 mv_matrix_y =
		glm::translate(glm::vec3(0.0f, 0.0f, 0.0f)) *
		//glm::rotate(glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *    // already model pointing up
		glm::scale(glm::vec3(0.2f, 0.5f, 0.2f)) *
		glm::mat4(1.0f);
	arrowY.mv_matrix = myGraphics.viewMatrix * mv_matrix_y;
	arrowY.proj_matrix = myGraphics.proj_matrix;

	glm::mat4 mv_matrix_z =
		glm::translate(glm::vec3(0.0f, 0.0f, 0.0f)) *
		glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
		glm::scale(glm::vec3(0.2f, 0.5f, 0.2f)) *
		glm::mat4(1.0f);
	arrowZ.mv_matrix = myGraphics.viewMatrix * mv_matrix_z;
	arrowZ.proj_matrix = myGraphics.proj_matrix;

	// Calculate floor position and resize
	myFloor.mv_matrix = myGraphics.viewMatrix *
		glm::translate(glm::vec3(0.0f, 0.0f, 0.0f)) *
		glm::scale(glm::vec3(1000.0f, 0.001f, 1000.0f)) *
		glm::mat4(1.0f);
	myFloor.proj_matrix = myGraphics.proj_matrix;

	// Calculate cylinder
	myCylinder.mv_matrix = myGraphics.viewMatrix *
		glm::translate(glm::vec3(-1.0f, 0.5f, 2.0f)) *
		glm::mat4(1.0f);
	myCylinder.proj_matrix = myGraphics.proj_matrix;

	// Calculate Line
	myLine.mv_matrix = myGraphics.viewMatrix *
		glm::translate(glm::vec3(1.0f, 0.5f, 2.0f)) *
		glm::mat4(1.0f);
	myLine.proj_matrix = myGraphics.proj_matrix;


	t += 0.01f; // increment movement variable


	if (glfwWindowShouldClose(myGraphics.window) == GL_TRUE) quit = true; // If quit by pressing x on window.

//Itteration for allShapes
	list<Shapes>::iterator itAllShapes;
	for (itAllShapes = allShapes.begin(); itAllShapes != allShapes.end(); itAllShapes++)
	{


	}
	bool colliding = isColliding(mySphere, myCube);
	cout << colliding << "\n";



}

void renderScene() {
	// Clear viewport - start a new frame.
	myGraphics.ClearViewport();

	// Draw objects in screen
	myFloor.Draw();
	myCube.Draw();
	mySphere.Draw();

	arrowX.Draw();
	arrowY.Draw();
	arrowZ.Draw();

	myLine.Draw();
	myCylinder.Draw();
}


// CallBack functions low level functionality.
void onResizeCallback(GLFWwindow* window, int w, int h) {    // call everytime the window is resized
	//myGraphics.windowWidth = w;
	//myGraphics.windowHeight = h;

	glfwGetFramebufferSize(window, &myGraphics.windowWidth, &myGraphics.windowHeight);

	myGraphics.aspect = (float)w / (float)h;
	myGraphics.proj_matrix = glm::perspective(glm::radians(50.0f), myGraphics.aspect, 0.1f, 1000.0f);
}

void onKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) { // called everytime a key is pressed
	if (action == GLFW_PRESS) keyStatus[key] = true;
	else if (action == GLFW_RELEASE) keyStatus[key] = false;
	if (keyStatus[GLFW_KEY_J] == true) {
		cx += 0.01;
	}
	if (keyStatus[GLFW_KEY_L] == true) {
		cx -= 0.01;
	}

	if (keyStatus[GLFW_KEY_I] == true) {
		cy += 0.01;
	}
	if (keyStatus[GLFW_KEY_K] == true) {
		cy -= 0.01;
	}
	// toggle showing mouse.
	if (keyStatus[GLFW_KEY_M]) {
		mouseEnabled = !mouseEnabled;
		myGraphics.ToggleMouse();
	}
	// If exit key pressed.
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void onMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {

}

void onMouseMoveCallback(GLFWwindow* window, double x, double y) {
	int mouseX = static_cast<int>(x);
	int mouseY = static_cast<int>(y);

	myGraphics.mouseX = mouseX;
	myGraphics.mouseY = mouseY;

	// helper variables for FPS camera
	if (myGraphics.cameraFirstMouse) {
		myGraphics.cameraLastX = (GLfloat)myGraphics.mouseX; myGraphics.cameraLastY = (GLfloat)myGraphics.mouseY; myGraphics.cameraFirstMouse = false;
	}
}

void onMouseWheelCallback(GLFWwindow* window, double xoffset, double yoffset) {
	int yoffsetInt = static_cast<int>(yoffset);
}

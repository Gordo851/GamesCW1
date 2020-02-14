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
#include <algorithm> 
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
void startup();
void updateCamera();
void updateSceneElements();
void renderScene();


float getSquareDistance(Shapes a, Shapes b);
void ApplyForce(Shapes& shape, glm::vec3 force);
void calculateMinMax(Shapes& shape);
bool isColliding(Shapes& shape1, Shapes& shape2);
glm::vec3 getXYZdistance(glm::vec3 point1, glm::vec3 point2);
glm::vec3 getXYZdistance(Shapes a, Shapes b);
glm::vec3 getposition(Shapes shape);
float getSquareDistance(Shapes a, Shapes b);
float getSquareDistance(glm::vec3 point1, glm::vec3 point2);
void checkCollisions();
void PositionalCorrection(Shapes& a, Shapes& b, float penetration, glm::vec3 normal);
void getAACubeNormal(Shapes& a, Shapes& b);
bool getAAcubeSphereNormal(Shapes& a, Shapes& b);

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
Cube        myCube2;
Sphere      mySphere;
Arrow       arrowX;
Arrow       arrowY;
Arrow       arrowZ;
Cube        myFloor;
Line        myLine;
Cylinder    myCylinder;
Cube cubeArray[10][10];

vector<Shapes*> allShapes;
vector<Collision*> allCollisions;


vector<Particle*> allParticles;
Particle particleArray[20];
Particle particle;

// Some global variable to do the animation.
float t = 0.001f;            // Global variable for animation


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

	return 0;
}

///////////////////////////////////////////////physics

void ApplyForce(Shapes& shape, glm::vec3 force)
{
	if (shape.mass != 0) {
		glm::vec3 Velocity = force / shape.mass;
		shape.velocity = shape.velocity + Velocity;
	}
	
}


void calculateRebound(Shapes& a, Shapes& b, glm::vec3 normal) {

	float e = std::min(a.e, b.e);

	if (b.mass == 0) {//shape 2 static
		glm::vec3 phat = (e + 1.0f) * a.mass * (a.velocity * normal) * normal;
		a.velocity = a.velocity + (((-phat) / a.mass));
		//std::cout << "shape 2 static \n";
	}
	else if (a.mass == 0) {//shape 1 static
		glm::vec3 phat = (e + 1.0f) * b.mass * (b.velocity * normal) * normal;
		b.velocity = b.velocity + (((phat) / b.mass));
		//std::cout << "shape 1 static \n";
	}
	else {
		glm::vec3 phat = (((e + 1.0f) * (a.velocity * normal - b.velocity * normal)) / ((1.0f / b.mass) + (1.0f / a.mass))) * normal;

		a.velocity = a.velocity + (((-phat) / a.mass));
		b.velocity = b.velocity + (((phat) / b.mass));
		//shape1.velocity = shape1.velocity + 2.0f * ((-shape1.velocity) * normal);
	}

	/*

	glm::vec3 relVelocity = b.velocity - a.velocity;

	float relVelocityAlongNormal = glm::dot(relVelocity, normal);
	cout << "here\n";
	//  if velocities are separating do nothing
	if (relVelocityAlongNormal > 0) {
		return;
	}

	// Calculate impulse scalar and e
	float e = std::min(a.e, b.e);
	float j = -(1 + e) * relVelocityAlongNormal;

	j = j /( a.invMass + b.invMass);
	// Apply impulse

	glm::vec3 impulse = j * normal;

	if (a.mass == 0) {
		b.velocity +=  impulse / b.mass;
	}
	else if (b.mass == 0) {
		a.velocity -= impulse / a.mass;
	}
	else {
		float mass_sum = a.mass + b.mass;
		float ratio = a.mass / mass_sum;
		a.velocity -= ratio * impulse / a.mass;
		ratio = b.mass / mass_sum;
		b.velocity += ratio * impulse / b.mass;

	}
	*/
}

void PositionalCorrection(Shapes& a, Shapes& b, float penetration,  glm::vec3 normal)
{
	
	//glm::vec3 aPos = getposition(a);
	//glm::vec3 bPos = getposition(b);
	
	float percent = 0.8f; // usually 20% to 80%
	float slop = 0.01f; // usually 0.01 to 0.1
	
	glm::vec3 correction = (std::max((penetration - slop), 0.0f) / (a.invMass + b.invMass)) * percent * normal;
	
	

	a.correction -= a.invMass * correction;
	b.correction += b.invMass * correction;
	/*
	a.w_matrix[3][0] = aPos.x;
	a.w_matrix[3][1] = aPos.y;
	a.w_matrix[3][2] = aPos.z;

	b.w_matrix[3][0] = aPos.x;
	b.w_matrix[3][1] = aPos.y;
	b.w_matrix[3][2] = aPos.z;
	*/
}

/**
calculate an objects vertex positions in the world view
*/
void calculateMinMax(Shapes& shape) {

	vector<GLfloat> newPositions;
	for (unsigned int i = 0; i < shape.vertexPositions.size(); i += 3) {
		glm::vec3 vec = glm::vec3(shape.vertexPositions[i], shape.vertexPositions[i + 1], shape.vertexPositions[i + 2]);
		glm::mat4 vertexInWorldPosition = shape.w_matrix * glm::translate(vec) * glm::mat4(1.0f);
		newPositions.push_back(vertexInWorldPosition[3][0]);
		newPositions.push_back(vertexInWorldPosition[3][1]);
		newPositions.push_back(vertexInWorldPosition[3][2]);
	}
	//cout << shape.vertexPositions.size() << "unmoved\n";
	shape.currentVertexPositions = newPositions;

	glm::vec3 min = glm::vec3(999999.0f);
	glm::vec3 max = glm::vec3(-999999.0f);

	for (unsigned int i = 0; i < newPositions.size(); i += 3) {
		for (int x = 0; x < 3; x++) {
			if (newPositions[i + x] > max[x]) {
				max[x] = newPositions[i + x];
			}
			if (newPositions[i + x] < min[x]) {
				min[x] = newPositions[i + x];
			}
		}
	}
		
	shape.min = min;
	shape.max = max;

}



bool isColliding(Shapes& shape1, Shapes& shape2) {

	if (shape1.collision_type == none || shape2.collision_type == none) {
		return false;
	} 
	else if (shape1.collision_type == AAcube && shape2.collision_type == AAcube) {
		//cout << "cube\n";
		if (shape1.max.x < shape2.min.x || shape1.min.x > shape2.max.x) {
			return false;
		}
		if (shape1.max.y < shape2.min.y || shape1.min.y > shape2.max.y) {
			return false;
		}
		if (shape1.max.z < shape2.min.z || shape1.min.z > shape2.max.z) {
			return false;
		}

		getAACubeNormal(shape1, shape2);

		return true;
	}
	else if (shape1.collision_type == sphere && shape2.collision_type == sphere) {
		//cout << "shpere\n";
		float radius = shape1.radius  + shape2.radius ;
		float radius2 = radius * radius;
		float diss = getSquareDistance(shape1, shape2);
		if ( diss < radius2) {
			diss = sqrt(diss);
			if (diss != 0)
			{
				Collision coll;
				
				// Distance is difference between radius and distanc
				coll.penetration = radius - diss;
				coll.normal = getXYZdistance(shape2, shape1)/diss;
				allCollisions.push_back(&coll);
				PositionalCorrection(shape1, shape2, coll.penetration, coll.normal);
				return true;
			}
			


			return true;
		}
		return false;
	}
	else if ((shape1.collision_type == AAcube && shape2.collision_type == sphere) || (shape1.collision_type == sphere && shape2.collision_type == AAcube)) {//////////////////temp code
		//cout << "cube\n";
		if (shape1.max.x < shape2.min.x || shape1.min.x > shape2.max.x) {
			return false;
		}
		if (shape1.max.y < shape2.min.y || shape1.min.y > shape2.max.y) {
			return false;
		}
		if (shape1.max.z < shape2.min.z || shape1.min.z > shape2.max.z) {
			return false;
		}

		getAACubeNormal(shape1, shape2);

		return true;
	}

}


bool getAAcubeSphereNormal(Shapes& a, Shapes& b) {
	return false;
	
}

void getAACubeNormal(Shapes& a, Shapes& b) {
	
	//https://gamedevelopment.tutsplus.com/tutorials/how-to-create-a-custom-2d-physics-engine-the-basics-and-impulse-resolution--gamedev-6331
	glm::vec3 xyzDiss = getXYZdistance(b, a);
	glm::vec3 overlap = glm::vec3(0.0f);
	
	Collision coll;
	
	overlap = (((a.max - a.min)/2.0f) + ((b.max - b.min) / 2.0f)) - abs(xyzDiss);
	
	
	

	
	if (overlap.x < overlap.y) {
		if (overlap.x < overlap.z) {
			if (xyzDiss.x < 0) {
				coll.normal = glm::vec3(-1.0, 0.0,0.0);
			}
			else {
				coll.normal = glm::vec3(1.0, 0.0, 0.0);
			}
			coll.penetration = overlap.x;
		}
		else {
			if (xyzDiss.z < 0) {
				coll.normal = glm::vec3(0.0, 0.0, -1.0);
			}
			else {
				coll.normal = glm::vec3(0.0, 0.0, 1.0);
			}
			coll.penetration = overlap.z;

		}
	}
	else {
		if (overlap.y < overlap.z) {
			if (xyzDiss.y < 0) {
				coll.normal = glm::vec3(0.0,-1.0, 0.0);
			}
			else {
				coll.normal = glm::vec3(0.0,  1.0, 0.0);
			}
			coll.penetration = overlap.y;
		}
		else {
			if (xyzDiss.z < 0) {
				coll.normal = glm::vec3(0.0, 0.0, -1.0);
			}
			else {
				coll.normal = glm::vec3(0.0, 0.0, 1.0);
			}
			coll.penetration = overlap.z;
		}
	}
	//coll.normal = getXYZdistance(a, b) / sqrt(getSquareDistance(a,b));
	allCollisions.push_back(&coll);
	PositionalCorrection(a, b, coll.penetration, coll.normal);
	
	
}

glm::vec3 getXYZdistance(glm::vec3 point1, glm::vec3 point2) {
	return point1 - point2;
}

glm::vec3 getXYZdistance(Shapes a, Shapes b) {
	glm::vec3 point1 = getposition(a);
	glm::vec3 point2 = getposition(b);
	return point1 - point2;
}

glm::vec3 getposition(Shapes shape) {
	return glm::vec3(shape.w_matrix[3][0], shape.w_matrix[3][1], shape.w_matrix[3][2]);
}

float getSquareDistance(Shapes a, Shapes b) {
	glm::vec3 positionA = getposition(a);
	glm::vec3 positionB = getposition(b);
	glm::vec3 distanceXYZ = getXYZdistance(positionA, positionB);

	return distanceXYZ.x * distanceXYZ.x + distanceXYZ.z * distanceXYZ.z + distanceXYZ.y * distanceXYZ.y;
}

float getSquareDistance(glm::vec3 point1, glm::vec3 point2) {
	glm::vec3 distanceXYZ = getXYZdistance( point1,  point2);

	return distanceXYZ.x * distanceXYZ.x + distanceXYZ.z * distanceXYZ.z + distanceXYZ.y * distanceXYZ.y;
}


void checkCollisions()
{

	int count = 0;
	for (int i = 0; i < allShapes.size(); i++)
	{
		Shapes& shape1 = *allShapes[i];
		if (shape1.velocity.x == 0 && shape1.velocity.y == 0 && shape1.velocity.z == 0) {//only calc new if moving

		}
		else {
			count++;
			calculateMinMax(shape1);
		}
		//shape1.fillColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
	}
	
	//Itteration for allShapes
	for (int i = 0; i < allShapes.size(); i++)
	{
		for (int j = i + 1; j < allShapes.size(); j++)
		{
			Shapes& shape1 = *allShapes[i];
			Shapes& shape2 = *allShapes[j];
			bool colliding = isColliding(shape1, shape2);
			if (colliding == true) {
				//shape1.fillColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				//shape2.fillColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				Collision& coll = *allCollisions[allCollisions.size() - 1];
				glm::vec3 normal = coll.normal;
				calculateRebound(shape1, shape2, normal);
				
			}
		}
	}
}


void applyGravity() {

	for (int i = 0; i < allShapes.size(); i++)//apply gravity
	{
		Shapes& shape1 = *allShapes[i];
		if (shape1.mass != 0) {
			ApplyForce(shape1, glm::vec3(0.0f, -0.1f * shape1.mass, 0.0f));
		}

	}

}
	/////////////particles stuff goes here//////////














//////////boid stuff goes here//////////////













///////////////start uo/////////////////


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
	myCube.collision_type = AAcube;

	myCube.w_matrix =
		glm::translate(glm::vec3(-4.0f, 6.0f, 4.0f)) *
		glm::mat4(1.0f);
	myCube.mass = 1.0f;
	myCube.invMass = 1.0f;
	myCube.fillColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	allShapes.push_back(&myCube);


	myCube2.Load();
	myCube2.mass = 0.0f;
	myCube2.invMass = 0.0f;
	myCube2.w_matrix =
		glm::translate(glm::vec3(-4.0f, 2.0f, 4.0f)) *
		glm::scale(glm::vec3(1.0f, 1.0f, 1.0f)) *
		glm::mat4(1.0f);
	myCube2.collision_type = AAcube;
	myCube2.fillColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	allShapes.push_back(&myCube2);
	
	//

	mySphere.Load();
	mySphere.w_matrix = glm::translate(glm::vec3(-2.0f, 1.0f, -3.0f)) *
		glm::rotate(-t, glm::vec3(0.0f, 1.0f, 0.0f)) *
		glm::rotate(-t, glm::vec3(1.0f, 0.0f, 0.0f)) *
		glm::mat4(1.0f);
	allShapes.push_back(&mySphere);
	mySphere.fillColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

	myCylinder.Load();
	myCylinder.mass = 1.0f;
	myCylinder.collision_type = AAcube;
	myCylinder.w_matrix = glm::translate(glm::vec3(1.5f, 1.0f, 2.0f)) *
		glm::mat4(1.0f);
	allShapes.push_back(&myCylinder);
	myCylinder.fillColor = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
	myCylinder.lineColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	
	for (int x = 0; x < 5; x += 1) {
		for (int z = 0; z < 5; z += 1) {

			cubeArray[x][z].Load();
			cubeArray[x][z].collision_type = AAcube;
			//cubeArray[x][z].radius = 0.5f;
			cubeArray[x][z].mass = 1.0f;
			cubeArray[x][z].invMass = 1.0f;
			cubeArray[x][z].w_matrix =
				glm::translate(glm::vec3((2 + x * 1.2), 3 * z + 1, 4.0f + 7 * 1.2 )) *
				glm::mat4(1.0f);
			cubeArray[x][z].fillColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
			allShapes.push_back(&cubeArray[x][z]);
		}
	}


	arrowX.Load(); arrowY.Load(); arrowZ.Load();
	arrowX.fillColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); arrowX.lineColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	arrowY.fillColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f); arrowY.lineColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	arrowZ.fillColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); arrowZ.lineColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

	myFloor.Load();
	myFloor.w_matrix = 
		glm::translate(glm::vec3(0.0f, 0.0f, 0.0f)) *
		glm::scale(glm::vec3(1000.0f, 0.001f, 1000.0f)) *
		glm::mat4(1.0f);
	myFloor.collision_type = AAcube;
	myFloor.mass = 0.0f;
	myFloor.invMass = 0.0f;

	myFloor.fillColor = glm::vec4(130.0f / 255.0f, 96.0f / 255.0f, 61.0f / 255.0f, 1.0f);    // Sand Colour
	myFloor.lineColor = glm::vec4(130.0f / 255.0f, 96.0f / 255.0f, 61.0f / 255.0f, 1.0f);    // Sand again
	allShapes.push_back(&myFloor);

	myLine.Load();
	myLine.fillColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	myLine.lineColor = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	myLine.lineWidth = 5.0f;

	///calc starting min max 
	for (int i = 0; i < allShapes.size(); i++)
	{
		Shapes& shape1 = *allShapes[i];
		
		calculateMinMax(shape1);
		
	}

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
	applyGravity();
	checkCollisions(); // check collistion





	// Calculate frame time/period -- used for all (physics, animation, logic, etc).
	GLfloat currentTime = (GLfloat)glfwGetTime();    // retrieve timelapse
	deltaTime = currentTime - lastTime;                // Calculate delta time
	lastTime = currentTime;                            // Save for next frame calculations.

	// Do not forget your ( T * R * S ) http://www.opengl-tutorial.org/beginners-tutorials/tutorial-3-matrices/



	for (int i = 0; i < allShapes.size(); i++)
	{
		Shapes& shape1 = *allShapes[i];
		shape1.velocity = shape1.velocity - (shape1.velocity * 0.1f) / (1 / deltaTime);//friction
		shape1.w_matrix = shape1.w_matrix * glm::translate(shape1.velocity / (1 / deltaTime)) * glm::mat4(1.0f);//added time
		if (shape1.mass != 0) {
			shape1.w_matrix[3][0] += shape1.correction.x;
			shape1.w_matrix[3][1] += shape1.correction.y;
			shape1.w_matrix[3][2] += shape1.correction.z;
			//cout << shape1.correction.x << " " << shape1.correction.y << " " << shape1.correction.z << " \n";
			shape1.correction = glm::vec3(0.0f);
		}
		
	}

	
	for (int i = 0; i < allShapes.size(); i++)
	{
		Shapes& shape1 = *allShapes[i];
		shape1.mv_matrix = myGraphics.viewMatrix * shape1.w_matrix;
		shape1.proj_matrix = myGraphics.proj_matrix;

	}


	

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
	/*
	// Calculate floor position and resize
	myFloor.mv_matrix = myGraphics.viewMatrix *
		glm::translate(glm::vec3(0.0f, 0.0f, 0.0f)) *
		glm::scale(glm::vec3(1000.0f, 0.001f, 1000.0f)) *
		glm::mat4(1.0f);
	myFloor.proj_matrix = myGraphics.proj_matrix;
	*/

	// Calculate Line
	myLine.mv_matrix = myGraphics.viewMatrix *
		glm::translate(glm::vec3(1.0f, 0.5f, 2.0f)) *
		glm::mat4(1.0f);
	myLine.proj_matrix = myGraphics.proj_matrix;


	t += 0.01f; // increment movement variable


	if (glfwWindowShouldClose(myGraphics.window) == GL_TRUE) quit = true; // If quit by pressing x on window.

}

void renderScene() {
	// Clear viewport - start a new frame.
	myGraphics.ClearViewport();

	// Draw objects in screen
	for (int i = 0; i < allShapes.size(); i++)
	{
		Shapes& shape1 = *allShapes[i];
		shape1.Draw();
	}
	//myFloor.Draw();
	arrowX.Draw();
	arrowY.Draw();
	arrowZ.Draw();
	myLine.Draw();
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
		glm::vec3 force = glm::vec3(0.1f, 0.0f, 0.0f);
		ApplyForce(myCube, force);
	}
	if (keyStatus[GLFW_KEY_L] == true) {
		glm::vec3 force = glm::vec3(-0.1f, 0.0f, 0.0f);
		ApplyForce(myCube, force);
	}

	if (keyStatus[GLFW_KEY_I] == true) {
		glm::vec3 force = glm::vec3(0.0f, 0.0f, 0.1f);
		ApplyForce(myCube, force);

	}
	if (keyStatus[GLFW_KEY_K] == true) {
		glm::vec3 force = glm::vec3(0.0f, 0.0f, -0.1f);
		ApplyForce(myCube, force);
	}
	if (keyStatus[GLFW_KEY_O] == true) {
		glm::vec3 force = glm::vec3(0.0f, 0.1f, 0.0f);
		ApplyForce(myCube, force);

	}
	if (keyStatus[GLFW_KEY_U] == true) {
		glm::vec3 force = glm::vec3(0.0f, -0.1f, 0.0f);
		ApplyForce(myCube, force);

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

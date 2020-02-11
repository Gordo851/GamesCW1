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
#include <glm/gtx/string_cast.hpp>
#include "graphics.h"
#include "shapes.h"
#include <limits>
#include <cstdlib>
#include <ctime>
// MAIN FUNCTIONS
float getDistanceBetweenCenters(Shapes shape1, Shapes shape2);
float getX(Shapes shape);
float getY(Shapes shape);
float getZ(Shapes shape);
void startup();
void updateCamera();
void updateSceneElements();
void renderScene();
void physicsEffects();
void calculateRebound(Shapes& shape1, Shapes& shape2, glm::vec3 normal);
void newVertPositions(Shapes& shape);
float getXDistanceBetweenCenters(Shapes shape1, Shapes shape2);
float getYDistanceBetweenCenters(Shapes shape1, Shapes shape2);
float getZDistanceBetweenCenters(Shapes shape1, Shapes shape2);
bool isCollidingXYZ(float Distance, vector<GLfloat> shape1VertPos, vector<GLfloat> shape2VertPos, XYZ xyz);
float getSquareDistanceBetweenCenters(Shapes shape1, Shapes shape2);
void updateWMatrix(Shapes& shape1);
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
Cube  cubeArray[6][10];
Cube  cubeArray2[5][7];
Particle particle;
Particle particleArray[20];
vector<Shapes*> allShapes;
vector<Particle*> allParticles;
int counter = 0;
// Some global variable to do the animation.
float t = 0.001f;            // Global variable for animation


float cx = 2.0f;
float cy = 0.5f;
float cz = 0.0f;
glm::vec3 collisionPoint;

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
	std::cout << "here\n";

	float cubex = getX(myCube);
	float cubey = getY(myCube);
	float cubez = getZ(myCube);
	float spherex = getX(mySphere);
	float spherey = getY(mySphere);
	float spherez = getZ(mySphere);

	float cubesphere = getDistanceBetweenCenters(myCube, mySphere);
	std::cout << "\n" << cubesphere;
	float cubesphere1 = getDistanceBetweenCenters(myCube, myCylinder);
	std::cout << "\n" << cubesphere1;
	float cubesphere2 = getDistanceBetweenCenters(myCylinder, mySphere);
	std::cout << "\n" << cubesphere2;

	/*
	cout << "here2  \n";
	for (int i = 0; i < allShapes.size(); i++)
	{
		cout << allShapes[i].collision_type << "\n";
	}
	cout << "here3\n";
	cout << &allShapes[0].w_matrix[0][1] << "\n";
	cout << allShapes[1].w_matrix[0][1] << "\n";
	cout << allShapes[2].w_matrix[0][1] << "\n";
	cout << allShapes[3].w_matrix[0][1] << "\n";
	cout << "here4\n";
	cout << myCube.w_matrix[0][1] << "\n";
	cout << myCube2.w_matrix[0][1] << "\n";
	cout << mySphere.w_matrix[0][1] << "\n";
	cout << myCylinder.w_matrix[0][1] << "\n";
	*/
	
	return 0;
}

void ApplyForce(Shapes& shape, glm::vec3 force)
{
	float accelerationX = force[0] / shape.mass;
	float accelerationY = force[1] / shape.mass;
	float accelerationZ = force[2] / shape.mass;
	glm::vec3 Velocity = glm::vec3(accelerationX, accelerationY, accelerationZ);
	//cout << "\nvel speed" << Velocity[0] << " " << Velocity[1] << " " << Velocity[2];
	//shape.linearMovement = shape.linearMovement + Velocity;
	shape.velocity = shape.velocity + Velocity;
}





void calculateGroundRebound(Shapes& shape1) {
	float e = 0.5;
	glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 phat = (e + 1.0f) * shape1.mass * (shape1.velocity * normal) * normal;
	shape1.velocity = shape1.velocity + (((-phat) / shape1.mass));


	//std::cout << "ground rebound \n";
}

glm::vec3 calculateNormal(Shapes& shape1, Shapes& shape2) {
	glm::vec3 normal = glm::vec3(0.0f, 0.0f, 0.0f);

	//float radiusDiff = shape1.radius / shape2.radius;

	if (shape1.collision_type == sphere || shape2.collision_type == sphere) {
		//cout << "sphere\n";
		float dist = getDistanceBetweenCenters(shape1, shape2);
		float distX = getXDistanceBetweenCenters(shape1, shape2);
		float distY = getYDistanceBetweenCenters(shape1, shape2);
		float distZ = getZDistanceBetweenCenters(shape1, shape2);

		normal = glm::vec3(distX / dist, distY / dist, distZ / dist);
		//cout << normal[0] << " " << normal[1] << " " << normal[2] <<"sphere\n";
	}
	//else if (shape2.collision_type == cube && radiusDiff > 0.8 && radiusDiff < 1.2) {

	//	float dist = getDistanceBetweenCenters(shape1, shape2);
	//	float distX = getXDistanceBetweenCenters(shape1, shape2);
	//	float distY = getYDistanceBetweenCenters(shape1, shape2);
	//	float distZ = getZDistanceBetweenCenters(shape1, shape2);

	//	normal = glm::vec3(distX / dist, distY / dist, distZ / dist);
	//	cout << "same\n";
	//}
	else if (shape2.collision_type == cube) {
		//cout << "diff\n";
		vector<GLfloat> shape2VertPos = shape2.currentVertexPositions;
		vector<GLfloat> shape1VertPos = shape1.currentVertexPositions;


		float shape2PosMaxX = 1 - numeric_limits<float>::max();
		float shape2PosMinX = numeric_limits<float>::max();

		float shape2PosMaxY = 1 - numeric_limits<float>::max();
		float shape2PosMinY = numeric_limits<float>::max();

		float shape2PosMaxZ = 1 - numeric_limits<float>::max();
		float shape2PosMinZ = numeric_limits<float>::max();

		for (unsigned int i = 0; i < shape2VertPos.size(); i += 3) {
			if (shape2VertPos[i + 0] > shape2PosMaxX) {
				shape2PosMaxX = shape2VertPos[i + 0];
			}

			if (shape2VertPos[i + 0] < shape2PosMinX) {
				shape2PosMinX = shape2VertPos[i + 0];
			}

			if (shape2VertPos[i + 1] > shape2PosMaxY) {
				shape2PosMaxY = shape2VertPos[i + 1];
			}
			if (shape2VertPos[i + 1] < shape2PosMinY) {
				shape2PosMinY = shape2VertPos[i + 1];
			}

			if (shape2VertPos[i + 2] > shape2PosMaxZ) {
				shape2PosMaxZ = shape2VertPos[i + 2];
			}
			if (shape2VertPos[i + 2] < shape2PosMinZ) {
				shape2PosMinZ = shape2VertPos[i + 2];
			}
		}

		float maxDistX = abs((collisionPoint[0] - shape2PosMaxX));
		float minDistX = abs((collisionPoint[0] - shape2PosMinX));
		float maxDistY = abs((collisionPoint[1] - shape2PosMaxY));
		float minDistY = abs((collisionPoint[1] - shape2PosMinY));
		float maxDistZ = abs((collisionPoint[2] - shape2PosMaxZ));
		float minDistZ = abs((collisionPoint[2] - shape2PosMinZ));

		float minDist = 100.0f;
		if (maxDistX < minDistX) {
			normal = glm::vec3(0.0f, 0.0f, 0.0f);
			normal[0] = 1.0f;
			minDist = maxDistX;
		}
		else {
			normal = glm::vec3(0.0f, 0.0f, 0.0f);
			normal[0] = -1.0f;
			minDist = minDistX;
		}
		if (maxDistY < minDist || minDistY < minDist) {
			if (maxDistY < minDistY) {

				normal = glm::vec3(0.0f, 0.0f, 0.0f);
				normal[1] = 1.0f;
				minDist = maxDistY;
			}
			else {
				normal = glm::vec3(0.0f, 0.0f, 0.0f);
				normal[1] = -1.0f;
				minDist = minDistY;
			}
		}

		if (maxDistZ < minDist || minDistZ < minDist) {
			if (maxDistZ < minDistZ) {

				normal = glm::vec3(0.0f, 0.0f, 0.0f);
				normal[2] = 1.0f;
			}
			else {
				normal = glm::vec3(0.0f, 0.0f, 0.0f);
				normal[2] = -1.0f;
			}
		}

	}

	return normal;
}

void calculateRebound(Shapes& shape1, Shapes& shape2, glm::vec3 normal) {
	//calcualte side of collision

	
	

	float e = 0.5f;

	if (shape2.mass == 0) {//shape 2 static
		glm::vec3 phat = (e + 1.0f) * shape1.mass * (shape1.velocity * normal) * normal;
		shape1.velocity = shape1.velocity + (((-phat) / shape1.mass));
		//std::cout << "shape 2 static \n";
	}
	else if (shape1.mass == 0) {//shape 1 static
		glm::vec3 phat = (e + 1.0f) * shape2.mass * (shape2.velocity * normal) * normal;
		shape2.velocity = shape2.velocity + (((phat) / shape2.mass));
		//std::cout << "shape 1 static \n";
	}
	else {
		glm::vec3 phat = (((e + 1.0f) * (shape1.velocity * normal - shape2.velocity * normal)) / ((1.0f / shape2.mass) + (1.0f / shape1.mass))) * normal;

		shape1.velocity = shape1.velocity + (((-phat) / shape1.mass));
		shape2.velocity = shape2.velocity + (((phat) / shape2.mass));
		//shape1.velocity = shape1.velocity + 2.0f * ((-shape1.velocity) * normal);
	}

	/*
	glm::vec3 velocityDiff = shape1.velocity - shape2.velocity;
	if (shape1.mass != 0) {
		glm::vec3 leverArm = collisionPoint - glm::vec3(getX(shape1), getY(shape1), getZ(shape1));

		shape1.rotVelocity[0] = leverArm[2] * velocityDiff[1] * 0.05f;
		shape1.rotVelocity[0] = shape1.rotVelocity[0] +leverArm[1] * velocityDiff[2] ;

		shape1.rotVelocity[1] = leverArm[0] * velocityDiff[2]  * 0.05f;
		shape1.rotVelocity[1] = shape1.rotVelocity[1] +leverArm[2] * velocityDiff[0]  ;

		shape1.rotVelocity[2] = leverArm[1] * velocityDiff[0]  * 0.05f;
		shape1.rotVelocity[2] = shape1.rotVelocity[2] + leverArm[0] * velocityDiff[1]  ;

		//shape1.rotVelocity = leverArm * velocityDiff * shape1.mass  * 0.05f;
		cout << shape1.rotVelocity[0] << " " << shape1.rotVelocity[1] << " " << shape1.rotVelocity[2] << " 1\n";
	}

	if (shape2.mass != 0) {
		glm::vec3 leverArm = collisionPoint - glm::vec3(getX(shape2), getY(shape2), getZ(shape2));

		shape2.rotVelocity[0] = leverArm[2] * velocityDiff[1] * 0.05f;
		shape2.rotVelocity[0] = shape2.rotVelocity[0] + leverArm[1] * velocityDiff[2] ;

		shape2.rotVelocity[1] = leverArm[0] * velocityDiff[2] * 0.05f;
		shape2.rotVelocity[1] = shape2.rotVelocity[1] + leverArm[2] * velocityDiff[0] ;

		shape2.rotVelocity[2] = leverArm[1] * velocityDiff[0] * 0.05f;
		shape2.rotVelocity[2] = shape2.rotVelocity[2] + leverArm[0] * velocityDiff[1] ;

		//shape2.rotVelocity = leverArm * velocityDiff * shape2.mass * 0.05f;
		cout << shape2.rotVelocity[0] << " " << shape2.rotVelocity[1] << " " << shape2.rotVelocity[2] << " 2\n";
	}
	*/
}

void applyFriction(Shapes& shape1, Shapes& shape2, glm::vec3 normal) {

	glm::vec3 velocityDiff = shape1.velocity - shape2.velocity;

	if (shape1.mass != 0 && shape2.mass != 0) {
		shape1.velocity = shape1.velocity - 0.01f * velocityDiff;
		shape2.velocity = shape2.velocity + 0.01f * velocityDiff;
	}
	
	
	

	

	if (normal[1] > 0 && shape1.mass != 0 ) {
		float force = 0.1 * shape1.mass * normal[1];
		ApplyForce(shape1, glm::vec3(0.0f, force, 0.0f));
	}
	if (normal[1] < 0 && shape2.mass != 0 ) {
		float force = -0.1 * shape2.mass * normal[1];
		ApplyForce(shape2, glm::vec3(0.0f, force, 0.0f));
	}

	if (normal[0] > 0 && shape1.mass != 0) {
		float force = 0.1 * shape1.mass * normal[0];
		ApplyForce(shape1, glm::vec3(force,0.0f, 0.0f));
	}
	if (normal[0] < 0 && shape2.mass != 0) {
		float force = -0.1 * shape2.mass * normal[0];
		ApplyForce(shape2, glm::vec3(force,0.0f, 0.0f));
	}
	if (normal[2] > 0 && shape1.mass != 0) {
		float force = 0.1 * shape1.mass * normal[2];
		ApplyForce(shape1, glm::vec3(0.0f,  0.0f, force));
	}
	if (normal[2] < 0 && shape2.mass != 0) {
		float force = -0.1 * shape2.mass * normal[2];
		ApplyForce(shape2, glm::vec3(0.0f, 0.0f, force));
	}
}

/**
get the objects X position
*/
float getX(Shapes shape) {
	
	float x_location = shape.w_matrix[3][0];
	return x_location;
}
/**
get the objects Y position
*/
float getY(Shapes shape) {
	
	float y_location = shape.w_matrix[3][1];
	return y_location;
}
/**
get the objects Z position
*/
float getZ(Shapes shape) {
	
	float z_location = shape.w_matrix[3][2];
	return z_location;
}
/**
get the objects X position
*/
float getoldX(Shapes shape) {

	float x_location = shape.w_matrix_old[3][0];
	return x_location;
}
/**
get the objects Y position
*/
float getoldY(Shapes shape) {

	float y_location = shape.w_matrix_old[3][1];
	return y_location;
}
/**
get the objects Z position
*/
float getoldZ(Shapes shape) {

	float z_location = shape.w_matrix_old[3][2];
	return z_location;
}
/**
get the distance in the Y axis between the centeres of two objects
*/
float getYDistanceBetweenCenters(Shapes shape1, Shapes shape2) {
	
	float shape1y = getY(shape1);	
	float shape2y = getY(shape2);
	return shape1y - shape2y;


}

/**
get the distance in the X axis between the centers of two objects
*/
float getXDistanceBetweenCenters(Shapes shape1, Shapes shape2) {

	float shape1x = getX(shape1);
	float shape2x = getX(shape2);
	return shape1x - shape2x;
}

/**
get the distance in the Z axis between the centers of two objects
*/
float getZDistanceBetweenCenters(Shapes shape1, Shapes shape2) {
	
	float shape1z = getZ(shape1);
	float shape2z = getZ(shape2);
	return shape1z - shape2z;
}

float getOldYDistanceBetweenCenters(Shapes shape1, Shapes shape2) {

	float shape1y = getoldY(shape1);
	float shape2y = getoldY(shape2);
	return shape1y - shape2y;


}

/**
get the distance in the X axis between the centers of two objects
*/
float getOldXDistanceBetweenCenters(Shapes shape1, Shapes shape2) {

	float shape1x = getoldX(shape1);
	float shape2x = getoldX(shape2);
	return shape1x - shape2x;
}

/**
get the distance in the Z axis between the centers of two objects
*/
float getOldZDistanceBetweenCenters(Shapes shape1, Shapes shape2) {

	float shape1z = getoldZ(shape1);
	float shape2z = getoldZ(shape2);
	return shape1z - shape2z;
}

/**
get the shortest distance between the centers of two objects
*/
float getDistanceBetweenCenters(Shapes shape1, Shapes shape2) {
	float distancex = getXDistanceBetweenCenters(shape1, shape2);
	float distancey = getYDistanceBetweenCenters(shape1, shape2);
	float distancez = getZDistanceBetweenCenters(shape1, shape2);
	return sqrt((distancex * distancex) + (distancey * distancey) + (distancez * distancez));
}

float getOldSquareDistanceBetweenCenters(Shapes shape1, Shapes shape2) {
	float distancex = getOldXDistanceBetweenCenters(shape1, shape2);
	float distancey = getOldYDistanceBetweenCenters(shape1, shape2);
	float distancez = getOldZDistanceBetweenCenters(shape1, shape2);
	return (distancex * distancex) + (distancey * distancey) + (distancez * distancez);
}

float getSquareDistanceBetweenCenters(Shapes shape1, Shapes shape2) {
	float distancex = getXDistanceBetweenCenters(shape1, shape2);
	float distancey = getYDistanceBetweenCenters(shape1, shape2);
	float distancez = getZDistanceBetweenCenters(shape1, shape2);
	return (distancex * distancex) + (distancey * distancey) + (distancez * distancez);
}

/**
calculate an objects vertex positions in the world view
*/
void newVertPositions(Shapes& shape) {
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

}



/**
calculate of an object is colliding in the x, y , or z axis ( set xyz to x, y ,or z for the required axis)
*/
bool isCollidingXYZ(float Distance, vector<GLfloat> shape1VertPos, vector<GLfloat> shape2VertPos, XYZ xyz) {
	
	float shape1Pos;
	float shape2Pos;
	if (Distance < 0) {
		//max in x for shape 1 min in x for shape2
		shape1Pos = 1 - numeric_limits<float>::max();
		shape2Pos = numeric_limits<float>::max();
		for (unsigned int i = 0; i < shape1VertPos.size(); i += 3) {
			if (shape1VertPos[i + (int)xyz] > shape1Pos) {
				shape1Pos = shape1VertPos[i + (int)xyz];
			}
		}

		for (unsigned int i = 0; i < shape2VertPos.size(); i += 3) {
			if (shape2VertPos[i + (int)xyz] < shape2Pos) {
				shape2Pos = shape2VertPos[i + (int)xyz];
			}
		}

		if (shape1Pos >= shape2Pos) {
			collisionPoint[xyz] = (shape1Pos + shape2Pos) / 2;
			return true;
		}
	}
	else {
		//max in x for shape 2 min in x for shape1
		shape1Pos = numeric_limits<float>::max();
		shape2Pos = 1 - numeric_limits<float>::max();
		for (unsigned int i = 0; i < shape1VertPos.size(); i += 3) {
			if (shape1VertPos[i + (int)xyz] < shape1Pos) {
				shape1Pos = shape1VertPos[i + (int)xyz];
			}
		}

		for (unsigned int i = 0; i < shape2VertPos.size(); i += 3) {
			if (shape2VertPos[i + (int)xyz] > shape2Pos) {
				shape2Pos = shape2VertPos[i + (int)xyz];
			}
		}

		if (shape1Pos <= shape2Pos) {
			collisionPoint[xyz] = (shape1Pos + shape2Pos) / 2;
			return true;
		}
	}

	return false;
}



/**
check if sphere and cube are colliding - called from iscolliding()
*/
bool cubeSphereCollision(Shapes sphere1, Shapes cube1) {
	
	vector<GLfloat> shape2VertPos = cube1.currentVertexPositions;
	vector<GLfloat> shape1VertPos;
	shape1VertPos.push_back(getX(sphere1));
	shape1VertPos.push_back(getY(sphere1));
	shape1VertPos.push_back(getZ(sphere1));
	float radius = sphere1.radius;///////////////
	
	//cout << shape1VertPos[0] << " " << shape1VertPos[1] << " " << shape1VertPos[2] << "\n";
	float distX = getXDistanceBetweenCenters(sphere1, cube1);
	float distY = getYDistanceBetweenCenters(sphere1, cube1);
	float distZ = getZDistanceBetweenCenters(sphere1, cube1);
	
	float alpha = atan2(abs(distX), abs(distY));
	
	float X = radius * sin(alpha);
	float Y = radius * cos(alpha);

	float alpha2 = atan2(abs(distX), abs(distZ));
	float Z = radius * cos(alpha2);
	if (distX < 0) {
		//max in x for shape 1 min in x for shape2
		shape1VertPos[0] += X;
	}
	else {
		shape1VertPos[0] -= X;
	}
	if (distY < 0) {
		//max in x for shape 1 min in x for shape2
		shape1VertPos[1] += Y;
	}
	else {
		shape1VertPos[1] -= Y;
	}
	if (distZ < 0) {
		//max in x for shape 1 min in x for shape2
		shape1VertPos[2] += Z;
	}
	else {
		shape1VertPos[2] -= Z;
	}

	//cout << shape1VertPos[0] << " " << shape1VertPos[1] << " " << shape1VertPos[2] << "\n \n";

	if (isCollidingXYZ(distX, shape1VertPos, shape2VertPos, x) == true) {

		if (isCollidingXYZ(distY, shape1VertPos, shape2VertPos, y) == true) {

			if (isCollidingXYZ(distZ, shape1VertPos, shape2VertPos, z) == true) {
				return true;
			}
		}

		

	}

	return false;
	

}

//check if two objects are colliding
bool isColliding(Shapes shape1, Shapes shape2) 
{
	
	if (shape1.collision_type == none || shape2.collision_type == none) {
		return false;
	}

	if ((((shape1.radius + shape2.radius) * 1.5f)* ((shape1.radius + shape2.radius) * 1.5f)) < (getSquareDistanceBetweenCenters(shape1, shape2))) {//stop complicated calcs if not near object
		return false;
	}


	if (shape1.collision_type == sphere && shape2.collision_type == sphere) {
		float distBetween = getSquareDistanceBetweenCenters(shape1, shape2);
		float radius = shape1.radius + shape2.radius;
		radius = radius * radius;
		if (distBetween < radius)
		{
			return true;
		}
		else {
			return false;
		}
	}
	else if (shape1.collision_type == cube && shape2.collision_type == cube) {

		
		
		vector<GLfloat> shape1VertPos = shape1.currentVertexPositions;
		vector<GLfloat> shape2VertPos = shape2.currentVertexPositions;

		float Dist = getXDistanceBetweenCenters(shape1, shape2);
		
		if (isCollidingXYZ(Dist, shape1VertPos, shape2VertPos, x) == true) {
		
			Dist = getYDistanceBetweenCenters(shape1, shape2);
			if (isCollidingXYZ(Dist, shape1VertPos, shape2VertPos, y) == true) {
				Dist = getZDistanceBetweenCenters(shape1, shape2);
				if (isCollidingXYZ(Dist, shape1VertPos, shape2VertPos, z) == true) {
					return true;
				}
			}
		}
		return false;
	}
	else if (shape1.collision_type == sphere && shape2.collision_type == cube) {
		return cubeSphereCollision(shape1, shape2);

	}
	else if (shape2.collision_type == sphere && shape1.collision_type == cube) {
		return cubeSphereCollision(shape2, shape1);

	}
	return false;

}

bool isGroundColliding(Shapes shape1) {
	vector<GLfloat> shape1VertPos = shape1.currentVertexPositions;
	float shape1Pos = numeric_limits<float>::max();
	for (unsigned int i = 0; i < shape1VertPos.size(); i += 3) {//calc lowest point in object
		if (shape1VertPos[i + 1] < shape1Pos) {
			shape1Pos = shape1VertPos[i + 1];
		}
	}

	if (shape1Pos <= 0.0f) {
		return true;
	}
	return false;
	
}

void checkCollisions()
{
	for (int i = 0; i < allShapes.size(); i++)
	{
		Shapes& shape1 = *allShapes[i];
		
		//shape1.fillColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
		if (shape1.mass != 0) {
			newVertPositions(shape1);//calcaute here to prevent repeated calculation
			
			bool hitground = isGroundColliding(shape1);//is colliding with ground

			if (hitground == true) {
				//std::cout << "ground hit \n";
				if (shape1.onGround == false) {
					calculateGroundRebound(shape1);
				}
				//calculateGroundRebound(shape1);
				shape1.onGround = true;
				//simulate friction on ground
				shape1.velocity[0] = shape1.velocity[0] * 0.98f;
				shape1.velocity[2] = shape1.velocity[2] * 0.98f;

			}
			else {

				shape1.onGround = false;
				//ApplyForce(shape1, glm::vec3(0.0f, -0.1f * shape1.mass, 0.0f));
				
			}
			
		}
	}
	
	//Itteration for allShapes
	for (int i = 0; i < allShapes.size(); i++)
	{
		for (int j = i +1 ; j < allShapes.size(); j++)
		{
			if (i != j)
			{
				Shapes& shape1 = *allShapes[i];
				Shapes& shape2 = *allShapes[j];
				if (shape1.mass == 0 && shape2.mass == 0) {//both object static do not need to calculate

				}
				else {
					bool colliding = isColliding(shape1, shape2);
					if (colliding == true)
					{

						//cout << collisionPoint[0] << " " << collisionPoint[1] << " " << collisionPoint[2] << ":collision point \n";
						//shape1.fillColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
						//shape2.fillColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
						glm::vec3 normal = calculateNormal(shape1, shape2);
						
						if (getSquareDistanceBetweenCenters(shape1, shape2) <= getOldSquareDistanceBetweenCenters(shape1, shape2)|| shape1.lastCollided != j || shape2.lastCollided != i) {//only calcuate rebound if objects moving appart
							
							calculateRebound(shape1, shape2, normal);
							
							
						}

						
						shape1.lastCollided = j;
						shape2.lastCollided = i;
						applyFriction(shape1, shape2, normal);
						
					}
					else {
						if (shape1.lastCollided == j) {
							shape1.lastCollided = -1;
						}
						if (shape2.lastCollided == i) {
							shape2.lastCollided = -1;
						}
					}

				}
				
			}
		}
	}
	for (int i = 0; i < allShapes.size(); i++)//apply gravity
	{
		Shapes& shape1 = *allShapes[i];
		if (shape1.onGround == false  && shape1.mass != 0  ) {
			ApplyForce(shape1, glm::vec3(0.0f, -0.1f * shape1.mass, 0.0f));
		}

	}
	
}

void CreateParticles() {
	for (int x = 0; x < size(particleArray); x += 1) {
		particleArray[x].Load();
		particleArray[x].collision_type = sphere;
		particleArray[x].w_matrix =
			glm::translate(glm::vec3(0.1f*x, 1.0f, 1.0f)) *
			glm::mat4(1.0f);
		particleArray[x].mass = 0.5f;
		particleArray[x].fillColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		allShapes.push_back(&particleArray[x]);
		allParticles.push_back(&particleArray[x]);	
	}	
}
void ResetParticles() {
	for (int x = 0; x < size(particleArray); x += 1) {
		particleArray[x].w_matrix =
			glm::translate(glm::vec3(0.1f * x, 1.0f, 1.0f)) *
			glm::mat4(1.0f);
		particleArray[x].fillColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		particleArray[x].DeathCount = 100;
	}
}

void DestoryParticles(Particle& shape1) {
	shape1.lineColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
	shape1.fillColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
	//shape1.~Particle();
}
void MoveParticles() {
	for (int i = 0; i < allParticles.size(); i++)
	{
		float MAX = 1.0f;
		Particle& shape1 = *allParticles[i];
		float x = -0.5f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / MAX));
		float y = -0.5f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / MAX));
		float z = -0.5f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / MAX));
		glm::vec3 force = glm::vec3(x, y, z);
		ApplyForce(shape1, force);
	}
}
void CountParticlesToDeath() {
	MoveParticles();
	for (int i = 0; i < allParticles.size(); i++)
	{
		Particle& shape1 = *allParticles[i];
		//cout << shape1.DeathCount;
		shape1.DeathCount -= 1;
		if (shape1.DeathCount == 0)
		{
			DestoryParticles(*allParticles[i]);
		}

	}
	//cout << "\n";
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

	CreateParticles();

	myCube.Load();
    myCube.collision_type = cube;

	myCube.w_matrix = 
		glm::translate(glm::vec3(-4.0f, 5.0f, 4.0f)) *
		glm::mat4(1.0f);
	myCube.mass = 1;
	myCube.fillColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	allShapes.push_back(&myCube);


	myCube2.Load();
	myCube2.mass = 0.0f;
	myCube2.w_matrix =
		glm::translate(glm::vec3(-4.0f, 1.0f,4.0f)) *
		glm::scale(glm::vec3(1.0f, 1.0f, 1.0f)) *
		glm::mat4(1.0f);
	myCube2.collision_type = cube;
	myCube2.fillColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	allShapes.push_back(&myCube2);
	myCube2.radius = myCube2.radius * 2;
	
	
	for (int x = 0; x < 5; x +=1) {
		for (int z = 0; z < 2; z +=1) {
			
			cubeArray[x][z].Load();
			cubeArray[x][z].collision_type = cube;
			cubeArray[x][z].mass = 1.0f;
			cubeArray[x][z].w_matrix =
				glm::translate(glm::vec3((2 + x * 1.2), 3.0f + z*4, 4.0f + z * 0.5f )) *
				glm::mat4(1.0f);
			cubeArray[x][z].fillColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
			allShapes.push_back(&cubeArray[x][z]);
		}
	}
	
	


	
	mySphere.Load();
	mySphere.w_matrix = glm::translate(glm::vec3(-2.0f, 5.0f, -3.0f)) *
		glm::rotate(-t, glm::vec3(0.0f, 1.0f, 0.0f)) *
		glm::rotate(-t, glm::vec3(1.0f, 0.0f, 0.0f)) *
		glm::mat4(1.0f);
	allShapes.push_back(&mySphere);
	mySphere.fillColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);    // You can change the shape fill colour, line colour or linewidth

	arrowX.Load(); arrowY.Load(); arrowZ.Load();
	arrowX.fillColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); arrowX.lineColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	arrowY.fillColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f); arrowY.lineColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	arrowZ.fillColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); arrowZ.lineColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

	

	myCylinder.Load();
	myCylinder.mass = 1.0f;
	myCylinder.collision_type = cube;
	myCylinder.w_matrix = glm::translate(glm::vec3(1.5f, 5.0f, 2.0f)) *
		glm::mat4(1.0f);
	
	allShapes.push_back(&myCylinder);
	myCylinder.fillColor = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
	myCylinder.lineColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	myLine.Load();
	myLine.fillColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	myLine.lineColor = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	myLine.lineWidth = 5.0f;

	myFloor.Load();
	myFloor.mass = 0.0f;
	myFloor.collision_type = cube;
	myFloor.w_matrix = glm::translate(glm::vec3(0.0f, -0.02f, 0.0f)) *
		glm::scale(glm::vec3(1000.0f, 0.001f, 1000.0f)) *
		glm::mat4(1.0f);
	myFloor.fillColor = glm::vec4(130.0f / 255.0f, 96.0f / 255.0f, 61.0f / 255.0f, 1.0f);    // Sand Colour
	myFloor.lineColor = glm::vec4(130.0f / 255.0f, 96.0f / 255.0f, 61.0f / 255.0f, 1.0f);    // Sand again
	


	///calculate starting vertex positions - mostly for static objects
	for (int i = 0; i < allShapes.size(); i++)
	{
		Shapes& shape1 = *allShapes[i];
		newVertPositions(shape1);//calcaute here to prevent repeated calculation
		
	}


	// Optimised Graphics
	myGraphics.SetOptimisations();        // Cull and depth testing
	srand(static_cast <unsigned> (time(0)));
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
void updateWMatrix(Shapes& shape1) {
	shape1.rotVelocity = shape1.rotVelocity * 0.9f;
	shape1.velocity = shape1.velocity * 0.999f;

	if (shape1.onGround == true && shape1.velocity[1] < 0.0f) {
		shape1.velocity[1] = 0.0f;
	}

	shape1.rotPosition = shape1.rotPosition + (shape1.rotVelocity / (1 / deltaTime));

	shape1.w_matrix_old = shape1.w_matrix;

	//shape1.w_matrix = glm::translate(glm::vec3(getX(shape1), getY(shape1), getZ(shape1)))  * glm::translate(shape1.velocity / (1 / deltaTime)) * glm::rotate(shape1.rotPosition[1], glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(shape1.rotPosition[0], glm::vec3(1.0f, 0.0f, 0.0f)) * glm::rotate(shape1.rotPosition[2], glm::vec3(0.0f, 0.0f, 1.0f)) * glm::mat4(1.0f);//added time
	shape1.w_matrix = shape1.w_matrix * glm::translate(shape1.velocity / (1 / deltaTime)) * glm::mat4(1.0f);//added time
	//shape1.w_matrix = shape1.w_matrix * glm::translate(shape1.velocity / (1 / deltaTime)) * glm::rotate(shape1.rotPosition[1], glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(shape1.rotPosition[0], glm::vec3(1.0f, 0.0f, 0.0f)) * glm::rotate(shape1.rotPosition[2], glm::vec3(0.0f, 0.0f, 1.0f)) * glm::mat4(1.0f);//added time

}


void updateSceneElements() {

	glfwPollEvents();                                // poll callbacks
	checkCollisions();
	// Calculate frame time/period -- used for all (physics, animation, logic, etc).
	GLfloat currentTime = (GLfloat)glfwGetTime();    // retrieve timelapse
	deltaTime = currentTime - lastTime;                // Calculate delta time
	lastTime = currentTime;                            // Save for next frame calculations.

	// Do not forget your ( T * R * S ) http://www.opengl-tutorial.org/beginners-tutorials/tutorial-3-matrices/
	// Calculate Cube position
	counter += 1;
	if (counter == 60)
	{
		ResetParticles();
	}

	for (int i = 0; i < allShapes.size(); i++)
	{
		Shapes& shape1 = *allShapes[i];
		updateWMatrix(shape1);
		
		shape1.mv_matrix = myGraphics.viewMatrix * shape1.w_matrix;
		shape1.proj_matrix = myGraphics.proj_matrix;

	}
	/*
	myCube.w_matrix = myCube.w_matrix * glm::translate(myCube.velocity/(1/deltaTime))* glm::mat4(1.0f);//added time
	//std::cout << "vel" << myCube.velocity[0] << " " << myCube.velocity[1] << " " << myCube.velocity[2] << "\n";
	myCube.mv_matrix = myGraphics.viewMatrix * myCube.w_matrix;
	myCube.proj_matrix = myGraphics.proj_matrix;

	// Calculate cylinder 
	myCylinder.w_matrix = myCylinder.w_matrix * glm::translate(myCylinder.velocity / (1 / deltaTime)) * glm::mat4(1.0f);
	myCylinder.mv_matrix = myGraphics.viewMatrix * myCylinder.w_matrix;

	myCylinder.proj_matrix = myGraphics.proj_matrix;

	myCube2.w_matrix =
		glm::translate(glm::vec3(-2.0f, 2.5f, 1.5f)) * 
		glm::rotate(glm::radians(-40.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
		glm::mat4(1.0f);
	myCube2.mv_matrix = myGraphics.viewMatrix * myCube2.w_matrix;
	myCube2.proj_matrix = myGraphics.proj_matrix;
	*/
	// calculate Sphere movement
	mySphere.w_matrix = glm::translate(glm::vec3(-2.0f, 0.5f, -1.5f)) *
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
	
	myFloor.mv_matrix = myGraphics.viewMatrix * myFloor.w_matrix;
		
	myFloor.proj_matrix = myGraphics.proj_matrix;

	

	// Calculate Line
	myLine.mv_matrix = myGraphics.viewMatrix *
		glm::translate(glm::vec3(1.0f, 0.5f, 2.0f)) *
		glm::mat4(1.0f);
	myLine.proj_matrix = myGraphics.proj_matrix;


	t += 0.01f; // increment movement variable
	//physicsEffects();
	
	CountParticlesToDeath();

	if (glfwWindowShouldClose(myGraphics.window) == GL_TRUE) quit = true; // If quit by pressing x on window.

	



}




void renderScene() {
	// Clear viewport - start a new frame.
	myGraphics.ClearViewport();
	// Draw objects in screen
	myFloor.Draw();


	for (int i = 0; i < allShapes.size(); i++)
	{
		Shapes& shape1 = *allShapes[i];
		shape1.Draw();

	}
	
	//myCube.Draw();
	//myCube2.Draw();
	//mySphere.Draw();

	arrowX.Draw();
	arrowY.Draw();
	arrowZ.Draw();

	myLine.Draw();
	//myCylinder.Draw();
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
		cy += 0.05;
	}
	if (keyStatus[GLFW_KEY_K] == true) {
		glm::vec3 force = glm::vec3(0.0f, 0.0f, -0.1f);
		ApplyForce(myCube, force);
	}
	if (keyStatus[GLFW_KEY_O] == true) {
		glm::vec3 force = glm::vec3(0.0f, 0.1f, 0.0f);
		ApplyForce(myCube, force);
		cz += 0.05;
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

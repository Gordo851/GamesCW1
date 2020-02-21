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
#include "Search.h"
#include "boidFlock.h"
#include "Windows.h"

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
bool getAAcubeSphereNormal2(Shapes& cube, Shapes& sphere);


void searchGraph();


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
//Cube cubeArray[10][10];

Particle myPlayer;
Cube myTarget;
float playerX = 1.0;
float playerY = 1.0;
float targetX = 17.0;
float targetY = 11.0;
//////////////////////////////////////
vector<Shapes*> allShapes;
vector<Collision*> allCollisions;

Cube  cubeArray[10][10];

boidFlock flock;
Boid boidArray[50];

boidFlock flock2;
Boid boidArray2[50];


vector<Particle*> allParticles;
Particle particleArray[20];
Particle particle;

// Some global variable to do the animation.
float t = 0.001f;            // Global variable for animation

//Search Variables
int mapArray[20][20];
Cube  mapCubeArray[20][20];
Cube  showSearchCubeArray[20][20];
Node node[400];
vector<Node*> allNodes;
vector<glm::vec2> path;
vector<glm::vec2> availableIndexes;

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
////////////////////////////////////////////A* Search
float CalculateHeuristic(float X, float Y)
{
	Shapes imaginaryPlace;
	imaginaryPlace.w_matrix =
		glm::translate(glm::vec3(X, 0.5f, Y)) *
		glm::mat4(1.0f);
	float heuristic = sqrt(getSquareDistance(imaginaryPlace, myTarget));//square root for now
	return heuristic;
}
void DrawMapOnScreen()
{

	for (int x = 0; x < 20; x += 1) {
		for (int z = 0; z < 20; z += 1) {
			if (mapArray[x][z] == 1)
			{
				mapCubeArray[x][z].Load();
				mapCubeArray[x][z].collision_type = AAcube;
				mapCubeArray[x][z].mass = 0.0f;
				mapCubeArray[x][z].invMass = 0.0f;
				
				mapCubeArray[x][z].w_matrix =
					glm::translate(glm::vec3(x, 0.5, z)) *
					glm::mat4(1.0f);
				mapCubeArray[x][z].fillColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
				allShapes.push_back(&mapCubeArray[x][z]);

			}
			else {
				mapCubeArray[x][z].collision_type = none;
			}
		}
	}
	
	for (int x = 0; x < 20; x += 1) {
		for (int z = 0; z < 20; z += 1) {
			{
				showSearchCubeArray[x][z].Load();
				showSearchCubeArray[x][z].mass = 0.0f;
				showSearchCubeArray[x][z].collision_type = none;
				showSearchCubeArray[x][z].w_matrix =
					glm::translate(glm::vec3(0.0f + x, 0.01f, 0.0f + z)) *
					glm::mat4(1.0f);
				showSearchCubeArray[x][z].fillColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				showSearchCubeArray[x][z].lineColor = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
				allShapes.push_back(&showSearchCubeArray[x][z]);
			}
		}
	}
	
}
void CreateMap()
{
	//randomly place obstacles
	float MAX = 1.0f;
	for (int x = 0; x < 20; x ++)
	{
		for (int y = 0; y < 20; y++)
		{
			float random = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / MAX));
			if (random > 0.7f)
			{
				mapArray[x][y] = 1;
			}
			else
			{
				mapArray[x][y] = 0;
			}
		}
	}
	//build sides
	for (int x = 0; x < 20; x++)
	{
		for (int y = 0; y < 20; y += 19)
		{
			mapArray[x][y] = 1;
		}
	}
	for (int x = 0; x < 20; x += 19)
	{
		for (int y = 0; y < 20; y++)
		{
			mapArray[x][y] = 1;
		}
	}
}
void movePlayer(float x, float y)
{
	playerX = x;
	playerY = y;
	myPlayer.possition = glm::vec3(playerX, 0.5f, playerY);
	myPlayer.velocity = (glm::vec3(0.0f, 0.0f, 0.0f));
	cout << playerX << " " << playerY << "\n";
}
void createSearchGraph()
{
	int index = 0;
	for (int x = 0; x < 20; x++)
	{
		for (int y = 0; y < 20; y ++)
		{
			//create node at x,y if that area is empty (and therefore passable by agent)
			if (mapArray[x][y] == 0)
			{
				node[index].index = glm::vec2(x,y);
				node[index].HeuristicCost = CalculateHeuristic(x, y);
				allNodes.push_back(&node[index]);
				//search all nodes within a radius of one of our node. If they are empty create a path between this node and that node. 
				if (0 < x < 20 and 0 < y < 20)
				{
					if (mapArray[x][y + 1] == 0)
					{
						node[index].travelUp = true;
					}
					if (mapArray[x][y - 1] == 0)
					{
						node[index].travelDown = true;
					}
					if (mapArray[x + 1][y] == 0)
					{
						node[index].travelLeft = true;
					}
					if (mapArray[x - 1][y] == 0)
					{
						node[index].travelRight = true;
					}
				}
				index += 1;
			}
		}
	}
	searchGraph();
}
Node findNodesByIndex(glm::vec2 index)
{
	for (int k = 0; k < allNodes.size(); k++)
	{
		Node testThisNode = *allNodes[k];
		if (testThisNode.index == index)
		{
			return testThisNode;
		}
	}
}

void showSearch()
{
	for (int i = 0; i < availableIndexes.size(); i++)
	{
		int x = availableIndexes[i][0];
		int y = availableIndexes[i][1];
		showSearchCubeArray[x][y].fillColor = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
	}
	// Update the camera transform based on interactive inputs or by following a predifined path.
	updateCamera();

	// Update position, orientations and any other relevant visual state of any dynamic elements in the scene.
	updateSceneElements();

	// Render a still frame into an off-screen frame buffer known as the backbuffer.
	renderScene();
	// Swap the back buffer with the front buffer, making the most recently rendered image visible on-screen.
	glfwSwapBuffers(myGraphics.window);        // swap buffers (avoid flickering and tearing
	Sleep(100);

}
void showChosenNode(Node nodeChosen)
{
	glm::vec2 position = nodeChosen.index;
	int x = position[0];
	int y = position[1];
	showSearchCubeArray[x][y].fillColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	
}
void searchGraph()
{
	Node currentNode = findNodesByIndex(glm::vec2(1, 1));
	Node targetNode = findNodesByIndex(glm::vec2(17, 11));
	vector<glm::vec2> usedIndexes;
	usedIndexes.push_back(glm::vec2(1,1));
	float totalCost = 10000.0f;
	float pathCost = 0.0f;
	float indexToBeDeleted;
	glm::vec2 indexToAddToUsed;
	int x = 0;
	while (currentNode.index != targetNode.index)
	{
		if (currentNode.travelUp == true)
		{
			bool add = true;
			glm::vec2 possibleNodeIndex = currentNode.index + glm::vec2(0, 1);
			for (int ind = 0; ind < usedIndexes.size(); ind++)
			{
				if (usedIndexes[ind] == possibleNodeIndex)
				{
					add = false;
				}
			}
			for (int ind = 0; ind < availableIndexes.size(); ind++)
			{
				if (availableIndexes[ind] == possibleNodeIndex)
				{
					add = false;
				}
			}
			if (add == true)
			{
				availableIndexes.push_back(possibleNodeIndex);
				for (int t = 0; t < allNodes.size(); t++)
				{
					Node& child = *allNodes[t];
					if (child.index == possibleNodeIndex)
					{
						child.parent = currentNode.index;
					}
				}
			}
		}
		if (currentNode.travelDown == true)
		{
			bool add = true;
			glm::vec2 possibleNodeIndex = currentNode.index + glm::vec2(0, -1);
			for (int ind = 0; ind < usedIndexes.size(); ind++)
			{
				if (usedIndexes[ind] == possibleNodeIndex)
				{
					add = false;
				}
			}
			if (add == true)
			{
				availableIndexes.push_back(possibleNodeIndex);
				for (int t = 0; t < allNodes.size(); t++)
				{
					Node& child = *allNodes[t];
					if (child.index == possibleNodeIndex)
					{
						child.parent = currentNode.index;
					}
				}
			}
		}
		if (currentNode.travelLeft == true)
		{
			bool add = true;
			glm::vec2 possibleNodeIndex = currentNode.index + glm::vec2(1, 0);
			for (int ind = 0; ind < usedIndexes.size(); ind++)
			{
				if (usedIndexes[ind] == possibleNodeIndex)
				{
					add = false;
				}
			}
			if (add == true)
			{
				availableIndexes.push_back(possibleNodeIndex);
				for (int t = 0; t < allNodes.size(); t++)
				{
					Node& child = *allNodes[t];
					if (child.index == possibleNodeIndex)
					{
						child.parent = currentNode.index;
					}
				}
			}
		}
		if (currentNode.travelRight == true)
		{
			bool add = true;
			glm::vec2 possibleNodeIndex = currentNode.index + glm::vec2(-1, 0);
			for (int ind = 0; ind < usedIndexes.size(); ind++)
			{
				if (usedIndexes[ind] == possibleNodeIndex)
				{
					add = false;
				}
			}
			if (add == true)
			{
				availableIndexes.push_back(possibleNodeIndex);
				for (int t = 0; t < allNodes.size(); t++)
				{
					Node& child = *allNodes[t];
					if (child.index == possibleNodeIndex)
					{
						child.parent = currentNode.index;
					}
				}
			}
		}

		totalCost = 1000.0f;
		showSearch();
		for (int i = 0; i < availableIndexes.size(); i++)
		{
			glm::vec2 indexHere = availableIndexes[i];
			Node possibleNext = findNodesByIndex(indexHere);
			Node childNode = possibleNext;
			while (childNode.index != glm::vec2(1, 1))
			{
				path.insert(path.begin(), childNode.index);
				for (int i = 0; i < allNodes.size(); i++)
				{
					Node& myParent = *allNodes[i];
					if (myParent.index == childNode.parent)
					{
						childNode = myParent;
					}
				}
			}
			pathCost = path.size();
			cout << pathCost << "\n";
			path.clear();
			float costOfPossible = possibleNext.HeuristicCost + pathCost;
			if (costOfPossible < totalCost)
			{
				totalCost = costOfPossible;
				currentNode = possibleNext;
				indexToAddToUsed = indexHere;
				indexToBeDeleted = i;
			}

		}


		availableIndexes.erase(availableIndexes.begin() + indexToBeDeleted);
		usedIndexes.push_back(indexToAddToUsed);
		showChosenNode(currentNode);
		x++;
	
	}
	//creating path for agent
	while (currentNode.index != glm::vec2(1, 1))
	{
		path.insert(path.begin(), currentNode.index);
		for (int i = 0; i < allNodes.size(); i++)
		{
			Node& myParent = *allNodes[i];
			if (myParent.index == currentNode.parent)
			{
				currentNode = myParent;
			}
		}
	}
	path.insert(path.begin(), currentNode.index);
	cout << "Search complete";
}
void moveAlongPath()
{
	for (int i = 0; i < path.size(); i++)
	{
		int x = path[i][0];
		int y = path[i][1];
		movePlayer(x, y);

		// Update the camera transform based on interactive inputs or by following a predifined path.
		updateCamera();

		// Update position, orientations and any other relevant visual state of any dynamic elements in the scene.
		updateSceneElements();

		// Render a still frame into an off-screen frame buffer known as the backbuffer.
		renderScene();
		// Swap the back buffer with the front buffer, making the most recently rendered image visible on-screen.
		glfwSwapBuffers(myGraphics.window);        // swap buffers (avoid flickering and tearing)

		Sleep(100);
	}
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

	float e = min(a.e, b.e);

	if (b.mass == 0) {//shape 2 static
		glm::vec3 phat = (e + 1.0f) * a.mass * (a.velocity * normal) * normal;
		a.velocity = a.velocity + (((-phat) / a.mass));
		//std::cout << "shape 2 static \n";
	}
	else if (a.mass == 0) {//shape 1 static
		glm::vec3 phat = (e + 1.0f) * b.mass * (b.velocity * normal) * normal;
		b.velocity = b.velocity - (((phat) / b.mass));
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

void PositionalCorrection(Shapes& a, Shapes& b, float penetration, glm::vec3 normal)
{


	float percent = 0.8f; // usually 20% to 80%
	float slop = 0.01f; // usually 0.01 to 0.1

	glm::vec3 correction = (max((penetration - slop), 0.0f) / (a.invMass + b.invMass)) * percent * normal;

	a.correction -= a.invMass * correction;
	b.correction += b.invMass * correction;

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

	if (shape.collision_type == AAcube) {

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


}



bool isColliding(Shapes& shape1, Shapes& shape2) {

	if (shape1.mass == 0 && shape2.mass == 0) {
		return false;
	}

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
		float radius = shape1.radius + shape2.radius;
		float radius2 = radius * radius;
		float diss = getSquareDistance(shape1, shape2);
		if (diss < radius2) {
			diss = sqrt(diss);
			if (diss != 0)
			{
				Collision coll;

				// Distance is difference between radius and distanc
				coll.penetration = radius - diss;
				coll.normal = getXYZdistance(shape2, shape1) / diss;
				allCollisions.push_back(&coll);
				PositionalCorrection(shape1, shape2, coll.penetration, coll.normal);
				return true;
			}



			return true;
		}
		return false;
	}
	else if ((shape1.collision_type == AAcube && shape2.collision_type == sphere)) {//////////////////temp code

		return getAAcubeSphereNormal2(shape1, shape2);
	}
	else if (shape1.collision_type == sphere && shape2.collision_type == AAcube) {
		return getAAcubeSphereNormal(shape1, shape2);
	}

}


bool getAAcubeSphereNormal2(Shapes& cube, Shapes& sphere) {//shape 1 sphere




	glm::vec3 spherePos = getposition(sphere);
	float radius = sphere.radius;

	float x = max(cube.min.x, min(spherePos.x, cube.max.x));
	float y = max(cube.min.y, min(spherePos.y, cube.max.y));
	float z = max(cube.min.z, min(spherePos.z, cube.max.z));


	float diss = (x - spherePos.x) * (x - spherePos.x) + (y - spherePos.y) * (y - spherePos.y) + (z - spherePos.z) * (z - spherePos.z);
	if (diss - radius * radius < 0) {//colliding

		glm::vec3 overlap = glm::vec3(0.0f);
		Collision coll;
		glm::vec3 xyzDiss = getXYZdistance(cube, sphere);
		overlap = ((((spherePos + radius) - (spherePos - radius)) / 2.0f) + ((cube.max - cube.min) / 2.0f)) - abs(xyzDiss);



		if (overlap.x < overlap.y) {
			if (overlap.x < overlap.z) {
				if (xyzDiss.x < 0) {
					coll.normal = glm::vec3(-1.0, 0.0, 0.0);
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
					coll.normal = glm::vec3(0.0, -1.0, 0.0);
				}
				else {
					coll.normal = glm::vec3(0.0, 1.0, 0.0);
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
		PositionalCorrection(sphere, cube, coll.penetration, coll.normal);

		return true;
	}
	return false;
}

bool getAAcubeSphereNormal(Shapes& sphere, Shapes& cube) {//shape 1 sphere

	glm::vec3 spherePos = getposition(sphere);
	float radius = sphere.radius;
	float x = max(cube.min.x, min(spherePos.x, cube.max.x));
	float y = max(cube.min.y, min(spherePos.y, cube.max.y));
	float z = max(cube.min.z, min(spherePos.z, cube.max.z));


	float diss = (x - spherePos.x) * (x - spherePos.x) + (y - spherePos.y) * (y - spherePos.y) + (z - spherePos.z) * (z - spherePos.z);
	if (diss - radius * radius < 0) {//colliding

		glm::vec3 overlap = glm::vec3(0.0f);
		Collision coll;
		glm::vec3 xyzDiss = getXYZdistance(cube, sphere);
		overlap = ((((spherePos + radius) - (spherePos - radius)) / 2.0f) + ((cube.max - cube.min) / 2.0f)) - abs(xyzDiss);



		if (overlap.x < overlap.y) {
			if (overlap.x < overlap.z) {
				if (xyzDiss.x < 0) {
					coll.normal = glm::vec3(-1.0, 0.0, 0.0);
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
					coll.normal = glm::vec3(0.0, -1.0, 0.0);
				}
				else {
					coll.normal = glm::vec3(0.0, 1.0, 0.0);
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
		PositionalCorrection(sphere, cube, coll.penetration, coll.normal);

		return true;
	}
	return false;
}

void getAACubeNormal(Shapes& a, Shapes& b) {

	//https://gamedevelopment.tutsplus.com/tutorials/how-to-create-a-custom-2d-physics-engine-the-basics-and-impulse-resolution--gamedev-6331
	glm::vec3 xyzDiss = getXYZdistance(b, a);
	glm::vec3 overlap = glm::vec3(0.0f);

	Collision coll;

	overlap = (((a.max - a.min) / 2.0f) + ((b.max - b.min) / 2.0f)) - abs(xyzDiss);


	if (overlap.x < overlap.y) {
		if (overlap.x < overlap.z) {
			if (xyzDiss.x < 0) {
				coll.normal = glm::vec3(-1.0, 0.0, 0.0);
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
				coll.normal = glm::vec3(0.0, -1.0, 0.0);
			}
			else {
				coll.normal = glm::vec3(0.0, 1.0, 0.0);
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
		if (shape1.mass != 0 && shape1.hasGravity == true) {
			ApplyForce(shape1, glm::vec3(0.0f, -0.1f * shape1.mass, 0.0f));
		}

	}

}
	/////////////particles stuff goes here//////////




//////////boid stuff goes here//////////////


void setupBoids() {



	for (int x = 0; x < size(boidArray); x++) {
		boidArray[x].Load();
		boidArray[x].collision_type = AAcube;
		//boidArray[x].radius = 1;
		boidArray[x].hasGravity = false;
		boidArray[x].w_matrix =
			glm::translate(glm::vec3((1.0f * x) - 50.0f, 1.0f, 1.0f)) *
			glm::mat4(1.0f);
		boidArray[x].mass = 0.01f;
		boidArray[x].invMass = 100.0f;
		boidArray[x].velocity = glm::vec3(0.1f);
		boidArray[x].colour = glm::vec3(0.5f, 0.5f, 0.5f);
		boidArray[x].fillColor = glm::vec4(0.5f, 0.0f, 0.5f, 1.0f);
		flock.useTarget = true;
		boidArray[x].speed = 2.0f;
		flock.boidList.push_back(&boidArray[x]);
		allShapes.push_back(flock.boidList[x]);

	}
	for (int x = 0; x < size(boidArray2); x++) {
		boidArray2[x].Load();
		boidArray2[x].collision_type = AAcube;
		//boidArray2[x].radius = 1;
		boidArray2[x].hasGravity = false;
		boidArray2[x].w_matrix =
			glm::translate(glm::vec3(1.0f * x, 1.0f, 15.0f)) *
			glm::mat4(1.0f);
		boidArray2[x].mass = 0.01f;
		boidArray2[x].invMass = 100.0f;
		boidArray2[x].velocity = glm::vec3(0.1f);
		boidArray2[x].speed = 2.0f;
		boidArray2[x].colour = glm::vec3(0.5f, 0.5f, 0.5f);
		boidArray2[x].fillColor = glm::vec4(0.5f, 0.5f, 0.0f, 1.0f);
		flock2.useTarget = true;
		flock2.boidList.push_back(&boidArray2[x]);
		allShapes.push_back(flock2.boidList[x]);
	}


}

void boidUpdate() {

	flock.target = getposition(myCube);
	flock.updatePositions();

	for (int i = 0; i < flock.boidList.size(); i++) {
		flock.steer(i);
	}


	flock2.target = getposition(myCube);

	flock2.updatePositions();

	for (int i = 0; i < flock2.boidList.size(); i++) {
		flock2.steer(i);
	}

}



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
	

	//load up boids
	setupBoids();

	//Load and draw map for A*
	CreateMap();
	DrawMapOnScreen();


	//Load and draw player to show search path
	
	myPlayer.Load();
	myPlayer.collision_type = sphere;
	myPlayer.mass = 0;
	myPlayer.invMass = 0;
	myPlayer.w_matrix = glm::translate(glm::vec3(playerX, 0.5f, playerY)) *
		glm::mat4(1.0f);
	myPlayer.fillColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	allShapes.push_back(&myPlayer);
	





	//Load a target cube (red) and cubes to show the available nodes in a search
	myTarget.Load();
	myTarget.collision_type = none;
	myTarget.w_matrix =
		glm::translate(glm::vec3(targetX, 0.5f, targetY)) *
		glm::mat4(1.0f);
	myTarget.mass = 0.0f;
	myTarget.fillColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	allShapes.push_back(&myTarget);
	
	// Load Geometry examples
	myCube2.Load();
	myCube2.mass = 1.0f;
	myCube2.invMass = 1.0f;
	myCube2.w_matrix =
		glm::translate(glm::vec3(-4.0f, 1.0f, 2.0f)) *
		glm::scale(glm::vec3(1.5f, 1.5f, 1.5f)) *
		glm::mat4(1.0f);
	myCube2.collision_type = AAcube;
	myCube2.fillColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	allShapes.push_back(&myCube2);
	myCube2.scale = glm::vec3(1.5, 1.5, 1.5);


	myCube.Load();
	myCube.collision_type = AAcube;

	myCube.w_matrix =
		glm::translate(glm::vec3(-4.0f, 4.0f, 4.0f)) *
		glm::mat4(1.0f);
	myCube.mass = 1.0f;
	myCube.invMass = 1.0f;
	myCube.fillColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	allShapes.push_back(&myCube);

	



	//
	/*
	mySphere.Load();
	mySphere.collision_type = AAcube;
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
	*/
	
	for (int x = 0; x < 3; x += 1) {
		for (int z = 0; z < 2; z += 1) {

			cubeArray[x][z].Load();
			cubeArray[x][z].collision_type = AAcube;
			//cubeArray[x][z].radius = 0.5f;
			cubeArray[x][z].mass = 1.0f;
			cubeArray[x][z].invMass = 1.0f;
			cubeArray[x][z].w_matrix =
				glm::translate(glm::vec3((-9 + z * 2 ), 3 + x * 2 , 5)) *
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
	myFloor.scale = glm::vec3(1000.0f, 0.001f, 1000.0f);

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
		if (shape1.mass == 0) {
			shape1.invMass = 0;
		}

		else {
			shape1.invMass = 1 / shape1.mass;
		}
		shape1.possition = getposition(shape1);
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
	
	boidUpdate();
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

		shape1.w_matrix = glm::translate(shape1.possition) * glm::translate(shape1.velocity / (1 / deltaTime)) * glm::rotate(shape1.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)) * glm::rotate(shape1.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(shape1.scale) * glm::mat4(1.0f);//added time

		if (shape1.mass != 0) {
			shape1.w_matrix[3][0] += shape1.correction.x;
			shape1.w_matrix[3][1] += shape1.correction.y;
			shape1.w_matrix[3][2] += shape1.correction.z;
			//cout << shape1.correction.x << " " << shape1.correction.y << " " << shape1.correction.z << " \n";
			shape1.correction = glm::vec3(0.0f);
		}

		shape1.possition = getposition(shape1);

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
		glm::vec3 force = glm::vec3(0.0f, 0.3f, 0.0f);
		ApplyForce(myCube, force);

	}
	if (keyStatus[GLFW_KEY_U] == true) {
		glm::vec3 force = glm::vec3(0.0f, -0.1f, 0.0f);
		ApplyForce(myCube, force);

	}
	

	if (keyStatus[GLFW_KEY_B] == true) {
		flock.useTarget = false;
		flock2.useTarget = false;

	}
	if (keyStatus[GLFW_KEY_V] == true) {
		flock.useTarget = true;
		flock2.useTarget = true;

	}

	// toggle showing mouse.
	if (keyStatus[GLFW_KEY_M]) {
		mouseEnabled = !mouseEnabled;
		myGraphics.ToggleMouse();
	}
	// If exit key pressed.
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	
	//Keys to move player, start search and then have player move along the returned path
	if (keyStatus[GLFW_KEY_KP_8]) {
		movePlayer(0, 1);
	}
	if (keyStatus[GLFW_KEY_KP_4]) {
		movePlayer(1, 0);
	}
	if (keyStatus[GLFW_KEY_KP_6]) {
		movePlayer(-1, 0);
	}
	if (keyStatus[GLFW_KEY_KP_5]) {
		movePlayer(0, -1);
	}

	if (keyStatus[GLFW_KEY_KP_7]) {
		createSearchGraph();
	}
	if (keyStatus[GLFW_KEY_KP_9]) {
		moveAlongPath();
	}
	
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

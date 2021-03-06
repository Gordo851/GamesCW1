

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

//bools to run specific parts of the assignment
bool aStarSearch = false;
bool boidAreAGo = true;
bool explosions = false;
bool fountains = false;

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
bool		mouseEnabled = true; // keep track of mouse toggle


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
Cube  cubeArray2[10][10];
boidFlock flock;
Boid boidArray[200];

boidFlock flock2;
Boid boidArray2[1];


vector<Particle*> allParticles;
Cube myParticleCube;
Particle particleArray[20];
Particle particle;
int counter = 0;
int lengthToReset = 110;
int fountainNumber = 0;
int frameNumber = 0;
bool letsExplode = false;
// Some global variable to do the animation.
float t = 0.001f;            // Global variable for animation

//Search Variables
int mapArray[20][20];
int showMapArray[20][20];
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
//Calculate the euclidean distance fo heuristic. Using methods used in collision detection
float CalculateHeuristic(float X, float Y)
{
	//create imaginary plae ie. where the player WOULD be if they moved here
	Shapes imaginaryPlace;
	imaginaryPlace.w_matrix =
		glm::translate(glm::vec3(X, 0.5f, Y)) *
		glm::mat4(1.0f);
	float heuristic = sqrt(getSquareDistance(imaginaryPlace, myTarget));//square root for now
	return heuristic;
}
//The map is drawn from an array of 20x20. If the array contains a 1 then a cube is laced at that x y position. The cubes forshowing the search graphically are placed lower than the maz. 
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
				if (showMapArray[x][z] == 1)
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

}
//Map is randomly generated. Current values mean 40% of the map will be blocked off
void CreateMap()
{
	//randomly place obstacles
	float MAX = 1.0f;
	for (int x = 0; x < 20; x++)
	{
		for (int y = 0; y < 20; y++)
		{
			float random = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / MAX));
			if (random > 0.7f)
			{
				mapArray[x][y] = 1;
				showMapArray[x][y] = 0;

			}
			else
			{
				mapArray[x][y] = 0;
				showMapArray[x][y] = 1;
			}
		}
	}
	//build sides
	for (int x = 0; x < 20; x++)
	{
		for (int y = 0; y < 20; y += 19)
		{
			mapArray[x][y] = 1;
			showMapArray[x][y] = 0;
		}
	}
	for (int x = 0; x < 20; x += 19)
	{
		for (int y = 0; y < 20; y++)
		{
			mapArray[x][y] = 1;
			showMapArray[x][y] = 0;
		}
	}
}
//Hard code to move player to path x,y coordinates
void movePlayer(float x, float y)
{
	playerX = x;
	playerY = y;
	myPlayer.possition = glm::vec3(playerX, 0.5f, playerY);
	myPlayer.velocity = (glm::vec3(0.0f, 0.0f, 0.0f));
}
// Create the nodes to be used in searching. Picks each node and finds if paths exist aroun that node to enable travel horizontally or vertically 
void createSearchGraph()
{
	int index = 0;
	for (int x = 0; x < 20; x++)
	{
		for (int y = 0; y < 20; y++)
		{
			//create node at x,y if that area is empty (and therefore passable by agent)
			if (mapArray[x][y] == 0)
			{
				node[index].index = glm::vec2(x, y);
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
//Method to enable searchin. Posible next nodes can be found by their index which is their x,y coordinates
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
//This method changes the colours of the searching array to visually show the search process.
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
//This method changes the colours of the searching array to visually show the search process.
void showChosenNode(Node nodeChosen)
{
	glm::vec2 position = nodeChosen.index;
	int x = position[0];
	int y = position[1];
	showSearchCubeArray[x][y].fillColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

}
//Search through the graph created above to make the optimal path.
void searchGraph()
{
	//Setup current and target nodes
	//Basic variables needed for searching - values don't matter as they are immediately changed
	Node currentNode = findNodesByIndex(glm::vec2(1, 1));
	Node targetNode = findNodesByIndex(glm::vec2(17, 11));
	vector<glm::vec2> usedIndexes;
	usedIndexes.push_back(glm::vec2(1, 1));
	float totalCost = 10000.0f;
	float pathCost = 0.0f;
	float indexToBeDeleted;
	glm::vec2 indexToAddToUsed;
	int x = 0;
	//While the search is not complete
	while (currentNode.index != targetNode.index)
	{
		//We check for the current node the directions of posible travel and add those nodes to an 'available' index and register the current node as their parent.
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

		//Reset the total cost from the last itteration else the program hangs on the lowest node and wont move to the next node if it's cost is higher
		totalCost = 1000.0f;
		//show the search visually
		showSearch();
		//Search through all available nodes, calculate their heuristic and path cost (based on the trail of their parents back to the origin) and set the current node to the best next possible one.
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

		//Delete the selected node from available nodes
		availableIndexes.erase(availableIndexes.begin() + indexToBeDeleted);
		usedIndexes.push_back(indexToAddToUsed);
		showChosenNode(currentNode);
		x++;

	}
	//Once the search is complete, go back through all of the parents to create the final path our agent will follow.
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
//Move the player along the entire path
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
//apply force to object shape
void ApplyForce(Shapes& shape, glm::vec3 force)
{
	if (shape.mass != 0) {
		glm::vec3 Velocity = force / shape.mass;
		shape.velocity = shape.velocity + Velocity;
	}

}

// calcualte the rebound of two objects
void calculateRebound(Shapes& a, Shapes& b, glm::vec3 normal) {

	float e = min(a.e, b.e);//get min e

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


}
/**correct for floating point error
*/
void PositionalCorrection(Shapes& a, Shapes& b, float penetration, glm::vec3 normal)
{


	float percent = 0.8f; // precent correction
	float slop = 0.01f; // alowable overlap

	glm::vec3 correction = (max((penetration - slop), 0.0f) / (a.invMass + b.invMass)) * percent * normal; // move object along normal 

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


/**
check if two shapes are colliding and sets the normal if they are
*/
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
	else if ((shape1.collision_type == AAcube && shape2.collision_type == sphere)) {

		return getAAcubeSphereNormal2(shape1, shape2);
	}
	else if (shape1.collision_type == sphere && shape2.collision_type == AAcube) {
		return getAAcubeSphereNormal(shape1, shape2);
	}

}

/**
check if cube and sphere are colliding and then calcualte the normal when a cube and sphere have colided
*/
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
/**
check if cube and sphere are colliding and then calcualte the normal when a cube and sphere have colided
*/
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
/**
calcualte the normal when two cubes have colided
*/
void getAACubeNormal(Shapes& a, Shapes& b) {


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
/**
returns a vector holding the x y and z distances between two points
*/
glm::vec3 getXYZdistance(glm::vec3 point1, glm::vec3 point2) {
	return point1 - point2;
}
/**
returns a vector holding the x y and z distances between two objects
*/
glm::vec3 getXYZdistance(Shapes a, Shapes b) {
	glm::vec3 point1 = getposition(a);
	glm::vec3 point2 = getposition(b);
	return point1 - point2;
}
/**
returns the position of a shape
*/
glm::vec3 getposition(Shapes shape) {
	return glm::vec3(shape.w_matrix[3][0], shape.w_matrix[3][1], shape.w_matrix[3][2]);
}
/**
returns the square distance between two shapes
*/
float getSquareDistance(Shapes a, Shapes b) {
	glm::vec3 positionA = getposition(a);
	glm::vec3 positionB = getposition(b);
	glm::vec3 distanceXYZ = getXYZdistance(positionA, positionB);

	return distanceXYZ.x * distanceXYZ.x + distanceXYZ.z * distanceXYZ.z + distanceXYZ.y * distanceXYZ.y;
}
/**
returns the square distance between two points
*/
float getSquareDistance(glm::vec3 point1, glm::vec3 point2) {
	glm::vec3 distanceXYZ = getXYZdistance(point1, point2);

	return distanceXYZ.x * distanceXYZ.x + distanceXYZ.z * distanceXYZ.z + distanceXYZ.y * distanceXYZ.y;
}

/**
loop thorugh all object pairs -
*/
void checkCollisions()
{


	for (int i = 0; i < allShapes.size(); i++)
	{
		Shapes& shape1 = *allShapes[i];
		if (shape1.velocity.x == 0 && shape1.velocity.y == 0 && shape1.velocity.z == 0) {//only calc new if moving

		}
		else {

			calculateMinMax(shape1); //update vertex positions of all shapes - if they have moved. - done here so only calculated once per frame
		}
		//shape1.fillColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
	}

	//Itteration for allShapes - 
	for (int i = 0; i < allShapes.size(); i++)
	{
		for (int j = i + 1; j < allShapes.size(); j++)
		{
			Shapes& shape1 = *allShapes[i];
			Shapes& shape2 = *allShapes[j];
			bool colliding = isColliding(shape1, shape2); // are two object colliding
			if (colliding == true) { // if yes calcualte rebound
				//shape1.fillColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				//shape2.fillColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				Collision& coll = *allCollisions[allCollisions.size() - 1];
				glm::vec3 normal = coll.normal;
				calculateRebound(shape1, shape2, normal);

			}
		}
	}
}

/**
apply a downward force on all objects if mass is not 0
can disable gravity  for specific object by setting hasgravity = false
*/
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
//Load particle objects and set their colour
void CreateParticles() {
	for (int x = 0; x < size(particleArray); x += 1) {
		particleArray[x].Load();
		particleArray[x].collision_type = sphere;
		particleArray[x].w_matrix =
			glm::translate(glm::vec3(1 + x, -1.0f, 0.0f)) *
			glm::mat4(1.0f);
		particleArray[x].mass = 0.1f;
		particleArray[x].fillColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		allShapes.push_back(&particleArray[x]);
		allParticles.push_back(&particleArray[x]);
	}
}
//Reset particles to base positions
void ResetParticles() {
	for (int x = 0; x < size(particleArray); x += 1) {
		particleArray[x].possition = (glm::vec3(0.1f * x, -1.0f, 1.0f));
		particleArray[x].velocity = glm::vec3(0.0f);
		particleArray[x].toRender = true;
		particleArray[x].DeathCount = 300;
	}
	letsExplode = false;
}
//This doesn't 'destroy' merely stops them being rendered so they vanish
void DestoryParticles(Particle& shape1) {
	shape1.toRender = false;
}
//Get particles in position to explode.
void setUpRandomExplosion() {
	ResetParticles();
	for (int i = 0; i < allParticles.size(); i++)
	{
		Particle& shape1 = *allParticles[i];
		for (int a = 0; a < 5; a++)
		{
			for (int b = 1; b < 5; b++)
			{
				shape1.possition = glm::vec3(a, 5, b);
				shape1.velocity = glm::vec3(0.0f);
			}
		}
	}
	letsExplode = true;
}
//Every thrid frame apply a random force to simulate an explosion.
void moveParticlesRandomExplosion() {
	for (int h = 0; h < allParticles.size(); h++)
	{
		Particle& shape1 = *allParticles[h];
		float MAX = 2.0f;
		float x = 1 - static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / MAX));
		float y = 1 - static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / MAX));
		float z = 1 - static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / MAX));
		glm::vec3 force = glm::vec3(x, y, z);
		ApplyForce(shape1, force);
	}
}
//fountain particles from an emitter. They will spurt up and out.
void fountainParticles() {
	float MAX = 1.0f;
	Particle& shape1 = *allParticles[fountainNumber];
	shape1.possition = glm::vec3(1, 1.5f, 1);
	shape1.velocity = glm::vec3(0.0f);
	float x = -0.5f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / MAX));
	float y = 1.0f;
	float z = -0.5f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / MAX));
	glm::vec3 force = glm::vec3(x, y, z);
	ApplyForce(shape1, force);
	fountainNumber += 1;
	if (fountainNumber == 20)
	{
		fountainNumber = 0;
	}
}
//Count all particles down towards death.
void CountParticlesToDeath() {
	for (int i = 0; i < allParticles.size(); i++)
	{
		Particle& shape1 = *allParticles[i];
		shape1.DeathCount -= 1;
		if (shape1.DeathCount == 0)
		{
			DestoryParticles(*allParticles[i]);
		}
	}
}



//////////boid stuff goes here//////////////

/**
*set up boids - load in boisa matrix and add to allshapes
*/
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
/**
called once avery frame - updates boids position
*/
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


	//Only load the objects we need for the demo
	if (explosions == true || fountains == true) {
		CreateParticles();
	}

	if (boidAreAGo == true) {
		//load up boids
		setupBoids();
		for (int x = 0; x < 5; x += 1) {
			for (int z = 0; z < 5; z += 1) {

				cubeArray2[x][z].Load();
				cubeArray2[x][z].collision_type = AAcube;
				//cubeArray[x][z].radius = 0.5f;
				cubeArray2[x][z].mass = 0.0f;
				cubeArray2[x][z].invMass = 0.0f;
				cubeArray2[x][z].w_matrix =
					glm::translate(glm::vec3((2 + z * 2), 0.5 + x * 2, 11 - z * 2)) *
					glm::mat4(1.0f);
				cubeArray2[x][z].fillColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				allShapes.push_back(&cubeArray2[x][z]);
			}
		}

		for (int x = 5; x < 10; x += 1) {
			for (int z = 5; z < 10; z += 1) {

				cubeArray2[x][z].Load();
				cubeArray2[x][z].collision_type = AAcube;
				//cubeArray[x][z].radius = 0.5f;
				cubeArray2[x][z].mass = 0.0f;
				cubeArray2[x][z].invMass = 0.0f;
				cubeArray2[x][z].w_matrix =
					glm::translate(glm::vec3((-20 + z * 2), -7 + x * 2, 11 - z * 2)) *
					glm::mat4(1.0f);
				cubeArray2[x][z].fillColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				allShapes.push_back(&cubeArray2[x][z]);
			}
		}
	}

	if (aStarSearch == true) {
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

	}
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

	mySphere.Load();
	mySphere.collision_type = sphere;
	mySphere.w_matrix = glm::translate(glm::vec3(-2.0f, 1.0f, -3.0f)) *
		glm::rotate(-t, glm::vec3(0.0f, 1.0f, 0.0f)) *
		glm::rotate(-t, glm::vec3(1.0f, 0.0f, 0.0f)) *
		glm::mat4(1.0f);
	allShapes.push_back(&mySphere);
	mySphere.fillColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	/*
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
				glm::translate(glm::vec3((-1 - z * 2), 3 + x * 2, 0)) *
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
	if (fountains == true) {
		myParticleCube.Load();
		myParticleCube.collision_type = none;

		myParticleCube.w_matrix =
			glm::translate(glm::vec3(0.0f, 0.0f, 0.0f)) *
			glm::mat4(1.0f);
		myParticleCube.possition = glm::vec3(1, 0.5, 1);
		myParticleCube.mass = 0.0f;
		myParticleCube.invMass = 0.0f;
		myParticleCube.fillColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		allShapes.push_back(&myParticleCube);
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
	CountParticlesToDeath();				//Ensure particles die on time
	boidUpdate();
	applyGravity();
	checkCollisions(); // check collistion
	//Every third frame produce a new droplet of water for the fountain
	if (fountains == true) {
		if (frameNumber % 3 == 0) {
			fountainParticles();
			frameNumber = 0;
		}
		frameNumber += 1;
	}
	//Reset the particles after a predecided number of frames. Will make them visible again if they have previously been destoryed.
	if (fountains == true || explosions == true) {
		if (counter == lengthToReset) {
			ResetParticles();
			counter = 0;
		}
		counter += 1;
	}
	// particles are randomly xploded
	if (letsExplode == true) {
		moveParticlesRandomExplosion();
	}
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
		// to render is used to hide particles that have been counted to death. 
		if (shape1.toRender == true) {
			shape1.Draw();
		}
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

	if (boidAreAGo == true) {
		if (keyStatus[GLFW_KEY_B] == true) {
			flock.useTarget = false;
			flock2.useTarget = false;

		}
		if (keyStatus[GLFW_KEY_V] == true) {
			flock.useTarget = true;
			flock2.useTarget = true;

		}
	}
	// toggle showing mouse.
	if (keyStatus[GLFW_KEY_M]) {
		mouseEnabled = !mouseEnabled;
		myGraphics.ToggleMouse();
	}
	// If exit key pressed.
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	if (aStarSearch == true) {
		//keys to begin search and move player.
		if (keyStatus[GLFW_KEY_KP_7]) {
			createSearchGraph();
		}
		if (keyStatus[GLFW_KEY_KP_9]) {
			moveAlongPath();
		}
	}
	if (fountains == true) {
		if (keyStatus[GLFW_KEY_KP_0]) {
			fountainParticles();
		}
	}
	if (explosions == true) {
		if (keyStatus[GLFW_KEY_KP_2]) {
			setUpRandomExplosion();
		}
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


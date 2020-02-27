#pragma once
#include <iostream>
#include <vector>
using namespace std;

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Node {
public:
	//base class for all nodes. Start off at origin with no paths. 
	Node();
	~Node();
	glm::vec2 index = glm::vec2(0, 0);
	glm::vec2 parent = glm::vec2(0, 0);
	float HeuristicCost = 0.0f;
	float pathCost = 1.0f;
	bool travelUp = false;
	bool travelDown = false;
	bool travelLeft = false;
	bool travelRight = false;
	
};

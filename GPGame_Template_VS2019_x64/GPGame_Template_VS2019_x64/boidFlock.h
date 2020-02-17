#pragma once


using namespace std;
#include <iostream>
#include <vector>
#include <algorithm> 
#include "shapes.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class boidFlock
{
public:
	vector<Boid*> boidList;
	vector<glm::vec3> boidPosition;
	vector<glm::vec3> boidVelocity;
	vector<glm::vec3> boidColour;

	boidFlock();
	void addBoid(Boid a);
	void removeBoid(Boid a);
	void updatePositions();
	void steer(int i);
	float getDistance(int i, int j);
	glm::vec3 target = glm::vec3(0.0f);
	bool useTarget = false;
};




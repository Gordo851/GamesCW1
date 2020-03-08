#pragma once
using namespace std;
#include <iostream>
#include <vector>
#include <algorithm> 
#include "shapes.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>


class AllMissle
{
public:
	vector<Missle*> missleList;
	vector<glm::vec3> misslePosition;
	vector<glm::vec3> boidVelocity;
	vector<glm::vec3> boidColour;

	AllMissle();
	void updatePositions();
	void steer(int i);
	float getDistance(int i, int j);
	glm::vec3 target = glm::vec3(0.0f);
};
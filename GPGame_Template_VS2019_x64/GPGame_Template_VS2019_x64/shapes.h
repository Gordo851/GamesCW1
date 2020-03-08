#pragma once

#include <iostream>
#include <vector>
using namespace std;

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>


enum CollisionType // collision types 
{
	sphere,
	AAcube,
	none
};



class Shapes {

public:
	Shapes();
	~Shapes();

	void Load();
	void Draw();
	void  checkErrorShader(GLuint shader);

	vector<GLfloat> vertexPositions;

	vector<GLfloat> currentVertexPositions;

	GLuint          program;
	GLuint          vao;
	GLuint          buffer;
	GLint           mv_location;
	GLint           proj_location;
	GLint           color_location;
	glm::mat4		proj_matrix = glm::mat4(1.0f);
	glm::mat4		mv_matrix = glm::mat4(1.0f);
	glm::mat4		w_matrix = glm::mat4(1.0f);

	glm::vec4		fillColor = glm::vec4(1.0, 0.0, 0.0, 1.0);
	glm::vec4		lineColor = glm::vec4(0.0, 0.0, 0.0, 1.0);
	float			lineWidth = 2.0f;
	//
	float mass = 1.0f;
	float invMass;
	glm::vec3 velocity = glm::vec3(0.0f);
	glm::vec3 possition = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);
	bool toRender = true;


	CollisionType collision_type = sphere;//defult collision sphere
	glm::vec3 min = glm::vec3(0.0f);
	glm::vec3 max = glm::vec3(0.0f);
	float radius = 0.5f;
	float e = 0.6f; //youngs modulus
	glm::vec3 correction = glm::vec3(0.0f);
	

	bool hasGravity = true;

protected:
	string rawData;			// Import obj file from Blender (note: no textures or UVs).
	void LoadObj();
};

struct Collision {
public:
	Shapes *a;
	Shapes *b;
	glm::vec3 normal;
	float penetration;
};

class Cube : public Shapes {
public:
	Cube();
	~Cube();
};

class Sphere : public Shapes {
public:
	Sphere();
	~Sphere();
};

class Arrow : public Shapes {
public:
	Arrow();
	~Arrow();
};

class Cylinder : public Shapes {
public:
	Cylinder();
	~Cylinder();
};

class Line : public Shapes {
public:
	Line();
	~Line();
};

class Particle : public Shapes {
public:
	//how mnay frames a particle will last
	int DeathCount = 100;
	Particle();
	~Particle();
private:
};

class Boid : public Shapes {
public:
	//glm::vec3 position;
	Boid();
	~Boid();
	float speed = 10.0f; // 1 - 4 works well
	float range = 5.0f; // sight range
	float minDist = 1.5f; // seperation distance
	glm::vec3 colour = glm::vec3(0.5f); // used to calcualte average colour

private:
};

class Missle : public Shapes {
public:
	//glm::vec3 position;
	Missle();
	~Missle();
	float speed = 10.0f; // 1 - 4 works well
	float range = 5.0f; // sight range
	float minDist = 1.5f; // seperation distance
	glm::vec3 colour = glm::vec3(0.5f); // used to calcualte average colour
	bool useTarget = false;
	bool fired = false;
	float deathCounter = 0; //simulate limited Fuel
private:
};

class EnemyShip : public Shapes {
public:
	EnemyShip();
	~EnemyShip();
	int missleAmmo = 10;
	int bulletAmmo = 20;
	int missleCounter = 0;
	float maxFuel = 100.0f;
};
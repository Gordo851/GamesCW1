#pragma once

#include <iostream>
#include <vector>
using namespace std;

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

enum Collision
{
	sphere,
	cube,
	none
};

enum XYZ {
	x = 0,
	y = 1,
	z = 2
};

class Shapes {

public:
	Shapes();
	~Shapes();

	void Load();
	void Draw();
	void  checkErrorShader(GLuint shader);

	vector<GLfloat> vertexPositions;

	GLuint          program;
	GLuint          vao;
	GLuint          buffer;
	GLint           mv_location;
	GLint           proj_location;
	GLint           color_location;
	glm::mat4		proj_matrix = glm::mat4(1.0f);
	glm::mat4		mv_matrix = glm::mat4(1.0f);
	glm::mat4		w_matrix = glm::mat4(1.0f);
	glm::mat4		w_matrix_old = glm::mat4(1.0f);
	
//physics properties
	//glm::vec3		linearMovement = glm::vec3(0.0f);
	glm::vec3		velocity = glm::vec3(0.0f);
	glm::vec3		oldSpeed = glm::vec3(0.0f);
	glm::vec3		newSpeed = glm::vec3(0.0f);
	float			mass = 1.0f; //All shapes have base mass of 0 by default - we consider this infante mass - makes them static

	glm::vec4		fillColor = glm::vec4(1.0, 0.0, 0.0, 1.0);
	glm::vec4		lineColor = glm::vec4(0.0, 0.0, 0.0, 1.0);
	float			lineWidth = 2.0f;
	Collision collision_type = sphere;//defult collision sphere
	float radius = 0.0f;
	int lastCollided = -1; //index of object last collidesd with
	bool onGround = false;

	//these are of efficensy 
	vector<GLfloat> currentVertexPositions;



protected:
	string rawData;			// Import obj file from Blender (note: no textures or UVs).
	void LoadObj();
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

class Particle: public Shapes{
public:
	int DeathCount = 100;
	Particle();
	~Particle();
private:
};
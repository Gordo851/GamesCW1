#include "shapes.h"
#include <iostream>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

Shapes::Shapes() {

};

Shapes::~Shapes() {

}



void Shapes::LoadObj() {

	std::vector< glm::vec3 > obj_vertices;
	std::vector< unsigned int > vertexIndices;
	istringstream rawDataStream(rawData);
	string dataLine;  int linesDone = 0;

	while (std::getline(rawDataStream, dataLine)) {
		if (dataLine.find("v ") != string::npos) {	// does this line have a vector?
			glm::vec3 vertex;

			int foundStart = dataLine.find(" ");  int foundEnd = dataLine.find(" ", foundStart + 1);
			vertex.x = stof(dataLine.substr(foundStart, foundEnd - foundStart));

			foundStart = foundEnd; foundEnd = dataLine.find(" ", foundStart + 1);
			vertex.y = stof(dataLine.substr(foundStart, foundEnd - foundStart));

			foundStart = foundEnd; foundEnd = dataLine.find(" ", foundStart + 1);
			vertex.z = stof(dataLine.substr(foundStart, foundEnd - foundStart));

			obj_vertices.push_back(vertex);
		}
		else if (dataLine.find("f ") != string::npos) { // does this line defines a triangle face?
			string parts[3];

			int foundStart = dataLine.find(" ");  int foundEnd = dataLine.find(" ", foundStart + 1);
			parts[0] = dataLine.substr(foundStart + 1, foundEnd - foundStart - 1);

			foundStart = foundEnd; foundEnd = dataLine.find(" ", foundStart + 1);
			parts[1] = dataLine.substr(foundStart + 1, foundEnd - foundStart - 1);

			foundStart = foundEnd; foundEnd = dataLine.find(" ", foundStart + 1);
			parts[2] = dataLine.substr(foundStart + 1, foundEnd - foundStart - 1);

			for (int i = 0; i < 3; i++) {		// for each part

				vertexIndices.push_back(stoul(parts[i].substr(0, parts[i].find("/"))));

				int firstSlash = parts[i].find("/"); int secondSlash = parts[i].find("/", firstSlash + 1);

				if (firstSlash != (secondSlash + 1)) {	// there is texture coordinates.
														// add code for my texture coordintes here.
				}
			}
		}

		linesDone++;
	}

	for (unsigned int i = 0; i < vertexIndices.size(); i += 3) {
		vertexPositions.push_back(obj_vertices[vertexIndices[i + 0] - 1].x);
		vertexPositions.push_back(obj_vertices[vertexIndices[i + 0] - 1].y);
		vertexPositions.push_back(obj_vertices[vertexIndices[i + 0] - 1].z);

		vertexPositions.push_back(obj_vertices[vertexIndices[i + 1] - 1].x);
		vertexPositions.push_back(obj_vertices[vertexIndices[i + 1] - 1].y);
		vertexPositions.push_back(obj_vertices[vertexIndices[i + 1] - 1].z);

		vertexPositions.push_back(obj_vertices[vertexIndices[i + 2] - 1].x);
		vertexPositions.push_back(obj_vertices[vertexIndices[i + 2] - 1].y);
		vertexPositions.push_back(obj_vertices[vertexIndices[i + 2] - 1].z);
	}
	//calc radius
	float maxdistance = 0;
	for (unsigned int i = 0; i < vertexPositions.size(); i += 3) {
		float dis = 0;
		dis = (sqrt(vertexPositions[i] * vertexPositions[i] + vertexPositions[i + 1] * vertexPositions[i + 1] + vertexPositions[i + 2] * vertexPositions[i + 2]));
		if (dis > maxdistance) {
			maxdistance = dis;
		}
	}
	radius = maxdistance;

	// calc invMass
	if (mass == 0) {
		invMass = 0;
	}
		
	else {
		invMass = 1 / mass;
	}
		
}


void Shapes::Load() {
	const char * vs_source[] = { R"(
#version 330 core

in vec4 position;
uniform mat4 mv_matrix;
uniform mat4 proj_matrix;

void main(void){
	gl_Position = proj_matrix * mv_matrix * position;
}
)" };

	
	const char * fs_source[] = { R"(
#version 330 core

uniform vec4 inColor;
out vec4 color;

void main(void){
	color = inColor;
}
)" };

	program = glCreateProgram();
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, fs_source, NULL);
	glCompileShader(fs);
	checkErrorShader(fs);

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, vs_source, NULL);
	glCompileShader(vs);
	checkErrorShader(vs);

	glAttachShader(program, vs);
	glAttachShader(program, fs);

	glLinkProgram(program);

	mv_location = glGetUniformLocation(program, "mv_matrix");
	proj_location = glGetUniformLocation(program, "proj_matrix");
	color_location = glGetUniformLocation(program, "inColor");

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER,
		vertexPositions.size() * sizeof(GLfloat),
		&vertexPositions[0],
		GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glLinkProgram(0);	// unlink
	glDisableVertexAttribArray(0); // Disable
	glBindVertexArray(0);	// Unbind
}

void Shapes::Draw() {
	glUseProgram(program);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);

	glUniformMatrix4fv(proj_location, 1, GL_FALSE, &proj_matrix[0][0]);
	glUniformMatrix4fv(mv_location, 1, GL_FALSE, &mv_matrix[0][0]);

	glUniform4f(color_location, fillColor.r, fillColor.g, fillColor.b, fillColor.a);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, vertexPositions.size() / 3);

	glUniform4f(color_location, lineColor.r, lineColor.g, lineColor.b, lineColor.a);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  glLineWidth(lineWidth);
	glDrawArrays(GL_TRIANGLES, 0, vertexPositions.size() / 3);
}


void Shapes::checkErrorShader(GLuint shader) {
	// Get log length
	GLint maxLength;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

	// Init a string for it
	std::vector<GLchar> errorLog(maxLength);

	if (maxLength > 1) {
		// Get the log file
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

		cout << "--------------Shader compilation error-------------\n";
		cout << errorLog.data();
	}
}

Cube::Cube() {
	// Exported from Blender a cube by default (OBJ File)
	rawData = R"(
v 0.500000 -0.500000 -0.500000
v 0.500000 -0.500000 0.500000
v -0.500000 -0.500000 0.500000
v -0.500000 -0.500000 -0.500000
v 0.500000 0.500000 -0.499999
v 0.499999 0.500000 0.500000
v -0.500000 0.500000 0.500000
v -0.500000 0.500000 -0.500000
f 1 3 4
f 8 6 5
f 5 2 1
f 6 3 2
f 7 4 3
f 1 8 5
f 1 2 3
f 8 7 6
f 5 6 2
f 6 7 3
f 7 8 4
f 1 4 8)";

	LoadObj();
}



Cube::~Cube() {

}

Sphere::Sphere() {

	rawData = R"(
o Object.1
v 0.000000 -0.000000 -0.525731
v 0.309017 -0.000000 -0.425325
v 0.154509 0.267616 -0.425325
v -0.154509 0.267616 -0.425325
v -0.309017 -0.000000 -0.425325
v -0.154508 -0.267617 -0.425325
v 0.154509 -0.267617 -0.425325
v 0.500000 -0.000000 -0.162460
v 0.250000 0.433012 -0.162460
v -0.250000 0.433012 -0.162460
v -0.500000 -0.000000 -0.162460
v -0.250000 -0.433013 -0.162460
v 0.250000 -0.433013 -0.162460
v 0.500000 -0.000000 0.162460
v 0.250000 0.433012 0.162460
v -0.250000 0.433012 0.162460
v -0.500000 -0.000000 0.162460
v -0.250000 -0.433013 0.162460
v 0.250000 -0.433013 0.162460
v 0.309017 -0.000000 0.425325
v 0.154509 0.267616 0.425325
v -0.154509 0.267616 0.425325
v -0.309017 -0.000000 0.425325
v -0.154508 -0.267617 0.425325
v 0.154509 -0.267617 0.425325
v 0.000000 -0.000000 0.525731

usemtl BaseColor_0
f 2 1 3
f 3 1 4
f 4 1 5
f 5 1 6
f 6 1 7
f 7 1 2
f 2 9 8
f 2 3 9
f 3 10 9
f 3 4 10
f 4 11 10
f 4 5 11
f 5 12 11
f 5 6 12
f 6 13 12
f 6 7 13
f 7 8 13
f 7 2 8
f 8 15 14
f 8 9 15
f 9 16 15
f 9 10 16
f 10 17 16
f 10 11 17
f 11 18 17
f 11 12 18
f 12 19 18
f 12 13 19
f 13 14 19
f 13 8 14
f 14 21 20
f 14 15 21
f 15 22 21
f 15 16 22
f 16 23 22
f 16 17 23
f 17 24 23
f 17 18 24
f 18 25 24
f 18 19 25
f 19 20 25
f 19 14 20
f 20 21 26
f 21 22 26
f 22 23 26
f 23 24 26
f 24 25 26
f 25 20 26
)";

	LoadObj();
}

Sphere::~Sphere() {

}

Arrow::Arrow() {

	rawData = R"(
o Cone
v 0.000000 0.800000 -0.100000
v 0.070711 0.800000 -0.070711
v 0.100000 0.800000 -0.000000
v 0.000000 1.000000 0.000000
v 0.070711 0.800000 0.070711
v -0.000000 0.800000 0.100000
v -0.070711 0.800000 0.070711
v -0.100000 0.800000 -0.000000
v -0.070711 0.800000 -0.070711
s off
f 4 7 6
f 5 7 2
f 4 8 7
f 3 4 5
f 5 4 6
f 4 9 8
f 4 1 9
f 2 1 4
f 2 4 3
f 9 1 2
f 2 3 5
f 5 6 7
f 7 8 9
f 9 2 7
o Cylinder
v 0.000000 0.000000 -0.050000
v 0.009755 0.900000 -0.049039
v 0.019134 0.000000 -0.046194
v 0.027779 0.900000 -0.041573
v 0.035355 0.000000 -0.035355
v 0.041573 0.900000 -0.027779
v 0.046194 0.000000 -0.019134
v 0.049039 0.900000 -0.009755
v 0.050000 0.000000 -0.000000
v 0.049039 0.900000 0.009755
v 0.046194 0.000000 0.019134
v 0.041573 0.900000 0.027779
v 0.035355 0.000000 0.035355
v 0.027779 0.900000 0.041573
v 0.019134 0.000000 0.046194
v 0.009755 0.900000 0.049039
v -0.000000 0.000000 0.050000
v -0.009755 0.900000 0.049039
v -0.019134 0.000000 0.046194
v -0.027779 0.900000 0.041573
v -0.035355 0.000000 0.035355
v -0.041574 0.900000 0.027778
v -0.046194 0.000000 0.019134
v -0.049039 0.900000 0.009754
v -0.050000 0.000000 -0.000000
v -0.049039 0.900000 -0.009755
v -0.046194 0.000000 -0.019134
v -0.041573 0.900000 -0.027779
v -0.035355 0.000000 -0.035355
v -0.027778 0.900000 -0.041574
v -0.019134 0.000000 -0.046194
v -0.009754 0.900000 -0.049039
s off
f 13 15 14
f 16 14 15
f 17 19 18
f 18 16 17
f 19 21 20
f 20 18 19
f 21 23 22
f 22 20 21
f 23 25 24
f 24 22 23
f 25 27 26
f 26 24 25
f 27 29 28
f 28 26 27
f 29 31 30
f 30 28 29
f 31 33 32
f 32 30 31
f 33 35 34
f 34 32 33
f 35 37 36
f 36 34 35
f 37 39 38
f 38 36 37
f 41 40 39
f 40 38 39
f 41 10 40
f 29 21 37
f 11 12 10
f 24 32 16
f 15 17 16
f 11 13 12
f 14 12 13
f 10 41 11
f 13 11 41
f 41 39 37
f 37 35 33
f 33 31 29
f 29 27 25
f 25 23 29
f 21 19 17
f 17 15 13
f 13 41 37
f 37 33 29
f 29 23 21
f 21 17 13
f 13 37 21
f 40 10 12
f 12 14 16
f 16 18 20
f 20 22 24
f 24 26 28
f 28 30 32
f 32 34 36
f 36 38 40
f 40 12 16
f 16 20 24
f 24 28 32
f 32 36 40
f 40 16 32
)";

	LoadObj();
}

Arrow::~Arrow() {

}

Cylinder::Cylinder() {

	rawData = R"(
o Cylinder
v 0.000000 -0.500000 -0.500000
v 0.000000 0.500000 -0.500000
v 0.293893 -0.500000 -0.404509
v 0.293893 0.500000 -0.404509
v 0.475528 -0.500000 -0.154508
v 0.475528 0.500000 -0.154508
v 0.475528 -0.500000 0.154509
v 0.475528 0.500000 0.154509
v 0.293893 -0.500000 0.404509
v 0.293893 0.500000 0.404509
v -0.000000 -0.500000 0.500000
v -0.000000 0.500000 0.500000
v -0.293893 -0.500000 0.404508
v -0.293893 0.500000 0.404508
v -0.475528 -0.500000 0.154509
v -0.475528 0.500000 0.154509
v -0.475528 -0.500000 -0.154509
v -0.475528 0.500000 -0.154509
v -0.293892 -0.500000 -0.404509
v -0.293892 0.500000 -0.404509
vt 1.000000 1.000000
vt 0.900000 0.500000
vt 1.000000 0.500000
vt 0.900000 1.000000
vt 0.800000 0.500000
vt 0.800000 1.000000
vt 0.700000 0.500000
vt 0.700000 1.000000
vt 0.600000 0.500000
vt 0.600000 1.000000
vt 0.500000 0.500000
vt 0.500000 1.000000
vt 0.400000 0.500000
vt 0.400000 1.000000
vt 0.300000 0.500000
vt 0.300000 1.000000
vt 0.200000 0.500000
vt 0.250000 0.490000
vt 0.021746 0.324164
vt 0.391068 0.055836
vt 0.200000 1.000000
vt 0.100000 0.500000
vt 0.100000 1.000000
vt -0.000000 0.500000
vt 0.978254 0.175836
vt 0.750000 0.010000
vt 0.521746 0.175836
vt 0.478254 0.324164
vt 0.391068 0.444164
vt 0.108932 0.444164
vt 0.021746 0.175836
vt 0.108932 0.055836
vt 0.250000 0.010000
vt 0.478254 0.175836
vt -0.000000 1.000000
vt 0.608932 0.444164
vt 0.750000 0.490000
vt 0.891068 0.444164
vt 0.978254 0.324164
vt 0.891068 0.055836
vt 0.608932 0.055836
vt 0.521746 0.324164
vn 0.3090 0.0000 -0.9511
vn 0.8090 0.0000 -0.5878
vn 1.0000 0.0000 0.0000
vn 0.8090 0.0000 0.5878
vn 0.3090 0.0000 0.9511
vn -0.3090 0.0000 0.9511
vn -0.8090 0.0000 0.5878
vn -1.0000 0.0000 0.0000
vn 0.0000 1.0000 0.0000
vn -0.8090 0.0000 -0.5878
vn -0.3090 0.0000 -0.9511
vn 0.0000 -1.0000 0.0000
usemtl None
s off
f 2/1/1 3/2/1 1/3/1
f 4/4/2 5/5/2 3/2/2
f 6/6/3 7/7/3 5/5/3
f 8/8/4 9/9/4 7/7/4
f 10/10/5 11/11/5 9/9/5
f 12/12/6 13/13/6 11/11/6
f 14/14/7 15/15/7 13/13/7
f 16/16/8 17/17/8 15/15/8
f 2/18/9 18/19/9 10/20/9
f 18/21/10 19/22/10 17/17/10
f 20/23/11 1/24/11 19/22/11
f 7/25/12 11/26/12 15/27/12
f 2/1/1 4/4/1 3/2/1
f 4/4/2 6/6/2 5/5/2
f 6/6/3 8/8/3 7/7/3
f 8/8/4 10/10/4 9/9/4
f 10/10/5 12/12/5 11/11/5
f 12/12/6 14/14/6 13/13/6
f 14/14/7 16/16/7 15/15/7
f 16/16/8 18/21/8 17/17/8
f 6/28/9 4/29/9 2/18/9
f 2/18/9 20/30/9 18/19/9
f 18/19/9 16/31/9 14/32/9
f 14/32/9 12/33/9 18/19/9
f 12/33/9 10/20/9 18/19/9
f 10/20/9 8/34/9 6/28/9
f 6/28/9 2/18/9 10/20/9
f 18/21/10 20/23/10 19/22/10
f 20/23/11 2/35/11 1/24/11
f 19/36/12 1/37/12 3/38/12
f 3/38/12 5/39/12 7/25/12
f 7/25/12 9/40/12 11/26/12
f 11/26/12 13/41/12 15/27/12
f 15/27/12 17/42/12 19/36/12
f 19/36/12 3/38/12 15/27/12
f 3/38/12 7/25/12 15/27/12
)";

	LoadObj();
}

Cylinder::~Cylinder() {

}


Line::Line() {

	rawData = R"(
o line
v 0.000000 0.000100 -0.500000
v 0.000000 0.000100 0.500000
v 0.000087 -0.000050 -0.500000
v 0.000087 -0.000050 0.500000
v -0.000087 -0.000050 -0.500000
v -0.000087 -0.000050 0.500000
vt 1.000000 1.000000
vt 0.666667 0.500000
vt 1.000000 0.500000
vt 0.666667 1.000000
vt 0.333333 0.500000
vt 0.457846 0.130000
vt 0.250000 0.490000
vt 0.042154 0.130000
vt 0.333333 1.000000
vt -0.000000 0.500000
vt 0.750000 0.490000
vt 0.957846 0.130000
vt 0.542154 0.130000
vt -0.000000 1.000000
vn 0.8660 0.5000 0.0000
vn -0.0000 -1.0000 -0.0000
vn 0.0000 0.0000 1.0000
vn -0.8660 0.5000 0.0000
vn 0.0000 0.0000 -1.0000
usemtl None
s off
f 2/1/1 3/2/1 1/3/1
f 4/4/2 5/5/2 3/2/2
f 4/6/3 2/7/3 6/8/3
f 6/9/4 1/10/4 5/5/4
f 1/11/5 3/12/5 5/13/5
f 2/1/1 4/4/1 3/2/1
f 4/4/2 6/9/2 5/5/2
f 6/9/4 2/14/4 1/10/4
)";

	LoadObj();
}

Line::~Line() {

}

Particle::Particle() {
	// Exported from Blender a cube by default (OBJ File)
	rawData = R"(
o Object.1
v -0.134911 -0.156052 -0.126000
v -0.134881 0.155717 -0.125999
v 0.135104 -0.000193 -0.126001
v -0.044894 -0.000177 0.126002

usemtl Turquoise_0
f 1 2 3
f 1 3 4
f 2 4 3
f 1 4 2)";

	LoadObj();
}

Particle::~Particle() {

}

Boid::Boid() {
	// Exported from Blender a cube by default (OBJ File)
	rawData = R"(
o Object.1
v -0.2210637 -0.2000062 0.1999999
v -0.2210639 -0.2000062 -0.1999999
v 0.4789539 0.0000198 -0.0000203
v -0.2210636 0.2000061 0.1999999
v -0.2210638 0.2000061 -0.1999999

usemtl Yellow_0
f 1 2 3
f 4 2 1
f 1 3 4
f 2 5 3
f 5 4 3
f 5 2 4)";

	LoadObj();
}

Boid::~Boid() {

}

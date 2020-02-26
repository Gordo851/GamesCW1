#include "boidFlock.h"


boidFlock::boidFlock() {

}

void boidFlock::addBoid(Boid b) {

	//boidList.push_back(b);
}

void boidFlock::removeBoid(Boid b) {
	//does nothing
}

/**
update three lists that hold all boid positions, velocity, and colour
*/
void boidFlock::updatePositions() {


	boidPosition.clear();
	boidVelocity.clear();
	boidColour.clear();
	for (int i = 0; i < boidList.size(); i++) {
		Boid& boidI = *boidList[i];
		boidI.fillColor = glm::vec4(boidI.colour, 1.0f);

		//glm::vec3 pos = glm::vec3(boidI.w_matrix[3][0], boidI.w_matrix[3][1], boidI.w_matrix[3][2]);
		boidPosition.push_back(boidI.possition);
		boidVelocity.push_back(boidI.velocity);
		boidColour.push_back(boidI.colour);
	}
}

/**
steer boid at index i
*/
void boidFlock::steer(int i) {
	Boid& boidI = *boidList[i];

	float range = boidI.range * boidI.range; // square
	float minDiss = boidI.minDist * boidI.minDist; // square

	float maxSpeed = boidI.speed; 
	//weights for boids three streering 
	float alignValue = 0.5; //move in same direction as other boids
	float cohesionValue = 1; // move to center of boid flock
	float seperationValue = 2.0; // dont move to close to center of boid flock

	glm::vec3 alignSteering = glm::vec3(0.0f);
	glm::vec3 cohesionSteering = glm::vec3(0.0f);
	glm::vec3 SepSteering = glm::vec3(0.0f);
	glm::vec3 averageColour = boidI.colour;


	int indexcounter = i;//used to set colour of flock

	int count = 0;//count number of boids in range
	int countSep = 0; // ount nmber of boids too close
	for (int j = 0; j < boidList.size(); j++) {
		if (i != j) {
			float diss = getDistance(i, j);//returns square diss
			if (diss < range) {//if boid is within range - i.e. boid can be "seen" by this boid
				if (diss < minDiss) { // if boid is too close
					countSep++;
					glm::vec3 diff = boidPosition[i] - boidPosition[j];
					diff /= diss;
					SepSteering += diff;
				}
				alignSteering += boidVelocity[j]; // add t osum of velocitys to take average
				cohesionSteering += boidPosition[j]; //get average position of boids that can be seen
				averageColour += boidColour[j]; // get average colour of boids that can be seen
				indexcounter += j; // get sum of index - used to calcualte the target colour of the flock
				count++;
			}
		}
	}
	if (countSep > 0) {
		SepSteering /= countSep;
		if (SepSteering != glm::vec3(0.0)) {
			SepSteering = glm::normalize(SepSteering);
		}
		SepSteering -= boidVelocity[i];
	}
	if (count > 0) {
		cohesionSteering = cohesionSteering / (float)count;
		cohesionSteering -= boidPosition[i];
		if (cohesionSteering != glm::vec3(0.0)) {
			cohesionSteering = glm::normalize(cohesionSteering);
		}
		cohesionSteering -= boidVelocity[i];

		alignSteering = alignSteering / (float)count;
		if (alignSteering != glm::vec3(0.0)) {
			alignSteering = glm::normalize(alignSteering);
		}
		alignSteering -= boidVelocity[i];

		
		//set colour target
		indexcounter = indexcounter % 9;
		float r = 0, g = 0, b = 0;
		if (indexcounter == 1) {
			r = 1.0;

		}else if (indexcounter == 6) {
			g = 1.0;
		}
		else if (indexcounter == 4) {
			b = 1.0;
		}else if (indexcounter == 8) {
			r = 0.5;
			g = 0.5;
		}
		else if (indexcounter == 5) {
			g = 0.5;
			b = 0.5;
		}
		else if (indexcounter == 2) {
			b = 0.5;
			r = 0.5;
		}
		else if (indexcounter == 3) {
			g = 0.3;
			b = 0.7;
			r = 0.3;
		}
		else if (indexcounter == 7) {
			b = 0.3;
			r = 0.3;
			g = 0.7;
		}
		else if (indexcounter == 0) {
			b = 0.3;
			r = 0.7;
			g = 0.3;
		}	

		glm::vec3 newcolour = glm::vec3(r, g, b);
		newcolour = newcolour * 0.4f + 0.6f * averageColour / (float)count;
		boidI.colour -= 0.01f* maxSpeed * (boidI.colour - (newcolour)); //change colour slowly to flock colour 
		
	}
	glm::vec3 v;
	if (useTarget == true) {//add target heading
		glm::vec3 targetSteer = target - boidPosition[i];
		if (targetSteer != glm::vec3(0.0)) {
			targetSteer = glm::normalize(targetSteer);
		}
		targetSteer = targetSteer * maxSpeed;
		targetSteer -= boidVelocity[i];
		v = boidI.velocity + 0.1f  * ((alignSteering * alignValue + cohesionSteering * cohesionValue + SepSteering * seperationValue) + targetSteer);
	}
	else {// dont use target 

		v = boidI.velocity + 0.1f  * ((alignSteering * alignValue + cohesionSteering * cohesionValue + SepSteering * seperationValue) );
	}
	
	
	if (v != glm::vec3(0.0)){
		v = glm::normalize(v);
	}

	///calc angle to rotate boid
	float angle = std::atan2(v.y,v.x);
	boidI.rotation.z = angle;
	double angleZ = -std::asin(v.z);
	boidI.rotation.y = angleZ;

	boidI.velocity = v * maxSpeed ; // set speed
}
//get square distance
float boidFlock::getDistance(int i, int j) {
	return (boidPosition[i].x - boidPosition[j].x) * (boidPosition[i].x - boidPosition[j].x) + (boidPosition[i].y - boidPosition[j].y) * (boidPosition[i].y - boidPosition[j].y) + (boidPosition[i].z - boidPosition[j].z) * (boidPosition[i].z - boidPosition[j].z);
}


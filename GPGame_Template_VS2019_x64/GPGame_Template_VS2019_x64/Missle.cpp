#include "Missle.h"
AllMissle::AllMissle() {

}
void AllMissle::updatePositions()
{
	misslePosition.clear();
	boidVelocity.clear();
	boidColour.clear();
	for (int i = 0; i < missleList.size(); i++) {
		Missle& missleI = *missleList[i];
		missleI.fillColor = glm::vec4(missleI.colour, 1.0f);

		//glm::vec3 pos = glm::vec3(boidI.w_matrix[3][0], boidI.w_matrix[3][1], boidI.w_matrix[3][2]);
		misslePosition.push_back(missleI.possition);
		boidVelocity.push_back(missleI.velocity);
		boidColour.push_back(missleI.colour);
	}
}
void AllMissle::steer(int i)
{
	Missle& missleI = *missleList[i];

	float range = missleI.range * missleI.range; // square
	float minDiss = missleI.minDist * missleI.minDist; // square

	float maxSpeed = missleI.speed;
	//weights for boids three streering 
	float alignValue = 0.5; //move in same direction as other boids
	float cohesionValue = 1; // move to center of boid flock
	float seperationValue = 2.0; // dont move to close to center of boid flock

	glm::vec3 alignSteering = glm::vec3(0.0f);
	glm::vec3 cohesionSteering = glm::vec3(0.0f);
	glm::vec3 SepSteering = glm::vec3(0.0f);
	glm::vec3 averageColour = missleI.colour;


	int indexcounter = i;//used to set colour of flock

	int count = 0;//count number of boids in range
	int countSep = 0; // ount nmber of boids too close
	for (int j = 0; j < missleList.size(); j++) {
		if (i != j) {
			float diss = getDistance(i, j);//returns square diss
			if (diss < range) {//if boid is within range - i.e. boid can be "seen" by this boid
				if (diss < minDiss) { // if boid is too close
					countSep++;
					glm::vec3 diff = misslePosition[i] - misslePosition[j];
					diff /= diss;
					SepSteering += diff;
				}
				alignSteering += boidVelocity[j]; // add t osum of velocitys to take average
				cohesionSteering += misslePosition[j]; //get average position of boids that can be seen
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
		cohesionSteering -= misslePosition[i];
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

		}
		else if (indexcounter == 6) {
			g = 1.0;
		}
		else if (indexcounter == 4) {
			b = 1.0;
		}
		else if (indexcounter == 8) {
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
		missleI.colour -= 0.01f * maxSpeed * (missleI.colour - (newcolour)); //change colour slowly to flock colour 

	}
	glm::vec3 v;
	if (missleI.useTarget == true) {//add target heading
		glm::vec3 targetSteer = target - misslePosition[i];
		if (targetSteer != glm::vec3(0.0)) {
			targetSteer = glm::normalize(targetSteer);
		}
		targetSteer = targetSteer * maxSpeed;
		targetSteer -= boidVelocity[i];
		v = missleI.velocity + 0.1f * ((alignSteering * alignValue + cohesionSteering * cohesionValue + SepSteering * seperationValue) + (0.3f * targetSteer));
	}
	else {// dont use target 

		v = missleI.velocity + 0.1f * ((alignSteering * alignValue + cohesionSteering * cohesionValue + SepSteering * seperationValue));
	}


	if (v != glm::vec3(0.0)) {
		v = glm::normalize(v);
	}

	///calc angle to rotate boid
	float angle = std::atan2(v.y, v.x);
	missleI.rotation.z = angle;
	double angleZ = -std::asin(v.z);
	missleI.rotation.y = angleZ;

	missleI.velocity = v * maxSpeed; // set speed
}
float AllMissle::getDistance(int i, int j)
{
	return (misslePosition[i].x - misslePosition[j].x) * (misslePosition[i].x - misslePosition[j].x) + (misslePosition[i].y - misslePosition[j].y) * (misslePosition[i].y - misslePosition[j].y) + (misslePosition[i].z - misslePosition[j].z) * (misslePosition[i].z - misslePosition[j].z);
}


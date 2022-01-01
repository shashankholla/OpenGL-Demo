
/*
Author: Shashank K Holla
Class: ECE 6122
Last Date Modified: 11/29/2021

Description: Each of the 15 UAVs is created using the ECE_UAV class, and their motion is controlled in this file.

*/

#include "ECE_UAV.hpp"


extern glm::vec3 spherePosition;


extern int INITIAL_WAIT;
extern int KINAMATICS_REFRESH;
extern float END_DEMO;
extern float POLL_THREAD;

extern float SPHERE_RADIUS;

extern std::atomic_bool stopUAVS;


/*
Given a start and end, return a random number between it
@param start - starting int
@param end - ending int
@return - random float

*/
float gen_random_float(int start, int end)
{
	static std::default_random_engine e;
	static std::uniform_real_distribution<> dis(start, end);
	return dis(e);
}


//Use the ModelMatrix to get the current position
void ECE_UAV::setCurrentPos() {
	currentPos = glm::vec3(this->modelMatrix[3][0], this->modelMatrix[3][1], this->modelMatrix[3][2]);
}

// Calculate new position from the velocity vector.
void ECE_UAV::calculatePos() {

	this->UAVMutex.lock();
	currentPos = glm::vec3(this->modelMatrix[3][0], this->modelMatrix[3][1], this->modelMatrix[3][2]);

	if (glm::length(spherePosition - currentPos) > SPHERE_RADIUS)
	{
		velocity = spherePosition - currentPos;
		this->modelMatrix = glm::translate(this->modelMatrix, (spherePosition - currentPos) / this->initSpeed);
	}
	else {
		normal = spherePosition - currentPos;

		//Randomize the velocity vector if UAV was constant
		if (!flag) {
			velocity.x += (gen_random_float(0, 1)) * 2 - 1;
			velocity.z += (gen_random_float(0, 1)) * 2 - 1;
			velocity.y += (gen_random_float(0, 1)) * 2 - 1;
			//printf("%f %f %f\n", velocity.x, velocity.y, velocity.z);
			flag = true;
		}

		// Find the dot product of the velocity vector and normal and subtract it from
		// the velocity vector to get the tangential component.
		float dotproduct = glm::dot(velocity, normal);
		float modNormal = glm::length(normal);
		glm::vec3 dotted = normal * (dotproduct) / (modNormal * modNormal);
		tangential = (velocity - dotted)*this->mass; // F = m*a
		if (glm::length(tangential) < 0.1f) {
			flag = false;
		}
		
		this->modelMatrix = glm::translate(this->modelMatrix, velocity / 10.0f);
	}
		this->UAVMutex.unlock();
}

// Start the threads
void ECE_UAV::start() {
	this->thisThread = std::thread(threadFunction, this);
}


//Compute new position 
void threadFunction(ECE_UAV* pUAV) {

	pUAV->setCurrentPos();
	std::this_thread::sleep_for(std::chrono::seconds(INITIAL_WAIT));
	while (!stopUAVS) {
		pUAV->calculatePos();
		std::this_thread::sleep_for(std::chrono::milliseconds(KINAMATICS_REFRESH));
	}
}
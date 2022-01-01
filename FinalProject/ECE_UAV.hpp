
/*
Author: Shashank K Holla
Class: ECE 6122
Last Date Modified: 11/29/2021

Description: Each of the 15 UAVs is created using the ECE_UAV class, and their motion is controlled in this file.

*/



#include<random>
#include <glm/glm.hpp>
#include<thread>
#include <glm/gtc/matrix_transform.hpp>
#include<atomic>
#include<mutex>

float gen_random_float(int, int);


//Custom struct to handle the object
class ECE_UAV {
public:
	float mass; //mass of the object
	
	bool flag = false; //flag to control velocity randomisation
	bool collided = false;
	int collidedWith = -1;

	glm::vec3 pos; //set current position
	glm::vec3 acc; //set current acceleration 
	glm::vec3 velocity = glm::vec3(0, 0, 0); //set current velocity

	glm::mat4 modelMatrix; //load the position model matrix for drawing
	glm::mat4 MVPMatrix;
	
	glm::vec3 normal; //calculate the normal vector to the sphere
	glm::vec3 tangential; // //calculate the tangetial vector to the sphere
	glm::vec3 currentPos;//update current position
	
	//Lock the variables to make it threadsafe
	std::recursive_mutex UAVMutex;

	float initSpeed = 0.0f;
	
	std::thread thisThread;
	
	void setCurrentPos();
	void calculatePos();
	void start();
	
};

void threadFunction(ECE_UAV* pUAV);

/*
Author: Shashank K Holla
Class: ECE 6122
Last Date Modified: 11/29/2021

Description: The launch point for the ECE6122 Project. This has a demo of 15 UAVs that fly over a 
football field towards a virtual sphere.

References:
http://www.opengl-tutorial.org/  - https://github.com/opengl-tutorials/ogl
https://learnopengl.com/
*/



// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
#include <common/controls.hpp>
#include <thread>
#include <random>


#include "ECE_UAV.hpp"
#include "Object.hpp"

class ECE_UAV;

float cameraX = 0;
float cameraZ = 5;

glm::vec3 spherePosition;
glm::vec3 lightPos = glm::vec3(4.0f, 4.0f, 4.0f);
glm::vec3 lightPosField = glm::vec3(4.0f, 4.0f, 4.0f);

std::atomic_bool stopUAVS = false;
void threadFunction(ECE_UAV*);

int INITIAL_WAIT = 6; //seconds
int KINAMATICS_REFRESH = 10; //milliseconds
float END_DEMO = 60; //seconds
float POLL_THREAD = 30e-3; //seconds

float SPHERE_RADIUS = 0.6f;


int setupOpenGl(int width, int height, char* title) {
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// Open a window and create its OpenGL context
	window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// Hide the mouse and enable unlimited mouvement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// Set the mouse at the center of the screen
	glfwPollEvents();
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);
}


int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}
	
	//Create a window of given size and title
	setupOpenGl(1024, 768, "UAV");
	
	//Set the background color to light blue
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
	
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);
	
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);


	//Create object structs for each object
	Object suzzy;
	Object field;
	Object sphere;
	
	
	//Control the transparency of the field using this
	GLuint TrasparencyIDField = glGetUniformLocation(field.programID, "Transparency");


	createObject("assets/obj/suzanne.obj", "assets/textures/uvmap.DDS", 1, suzzy, "assets/shaders/StandardShading.vertexshader", "assets/shaders/StandardShading.fragmentshader");
	createObject("assets/obj/field.obj", "assets/textures/ff.bmp", 0, field, "assets/shaders/StandardShading.vertexshader", "assets/shaders/StandardShading.fragmentshader");
	createObject("assets/obj/sphere.obj", "assets/textures/ff.bmp", 0, sphere, "assets/shaders/StandardShading.vertexshader", "assets/shaders/Red.fragmentshader");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glm::vec3 transparency = glm::vec3(0.1f, 0.1f, 0.1f);
	
	
	// For speed computation
	double lastTime = glfwGetTime();
	double lastTimeT = glfwGetTime();
	int nbFrames = 0;
	bool flip = true;

	//Location of the sphere
	spherePosition = glm::vec3(0.0f, 2.0f, 0.0f);
	
	
	float x, y;
	int NUMBEROFMODELS = 15;
	std::vector<glm::mat4> ModelMatrices = std::vector<glm::mat4>(NUMBEROFMODELS, glm::mat4(1.0));
	std::vector<glm::mat4> MVPMatricies = std::vector<glm::mat4>(NUMBEROFMODELS);
	std::vector<ECE_UAV*> myUAVS;
	
	//Sete the initial position of the UAVs
	for (int i = 0; i < NUMBEROFMODELS; i++) {
		x = i % 5 - 2;
		y = i % 3 - 1;
		ECE_UAV* newUAV = new ECE_UAV;
		newUAV->mass = 1;
		ModelMatrices[i] = glm::translate(ModelMatrices[i], glm::vec3(x, 0.0f, y));
		ModelMatrices[i] = glm::scale(ModelMatrices[i], glm::vec3(0.05f, 0.05f, 0.05f));
		newUAV->modelMatrix = ModelMatrices[i];
		newUAV->initSpeed = (gen_random_float(0, 1)*2+3.0f);
		newUAV->setCurrentPos();
		myUAVS.push_back(newUAV);
		
		//Start the respective threads
		myUAVS[i]->start();
	}


	//Start the forever loop

	do {
		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;

		if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1sec ago
			// printf and reset
			printf("%f ms/frame\n", 1000.0 / double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;

		}

		if (currentTime - lastTimeT >= POLL_THREAD) {
			lastTimeT += 0.03;

			//Control the transparency of the UAVs
			if (flip) {
				transparency.x += 0.05;

				if (transparency.x > 1) {
					flip = false;
				}

			}

			else {
				transparency.x -= 0.05;

				if (transparency.x < 0.5) {
					flip = true;
				}
			}

			//Collision detection 
			for (int i = 0; i < NUMBEROFMODELS; i++) {
				for (int j = 0; j < NUMBEROFMODELS; j++) {
					if (i != j) {
						ECE_UAV* modelA = myUAVS[i];
						ECE_UAV* modelB = myUAVS[j];

						
						//If the distance of the centers is less than 0.1, then collided!
						//Compute the reflected 
						if (glm::length(modelA->currentPos - modelB->currentPos) < 0.1f) {
							
							if (modelA->collided || modelB->collided) {
								continue;
							}

							glm::vec3 normalBtoA = modelA->currentPos - modelB->currentPos;
							glm::vec3 normalAtoB = modelB->currentPos - modelA->currentPos;
							modelA->velocity = glm::reflect(modelA->velocity, normalBtoA);
							modelB->velocity = glm::reflect(modelB->velocity, normalAtoB);
							modelA->collided = true;
							modelB->collided = true;
							modelA->collidedWith = j;
							modelB->collidedWith = i;
						}
						else {
							if ((modelA->collidedWith == j) && (modelB->collidedWith == i)) {
								modelA->collided = false;
								modelB->collided = false;
							}
							
						}
					}
				}
			}
		}


		glm::mat4 ViewMatrix;
		
		//Camera effects based on time
		if (((currentTime) < 4) && ((currentTime) > 0)) {
				cameraX -= 0.005;
				ViewMatrix = glm::lookAt(glm::vec3(0.0f, 6.0f + cameraX, 0.1f), glm::vec3(0.0f, 0.0f + cameraX, 0.0f), glm::vec3(0, 1, 0));
		}

		//Camera effects based on time
		if ( ((currentTime) < 6) && ((currentTime) > 4)) {
			cameraX -= 0.005;
			ViewMatrix = glm::lookAt(glm::vec3(4.0f+cameraX, 3.0f, -3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0, 1, 0));
		}
		//Camera effects based on time
		if (((currentTime) > 6) && ((currentTime) < 6.0001)) {
			cameraX = 1;
		}

		//Camera effects based on time
		if (((currentTime) > 6.0001) && ((currentTime) < 15.01)) {
			cameraX -= 0.005;
			ViewMatrix = glm::lookAt(glm::vec3(8.0f + cameraX, 5.0f, -3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0, 1, 0));
		}

		//Camera effects based on time
		if (((currentTime) > 15.01)) {
			cameraZ -= 0.005;
			ViewMatrix = glm::lookAt(glm::vec3(5.0f, 4.0f, 3.0f+cameraZ), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0, 1, 0));
			//lightPosField -= glm::vec3(0.0005f, 0, 0);
		}

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		

		//Set lights for UAVs
		glUseProgram(suzzy.programID);
		lightPos = glm::vec3(4, 4, 4);
		glUniform3f(suzzy.LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniformMatrix4fv(suzzy.ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"
		glUniform3f(suzzy.TrasparencyID, transparency.x, transparency.x, transparency.x);


		glUseProgram(suzzy.programID);
		glm::mat4 ModelMatrix1 = glm::mat4(1.0);
		glm::mat4 MVP1 = ProjectionMatrix * ViewMatrix * ModelMatrix1;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(suzzy.MatrixID, 1, GL_FALSE, &MVP1[0][0]);
		glUniformMatrix4fv(suzzy.ModelMatrixID, 1, GL_FALSE, &ModelMatrix1[0][0]);

		// Bind texture for UAV
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, suzzy.Texture);
		glUniform1i(suzzy.TextureID, 0);


		
		
		//Draw Suzzies
		for (int i = 0; i < NUMBEROFMODELS; i++) {
			x = i % 5 - 2;
			y = i % 3 - 1;

			myUAVS[i]->UAVMutex.lock();
			
			myUAVS[i]->MVPMatrix = ProjectionMatrix * ViewMatrix * myUAVS[i]->modelMatrix;
			glUniformMatrix4fv(suzzy.MatrixID, 1, GL_FALSE, &myUAVS[i]->MVPMatrix[0][0]);
			glUniformMatrix4fv(suzzy.ModelMatrixID, 1, GL_FALSE, &myUAVS[i]->modelMatrix[0][0]);
			
			myUAVS[i]->UAVMutex.unlock();
			
			drawObject(suzzy);
		}



		// Draw Field
		glUseProgram(field.programID);
		glUniform3f(field.LightID, lightPosField.x, lightPosField.y, lightPosField.z);
		glUniform3f(field.TrasparencyID, 1.0f, 1.0f, 1.0f);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, field.Texture);
		glUniform1i(field.TextureID, 1);
		modifyObject(field, glm::vec3(-0.1f, -0.1f, 0.0f), 3.14f / 2, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.32f, 1.0f, 1.37f), ProjectionMatrix, ViewMatrix);
		drawObject(field);


		// Draw Sphere
		glUseProgram(sphere.programID);
		glUniform3f(sphere.LightID, lightPosField.x, lightPosField.y, lightPosField.z);
		glUniform1i(sphere.TextureID, 1);
		modifyObject(sphere, spherePosition, float(currentTime / 3.14f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.15f, 0.15f, 0.15f), ProjectionMatrix, ViewMatrix);
		drawObject(sphere);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		if (currentTime > END_DEMO) {
			stopUAVS = true;
			break;
		}

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);
	
	stopUAVS = true;

	for (int i = 0; i < NUMBEROFMODELS; i++) {
		if (myUAVS[i]->thisThread.joinable()) {
			myUAVS[i]->thisThread.join();
		}
	}


	// Cleanup VBO and shader
	glDeleteVertexArrays(1, &VertexArrayID);
	cleanup(suzzy);
	cleanup(field);
	cleanup(sphere);
	
	// Close OpenGL window and terminate GLFW
	glfwTerminate();
	return 0;
}

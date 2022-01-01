/*
Author: Shashank K Holla
Class: ECE 6122
Last Date Modified: 11/29/2021

Description: To make objects easier to create, maintain and destroy, these helper
functions are written.

*/



#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include<random>
#include <glm/glm.hpp>
#include<thread>
#include <glm/gtc/matrix_transform.hpp>
#include<atomic>


struct Object {
	GLuint programID;
	GLuint MatrixID;
	GLuint ViewMatrixID;
	GLuint ModelMatrixID;
	GLuint TrasparencyID;
	GLuint Texture;
	GLuint TextureID;
	GLuint LightID;
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	std::vector<unsigned short> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	GLuint vertexbuffer;
	GLuint uvbuffer;
	GLuint normalbuffer;
	GLuint elementbuffer;
};

void cleanup(Object& obj);
void createObject(char* pathToObj, char* pathToTexture, bool dds, Object& obj, char* vertexShader, char* fragShader);
void modifyObject(Object& obj, glm::vec3 translation, float rotation, glm::vec3 rotationAxis, glm::vec3 scale, glm::mat4 ProjectionMatrix, glm::mat4 ViewMatrix);

void drawObject(Object& obj);
/*
Author: Shashank K Holla
Class: ECE 6122
Last Date Modified: 11/29/2021

Description: To make objects easier to create, maintain and destroy, these helper
functions are written.

*/




#include "Object.hpp"
#include <common/shader.hpp>
#include <common/texture.hpp>

#include <common/objloader.hpp>
#include <common/vboindexer.hpp>


//Delete all buffers of a given object
void cleanup(Object& obj) {
	glDeleteBuffers(1, &obj.vertexbuffer);
	glDeleteBuffers(1, &obj.uvbuffer);
	glDeleteBuffers(1, &obj.normalbuffer);
	glDeleteBuffers(1, &obj.elementbuffer);
	glDeleteProgram(obj.programID);
	glDeleteTextures(1, &obj.Texture);
}


//Create an objejct from its obj, texture, shaders
void createObject(char* pathToObj, char* pathToTexture, bool dds, Object& obj, char* vertexShader, char* fragShader) {
	
	obj.programID = LoadShaders(vertexShader, fragShader);
	// Get a handle for our "MVP" uniform
	obj.MatrixID = glGetUniformLocation(obj.programID, "MVP");
	obj.ViewMatrixID = glGetUniformLocation(obj.programID, "V");
	obj.ModelMatrixID = glGetUniformLocation(obj.programID, "M");
	obj.TrasparencyID = glGetUniformLocation(obj.programID, "Transparency");
	if (dds)
	{
		obj.Texture = loadDDS(pathToTexture);
	}
	else
	{
		obj.Texture = loadBMP_custom(pathToTexture);
	}
	// Get a handle for our "myTextureSampler" uniform
	obj.TextureID = glGetUniformLocation(obj.programID, "myTextureSampler");
	// Read our .obj file
	bool res = loadOBJ(pathToObj, obj.vertices, obj.uvs, obj.normals);

	indexVBO(obj.vertices, obj.uvs, obj.normals, obj.indices, obj.indexed_vertices, obj.indexed_uvs, obj.indexed_normals);

	glGenBuffers(1, &obj.vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, obj.vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, obj.indexed_vertices.size() * sizeof(glm::vec3), &obj.indexed_vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &obj.uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, obj.uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, obj.indexed_uvs.size() * sizeof(glm::vec2), &obj.indexed_uvs[0], GL_STATIC_DRAW);

	glGenBuffers(1, &obj.normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, obj.normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, obj.indexed_normals.size() * sizeof(glm::vec3), &obj.indexed_normals[0], GL_STATIC_DRAW);
	// Generate a buffer for the indices as well
	glGenBuffers(1, &obj.elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj.indices.size() * sizeof(unsigned short), &obj.indices[0], GL_STATIC_DRAW);
	glUseProgram(obj.programID);
	obj.LightID = glGetUniformLocation(obj.programID, "LightPosition_worldspace");
}


//Change the position, rotation and scale of an object
void modifyObject(Object& obj, glm::vec3 translation, float rotation, glm::vec3 rotationAxis, glm::vec3 scale, glm::mat4 ProjectionMatrix, glm::mat4 ViewMatrix) {
	glm::mat4 ModelMatrix4 = glm::mat4(1.0);
	ModelMatrix4 = glm::translate(ModelMatrix4, translation);
	ModelMatrix4 = glm::rotate(ModelMatrix4, rotation, rotationAxis);
	ModelMatrix4 = glm::scale(ModelMatrix4, scale); //width, height, length
	glm::mat4 MVP4 = ProjectionMatrix * ViewMatrix * ModelMatrix4;
	glUniformMatrix4fv(obj.ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
	glUniformMatrix4fv(obj.MatrixID, 1, GL_FALSE, &MVP4[0][0]);
	glUniformMatrix4fv(obj.ModelMatrixID, 1, GL_FALSE, &ModelMatrix4[0][0]);

}

void drawObject(Object& obj) {
	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, obj.vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, obj.uvbuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	// 3rd attribute buffer : normals
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, obj.normalbuffer);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	// Index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.elementbuffer);
	// Draw the triangles !
	glDrawElements(GL_TRIANGLES, obj.indices.size(), GL_UNSIGNED_SHORT, (void*)0);
}
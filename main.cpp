#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <shader.hpp>

#include <iostream>
#include <cstdlib>
#include <vector>
#include <fstream>
#include <sstream>
#include <math.h>
#include <chrono>
//#include <stdexcept>

const int width = 1280;
const int height = 720;

const std::string MODEL_PATH = "models/cube.obj";
const std::string MATERIAL_DIR = "models";
const std::string TEXTURE_PATH = "tex";

class ModelViewer {
public:
	void run() {
		loadModel();
		initialize();
		
		mainLoop();
		cleanup();
	}
private:

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> mats;

	GLFWwindow* window;
	GLuint VertexArrayID;
	std::vector<GLuint> vertexbuffer;
	std::vector <GLuint> uvbuffer;
	GLuint programID;
	GLuint matrixID;
	GLuint *textures;
	std::vector<GLfloat> vertices;
	std::vector<std::vector<GLfloat>> materialVertices;
	std::vector<GLfloat> texCoords;
	std::vector<unsigned int> indices;

	glm::mat4 ProjectionMatrix;
	glm::mat4 ViewMatrix;
	glm::mat4 ModelMatrix;
	glm::mat4 MVP;

	void mainLoop() {
		do {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glUseProgram(programID);

			// 1st attribute buffer : vertices
			glEnableVertexAttribArray(0);
			for (auto buffer : vertexbuffer) {
				glBindBuffer(GL_ARRAY_BUFFER, buffer);
				glVertexAttribPointer(
					0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
					3,                  // size
					GL_FLOAT,           // type
					GL_FALSE,           // normalized?
					0,                  // stride
					(void*)0            // array buffer offset
				);
			}
			glEnableVertexAttribArray(1);
			for (auto buffer : uvbuffer) {
				glBindBuffer(GL_ARRAY_BUFFER, buffer);
				glVertexAttribPointer(
					1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
					2,                                // size
					GL_FLOAT,                         // type
					GL_FALSE,                         // normalized?
					0,                                // stride
					(void*)0                          // array buffer offset
				);
			}

			//computeMatricesFromInputs();
			updateMatrices();
			MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

			glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);

			glDrawArrays(GL_TRIANGLES, 0, vertices.size()); 
			//glDrawArrays(GL_TRIANGLES, 0, vertices.size()); 
			glDisableVertexAttribArray(0);

			glfwSwapBuffers(window);
			glfwPollEvents();

		}

		while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

	}

	void updateMatrices() {
		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();

		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
	
		ModelMatrix = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ProjectionMatrix = glm::perspective(glm::radians(45.0f), width / (float)height, 0.1f, 100.0f);;
		ViewMatrix = glm::lookAt(glm::vec3(3.0f, 3.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));;
	}

	/*void computeMatricesFromInputs() {	
		double currentTime = glfwGetTime();
		float deltaTime = float(currentTime - lastTime);
		lastTime = currentTime;

		//glfwSetCursorPos(window, width / 2.0, height / 2.0);

		horizontalAngle += mouseSpeed * deltaTime * float(width / 2 - xpos);
		verticalAngle += mouseSpeed * deltaTime * float(height / 2 - ypos);
		

		glm::vec3 direction(
			cos(verticalAngle) * sin(horizontalAngle),
			sin(verticalAngle),
			cos(verticalAngle) * cos(horizontalAngle)
		);

		glm::vec3 right = glm::vec3(
			sin(horizontalAngle - 3.14f / 2.0f),
			0,
			cos(horizontalAngle - 3.14f / 2.0f)
		);

		glm::vec3 up = glm::cross(right, direction);


		ProjectionMatrix = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
		
		ViewMatrix = glm::lookAt(
			position,           // Camera is here
			position + direction, // and looks here : at the same position, plus "direction"
			up                  // Head is up (set to 0,-1,0 to look upside-down)
		);

	}*/

	void loadModel() {
		std::string err;
		//load model data
		if (!tinyobj::LoadObj(&attrib, &shapes, &mats, &err, MODEL_PATH.c_str(), MATERIAL_DIR.c_str())) {
			throw std::runtime_error(err);
		}
	}

	void initialize() {
		glewExperimental = true;
		if (!glfwInit())
		{
			throw std::exception("Failed to initialize GLFW");
		}

		glfwWindowHint(GLFW_SAMPLES, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);


		window = glfwCreateWindow(width, height, "ModelViewer", NULL, NULL);
		if (window == NULL) {
			throw std::exception("Failed to open GLFW window");
			glfwTerminate();
		}

		glfwMakeContextCurrent(window);

		if (glewInit() != GLEW_OK) {
			throw std::exception("Failed to initialize GLEW");
		}

		glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

		

		vertexbuffer.resize(mats.size());
		uvbuffer.resize(mats.size());

		//generate texture array

		textures = new GLuint[shapes.size()];
		glGenTextures(shapes.size(), textures);
		for (int i = 0; i < 1; i++) {
			for (auto index : shapes[i].mesh.indices) {
				
				vertices.push_back(attrib.vertices[3 * index.vertex_index + 0]);
				vertices.push_back(attrib.vertices[3 * index.vertex_index + 1]);
				vertices.push_back(attrib.vertices[3 * index.vertex_index + 2]);
				texCoords.push_back(attrib.texcoords[2 * index.texcoord_index + 0]);
				texCoords.push_back(1.0f - attrib.texcoords[2 * index.texcoord_index + 1]);
			}
			
			glBindTexture(GL_TEXTURE_2D, textures[i]);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

			int texWidth, texHeight, texChannels;
			stbi_uc* pixels = stbi_load((TEXTURE_PATH +"/"+ mats[i].diffuse_texname).c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
			
		}

		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		programID = LoadShaders("VertexShader.vertexshader", "FragmentShader.fragmentshader");



		for (int i = 0; i < vertexbuffer.size(); i++) {
			glGenBuffers(1, &vertexbuffer[i]);
			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[i]);
			//glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

			glGenBuffers(1, &uvbuffer[i]);
			glBindBuffer(GL_ARRAY_BUFFER, uvbuffer[i]);
			//glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);
			glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(GLfloat), texCoords.data(), GL_STATIC_DRAW);
		}
		

		matrixID = glGetUniformLocation(programID, "MVP");

		glEnable(GL_DEPTH_TEST);

		glDepthFunc(GL_LESS);

	}

	void cleanup() {
		delete [] textures;
		
		glDeleteBuffers(vertexbuffer.size(), vertexbuffer.data());
		glDeleteBuffers(uvbuffer.size(), uvbuffer.data());
		glDeleteVertexArrays(1, &VertexArrayID);
		glDeleteProgram(programID);

		glfwTerminate();
	}

};

int main() {

	ModelViewer application;

	try {
		application.run();
	}
	catch (const std::runtime_error &e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;

}
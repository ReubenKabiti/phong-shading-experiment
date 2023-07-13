#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <unistd.h>
#include <iostream>
#include <cmath>
#include "util/util.h"

uint32_t CreateShader(char* vss, char *fss);

static uint32_t gs_iScreenWidth = 800;
static uint32_t gs_iScreenHeight = 600;

static glm::mat4 gs_mProjectionMat;
void WindowSizeChanged(GLFWwindow* window, int w, int h)
{
	uint32_t gs_iScreenWidth = w;
	uint32_t gs_iScreenHeight = h;
	gs_mProjectionMat = glm::perspective((float)M_PI/4.0f, (float)w/h, 0.1f, 1000.0f);
	glViewport(0, 0, w, h);
}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(gs_iScreenWidth, gs_iScreenHeight, "Phong shading", nullptr, nullptr);
	glfwSetWindowSizeCallback(window, WindowSizeChanged);
	glfwMakeContextCurrent(window);

	glewInit();
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, gs_iScreenWidth, gs_iScreenHeight);

	// load the model
	std::pair<std::vector<float>, std::vector<uint32_t>> model = Util::LoadObj("assets/monkey.obj");
	model.first = Util::GenerateNormals(model.first, model.second);
	const char* vss = "#version 330 core\n"
		"layout (location = 0) in vec3 aPos;"
		"layout (location = 1) in vec3 aNormal;"
		"uniform mat4 MVP;"
		"uniform mat4 model;"
		"out vec3 normal;"
		"out vec3 fragPos;"
		"void main() {"
		"	gl_Position = MVP * vec4(aPos, 1);"
		"	fragPos = vec3(model * vec4(aPos, 1));"
		"	normal = aNormal;"
		"}";
	const char* fss = "#version 330 core\n"
		"in vec3 normal;"
		"in vec3 fragPos;"
		"out vec4 color;"
		"uniform vec3 lightPosition;"
		"uniform vec3 cameraPosition;"
		"uniform vec4 lightColor;"
		"uniform vec4 objectColor;"
		"uniform float ambient;"
		"uniform float specular;"
		"uniform float roughness;"
		"void main() {"
		"	vec4 aColor = ambient * objectColor;"
		"	vec3 lightDirection = normalize(lightPosition - fragPos);"
		"	vec4 dColor = (1 - specular) * max(0, dot(normal, lightDirection)) * objectColor;"
		"	vec3 halfVec = reflect(-lightDirection, normal);"
		"	vec3 viewDirection = normalize(cameraPosition - fragPos);"
		"	float spec = pow(max(0, dot(viewDirection, halfVec)), roughness);"
		"	vec4 sColor = specular * spec * objectColor;"
		"	color = aColor + dColor + sColor;"
		"}";

	uint32_t vao, vbo, ebo;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	glBufferData(GL_ARRAY_BUFFER, model.first.size() * sizeof(float), model.first.data(), GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.second.size() * 4, model.second.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), nullptr);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	uint32_t program = CreateShader((char*)vss, (char*)fss);

	glUseProgram(program);

	glm::vec3 cameraPosition(0, 5, 10);
	glm::vec3 objectPosition(0, 0, 0);
	glm::mat4 modelMat(1);
	glm::mat4 viewMat = glm::lookAt(cameraPosition, glm::vec3(0), glm::vec3(0, 1, 0));
	gs_mProjectionMat = glm::perspective((float)M_PI/4.0f, (float)gs_iScreenWidth/gs_iScreenHeight, 0.1f, 1000.0f);

	glm::vec3 lightPosition(3, 1, 0);
	glm::vec4 lightColor(1, 1, 1, 1);
	glm::vec4 objectColor(1, 1, 1, 1);
	float ambient = 0.3f;
	float specular = 0.5f;
	float roughness = 32;
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		modelMat = glm::translate(glm::mat4(1), objectPosition);
		glm::mat4 modelViewProjectionMat = gs_mProjectionMat * viewMat * modelMat;

		// set the uniforms
		int32_t mvpLocation = glGetUniformLocation(program, "MVP");
		glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(modelViewProjectionMat));

		int32_t modelLocation = glGetUniformLocation(program, "model");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(modelMat));

		int32_t lightPositionLocation = glGetUniformLocation(program, "lightPosition");
		glUniform3f(lightPositionLocation, lightPosition.x, lightPosition.y, lightPosition.z);

		int32_t cameraPositionLocation = glGetUniformLocation(program, "cameraPosition");
		glUniform3f(cameraPositionLocation, cameraPosition.x, cameraPosition.y, cameraPosition.z);

		int32_t lightColorLocation = glGetUniformLocation(program, "lightColor");
		glUniform4f(lightColorLocation, lightColor.x, lightColor.y, lightColor.z, lightColor.w);

		int32_t objectColorLocation = glGetUniformLocation(program, "objectColor");
		glUniform4f(objectColorLocation, objectColor.x, objectColor.y, objectColor.z, objectColor.w);

		int32_t ambientLocation = glGetUniformLocation(program, "ambient");
		glUniform1f(ambientLocation, ambient);

		int32_t specularLocation = glGetUniformLocation(program, "specular");
		glUniform1f(specularLocation, specular);

		int32_t roughnessLocation = glGetUniformLocation(program, "roughness");
		glUniform1f(roughnessLocation, roughness);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDrawElements(GL_TRIANGLES, model.second.size(), GL_UNSIGNED_INT, nullptr);
		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
}

uint32_t CreateShader(char* vss, char *fss)
{
	uint32_t vs = glCreateShader(GL_VERTEX_SHADER);
	uint32_t fs = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vs, 1, &vss, nullptr);
	glShaderSource(fs, 1, &fss, nullptr);
	glCompileShader(vs);
	glCompileShader(fs);

	int s;
	char infoLog[512];
	glGetShaderiv(vs, GL_COMPILE_STATUS, &s);
	if (!s)
	{
		glGetShaderInfoLog(vs, 512, nullptr, infoLog);
		std::cout << "Vertex shader failed to compile!\n" << infoLog << std::endl;
	}
	glGetShaderiv(fs, GL_COMPILE_STATUS, &s);
	if (!s)
	{
		glGetShaderInfoLog(fs, 512, nullptr, infoLog);
		std::cout << "Fragment shader failed to compile!\n" << infoLog << std::endl;
	}


	uint32_t program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &s);
	if (!s)
	{
		glGetProgramInfoLog(program, 512, nullptr, infoLog);
		std::cout << "Failed to link program\n" << infoLog << std::endl;
	}
	return program;
}

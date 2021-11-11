
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glad/glad.h> //must be included before glfw 
#include <GLFW/glfw3.h>
#include "header/Shader.h"
#define _USE_MATH_DEFINES
#include <math.h>

void initialize();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
//settings
const unsigned int SCREEN_WIDTH = 800, SCREEN_HEIGHT = 600;

glm::mat4 camera1 = glm::mat4(1.0f);
glm::mat4 camera2 = glm::mat4(1.0f);

bool toggleCamera{ true };


int main() {
    
    initialize();
    //create window
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Procedural Terrain", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    //load opengl functions using glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);


    //Callback functions
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    
    Shader myShader{ "shaders/vertex.vert", "shaders/fragment.frag" };
    myShader.use();
    //glm::mat4 camera1{ 1.0f };// = glm::mat4(1.0f);
    //glm::mat4 camera2{ 1.0f };
    glm::mat4 perspective = glm::perspective(static_cast<float>(M_PI) / 4.0f, static_cast<float>(SCREEN_WIDTH) / SCREEN_HEIGHT, 0.1f, 100.0f);
   
    myShader.setMat4("P", perspective);

    camera2 = glm::rotate(camera2, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    camera2 = glm::translate(camera2, glm::vec3(0.0f, -12.0f, -12.0f));

    camera1 = glm::translate(camera1, glm::vec3(0.0f, 0.0f, -3.0f));
   
    // Create test triangle
    float vertices[] = {
    -1.5f, -1.5f, 0.0f,
     1.5f, -1.5f, 0.0f,
     0.0f,  1.5f, 0.0f
    };

    float cameraPoints[] = {
        //near
        -1.0f, -1.0f, -1.0,
        -1.0f,  1.0f, -1.0,
         1.0f, -1.0f, -1.0,
         1.0f,  1.0f, -1.0,

         //far
        -1.0f, -1.0f,  1.0,
        -1.0f,  1.0f,  1.0,
         1.0f, -1.0f,  1.0,
         1.0f,  1.0f,  1.0,
    };

    //Create vertex buffer object
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO); //bind
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); //add data, GL_DYNAMIC_DRAW if data is changed alot  

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    //Create vertex buffer object
    unsigned int VBOcamera, VAOcamera;
    glGenVertexArrays(1, &VAOcamera);
    glGenBuffers(1, &VBOcamera);
    glBindVertexArray(VAOcamera);

    glBindBuffer(GL_ARRAY_BUFFER, VBOcamera); //bind
    glBufferData(GL_ARRAY_BUFFER, sizeof(cameraPoints), cameraPoints, GL_STATIC_DRAW); //add data, GL_DYNAMIC_DRAW if data is changed alot  

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        myShader.use();

        toggleCamera ? myShader.setMat4("MV", camera1) : myShader.setMat4("MV", camera2);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glBindVertexArray(VAOcamera);
        glDrawArrays(GL_LINE_LOOP, 0, 4);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
	return 0;
}


void initialize() {
    //list of possible settings: https://www.glfw.org/docs/latest/window.html#window_hints
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        camera1 = glm::translate(camera1, glm::vec3(0.0f, 0.0f, 0.005f));
        camera2 = glm::translate(camera2, glm::vec3(0.0f, 0.0f, 0.005f));
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        camera1 = glm::translate(camera1, glm::vec3(0.0f, 0.0f, -0.005f));
        camera2 = glm::translate(camera2, glm::vec3(0.0f, 0.0f, -0.005f));
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        camera1 = glm::translate(camera1, glm::vec3(-0.001f, 0.0f, 0.0f));
        camera2 = glm::translate(camera2, glm::vec3(-0.001f, 0.0f, 0.0f));
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        camera1 = glm::translate(camera1, glm::vec3(0.001f, 0.0f, 0.0f));
        camera2 = glm::translate(camera2, glm::vec3(0.001f, 0.0f, 0.0f));
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        toggleCamera = !toggleCamera;
    }
}
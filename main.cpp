
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glad/glad.h> //must be included before glfw 
#include <GLFW/glfw3.h>
#include "header/Shader.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "header/Model.h"

#include "header/CameraControl.h"

void initialize();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_position_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void printmat4(const glm::mat4& mat);

//settings
const unsigned int SCREEN_WIDTH = 800, SCREEN_HEIGHT = 600;

glm::mat4 camera2 = glm::mat4(1.0f);

bool toggleCamera{ true };

CameraControl camera1Control{ glm::vec3{ 0.0f, 0.0f, -3.0f } };

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
    glfwSetKeyCallback(window, key_callback); //recieve callback when key is pressed or released 
    glfwSetCursorPosCallback(window, mouse_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    Shader myShader{ "shaders/vertex.vert", "shaders/fragment.frag" };
    Shader cameraShader{ "shaders/cameraVertex.vert", "shaders/cameraFragment.frag" };

    myShader.use();

    glm::mat4 perspective = glm::perspective(glm::radians(45.0f), static_cast<float>(SCREEN_WIDTH) / SCREEN_HEIGHT, 0.1f, 30.0f);
    glm::mat4 perspective2 = glm::perspective(glm::radians(45.0f), static_cast<float>(SCREEN_WIDTH) / SCREEN_HEIGHT, 0.1f, 100.0f);
   
    myShader.setMat4("P", perspective2);
    cameraShader.use();
    cameraShader.setMat4("P", perspective2);

    camera2 = glm::translate(camera2, glm::vec3(0.0f, -5.0f, -25.0f));
    camera2 = glm::rotate(camera2, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
   
    // Create test triangle
    std::vector<Vertex> testVertices;
    Vertex v1, v2, v3;
    v1.position = glm::vec3{ -1.5f, -1.5f, 0.0f };
    v2.position = glm::vec3{ 1.5f, -1.5f, 0.0f };
    v3.position = glm::vec3{ 0.0f,  1.5f, 0.0f };
    testVertices.push_back(v1);
    testVertices.push_back(v2);
    testVertices.push_back(v3);

    std::vector<unsigned int> testIndices;
    testIndices.push_back(0);
    testIndices.push_back(1);
    testIndices.push_back(2);

    Mesh testTriangle{ testVertices, testIndices };

    float cameraPoints[] = {
        //near
        -1.0f, -1.0f,  -1.0f, //1
         1.0f, -1.0f,  -1.0f, //2
         1.0f,  1.0f,  -1.0f, //3
        -1.0f,  1.0f,  -1.0f, //4
        -1.0f, -1.0f,  -1.0f, //1

        //far
        -1.0f, -1.0f,  1.0f, //5
         1.0f, -1.0f,  1.0f, //6
         1.0f,  1.0f,  1.0f, //7
        -1.0f,  1.0f,  1.0f, //8
        -1.0f, -1.0f,  1.0f, //5

        -1.0f,  1.0f,  1.0f, //8
        -1.0f,  1.0f, -1.0f, //4
         1.0f,  1.0f,  -1.0f, //3
         1.0f,  1.0f,  1.0f, //7
         1.0f, -1.0f,  1.0f, //6
         1.0f, -1.0f,  -1.0f, //2
    };


    //Create vertex buffer object for camera
    unsigned int VBOcamera, VAOcamera;
    glGenVertexArrays(1, &VAOcamera);
    glGenBuffers(1, &VBOcamera);
    glBindVertexArray(VAOcamera);

    glBindBuffer(GL_ARRAY_BUFFER, VBOcamera); //bind
    glBufferData(GL_ARRAY_BUFFER, sizeof(cameraPoints), cameraPoints, GL_STATIC_DRAW); //add data, GL_DYNAMIC_DRAW if data is changed alot  

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    /*****************
    *  Render loop   *
    *****************/
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        camera1Control.pollMouse(window);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        myShader.use();

        glm::mat4 camera1 = camera1Control.computeCameraMatrix();

        toggleCamera ? myShader.setMat4("V", camera1) : myShader.setMat4("V", camera2);
        toggleCamera ? myShader.setMat4("P", perspective) : myShader.setMat4("P", perspective2);

        float time = glfwGetTime();
        glm::mat4 modelM = glm::mat4(1.0f);
        modelM = glm::translate(modelM, glm::vec3(3.0f, 0.0f, -4.0f));
        modelM = glm::rotate(modelM, glm::radians(45.0f * time), glm::vec3(0.0f, 1.0f, 0.0f));
        modelM = glm::scale(modelM, glm::vec3(0.5f));

        myShader.setMat4("M", modelM);

        testTriangle.draw(GL_TRIANGLES);

        /*modelM = glm::mat4(1.0f);
        myShader.setMat4("M", modelM);*/
        auto invproj = glm::inverse(perspective * camera1);
        cameraShader.use();
        cameraShader.setMat4("InvP", invproj);
        toggleCamera ? cameraShader.setMat4("V", camera1) : cameraShader.setMat4("V", camera2);
        //cameraShader.setMat4("V", camera1);

        glBindVertexArray(VAOcamera);
        glDrawArrays(GL_LINE_STRIP, 0, 16);

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
        camera1Control.moveCamera(glm::vec3(0.0f, 0.0f, 0.015f));
        camera2 = glm::translate(camera2, glm::vec3(0.0f, 0.0f, 0.015f));
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        camera1Control.moveCamera(glm::vec3(0.0f, 0.0f, -0.015f));
        camera2 = glm::translate(camera2, glm::vec3(0.0f, 0.0f, -0.015f));
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        camera1Control.moveCamera(glm::vec3(-0.01f, 0.0f, 0.0f));
        camera2 = glm::translate(camera2, glm::vec3(-0.01f, 0.0f, 0.0f));
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        camera1Control.moveCamera(glm::vec3(0.01f, 0.0f, 0.0f));
        camera2 = glm::translate(camera2, glm::vec3(0.01f, 0.0f, 0.0f));
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        toggleCamera = !toggleCamera;
    }
    //Reset camera rotation
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        camera1Control.resetCamera();
    }
}

void mouse_position_callback(GLFWwindow* window, double xpos, double ypos) {
    //std::cout << "mouse moved: " << xpos << " " << ypos << '\n';
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        //glfwGetCursorPos(window, &START_MOUSE_X, &START_MOUSE_Y);
        camera1Control.setMouseStartPos(window);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    std::cout << "mouse wheel scrolled " << xoffset << " " << yoffset << '\n';
}

void printmat4(const glm::mat4& mat) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            std::cout << mat[j][i] << " ";
        }
        std::cout << '\n';
    }
}
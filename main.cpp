
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
    glm::mat4 perspective = glm::perspective(glm::radians(45.0f), static_cast<float>(SCREEN_WIDTH) / SCREEN_HEIGHT, 0.1f, 30.0f);
    glm::mat4 perspective2 = glm::perspective(glm::radians(45.0f), static_cast<float>(SCREEN_WIDTH) / SCREEN_HEIGHT, 0.1f, 100.0f);

   
    myShader.setMat4("P", perspective2);

    camera1 = glm::translate(camera1, glm::vec3(0.0f, 0.0f, -3.0f));

    camera2 = glm::translate(camera2, glm::vec3(0.0f, -4.0f, -20.0f));
    camera2 = glm::rotate(camera2, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    //easier to understand matrice printr?
    //for (int i = 0; i < 4; ++i) {
    //    for (int j = 0; j < 4; ++j) {
    //        std::cout << camera2[j][i] << " ";
    //    }
    //    std::cout << '\n';
    //}
   
    // Create test triangle
    float vertices[] = {
    -1.5f, -1.5f, 0.0f,
     1.5f, -1.5f, 0.0f,
     0.0f,  1.5f, 0.0f
    };

    float cameraPoints[] = {
        ////near
        //-1.0f, -1.0f,  -1.0f,
        // 1.0f, -1.0f,  -1.0f,
        // 1.0f,  1.0f,  -1.0f,
        //-1.0f,  1.0f,  -1.0f,
        // //far
        //-1.0f, -1.0f,  1.0f,
        // 1.0f, -1.0f,  1.0f,
        // 1.0f,  1.0f,  1.0f,
        //-1.0f,  1.0f,  1.0f,

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

    //Create vertex buffer object
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO); //bind
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); //add data, GL_DYNAMIC_DRAW if data is changed alot  

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);




    auto invproj = glm::inverse(perspective * camera1);
    float points[3 * 16];
    for (int i = 0; i < 3 * 16; i += 3) {
        glm::vec4 v = invproj * glm::vec4(cameraPoints[i], cameraPoints[i + 1], cameraPoints[i + 2], 1.0f);
        glm::vec3 worldV = v / v.w;
        std::cout << worldV.x << " " << worldV.y << " " << worldV.z << '\n';
        points[i] = worldV.x;
        points[i + 1] = worldV.y;
        points[i + 2] = worldV.z;
    }

    //Create vertex buffer object for camera
    unsigned int VBOcamera, VAOcamera;
    glGenVertexArrays(1, &VAOcamera);
    glGenBuffers(1, &VBOcamera);
    glBindVertexArray(VAOcamera);

    glBindBuffer(GL_ARRAY_BUFFER, VBOcamera); //bind
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW); //add data, GL_DYNAMIC_DRAW if data is changed alot  

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        myShader.use();

        toggleCamera ? myShader.setMat4("V", camera1) : myShader.setMat4("V", camera2);





        float time = glfwGetTime();
        glm::mat4 modelM = glm::mat4(1.0f);
        modelM = glm::rotate(modelM, glm::radians(45.0f * time), glm::vec3(0.0f, 1.0f, 0.0f));
        modelM = glm::scale(modelM, glm::vec3(0.5f));

        myShader.setMat4("M", modelM);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glCullFace(GL_FRONT);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        modelM = glm::mat4(1.0f);
        myShader.setMat4("M", modelM);
        
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
        camera1 = glm::translate(camera1, glm::vec3(0.0f, 0.0f, 0.015f));
        camera2 = glm::translate(camera2, glm::vec3(0.0f, 0.0f, 0.015f));
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        camera1 = glm::translate(camera1, glm::vec3(0.0f, 0.0f, -0.015f));
        camera2 = glm::translate(camera2, glm::vec3(0.0f, 0.0f, -0.015f));
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        camera1 = glm::translate(camera1, glm::vec3(-0.01f, 0.0f, 0.0f));
        camera2 = glm::translate(camera2, glm::vec3(-0.01f, 0.0f, 0.0f));
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        camera1 = glm::translate(camera1, glm::vec3(0.01f, 0.0f, 0.0f));
        camera2 = glm::translate(camera2, glm::vec3(0.01f, 0.0f, 0.0f));
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        toggleCamera = !toggleCamera;
    }
}
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/noise.hpp>
#include <glad/glad.h> //must be included before glfw 
#include <GLFW/glfw3.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>

#include "header/Shader.h"
#include "header/Mesh.h"
#include "header/CameraControl.h"
#include "header/ChunkHandler.h"
#include "header/CameraPlane.h"
#include "header/glugg.h"
#include "header/Tree.h"
#define STB_IMAGE_IMPLEMENTATION
#include "header/stb_image.h"
#include "header/PoissonGenerator.h"

void initialize();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_position_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
std::vector<CameraPlane> computeCameraPlanes(const std::vector<glm::vec3>& points);
void updateCamera2();
void sortBillBoards(std::vector<glm::mat4>& billboards, const glm::vec3& pos);


void printmat4(const glm::mat4& mat);

//settings
int constexpr gridSize{ 11 };
int constexpr nrVertices{ 161 };
float constexpr spacing{ 0.075f };
int constexpr NROFTREES{ 6 };

const unsigned int SCREEN_WIDTH = 1600, SCREEN_HEIGHT = 900;

glm::mat4 camera2 = glm::mat4(1.0f);
bool toggleCamera{ true }; //Toggles between camera 1 and 2 
CameraControl camera1Control{ glm::vec3{ 0.0f, 1.0f, 0.0f } };
float deltaTime = 0.0f, lastFrame = 0.0f;
bool cull = false, useLOD = true, wireFrame = false, drawbb = false;

bool heat = false, moist = false, biome = false, normal = false, drawbranches = true;


//Callback function to create new trees when generating new chunk
struct TreeGenerator {
    unsigned int ID;
    std::vector<Tree*>& trees;

    Biome& biomeGenerator;

    unsigned int treeTextureID;

    PoissonGenerator::DefaultPRNG prng;
    std::vector<PoissonGenerator::Point> points = PoissonGenerator::generatePoissonPoints(NROFTREES, prng, false);

    /// <summary>
    /// Creates a new tree inside the bounding box of min - max x and z
    /// </summary>
    void operator()(ChunkHandler& chandler, float minX, float maxX, float minZ, float maxZ) {


        for (int i = 0; i < points.size(); i++) {
            float x = map(points[i].x, 0, 1, minX, maxX);
            float z = map(points[i].y, 0, 1, minZ, maxZ);
            glm::vec3 offset{ 0.0f, -0.1f, 0.0f };
            auto position = chandler.getPointOnTerrain(x, z) + offset;

            bool validpos = validate(position, chandler);

            if (!validpos)
                continue;

            auto biomeType = biomeGenerator.getBiomeAtPosition(position.x, position.z);
            auto weights = biomeGenerator.computeWeatherValues(position.x, position.z);

            auto tree = Tree::Create(biomeType, ID, position, treeTextureID, weights);

            if (tree)
                trees.push_back(tree);
        }
        
        //for (int i = 0; i < nroftrees; i++) {
        //    //validate position
        //    //TODO find more efficient method of finding a valid position 
        //    bool validposition = false;
        //    glm::vec3 position{ 0.0f };
        //    int iterations = 0;
        //    do {
        //        float x = random(minX, maxX);
        //        float z = random(minZ, maxZ);
        //        glm::vec3 offset{ 0.0f, -0.1f, 0.0f };
        //        position = chandler.getPointOnTerrain(x, z) + offset;
        //        validposition = validate(position, chandler);

        //        ++iterations;

        //    } while (iterations < 50 && !validposition);

        //    //std::cout << "it took " << iterations << " iterations to find correct position \n";
        //    
        //    //Give position to Biome map determine biome 
        //    auto biomeType = biomeGenerator.getBiomeAtPosition(position.x, position.z);

        //    //create tree depending on what biome it is
        //    Tree* tree = nullptr;

        //    if(validposition)
        //        tree = Tree::Create(biomeType, ID, position, treeTextureID);
        //    
        //    //trees.push_back(MakeTree(ID, position));
        //    if(tree)
        //        trees.push_back(tree);
        //}
    }
    const int nroftrees{ NROFTREES };
private:
    float random(float low, float high) {
        return low + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (high - low)));
    }
    
    /// <summary>
    /// Remap value from range [low1, high1] to new range [low2, high2]
    /// </summary>
    float map(float value, float low1, float high1, float low2, float high2) const {
        return low2 + (value - low1) * (high2 - low2) / (high1 - low1);
    }

    bool validate(const glm::vec3& p, ChunkHandler& chandler) {
        //Check water level
        if (p.y <= -1.5f)
            return false;

        //"randomly" remove ca 50% of positions
        if (glm::simplex(p) < 0.0f)
            return false;

        //check normal angle at location
        float offset = 0.1f;
        auto n = chandler.getPointOnTerrain(p.x, p.z - offset);
        auto s = chandler.getPointOnTerrain(p.x, p.z + offset);
        auto w = chandler.getPointOnTerrain(p.x - offset, p.z);
        auto e = chandler.getPointOnTerrain(p.x + offset, p.z);

        auto e1 = n - s;
        auto e2 = w - e;
        glm::vec3 normal_cross = glm::normalize(glm::cross(e1, e2));
        glm::vec3 ground{ 0.0f, 1.0f, 0.0f };
        float crossAngle = std::acosf(glm::dot(normal_cross, ground)) * 180 / 3.14f;

        return crossAngle < 40;
    }
};

struct BranchVertex {
    glm::vec3 pos;
    glm::vec2 st;
};

float random(float low, float high) {
    return low + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (high - low)));
}

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

    /*** Shaders ***/
    Shader myShader{ "shaders/vertex.vert", "shaders/fragment.frag" };
    Shader cameraShader{ "shaders/cameraVertex.vert", "shaders/cameraFragment.frag" };
    Shader boundingBoxShader{ "shaders/boundingBoxVertex.vert", "shaders/boundingBoxFragment.frag" };
    Shader treeShader{ "shaders/treeVertex.vert", "shaders/treeFragment.frag" };
    Shader branchShader{ "shaders/branchVertex.vert", "shaders/branchFragment.frag" };

    //PoissonGenerator::DefaultPRNG prng;
    //auto points = PoissonGenerator::generatePoissonPoints(30, prng, false);
    //for (auto p : points) {
    //    std::cout << p.x << " " << p.y << '\n';
    //}

    /* load texture */
    treeShader.use();
    int width, height, nrChannels;
    auto filename{ "textures/bark2.tga" };
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Could not load texture at " << filename << '\n';
    }
    stbi_image_free(data);


    glm::mat4 perspective = glm::perspective(glm::radians(45.0f), static_cast<float>(SCREEN_WIDTH) / SCREEN_HEIGHT, 1.0f, 70.0f);
    glm::mat4 perspective2 = glm::perspective(glm::radians(45.0f), static_cast<float>(SCREEN_WIDTH) / SCREEN_HEIGHT, 0.1f, 200.0f);

    /*** Always use the larger perspective to render camera frustum, otherwise it risk being culled in viewport ***/
    cameraShader.use();
    cameraShader.setMat4("P", perspective2);
    /*** Set camera 2 start position and rotation ***/
    camera2 = glm::translate(camera2, glm::vec3(0.0f, -5.0f, -25.0f));
    camera2 = glm::rotate(camera2, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    
    Vertex v1{ glm::vec3{-1.0f, -1.0f, -1.0f} }, v2{ glm::vec3{1.0f, -1.0f, -1.0f} }, v3{ glm::vec3{1.0f, 1.0f, -1.0f} },
        v4{ glm::vec3{-1.0f, 1.0f, -1.0f} }, v5{ glm::vec3{-1.0f, -1.0f, 1.0f} }, v6{ glm::vec3{1.0f, -1.0f, 1.0f} },
        v7{ glm::vec3{1.0f, 1.0f, 1.0f} }, v8{ glm::vec3{-1.0f, 1.0f, 1.0f} };
    
    std::vector<Vertex> campoints{ v1, v2, v3, v4, v5, v6, v7, v8 };
    std::vector<unsigned int> camIndices{ 
        0, 1, 2, 3, 0, //near plane
        4, 5, 6, 7, 4, //far plane
        7, 3, 2, 6, 5, 1 //diagonal lines
    };

    Mesh camera1Mesh{ campoints, camIndices };

    //std::vector<std::pair<GLuint, int>> trees;
    std::vector<Tree*> trees;
    trees.reserve(100);
    Biome biomeGenerator{};
    TreeGenerator treeGenerator{ treeShader.ID, trees, biomeGenerator, textureID };
    ChunkHandler chandler{gridSize, nrVertices, spacing , treeGenerator, biomeGenerator};   // (gridSize, nrVertices, spacing, yScale)


    ///* setup quad */
    //std::vector<BranchVertex> quad{
    //    BranchVertex{glm::vec3{ -1, -1, 0}, glm::vec2{0,0}},
    //    BranchVertex{glm::vec3{ 1, -1, 0}, glm::vec2{1,0}},
    //    BranchVertex{glm::vec3{ 1, 1, 0 }, glm::vec2{1,1}},
    //    BranchVertex{glm::vec3{ -1, 1, 0 }, glm::vec2{0,1}}
    //};

    //std::vector<unsigned int> quadIndices{
    //    0, 1, 3,
    //    1, 2, 3 };

    //unsigned int VAO, EBO, VBO;

    //glGenVertexArrays(1, &VAO);
    //glGenBuffers(1, &VBO);
    //glGenBuffers(1, &EBO);

    //glBindVertexArray(VAO);
    //glBindBuffer(GL_ARRAY_BUFFER, VBO);

    //glBufferData(GL_ARRAY_BUFFER, quad.size() * sizeof(BranchVertex), &quad[0], GL_STATIC_DRAW); //bind vertex data

    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, quadIndices.size() * sizeof(unsigned int), &quadIndices[0], GL_STATIC_DRAW);

    //// vertex positions
    //glEnableVertexAttribArray(0);
    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BranchVertex), (void*)0);

    //glEnableVertexAttribArray(1);
    //glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(BranchVertex), (void*)offsetof(BranchVertex, st));

    //glBindVertexArray(0);

    ////set quad texture 
    //branchShader.use();
    //auto filename2{ "textures/leaf.png" };
    //int w, h, nr;
    //auto data2 = stbi_load(filename2, &w, &h, &nr, 0);
    //unsigned int branchtextureID;
    //std::cout << "nr channels in leaf " << nr << '\n';
    //glGenTextures(1, &branchtextureID);
    //glBindTexture(GL_TEXTURE_2D, branchtextureID);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //if (data2) {
    //    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data2);
    //    glGenerateMipmap(GL_TEXTURE_2D);
    //}
    //else {
    //    std::cout << "Could not load texture at " << filename2 << '\n';
    //}
    //stbi_image_free(data2);
    
    /* end of quad */

    //OpenGL render Settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    //glEnable(GL_BLEND); // blending -> alpha 
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    Quad::init(branchShader.ID);
    Quad branch;
    branch.setupQuad();
    
    //int count = 2000000;
    //modelMatrices.reserve(count + modelMatrices.size());

    //for (int i = 0; i < count; i++) {
    //    auto m = glm::mat4{ 1.0f };
    //    float x = random(-100, 100);
    //    float y = random(-3, 10);
    //    float z = random(-100, 100);
    //    m = glm::translate(m, glm::vec3{ x,y,z });
    //    modelMatrices.push_back(m);

    //}
    std::cout << "nr of trees " << trees.size() << '\n';
    std::cout << "nr of billboards " <<  modelMatrices.size() << '\n';
    branch.setBranchMatrices();

    //printmat4(modelMatrices[0]);
    //std::cout << modelMatrices[0][3][0] << " " << modelMatrices[0][3][1] << " " << modelMatrices[0][3][2] << '\n';

    /*****************
    *  Render loop   *
    *****************/
    float timer{ 0.0f };
    while (!glfwWindowShouldClose(window))
    {
        //compute delta time between frames.
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        //display fps
        if (timer <= 0.0f) {
            std::string title = "fps: " + std::to_string(1.0f / deltaTime);
            glfwSetWindowTitle(window, title.c_str());
            timer = 0.1f;
        }
        else
        {
            timer -= deltaTime;
        }
        //process inputs 
        processInput(window);
        camera1Control.pollMouse(window, SCREEN_WIDTH, SCREEN_HEIGHT);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 camera1 = camera1Control.computeCameraViewMatrix();
        glm::mat4 modelM = glm::mat4(1.0f);

        /*** Draw camera frustum ***/
        auto invproj = glm::inverse(perspective * camera1);
        cameraShader.use();
        cameraShader.setMat4("InvP", invproj);
        toggleCamera ? cameraShader.setMat4("V", camera1) : cameraShader.setMat4("V", camera2);
        camera1Mesh.draw(GL_LINE_STRIP);

        /*** Compute camera frustum points in world coordinates ***/
        std::vector<glm::vec3> worldCamPoints;
        worldCamPoints.reserve(campoints.size());
        for (int i = 0; i < campoints.size(); ++i) {
            glm::vec4 p = invproj * glm::vec4(campoints[i].position, 1.0f);
            glm::vec3 c = glm::vec3{ p.x / p.w, p.y / p.w, p.z / p.w };
            worldCamPoints.push_back(c);
        }
        auto planes = computeCameraPlanes(worldCamPoints);

        /*** Cull terrain ***/
        if (cull) {
            chandler.cullTerrain(cull);
        }
        else {
            chandler.cullTerrainChunk(planes);
        }

        /*** Update terrain chunks ***/
        chandler.updateChunks(camera1Control.getCameraPosition());

        /*** Draw terrain chunks ***/
        myShader.use();
        myShader.setMat4("M", modelM);
        toggleCamera ? myShader.setMat4("V", camera1) : myShader.setMat4("V", camera2);
        toggleCamera ? myShader.setMat4("P", perspective) : myShader.setMat4("P", perspective2);
        //Draw terrain color or distance color
        if(glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
            myShader.setBool("colorDistance", true);
        if(glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
            myShader.setBool("colorDistance", false);

        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
            myShader.setBool("heat", heat);
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
            myShader.setBool("moist", moist);
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
            myShader.setBool("biome", biome);
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
            myShader.setBool("normalC", normal);

        if (wireFrame) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            if (useLOD)
                chandler.draw(camera1Control.getCameraPosition());
            else
                chandler.drawWithoutLOD();
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        else {
            if (useLOD)
                chandler.draw(camera1Control.getCameraPosition());
            else
                chandler.drawWithoutLOD();
        }
        /*** Draw trees ***/
        treeShader.use();
        treeShader.setMat4("M", modelM);
        toggleCamera ? treeShader.setMat4("V", camera1) : treeShader.setMat4("V", camera2);
        toggleCamera ? treeShader.setMat4("P", perspective) : treeShader.setMat4("P", perspective2);
        for (const Tree* tree : trees) {
            tree->draw();
        }
        //for (int i = 0; i < trees.size(); i++) {
            //glBindVertexArray(trees[i].first);	// Select VAO
            //glDrawArrays(GL_TRIANGLES, 0, trees[i].second);
        //}

        /*** Draw bounding boxes around chunks  ***/
        boundingBoxShader.use();
        boundingBoxShader.setMat4("M", modelM);
        toggleCamera ? boundingBoxShader.setMat4("V", camera1) : boundingBoxShader.setMat4("V", camera2);
        toggleCamera ? boundingBoxShader.setMat4("P", perspective) : boundingBoxShader.setMat4("P", perspective2);
        if(drawbb)
            chandler.drawBoundingBox();


        /*** Draw tree branches ***/
        branchShader.use();
        //modelM = glm::translate(modelM, glm::vec3{ 0, 3.5f, 0 });
        //branchShader.setMat4("M", modelM);
        toggleCamera ? branchShader.setMat4("V", camera1) : branchShader.setMat4("V", camera2);
        toggleCamera ? branchShader.setMat4("P", perspective) : branchShader.setMat4("P", perspective2);
        

        if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
            branch.updateBranchMatrices();
            std::cout << modelMatrices.size() << '\n';
        }

        if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
            sortBillBoards(modelMatrices, camera1Control.getCameraPosition());
            std::cout << " sorting billboards \n";
        }

        if (drawbranches) {
            //for (const glm::mat4& m : modelMatrices)
            //{
            //    branchShader.setMat4("M", m);
            //    branch.draw();
            //}
            branch.draw();
        }
        
        //branch.draw();
        //Quad::draw();
        /*
        glEnable(GL_BLEND); // blending -> alpha 
        glBindTexture(GL_TEXTURE_2D, branchtextureID);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, quadIndices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glDisable(GL_BLEND);
        */
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
	return 0;
}


void sortBillBoards(std::vector<glm::mat4>& billboards, const glm::vec3& pos) {
    std::sort(billboards.begin(), billboards.end(), [&pos](const glm::mat4& a, const glm::mat4& b) {
        glm::vec3 _a{ a[3][0], a[3][1], a[3][2] };
        glm::vec3 _b{ b[3][0], b[3][1], b[3][2] };

        auto d1 = glm::distance(_a, pos);
        auto d2 = glm::distance(_b, pos);

        return d1 > d2;
        });
}

std::vector<CameraPlane> computeCameraPlanes(const std::vector<glm::vec3>& points) {
    //top plane
    glm::vec3 e1 = points[7] - points[3];
    glm::vec3 e2 = points[2] - points[3];
    CameraPlane p1{ points[3], glm::normalize(glm::cross(e2, e1)) };
    //std::cout << "top plane normal y: " << p1.normal.y << '\n';
    //bottom plane
    e1 = points[4] - points[0];
    e2 = points[1] - points[0];
    CameraPlane p2{ points[0], glm::normalize(glm::cross(e1, e2)) };
    //std::cout << "bottom plane normal y: " << p2.normal.y << '\n';
    //left plane 
    e1 = points[7] - points[3];
    e2 = points[0] - points[3];
    CameraPlane p3{ points[3], glm::normalize(glm::cross(e1, e2)) };
    //std::cout << "left plane normal x: " << p3.normal.x << '\n';
    //rightplane
    e1 = points[6] - points[2];
    e2 = points[1] - points[2];
    CameraPlane p4{ points[2], glm::normalize(glm::cross(e2, e1)) };
    //std::cout << "right plane normal x: " << p4.normal.x << '\n';
    //far plane
    e1 = points[7] - points[6];
    e2 = points[5] - points[6];
    CameraPlane p5{ points[6], glm::normalize(glm::cross(e2, e1)) };
    //std::cout << "far plane normal z: " << p5.normal.z << '\n';
    //near plane ?? 
    e1 = points[3] - points[2];
    e2 = points[1] - points[2];
    CameraPlane p6{ points[2], glm::normalize(glm::cross(e1, e2)) };
    //std::cout << "near plane normal z: " << p6.normal.z << '\n';

    //std::cout << '\n';
    //TODO check if near plane is necessary
    return std::vector<CameraPlane>{p1, p2, p3, p4, p5};
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
        camera1Control.moveCamera(FORWARD, deltaTime);
        updateCamera2();
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        camera1Control.moveCamera(BACKWARD, deltaTime);
        updateCamera2();
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        camera1Control.moveCamera(LEFT, deltaTime);
        updateCamera2();
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        camera1Control.moveCamera(RIGHT, deltaTime);
        updateCamera2();
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        camera1Control.moveCamera(DESCEND, deltaTime);
        updateCamera2();
    }

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        camera1Control.moveCamera(ASCEND, deltaTime);
        updateCamera2();
    }
   
}

void updateCamera2() {
    camera2 = glm::mat4{ 1.0f };
    camera2 = glm::rotate(camera2, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    camera2 = glm::translate(camera2, -camera1Control.getCameraPosition());
    camera2 = glm::translate(camera2, glm::vec3(0.0f, -70.0f, -60.0f));
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
    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        cull = !cull;
    }
    if (key == GLFW_KEY_V && action == GLFW_PRESS) {
        wireFrame = !wireFrame;
    }
    if (key == GLFW_KEY_B && action == GLFW_PRESS) {
        drawbb = !drawbb;
    }
    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        useLOD = !useLOD;
    }
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        heat = !heat;
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        moist = !moist;
    }
    if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
        biome = !biome;
    }
    if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
        normal = !normal;
    }
    if (key == GLFW_KEY_5 && action == GLFW_PRESS) {
        drawbranches = !drawbranches;
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
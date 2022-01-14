#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#include <utility>
//#include <glad/glad.h> //must be included before glfw 
#include <glm/glm.hpp>
#include "glugg.h"
#include "Biome.h"
#include "stb_image.h"
#include <vector>
static unsigned int branchTextureID, branchShaderID;

static std::vector<glm::mat4> modelMatrices;


class Quad {
public:

    void draw() {
        glEnable(GL_BLEND); // blending -> alpha 
        glDisable(GL_CULL_FACE);
        glBindTexture(GL_TEXTURE_2D, branchTextureID);
        glBindVertexArray(VAO);
        glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0, modelMatrices.size());
        glBindVertexArray(0);
        glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);

    }

    static void init(unsigned int shaderProgram) {
        branchShaderID = shaderProgram;
        loadTexture();
    }

    void setupQuad() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(QuadVertex), &vertices[0], GL_STATIC_DRAW); //bind vertex data

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // vertex positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void*)offsetof(QuadVertex, st));

        glBindVertexArray(0);
    }

    void updateBranchMatrices() {
        glBindVertexArray(VAO);
        //unsigned int buffer;
        //glGenBuffers(1, &matrixVBO);
        glBindBuffer(GL_ARRAY_BUFFER, matrixVBO);
        glBufferData(GL_ARRAY_BUFFER, modelMatrices.size() * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

        std::size_t vec4Size = sizeof(glm::vec4);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(1 * vec4Size));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);

        glBindVertexArray(0);
    }

    void setBranchMatrices() {
        glBindVertexArray(VAO);
        //unsigned int buffer;
        glGenBuffers(1, &matrixVBO);
        glBindBuffer(GL_ARRAY_BUFFER, matrixVBO);
        glBufferData(GL_ARRAY_BUFFER, modelMatrices.size() * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

        std::size_t vec4Size = sizeof(glm::vec4);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(1 * vec4Size));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);

        glBindVertexArray(0);
    }

private:

    static void loadTexture() {
        glUseProgram(branchShaderID);
        auto filename{ "textures/leaf.png" };
        int width, height, nrChannels;
        auto data = stbi_load(filename, &width, &height, &nrChannels, 0);
        //unsigned int branchtextureID;
        glGenTextures(1, &branchTextureID);
        glBindTexture(GL_TEXTURE_2D, branchTextureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else {
            std::cout << "Could not load texture at " << filename << '\n';
        }
        stbi_image_free(data);
    }

    struct QuadVertex {
        glm::vec3 pos;
        glm::vec2 st;
    };

    std::vector<QuadVertex> vertices{
    QuadVertex{glm::vec3{ -1, -1, 0}, glm::vec2{0,0}},
    QuadVertex{glm::vec3{ 1, -1, 0}, glm::vec2{1,0}},
    QuadVertex{glm::vec3{ 1, 1, 0 }, glm::vec2{1,1}},
    QuadVertex{glm::vec3{ -1, 1, 0 }, glm::vec2{0,1}}
    };

    std::vector<unsigned int> indices{
    0, 1, 3,
    1, 2, 3 };

    unsigned int VAO, VBO, EBO, matrixVBO;


};

class Tree
{
public:
    virtual ~Tree() = default; //TODO: create destructor

    static Tree* Create(BiomeType biome, unsigned int _shaderID, const glm::vec3& pos, unsigned int _treeTextureID, const glm::vec2& weights);

    void draw() const {
        glBindTexture(GL_TEXTURE_2D, textureID);
        glBindVertexArray(modelID);	// Select VAO
        glDrawArrays(GL_TRIANGLES, 0, nrVertices);
    }

protected: 
    Tree(unsigned int _shaderID, const glm::vec3& pos, unsigned int _textureID) : shaderID{ _shaderID }, position{ pos }, textureID{ _textureID }, modelID{ 0 }, nrVertices{ 0 } { }

    /// <summary>
    /// Creates a tree with root at position for shader with ID 
    /// </summary>
    virtual std::pair<GLuint, int> MakeTree(GLuint program, glm::vec3 position) = 0;

    /*** Helper functions to create trees ***/
   /// <summary>
   /// Create a cylinder 
   /// </summary>
   /// <param name="aSlices"></param>
   /// <param name="height"></param>
   /// <param name="topwidth"></param>
   /// <param name="bottomwidth"></param>
    void MakeCylinderAlt(int aSlices, float height, float topwidth, float bottomwidth)
    {
        gluggMode(GLUGG_TRIANGLE_STRIP);
        glm::vec3 top = glm::vec3{ 0, height, 0 };
        glm::vec3 center = glm::vec3(0, 0, 0);
        glm::vec3 bn = glm::vec3(0, -1, 0); // Bottom normal
        glm::vec3 tn = glm::vec3(0, 1, 0); // Top normal

        for (float a = 0.0; a < 2.0 * M_PI + 0.0001; a += 2.0 * M_PI / aSlices)
        {
            float a1 = a;

            glm::vec3 p1 = glm::vec3(topwidth * cos(a1), height, topwidth * sin(a1));
            glm::vec3 p2 = glm::vec3(bottomwidth * cos(a1), 0, bottomwidth * sin(a1));
            glm::vec3 pn = glm::vec3(cos(a1), 0, sin(a1));

            // Done making points and normals. Now create polygons!
            gluggNormalv(pn);
            gluggTexCoord(height, a1 / M_PI);
            gluggVertexv(p2);
            gluggTexCoord(0, a1 / M_PI);
            gluggVertexv(p1);
        }

        // Then walk around the top and bottom with fans
        gluggMode(GLUGG_TRIANGLE_FAN);
        gluggNormalv(bn);
        gluggVertexv(center);
        // Walk around edge
        for (float a = 0.0; a <= 2.0 * M_PI + 0.001; a += 2.0 * M_PI / aSlices)
        {
            glm::vec3 p = glm::vec3(bottomwidth * cos(a), 0, bottomwidth * sin(a));
            gluggVertexv(p);
        }
        // Walk around edge
        gluggMode(GLUGG_TRIANGLE_FAN); // Reset to new fan
        gluggNormalv(tn);
        gluggVertexv(top);
        for (float a = 2.0 * M_PI; a >= -0.001; a -= 2.0 * M_PI / aSlices)
        {
            glm::vec3 p = glm::vec3(topwidth * cos(a), height, topwidth * sin(a));
            gluggVertexv(p);
        }
    }

    /// <summary>
    /// Draw random value between low and high
    /// </summary>
    float random(float low, float high) {
        return low + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (high - low)));
    }

private:
    //constructor ish?
    void constructTree() {
        glUseProgram(shaderID); //Activate shader 
        auto [ID, count] = MakeTree(shaderID, position);
        modelID = ID;
        nrVertices = count;
    }

    unsigned int shaderID;
    unsigned int modelID;
    int nrVertices;
    glm::vec3 position;
    unsigned int textureID;
};

class Spruce : public Tree {
public:
    Spruce(unsigned int shaderID, const glm::vec3& pos, unsigned int textureID) : Tree{ shaderID, pos, textureID } {}

    ~Spruce() = default;
private:

    std::pair<GLuint, int> MakeTree(GLuint program, glm::vec3 position) override
    {
        gluggSetPositionName("position");
        gluggSetNormalName("normal");
        gluggSetTexCoordName("in_st");

        gluggBegin(GLUGG_TRIANGLES);
        // Between gluggBegin and gluggEnd, call MakeCylinderAlt plus glugg transformations
        // to create a tree.
        gluggTranslate(position);

        float height = 2.0f;
        MakeCylinderAlt(10, height, 0.1, 0.15);
        CreateTreeBranches(0, 4, height);
        int count;
        GLuint id = gluggEnd(&count, program, 0);

        return std::make_pair(id, count);
    }

    /// <summary>
    /// Creates branches 
    /// </summary>
    /// <param name="count"></param>
    /// <param name="maxlevel"></param>
    /// <param name="height"></param>
    void CreateTreeBranches(int count, int maxlevel, float height) {
        if (count == maxlevel)
        {
            
            //create leaf branch
            gluggTranslate(0, height / (count + 1) + 0.3, 0);
            gluggRotate(M_PI / 2.0f, glm::vec3{ 0,0,1 });

            gluggPushMatrix();
                gluggScale(0.3, 0.3, 0.3);
                modelMatrices.push_back(gluggCurrentMatrix());
            gluggPopMatrix();

            gluggRotate(M_PI / 2.0f, glm::vec3{ 1,0,0 });
            gluggScale(0.3, 0.3, 0.3);
            modelMatrices.push_back(gluggCurrentMatrix());
            //TODO Maby add more branches l8rs



            return;
        }
        ++count;
        int nrOfBranches = (rand() % 6) + 2;
        //gluggPushMatrix();

        gluggTranslate(0, height / count, 0);
        float angle = 2 * M_PI / nrOfBranches;

        for (int i = 0; i < nrOfBranches; ++i) {
            gluggPushMatrix();
            //float r = (double) rand() / RAND_MAX;
            gluggRotate(angle * i, 0, 1, 0);
            gluggRotate(30 * M_PI / 180, 0, 0, 1);
            MakeCylinderAlt(10, height / (count + 1), 0.1 / (1 + count), 0.1 / count);
            CreateTreeBranches(count, maxlevel, height);
            gluggPopMatrix();
        }
    }
};

class Cactus : public Tree {
public:

    Cactus(unsigned int shaderID, const glm::vec3& pos, unsigned int textureID) : Tree{ shaderID, pos, textureID } {}

    ~Cactus() = default;
private:

    std::pair<GLuint, int> MakeTree(GLuint program, glm::vec3 position) override
    {
        gluggSetPositionName("position");
        gluggSetNormalName("normal");
        gluggSetTexCoordName("in_st");

        gluggBegin(GLUGG_TRIANGLES);
        // Between gluggBegin and gluggEnd, call MakeCylinderAlt plus glugg transformations
        // to create a tree.
        gluggTranslate(position);
        MakeCylinderAlt(10, 2, 0.1, 0.15);
        CreateTreeBranches(2.0f);
        int count;
        GLuint id = gluggEnd(&count, program, 0);

        return std::make_pair(id, count);
    }

    /// <summary>
    /// Creates branches 
    /// </summary
    /// <param name="height"></param>
    void CreateTreeBranches(float height) {
        int nrOfBranches = (rand() % 4) + 1; // 1 - 4

        float angle = 2 * M_PI / nrOfBranches; //angle around Y

        for (int i = 0; i < nrOfBranches; ++i) {
            gluggPushMatrix();
            float r = random(0.2f, 0.7f) * height;
            gluggTranslate(0, r, 0);

            float angleoffset = random(-angle / 2, angle / 2);
            gluggRotate(angle * i + angleoffset, 0, 1, 0); //angle around Y
            //create arc
            gluggRotate(90 * M_PI / 180, 0, 0, 1);
            MakeCylinderAlt(10, 0.2, 0.09, 0.1);
            gluggTranslate(0, 0.15, 0);

            gluggRotate(-15 * M_PI / 180, 0, 0, 1);
            MakeCylinderAlt(10, 0.2, 0.09, 0.09);
            gluggTranslate(0, 0.15, 0);

            gluggRotate(-25 * M_PI / 180, 0, 0, 1);
            MakeCylinderAlt(10, 0.2, 0.09, 0.09);
            gluggTranslate(0, 0.15, 0);


            gluggRotate(-25 * M_PI / 180, 0, 0, 1);
            MakeCylinderAlt(10, 0.2, 0.08, 0.09);
            gluggTranslate(0, 0.15, 0);

            gluggRotate(-10 * M_PI / 180, 0, 0, 1);
            MakeCylinderAlt(10, 0.2, 0.08, 0.08);

            //last piece
            gluggTranslate(0, 0.2, 0);
            MakeCylinderAlt(10, 0.5, 0.05, 0.08);

            gluggPopMatrix();
        }
    }
};

class DeadTree : public Tree {
public:
    DeadTree(unsigned int shaderID, const glm::vec3& pos, unsigned int textureID, float _moisture, float _temperature) :
        Tree{ shaderID, pos, textureID }, moisture{ _moisture }, temperature{ _temperature } {}

    ~DeadTree() = default;
private:

    float moisture;
    float temperature;

    std::pair<GLuint, int> MakeTree(GLuint program, glm::vec3 position) override
    {
        gluggSetPositionName("position");
        gluggSetNormalName("normal");
        gluggSetTexCoordName("in_st");

        gluggBegin(GLUGG_TRIANGLES);
        // Between gluggBegin and gluggEnd, call MakeCylinderAlt plus glugg transformations
        // to create a tree.
        gluggTranslate(position);

        float baseHeight = 2.0f, topWidth = 0.1f, bottomWidth = 0.1f;
        baseHeight *= 0.5f + temperature; //weight height of tree depending on moisture level 
        MakeCylinderAlt(10, baseHeight, topWidth, bottomWidth);
        CreateTreeBranches(0, 4, baseHeight);
        int count;
        GLuint id = gluggEnd(&count, program, 0);

        return std::make_pair(id, count);
    }

    /// <summary>
    /// Creates branches 
    /// </summary>
    /// <param name="count"></param>
    /// <param name="maxlevel"></param>
    /// <param name="height"></param>
    void CreateTreeBranches(int count, int maxlevel, float height) {
        if (count == maxlevel)
        {

            //create leaf branch
            //gluggTranslate(0, height / (count + 1) + 0.3, 0);
            //gluggRotate(M_PI / 2.0f, glm::vec3{ 0,0,1 });

            //gluggPushMatrix();
            //gluggScale(0.3, 0.3, 0.3);
            //modelMatrices.push_back(gluggCurrentMatrix());
            //gluggPopMatrix();

            //gluggRotate(M_PI / 2.0f, glm::vec3{ 1,0,0 });
            //gluggScale(0.3, 0.3, 0.3);
            //modelMatrices.push_back(gluggCurrentMatrix());
            //TODO Maby add more branches l8rs



            return;
        }
        ++count;

        int nrOfBranches = (rand() % static_cast<int>(std::floor(1.0f / moisture))) + 1;
        //gluggPushMatrix();

        gluggTranslate(0, height / count, 0);
        float angle = 2 * M_PI / nrOfBranches;
        for (int i = 0; i < nrOfBranches; ++i) {
            float a = random(5 * M_PI / 180.0f, 55 * M_PI / 180.0f);
            float b = random(-angle / 3, angle / 3);
            gluggPushMatrix();
            //float r = (double) rand() / RAND_MAX;
            gluggRotate((angle + b) * i, 0, 1, 0);
            gluggRotate(a, 0, 0, 1);
            MakeCylinderAlt(10, height / (count + 1), 0.1 / (1 + count), 0.1 / count);
            CreateTreeBranches(count, maxlevel, height);
            gluggPopMatrix();
        }
    }
};

Tree* Tree::Create(BiomeType biome, unsigned int _shaderID, const glm::vec3& pos, unsigned int _treeTextureID, const glm::vec2& weights) {
    Tree* ptr;
    switch (biome)
    {
    case ice:
        ptr = new DeadTree{ _shaderID, pos, _treeTextureID, weights.y, weights.x };
        break;
    case tundra:
        return nullptr;
        break;
    case woodland:
        ptr = new Spruce{ _shaderID, pos, _treeTextureID };
        break;
    case desert:
        ptr = new Cactus{ _shaderID, pos, _treeTextureID };
        break;
    default:
        return nullptr;
        break;
    }
    //if (ptr != nullptr)
    //{
    ptr->constructTree();
    //}
    return ptr;
}
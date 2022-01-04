#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#include <utility>
//#include <glad/glad.h> //must be included before glfw 
#include <glm/glm.hpp>
#include "glugg.h"
#include "Biome.h"

class Tree
{
public:
    virtual ~Tree() = default; //TODO: create destructor

    static Tree* Create(BiomeType biome, unsigned int _shaderID, const glm::vec3& pos);

    void draw() const {
        glBindVertexArray(modelID);	// Select VAO
        glDrawArrays(GL_TRIANGLES, 0, nrVertices);
    }

protected: 
    Tree(unsigned int _shaderID, const glm::vec3& pos) : shaderID{ _shaderID }, position{ pos }, modelID{ 0 }, nrVertices{ 0 } { }

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
};

class Spruce : public Tree {
public:
    Spruce(unsigned int shaderID, const glm::vec3& pos) : Tree{ shaderID, pos } {}

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

    Cactus(unsigned int shaderID, const glm::vec3& pos) : Tree(shaderID, pos) {}

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

Tree* Tree::Create(BiomeType biome, unsigned int _shaderID, const glm::vec3& pos) {
    Tree* ptr;
    switch (biome)
    {
    case ice:
        return nullptr;
        break;
    case tundra:
        return nullptr;
        break;
    case woodland:
        ptr = new Spruce(_shaderID, pos);
        break;
    case desert:
        ptr = new Cactus(_shaderID, pos);
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
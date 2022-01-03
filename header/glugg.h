#ifndef _GLUGG_
#define _GLUGG_


// GLUGG interface
// GLUGG is the OpenGL Utilties for Geometry Generation
// By Ingemar Ragnemalm 2013-2021

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>	
#include <GLFW/glfw3.h>


void gluggBegin(int theMode);
void gluggVertex(float x, float y, float z);
void gluggVertexv(glm::vec3 p);
void gluggNormal(float x, float y, float z);
void gluggNormalv(glm::vec3 n);
void gluggTexCoord(float s, float t);
GLuint gluggEnd(int *count, GLuint program, char optimize);
void gluggDispose();

void gluggMode(int newMode);
enum
{
	GLUGG_TRIANGLES = 0,
	//GLUGG_QUADS,
	GLUGG_TRIANGLE_FAN,
	GLUGG_TRIANGLE_STRIP
};

//void gluggBuildBezier(Point3D *vertices, int *indices, float step);
//GLint gluggBuildBezierPatchModel(Point3D *vertices, int *indices, int startPatch, int endPatch, int *count, GLuint program, float step);

// Inspect internal variables (use before gluggEnd, which clears them!)
GLfloat *gluggGetVertexArray();
GLfloat *gluggGetNormalsArray();
GLfloat *gluggGetTexCoordArray();
GLuint *gluggGetIndexArray();
int gluggGetNumVertices();
int gluggGetNumIndices();
void gluggArraysToElements(); // Needed before gluggGetNumIndices

void gluggSetPositionName(const char *name);
void gluggSetNormalName(const char *name);
void gluggSetTexCoordName(const char *name);

// Matrix stack support
void gluggPushMatrix();
void gluggPullMatrix();
void gluggPopMatrix();
void gluggTranslate(const glm::vec3& p);
void gluggTranslate(GLfloat x, GLfloat y, GLfloat z);
void gluggRotate(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void gluggRotate(float angle, const glm::vec3& axis);
void gluggScale(GLfloat x, GLfloat y, GLfloat z);
glm::mat4 gluggCurrentMatrix();
void gluggSetMatrix(const glm::mat4& m);
void gluggMultMatrix(const glm::mat4& m);


#endif

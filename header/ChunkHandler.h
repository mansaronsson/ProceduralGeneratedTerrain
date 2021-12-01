#pragma once
#include <vector>
#include "Model.h"
#include <glm/gtc/noise.hpp>
#include "BoundingBox.h"
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include "CameraPlane.h"

enum chunkChecker {
	inside, up, down, left, right
};

class ChunkHandler {
public:
	/// <summary>
	/// Create a chunk handler 
	/// </summary>
	/// <param name="nrVertices">number of vertecies per chunk</param>
	/// <param name="spacing">distance between vertices</param>
	/// <param name="yscale">how much to scale in the y direction</param>
	ChunkHandler(size_t _gridSize, size_t _nrVertices, float _spacing, float _yscale);


	void cullTerrain(bool cull) {
		for (auto chunk : chunks) {
			chunk->drawChunk = cull;
		}
	}

	void draw() {
		for (auto chunk : chunks)	
		{
			chunk->draw();
		}

	}

	void drawBoundingBox() {
		for (auto chunk : chunks) {
			chunk->drawBoundingBox();
		}
	}
	/// <summary>
	/// Check if the camera is inside a chunk on x,z plane 
	/// </summary>
	/// <param name="camPos"></param>
	void checkChunk(const glm::vec3& camPos);

	/// <summary>
	/// Update chunk the camera is currently over ie. get center chunk
	/// </summary>
	/// <param name="ch">what chunk direction did we move into</param>
	void updateCurrentChunk(chunkChecker ch);

	/// <summary>
	/// Evaluate if a chunk is within the camera frustum
	/// based on psuedo code in figure 4 http://www.cse.chalmers.se/~uffe/vfc_bbox.pdf
	/// </summary>
	/// <param name="cameraPlanes"></param>
	void cullTerrainChunk(const std::vector<CameraPlane>& cameraPlanes);

private:
	//using enum chunkChecker;
	class Chunk {
	public:
		/// <summary>
		/// Create a chunk at position xpos, zpos, with size as number of vertices
		/// </summary>
		/// <param name="_size">number of vertices in the chunk</param>
		/// <param name="xpos">start position x</param>
		/// <param name="zpos">start position z</param>
		/// <param name="_spacing">how much space between each vertex</param>
		/// <param name="yscale"></param>
		/// <param name="frequency"></param>
		Chunk(size_t _size, float xpos, float zpos, float _spacing, float yscale, float frequency);

		glm::vec3 getPostition() {
			return mesh.vertices[0].position;
		}

		void draw() {
			if(drawChunk)
				mesh.draw(GL_TRIANGLES);
		}
		void drawBoundingBox() {
			if (drawChunk)
				boundingBox.draw();
		}
		
		chunkChecker isInside(const glm::vec3& pos);

		/// <summary>
		/// returns the p and n-vertex of the bounding box computed from the normal n of the plane
		/// table ref: https://old.cescg.org/CESCG-2002/DSykoraJJelinek/
		/// </summary>
		/// <param name="n">plane normal</param>
		/// <returns></returns>
		std::pair<glm::vec3, glm::vec3> computePN(const glm::vec3& n) const;

		bool drawChunk = true;

	private:
		unsigned int index(int w, int d) {
			return w + size * d;

		}
		//Number of vertices
		const size_t size;
		Mesh mesh;
		BoundingBox boundingBox;
	};
	/*End of chunk class*/

	unsigned int index(int col, int row) {
		return col + gridSize * row;
	}

	const size_t gridSize;
	const size_t nrVertices;
	const float spacing;
	const float yscale;

	std::vector<Chunk*> chunks;
	Chunk* currentChunk;
};
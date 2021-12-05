#pragma once
#include <vector>
#include "Model.h"
#include <glm/gtc/noise.hpp>
#include "BoundingBox.h"
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include "CameraPlane.h"
#include <queue>
#include <thread>

enum chunkChecker {
	inside, up, down, left, right
};

using chunkInfo = std::tuple<std::vector<Vertex>, std::vector<unsigned int>, std::vector<glm::vec3>, chunkChecker, size_t>;

class ChunkHandler {
public:
	/// <summary>
	/// Create a chunk handler 
	/// </summary>
	/// <param name="_gridSize"> number of chunks in a A x A grid </param>
	/// <param name="nrVertices">number of vertecies per chunk excluding skirts</param>
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
	chunkChecker checkChunk(const glm::vec3& camPos);

	void generateChunk(chunkChecker cc, size_t id);

	/// <summary>
	/// Update chunk the camera is currently over ie. get center chunk
	/// </summary>
	/// <param name="ch">what chunk direction did we move into</param>
	void updateChunks(const glm::vec3& camPos);

	/// <summary>
	/// Evaluate if a chunk is within the camera frustum
	/// based on psuedo code in figure 4 http://www.cse.chalmers.se/~uffe/vfc_bbox.pdf
	/// </summary>
	/// <param name="cameraPlanes"></param>
	void cullTerrainChunk(const std::vector<CameraPlane>& cameraPlanes);

private:
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
		Chunk(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const std::vector<glm::vec3>& bBox, size_t _size);

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
		/// <summary>
		/// Create noisy point at position x,z computes height y
		/// TODO: make noise dependent on variables
		/// </summary>
		glm::vec3 createPointWithNoise(float x, float z, float* minY = nullptr, float* maxY = nullptr) const;
		/// <summary>
		/// Helper function computes x & z position in grid
		/// </summary>
		std::pair<float, float> computeXZpos(int width, int depth) const;
		/// <summary>
		/// Helper function create fake vertex to help compute normal
		/// </summary>
		glm::vec3 createFakeVertex(int width, int depth) const;
		
		/// <summary>
		/// Compute normalized normal at point v0 from neighboring triangles connected by points p, in order: ne, n, nw, w, sw, s, se, e
		/// triangle order can be seen in project notes
		/// </summary>
		/// <param name="p">: neighboring poins: ne, n, nw, w, sw, s, se, e </param>
		/// <param name="v0">: starting point </param>
		glm::vec3 computeNormal(const std::vector<glm::vec3>& p,const glm::vec3& v0) const;


		chunkChecker isInside(const glm::vec3& pos);

		/// <summary>
		/// returns the p and n-vertex of the bounding box computed from the normal n of the plane
		/// table ref: https://old.cescg.org/CESCG-2002/DSykoraJJelinek/
		/// </summary>
		/// <param name="n">plane normal</param>
		/// <returns></returns>
		std::pair<glm::vec3, glm::vec3> computePN(const glm::vec3& n) const;

		bool drawChunk = true;
		size_t id;

	private:
		//Helper variables, start pos x & z and spacing between vertices
		float XPOS, ZPOS, SPACING;

		unsigned int index(int w, int d) {
			return w + size * d;
		}
		//Number of vertices in chunk
		const size_t size;
		Mesh mesh;
		BoundingBox boundingBox;
	};
	/*End of chunk class*/

	unsigned int index(int col, int row) {
		return col + gridSize * row;
	}

	unsigned int chunkIndex(int w, int d) {
		return w + nrVertices * d;
	}

	const size_t gridSize;
	const size_t nrVertices;
	const float spacing;
	const float yscale;

	int renderCounter{ static_cast<int>(gridSize) };

	Chunk* currentChunk;
	std::vector<Chunk*> chunks;

	std::queue<chunkInfo> renderQ;
	std::queue<chunkChecker> moveQ;
};
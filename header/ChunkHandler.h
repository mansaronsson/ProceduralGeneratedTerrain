#pragma once
#include <vector>
#include "Mesh.h"
#include <glm/gtc/noise.hpp>
#include "BoundingBox.h"
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include "CameraPlane.h"
#include <queue>
#include <thread>
#include <mutex>
#include <future>
#include "Biome.h"

enum chunkChecker {
	inside, up, down, left, right
};

class ChunkHandler {
public:
	/// <summary>
	/// Create a chunk handler 
	/// </summary>
	/// <param name="_gridSize"> number of chunks in a A x A grid </param>
	/// <param name="nrVertices">number of vertecies per chunk excluding skirts</param>
	/// <param name="spacing">distance between vertices</param>
	/// <param name="yscale">how much to scale in the y direction</param>
	ChunkHandler(unsigned int _gridSize, unsigned int _nrVertices, float _spacing, std::function<void(ChunkHandler&, float, float, float, float)> func, const Biome& _biomeGenerator);

	void nrofchunks() {
		std::cout << "current chunks in list: " << chunks.size() << '\n';
	}

	void cullTerrain(bool cull) {
		for (auto chunk : chunks) {
			chunk->drawChunk = cull;
		}
	}

	void draw(const glm::vec3& camposition) {

		//auto p0 = currentChunk->getPostition(currentChunk->index(currentChunk->nrVertices / 2, currentChunk->nrVertices / 2));
		for (Chunk* chunk : chunks)	
		{
			auto p1 = chunk->getPostition(chunk->index(chunk->nrVertices / 2, chunk->nrVertices / 2));
			int lod = computeLOD(camposition, p1);
			chunk->draw(lod);
		}
	}

	void drawWithoutLOD() {
		for (Chunk* chunk : chunks) {
			chunk->draw(1);
		}
	}

	void drawBoundingBox() {
		for (auto chunk : chunks) {
			chunk->drawBoundingBox();
		}
	}

	int computeLOD(const glm::vec3& position, const glm::vec3& chunkposition) {
		float d = glm::distance(position, chunkposition);
		float chunkWidth = (nrVertices - 1) * spacing;
		// Highest lod 
		if (d < 1.5f * chunkWidth) {
			return 1;
		}
		// Mid lod
		if (d <= 2.5f * chunkWidth)
		{
			return 2;
		}
		// Lowest lod
		else if (d <= 3.5f * chunkWidth) {
			return 4;
		}
		else if (d <= 5.0f * chunkWidth)
			return 8;
		else
			return 16;
	}

	/// <summary>
	/// return vec3 position at width, depth x,z
	/// </summary>
	glm::vec3 getPointOnTerrain(float x, float z) {
		return biomeGenerator.computeVertexPointFromBiomes(x, z);
	}

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
		Chunk(unsigned int _size, unsigned int lod, float xpos, float zpos, float _spacing, unsigned int _id, const Biome& _biomeGenerator);
		//Chunk(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const std::vector<glm::vec3>& bBox, size_t _size);

		~Chunk() {
			mesh.deleteMesh();
			boundingBox.deleteBoundingBox();
			if (higherLod != nullptr)
			{
				delete higherLod;
			}
		}

		glm::vec3 getPostition(int index = 0) {
			return mesh.vertices[index].position;
		}

		void draw(int _lod) {
			if (drawChunk) {
				if(_lod == lod)
					mesh.draw(GL_TRIANGLES);
				else
				{
					if(higherLod != nullptr)
					{
						higherLod->draw(_lod);
					}
				}
			}
		}
		void drawBoundingBox() {
			if (drawChunk)
				boundingBox.draw();
		}

		void bakeMeshes();


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

		glm::vec3 setColorFromLOD();

		chunkChecker checkMovement(const glm::vec3& pos);

		/// <summary>
		/// returns the p and n-vertex of the bounding box computed from the normal n of the plane
		/// table ref: https://old.cescg.org/CESCG-2002/DSykoraJJelinek/
		/// </summary>
		/// <param name="n">plane normal</param>
		/// <returns></returns>
		std::pair<glm::vec3, glm::vec3> computePN(const glm::vec3& n) const;

		Chunk* generateLOD(unsigned int nrVerticies, unsigned int _lod, float xpos, float zpos, float _spacing, unsigned int id);

		unsigned int index(int w, int d) {
			return w + nrVertices * d;
		}

		bool drawChunk = true;
		unsigned int id;
		unsigned int lod;
		const unsigned int nrVertices;	//Number of vertices in chunk
		const Biome& biomeGenerator;



	private:
		//Helper variables, start pos x & z and spacing between vertices
		float XPOS, ZPOS, SPACING;

		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<glm::vec3> points;
		glm::vec3 color;

		Mesh mesh;
		BoundingBox boundingBox;
		Chunk* higherLod;
	};
	/*End of chunk class*/

	unsigned int index(int col, int row, int size) {
		return col + size * row;
	}

	void generateChunk(const std::pair<float, float>& newPos, unsigned int nrVeritices, float spacing, unsigned int id, chunkChecker cc = chunkChecker::inside);

	/// <summary>
	/// Check if the camera is inside a chunk on x,z plane 
	/// </summary>
	/// <param name="camPos"></param>
	chunkChecker checkChunk(const glm::vec3& camPos);

	std::pair<float, float> newChunkPosition(chunkChecker cc, unsigned int gridId) const;

	std::mutex mu;

	const unsigned int gridSize;
	const unsigned int nrVertices;
	const float spacing;

	int renderCounter{ static_cast<int>(gridSize) };

	Chunk* currentChunk;
	std::vector<Chunk*> chunks;
	const Biome& biomeGenerator;

	using chunkInfo = std::tuple<Chunk*, chunkChecker>;
	std::queue<chunkInfo> renderQ;
	std::queue<chunkChecker> moveQ;

	std::function<void(ChunkHandler&, float, float, float, float)> callbackfunc;
};
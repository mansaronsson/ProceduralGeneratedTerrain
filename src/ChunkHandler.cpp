#include "..\header\ChunkHandler.h"
#include <iostream>

ChunkHandler::ChunkHandler(size_t _gridSize, size_t _nrVertices, float _spacing, float _yscale)
	: gridSize{ (_gridSize % 2 == 0 ? (_gridSize + 1) : _gridSize) }, nrVertices{ _nrVertices + 2 }, spacing{ _spacing }, yscale{ _yscale }, currentChunk{ nullptr }
{
	size_t size = nrVertices; //two extra rows / columns for the skirts
	float width = (size  - 3 ) * spacing; //width of 1 chunk, -3 due to extra skirts

	for (int row = 0; row < gridSize; ++row) {
		float zpos = -width * (static_cast<float>(gridSize) / 2.0f) + row * width;
		for (int col = 0; col < gridSize; ++col) {
			float xpos = -width * (static_cast<float>(gridSize) / 2.0f) + col * width;

			chunks.push_back(new Chunk{ size, xpos, zpos, spacing, yscale, 2.5f });

		}
	}
	currentChunk = chunks[gridSize * gridSize / 2];
}
 
glm::vec3 ChunkHandler::Chunk::createPointWithNoise(float x, float z, float* minY, float* maxY ) const{
	/*** Apply noise to the height ie. y component using fbm ***/
	int octaves = 6;
	float noiseSum = 0.0f;
	float seed = 0.1f;
	float amplitude = 3.0f;
	float gain = 0.5; //How much to increase / decrease each octave
	float lacunarity = 2.0; //How much to increase / decrease frequency each octave ie. how big steps to take in the noise space
	float freq = 0.09f;

	//Add noise from all octaves
	for (int i = 0; i < octaves; ++i) {
		noiseSum += amplitude * glm::perlin(glm::vec3((x + 1) * freq, (z + 1) * freq, seed));
		freq *= lacunarity;
		amplitude *= gain;
	}

	float noiseY = noiseSum;

	glm::vec3 pos{ x, noiseY, z };

	if (minY != nullptr && maxY != nullptr) {
		*minY = *minY > noiseY ? noiseY : *minY;
		*maxY = *maxY < noiseY ? noiseY : *maxY;
	}
	return pos;
}

std::pair<float, float> ChunkHandler::Chunk::computeXZpos(int width, int depth) const {
	float x = XPOS + width * SPACING;
	float z = ZPOS + depth * SPACING;
	return std::pair<float, float>{x, z};
}

glm::vec3 ChunkHandler::Chunk::createFakeVertex(int width, int depth) const {
	auto [x, z] = computeXZpos(width, depth);
	return createPointWithNoise(x, z);
}

glm::vec3 ChunkHandler::Chunk::computeNormal(const std::vector<glm::vec3>& p,const glm::vec3& v0) const {
	glm::vec3 ne = p[0], n = p[1], nw = p[2], w = p[3], sw = p[4], s = p[5], se = p[6], e = p[7];
	glm::vec3 normal = { 0.0f, 0.0f, 0.0f };
	glm::vec3 e1, e2, e3;
	//t1 & t6
	e1 = nw - v0;
	e2 = w - v0;
	e3 = n - v0;
	normal += glm::cross(e1, e2);
	normal += glm::cross(e3, e1);
	//t2
	e1 = w - v0;
	e2 = s - v0;
	normal += glm::cross(e1, e2);
    //t3 & t4 
	e1 = s - v0;
	e2 = se - v0;
	e3 = e - v0;
	normal += glm::cross(e1, e2);
	normal += glm::cross(e3, e1);
	//t5
	e1 = e - v0;
	e2 = ne - v0;
	normal += glm::cross(e1, e2);
	//Divide by the 6 normals we summed together and normalize vector 
	normal /= 6;
	normal = glm::normalize(normal);

	return normal;
}

ChunkHandler::Chunk::Chunk(size_t _size, float xpos, float zpos, float _spacing, float yscale, float frequency) : size{ _size } {
	XPOS = xpos;
	ZPOS = zpos;
	SPACING = _spacing;
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	vertices.reserve(size * size);
	indices.reserve(6 * (size - 2) * (size - 2) + 3 * size * 4 - 18); // se notes in lecture 6 

	//Need min and max height of this chunk to compute the bounding box
	float minY = std::numeric_limits<float>::max();
	float maxY = std::numeric_limits<float>::min();

	/*** Compute vertex positions and indices ***/
	for (int depth = 0; depth < size; ++depth)
	{
		for (int width = 0; width < size; ++width) {
			float x = xpos + width * _spacing;
			float z = zpos + depth * _spacing;

			/*** Skirts should be at the same x and z position as the next / previous vertex ***/
			if (depth == 0) {
				z = zpos + (depth + 1) * _spacing;
			}
			if (depth == size - 1) {
				z = zpos + (depth - 1) * _spacing;
			}
			if (width == 0) {
				x = xpos + (width + 1) * _spacing;
			}
			if (width == size - 1) {
				x = xpos + (width - 1) * _spacing;
			}

			if (depth == 0 || depth == size - 1 || width == 0 || width == size - 1) //edges of grid ie. skirts
			{
				float skirtDepth = -3.0f;
				glm::vec3 pos{ x, skirtDepth, z };
				vertices.push_back({ pos });
			}
			else //Non edges compute noise value for the y-component
			{
				auto pos = createPointWithNoise(x, z, &minY, &maxY);
				vertices.push_back({ pos });
			}

			//add indices to create triangle list
			if (width < size - 1 && depth < size - 1) {
				unsigned int i1, i2, i3, i4;
				i1 = index(width, depth); //current
				i2 = index(width, depth + 1); //bottom
				i3 = index(width + 1, depth + 1); //bottom right
				i4 = index(width + 1, depth); // right 

				/*
					i1--<--i4
					 |\    |
					 v \   ^
					 |  \  |
					 |   \ |
					i2-->--i3
				*/

				//left triangle diagonal from top left to bottom right 
				indices.push_back(i1);
				indices.push_back(i2);
				indices.push_back(i3);
				//right triangle
				indices.push_back(i1);
				indices.push_back(i3);
				indices.push_back(i4);
			}
		}
	}
	/*** Compute edge & skirt normals ***/
	//Top row, visit each column
	int depth = 1;
	for (int width = 1; width < size - 1; ++width) {
		glm::vec3 v0 = vertices[index(width, depth)].position; //current vertex
		glm::vec3 ne, n, nw, w, sw, s, se, e;
		//Find all surrounding vertices 
		s = vertices[index(width, depth + 1)].position;
		nw = createFakeVertex(width - 1, depth - 1);
		n = createFakeVertex(width, depth - 1);
		ne = createFakeVertex(width + 1, depth - 1);
		if (width == 1) { //top leftmost vertex 
			w = createFakeVertex(width - 1, depth);
			sw = createFakeVertex(width - 1, depth + 1);
		}
		else {
			w = vertices[index(width - 1, depth)].position;
			sw = vertices[index(width - 1, depth + 1)].position;
		}
		if (width == size - 2) { //top rightmost vertex 
			e = createFakeVertex(width + 1, depth);
			se = createFakeVertex(width + 1, depth + 1);
		}
		else {
			e = vertices[index(width + 1, depth)].position;
			se = vertices[index(width + 1, depth + 1)].position;
		}
		//compute normal 
		std::vector<glm::vec3> neighbors{ ne, n, nw, w, sw, s, se, e };
		glm::vec3 normal = computeNormal(neighbors, v0);
		vertices[index(width, depth)].normal = normal;
		vertices[index(width, depth - 1)].normal = normal; //n skirt
		if (width == 1) {
			vertices[index(width - 1, depth - 1)].normal = normal; //nw skirt
			vertices[index(width - 1, depth)].normal = normal; //w skirt
		}
		if (width == size -2) {
			vertices[index(width + 1, depth - 1)].normal = normal; //ne skirt
			vertices[index(width - 1, depth)].normal = normal; //e skirt
		}
	} //End of top row

	//Bottom row visit each column
	depth = size - 2;
	for (int width = 1; width < size - 1; ++width) {
		glm::vec3 v0 = vertices[index(width, depth)].position; //current vertex
		glm::vec3 ne, n, nw, w, sw, s, se, e;
		//Find all surrounding vertices		
		s = createFakeVertex(width, depth + 1);
		sw = createFakeVertex(width - 1, depth + 1);
		se = createFakeVertex(width + 1, depth + 1);
		n = vertices[index(width, depth - 1)].position;
		if (width == 1) { //bottom leftmost vertex 
			w = createFakeVertex(width - 1, depth);
			nw = createFakeVertex(width - 1, depth - 1);
		}
		else {
			w = vertices[index(width - 1, depth)].position;
			nw = vertices[index(width - 1, depth - 1)].position;
		}
		if (width == size - 2) { //bottom rightmost vertex 
			e = createFakeVertex(width + 1, depth);
			ne = createFakeVertex(width + 1, depth - 1);
		}
		else {
			e = vertices[index(width + 1, depth)].position;
			ne = vertices[index(width + 1, depth - 1)].position;
		}
		//Compute normal
		std::vector<glm::vec3> neighbors{ ne, n, nw, w, sw, s, se, e };
		glm::vec3 normal = computeNormal(neighbors, v0);
		vertices[index(width, depth)].normal = normal;
		vertices[index(width, depth + 1)].normal = normal; //s skirt
		if (width == 1) {
			vertices[index(width - 1, depth + 1)].normal = normal; //sw skirt
			vertices[index(width - 1, depth)].normal = normal; //w skirt
		}
		if (width == size - 2) {
			vertices[index(width + 1, depth + 1)].normal = normal; //se skirt
			vertices[index(width - 1, depth)].normal = normal; //e skirt
		}
	}//End of bottom row

	//Left column visit every row except top and bottom
	int width = 1;
	for (int depth = 2; depth < size - 2; ++depth) { //+2 - 2 range skips top and bottom row since they are already computed
		glm::vec3 v0 = vertices[index(width, depth)].position; //current vertex
		glm::vec3 ne, n, nw, w, sw, s, se, e;
		//Find all surrounding vertices
		n = vertices[index(width, depth - 1)].position;
		ne = vertices[index(width + 1, depth - 1)].position;
		e = vertices[index(width + 1, depth)].position;
		se = vertices[index(width + 1, depth + 1)].position;
		s = vertices[index(width, depth + 1)].position;

		nw = createFakeVertex(width - 1, depth - 1);
		w = createFakeVertex(width - 1, depth);
		sw = createFakeVertex(width - 1, depth + 1);
		//Compute normal
		std::vector<glm::vec3> neighbors{ ne, n, nw, w, sw, s, se, e };
		glm::vec3 normal = computeNormal(neighbors, v0);
		vertices[index(width, depth)].normal = normal;
		vertices[index(width - 1, depth)].normal = normal; //w skirt
	}//End of left row

	//Right column visit every row except top and bottom
	width = size - 2;
	for (int depth = 2; depth < size - 2; ++depth) { //+2 - 2 range skips top and bottom row since they are already computed
		glm::vec3 v0 = vertices[index(width, depth)].position; //current vertex
		glm::vec3 ne, n, nw, w, sw, s, se, e;
		//Find all vertices
		n = vertices[index(width, depth - 1)].position;
		nw = vertices[index(width - 1, depth - 1)].position;
		w = vertices[index(width - 1, depth)].position;
		sw = vertices[index(width - 1, depth + 1)].position;
		s = vertices[index(width, depth + 1)].position;

		e = createFakeVertex(width + 1, depth);
		se = createFakeVertex(width + 1, depth + 1);
		ne = createFakeVertex(width + 1, depth - 1);
		//Compute normal
		std::vector<glm::vec3> neighbors{ ne, n, nw, w, sw, s, se, e };
		glm::vec3 normal = computeNormal(neighbors, v0);
		vertices[index(width, depth)].normal = normal;
		vertices[index(width + 1, depth)].normal = normal; //e skirt
	}//End of right column

	//Compute normal by weighting all connected triangles ignoring the first row/column + skirts
	for (int depth = 2; depth < size - 2; ++depth)
	{
		for (int width = 2; width < size -2; ++width) {
			glm::vec3 v0 = vertices[index(width, depth)].position; //current
			//Retrieve neighboring points
			glm::vec3 ne = vertices[index(width + 1, depth - 1)].position;
			glm::vec3 n = vertices[index(width, depth - 1)].position;
			glm::vec3 nw = vertices[index(width - 1, depth - 1)].position;
			glm::vec3 w = vertices[index(width - 1, depth)].position;
			glm::vec3 sw = vertices[index(width - 1, depth + 1)].position;
			glm::vec3 s = vertices[index(width, depth + 1)].position;
			glm::vec3 se = vertices[index(width + 1, depth + 1)].position;
			glm::vec3 e = vertices[index(width + 1, depth)].position;

			std::vector<glm::vec3> neighbors{ ne, n, nw, w, sw, s, se, e };

			glm::vec3 normal = computeNormal(neighbors, v0);
			vertices[index(width, depth)].normal = normal;
		}
	}

	mesh = Mesh{ vertices, indices };

	//create boundingbox ignoring the extra row and column added by the skirts
	//max x and z already had size - 1 before skirts were added 
	float minX = xpos + _spacing; 
	float maxX = xpos + (size - 2) * _spacing;
	float minZ = zpos + _spacing;
	float maxZ = zpos + (size - 2) * _spacing;

	//Important theese are given in correct order -> see BoundingBox.h ctor
	glm::vec3 p1{ minX, maxY, minZ }, p2{ maxX, maxY, minZ }, p3{ maxX, maxY, maxZ }, p4{ minX, maxY, maxZ },
		p5{ minX, minY, minZ }, p6{ maxX, minY, minZ }, p7{ maxX, minY, maxZ }, p8{ minX, minY, maxZ };
	//
	//std::cout << glm::to_string(p1) << " " << glm::to_string(p2) << " " << glm::to_string(p3) << '\n'
	//	<< glm::to_string(p4) << " " << glm::to_string(p5) << " " << glm::to_string(p6) << '\n'
	//	<< glm::to_string(p7) << " " << glm::to_string(p8) << '\n' << '\n';

	boundingBox = BoundingBox{ p1, p2, p3, p4, p5, p6, p7, p8 };
}

chunkChecker ChunkHandler::Chunk::isInside(const glm::vec3& pos)
{
	//std::cout << glm::to_string(pos) << "\n";
	chunkChecker ch = inside;
	auto v1 = mesh.vertices[0].position; //first vertex in chunk
	auto v2 = mesh.vertices[mesh.vertices.size() - 1].position; // last vertex in chunk

	//Check if position is within chunk borders spanned by v1 and v2 in the x,z plane
	if (pos.z < v1.z) {
		ch = up;
		return ch;
	}
	if (pos.z > v2.z) {
		ch = down;
		return ch;
	}
	if (pos.x < v1.x) {
		ch = left;
		return ch;
	}
	if (pos.x > v2.x) {
		ch = right;
		return ch;
	}
	//Position is inside borders
	return ch;
}

std::pair<glm::vec3, glm::vec3> ChunkHandler::Chunk::computePN(const glm::vec3& n) const
{
	if (n.x > 0 && n.y > 0 && n.z > 0) { // + + +
		return std::pair<glm::vec3, glm::vec3>{boundingBox.p3, boundingBox.p5};
	}
	if (n.x > 0 && n.y > 0 && n.z < 0) { // + + -
		return std::pair<glm::vec3, glm::vec3>{boundingBox.p2, boundingBox.p8};
	}
	if (n.x > 0 && n.y < 0 && n.z > 0) { // + - +
		return std::pair<glm::vec3, glm::vec3>{boundingBox.p7, boundingBox.p1};
	}
	if (n.x > 0 && n.y < 0 && n.z < 0) { // + - - 
		return std::pair<glm::vec3, glm::vec3>{boundingBox.p6, boundingBox.p4};
	}
	if (n.x < 0 && n.y > 0 && n.z > 0) { // - +  +
		return std::pair<glm::vec3, glm::vec3>{boundingBox.p4, boundingBox.p6};
	}
	if (n.x < 0 && n.y > 0 && n.z < 0) { // - + -
		return std::pair<glm::vec3, glm::vec3>{boundingBox.p1, boundingBox.p7};
	}
	if (n.x < 0 && n.y < 0 && n.z > 0) { // - - +
		return std::pair<glm::vec3, glm::vec3>{boundingBox.p8, boundingBox.p2};
	}
	if (n.x < 0 && n.y < 0 && n.z < 0) { // - - -
		return std::pair<glm::vec3, glm::vec3>{boundingBox.p5, boundingBox.p3};
	}
}

void ChunkHandler::checkChunk(const glm::vec3& camPos)
{
	chunkChecker ch = currentChunk->isInside(camPos);
	if (ch != inside) {
		updateCurrentChunk(ch);
	}
}


// TODO: Fix the memory leaks this function creates
void ChunkHandler::updateCurrentChunk(chunkChecker ch)
{
	switch (ch)
	{
	case chunkChecker::up:

		for (size_t i = 0; i < gridSize*gridSize; ++i) {
			size_t j = gridSize * gridSize - 1 - i;
			// Delete bottom chunks
			if (j > gridSize * (gridSize - 1)) {
				delete chunks[j];
				chunks[j] = nullptr;
			}

			// Move pointers
			if (j >= gridSize) {
				chunks[j] = chunks[j - gridSize];
			}
			// Create new chunks above previous top chunks
			else {
				glm::vec3 pos = chunks[j]->getPostition();
				chunks[j] = new Chunk{ nrVertices, pos.x, pos.z - (nrVertices-1) * spacing, spacing, yscale, 2.5f };
			}
		}

		currentChunk = chunks[gridSize * gridSize / 2];

		std::cout << "up\n";
		break;

	case chunkChecker::down:

		for (size_t i = 0; i < gridSize * gridSize; ++i) {
			
			// Delete top chunks
			if (i < gridSize) {
				delete chunks[i];
				chunks[i] = nullptr;
			}

			// Move pointers
			if (i < gridSize * (gridSize - 1)) {
				chunks[i] = chunks[i + gridSize];
			}
			// Create new chunks below previous bot chunks
			else {
				glm::vec3 pos = chunks[i]->getPostition();
				chunks[i] = new Chunk{ nrVertices, pos.x, pos.z + (nrVertices - 1) * spacing, spacing, yscale, 2.5f };
			}
		}

		currentChunk = chunks[gridSize * gridSize / 2];

		std::cout << "down\n";
		break;

	case chunkChecker::left:

		for (size_t i = 0; i < gridSize * gridSize; ++i) {
			
			size_t j = gridSize * gridSize - 1 - i;

			// Delete right chunks
			if (j % gridSize == gridSize - 1) {
				delete chunks[j];
				chunks[j] = nullptr;
			}

			// Move pointers
			if (j % gridSize != 0) {
				chunks[j] = chunks[j - 1];
			}
			// Create new chunks to the left of previous left chunks
			else {
				glm::vec3 pos = chunks[j]->getPostition();
				chunks[j] = new Chunk{ nrVertices, pos.x - (nrVertices - 1) * spacing, pos.z, spacing, yscale, 2.5f };
			}
		}

		currentChunk = chunks[gridSize * gridSize / 2];

		std::cout << "left\n";
		break;

	case chunkChecker::right:

		for (size_t i = 0; i < gridSize * gridSize; ++i) {

			// Delete left chunks
			if (i % gridSize == 0) {
				delete chunks[i];
				chunks[i] = nullptr;
			}

			// Move pointers
			if (i % gridSize != gridSize - 1) {
				chunks[i] = chunks[i + 1];
			}
			// Create new chunks to the right of previous right chunks
			else {
				glm::vec3 pos = chunks[i]->getPostition();
				chunks[i] = new Chunk{ nrVertices, pos.x + (nrVertices - 1) * spacing, pos.z, spacing, yscale, 2.5f };
			}
		}

		currentChunk = chunks[gridSize * gridSize / 2];

		std::cout << "right\n";
		break;

	default:
		break;
	}
}

void ChunkHandler::cullTerrainChunk(const std::vector<CameraPlane>& cameraPlanes)
{
	for (Chunk* chunk : chunks) {
		bool outside = false, intersects = false;


		for (const CameraPlane& plane : cameraPlanes) {
			auto [p, n] = chunk->computePN(plane.normal);
			float a = plane.evaluatePlane(n);
			if (a > 0) // outside 
			{
				//chunk->drawChunk = false;
				outside = true;
				//std::cout << "Culling chunk \n";
				break;
			}

			float b = plane.evaluatePlane(p);
			if (b > 0) // intersects
				intersects = true;
		}

		//bounding box intersect, not nessesary for us?
		//if (intersects)
		//	return;

		//bounding box is inside -> draw object
		chunk->drawChunk = !outside;
	}
}
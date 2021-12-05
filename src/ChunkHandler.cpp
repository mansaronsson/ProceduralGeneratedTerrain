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

			chunks.push_back(new Chunk{ nrVertices, xpos, zpos, spacing, yscale, 2.5f });
			chunks.back()->id = index(col, row);
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
	// glm::vec3 p1{ minX, maxY, minZ }, p2{ maxX, maxY, minZ }, p3{ maxX, maxY, maxZ }, p4{ minX, maxY, maxZ },
	// 	p5{ minX, minY, minZ }, p6{ maxX, minY, minZ }, p7{ maxX, minY, maxZ }, p8{ minX, minY, maxZ };

	std::vector<glm::vec3> points{ { minX, maxY, minZ }, { maxX, maxY, minZ }, { maxX, maxY, maxZ }, { minX, maxY, maxZ },
				{ minX, minY, minZ }, { maxX, minY, minZ }, { maxX, minY, maxZ }, { minX, minY, maxZ } };
	//
	//std::cout << glm::to_string(p1) << " " << glm::to_string(p2) << " " << glm::to_string(p3) << '\n'
	//	<< glm::to_string(p4) << " " << glm::to_string(p5) << " " << glm::to_string(p6) << '\n'
	//	<< glm::to_string(p7) << " " << glm::to_string(p8) << '\n' << '\n';

	// boundingBox = BoundingBox{ p1, p2, p3, p4, p5, p6, p7, p8 };
	boundingBox = BoundingBox{ points };
}

ChunkHandler::Chunk::Chunk(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const std::vector<glm::vec3>& bBox, size_t _size)
	: size{_size}
{

	mesh = Mesh{ vertices, indices };
	boundingBox = BoundingBox{ bBox };
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
		return std::pair<glm::vec3, glm::vec3>{boundingBox.getPoint(2), boundingBox.getPoint(4)};
	}
	if (n.x > 0 && n.y > 0 && n.z < 0) { // + + -
		return std::pair<glm::vec3, glm::vec3>{boundingBox.getPoint(1), boundingBox.getPoint(7)};
	}
	if (n.x > 0 && n.y < 0 && n.z > 0) { // + - +
		return std::pair<glm::vec3, glm::vec3>{boundingBox.getPoint(6), boundingBox.getPoint(0)};
	}
	if (n.x > 0 && n.y < 0 && n.z < 0) { // + - - 
		return std::pair<glm::vec3, glm::vec3>{boundingBox.getPoint(5), boundingBox.getPoint(3)};
	}
	if (n.x < 0 && n.y > 0 && n.z > 0) { // - +  +
		return std::pair<glm::vec3, glm::vec3>{boundingBox.getPoint(3), boundingBox.getPoint(5)};
	}
	if (n.x < 0 && n.y > 0 && n.z < 0) { // - + -
		return std::pair<glm::vec3, glm::vec3>{boundingBox.getPoint(0), boundingBox.getPoint(6)};
	}
	if (n.x < 0 && n.y < 0 && n.z > 0) { // - - +
		return std::pair<glm::vec3, glm::vec3>{boundingBox.getPoint(7), boundingBox.getPoint(1)};
	}
	if (n.x < 0 && n.y < 0 && n.z < 0) { // - - -
		return std::pair<glm::vec3, glm::vec3>{boundingBox.getPoint(4), boundingBox.getPoint(2)};
	}
}

// void ChunkHandler::checkChunk(const glm::vec3& camPos)
chunkChecker ChunkHandler::checkChunk(const glm::vec3& camPos)
{
	return currentChunk->isInside(camPos);
}

void ChunkHandler::generateChunk(chunkChecker cc, size_t id)
{
	const float frequency{ 2.5f };	// Should be changed in later versions
	
	glm::vec3 prevPos = chunks[id]->getPostition();
	float posX;
	float posZ;

	switch (cc) {
	case chunkChecker::up:
		posX = prevPos.x;
		posZ = prevPos.z - (nrVertices - 1) * spacing;
		break;
	case chunkChecker::down:
		posX = prevPos.x;
		posZ = prevPos.z + (nrVertices - 1) * spacing;
		break;
	case chunkChecker::left:
		posX = prevPos.x - (nrVertices - 1) * spacing;
		posZ = prevPos.z;
		break;
	case chunkChecker::right:
		posX = prevPos.x + (nrVertices - 1) * spacing;
		posZ = prevPos.z;
		break;
	default:
		break;
	}

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	vertices.reserve(nrVertices * nrVertices);
	indices.reserve(6 * (nrVertices - 2) * (nrVertices - 2) + 3 * nrVertices * 4 - 18); // se notes in lecture 6 

	float minY = std::numeric_limits<float>::max();
	float maxY = std::numeric_limits<float>::min();

	for (int depth = 0; depth < nrVertices; ++depth)
	{
		for (int width = 0; width < nrVertices; ++width) {
			float x = posX + width * spacing;
			float z = posZ + depth * spacing;
			float noiseY = glm::perlin(glm::vec3(x / frequency, z / frequency, 0.1f)); // last component seed 
			noiseY *= yscale;


			vertices.push_back({ { x, noiseY, z } });

			minY = minY > noiseY ? noiseY : minY;
			maxY = maxY < noiseY ? noiseY : maxY;


			unsigned int i1, i2, i3, i4;
			if (width < nrVertices - 1 && depth < nrVertices - 1) {
				i1 = chunkIndex(width, depth); //current
				i2 = chunkIndex(width, depth + 1); //bottom
				i3 = chunkIndex(width + 1, depth + 1); //bottom right
				i4 = chunkIndex(width + 1, depth); // right 

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

	//compute normal by weighting all connected triangles
	for (int depth = 0; depth < nrVertices; ++depth)
	{
		for (int width = 0; width < nrVertices; ++width) {

			//neighbor flags 
			bool up, left, right, down;
			up = depth - 1 >= 0;
			down = depth + 1 < nrVertices;
			left = width - 1 >= 0;
			right = width + 1 < nrVertices;

			int counter{ 0 };
			glm::vec3 normal{ 0.0f,0.0f,0.0f };

			glm::vec3 v0 = vertices[chunkIndex(width, depth)].position; //current
			//t1 & t6
			if (up && left)
			{
				glm::vec3 v1 = vertices[chunkIndex(width - 1, depth - 1)].position; //nw
				glm::vec3 v2 = vertices[chunkIndex(width - 1, depth)].position; //w
				glm::vec3 v3 = vertices[chunkIndex(width, depth - 1)].position; //n
				glm::vec3 e1 = v1 - v0;
				glm::vec3 e2 = v2 - v0;
				glm::vec3 e3 = v3 - v0;
				normal += glm::cross(e1, e2);
				normal += glm::cross(e3, e1);
				counter += 2;

			}
			//t2
			if (left && down) {
				glm::vec3 v1 = vertices[chunkIndex(width - 1, depth)].position; //w
				glm::vec3 v2 = vertices[chunkIndex(width, depth + 1)].position; //s
				glm::vec3 e1 = v1 - v0;
				glm::vec3 e2 = v2 - v0;
				normal += glm::cross(e1, e2);
				++counter;
			}
			//t3 & t4
			if (down && right) {
				glm::vec3 v1 = vertices[chunkIndex(width, depth + 1)].position; //s
				glm::vec3 v2 = vertices[chunkIndex(width + 1, depth + 1)].position; //se
				glm::vec3 v3 = vertices[chunkIndex(width + 1, depth)].position; //e
				glm::vec3 e1 = v1 - v0;
				glm::vec3 e2 = v2 - v0;
				glm::vec3 e3 = v3 - v0;
				normal += glm::cross(e1, e2);
				normal += glm::cross(e3, e1);
				counter += 2;
			}
			//t5
			if (up && right) {
				glm::vec3 v1 = vertices[chunkIndex(width + 1, depth)].position; //e
				glm::vec3 v2 = vertices[chunkIndex(width + 1, depth - 1)].position; //ne
				glm::vec3 e1 = v1 - v0;
				glm::vec3 e2 = v2 - v0;
				normal += glm::cross(e1, e2);
				++counter;
			}

			normal /= counter;
			normal = glm::normalize(normal);
			vertices[chunkIndex(width, depth)].normal = normal;
		}
	}

	//create boundingbox
	float minX = posX;
	float maxX = posX + (nrVertices - 1) * spacing;
	float minZ = posZ;
	float maxZ = posZ + (nrVertices - 1) * spacing;

	std::vector<glm::vec3> bBox{ { minX, maxY, minZ }, { maxX, maxY, minZ }, { maxX, maxY, maxZ }, { minX, maxY, maxZ },
		{ minX, minY, minZ }, { maxX, minY, minZ }, { maxX, minY, maxZ }, { minX, minY, maxZ } };

	renderQ.push({vertices, indices, bBox, cc, id});
}


// TODO: Fix the memory leaks this function creates
void ChunkHandler::updateChunks(const glm::vec3& camPos)
{
	chunkChecker cc{ this->checkChunk(camPos) };

	std::vector<size_t> ids;
	switch (cc)
	{
	case chunkChecker::inside:
		break;

	case chunkChecker::up:

		currentChunk = chunks[currentChunk->id - gridSize];
		moveQ.push(cc);
		break;

	case chunkChecker::down:

		currentChunk = chunks[currentChunk->id + gridSize];
		moveQ.push(cc);
		break;

	case chunkChecker::left:

		currentChunk = chunks[currentChunk->id - 1];
		moveQ.push(cc);
		break;

	case chunkChecker::right:

		currentChunk = chunks[currentChunk->id + 1];
		moveQ.push(cc);
		break;
	}

	// MoveQ evaluator
	if (!moveQ.empty() && renderCounter == gridSize)
	{
		renderCounter = 0;
		std::vector<size_t> ids;

		chunkChecker cc = moveQ.front();
		moveQ.pop();

		switch (cc)
		{
		case chunkChecker::up:
			for (size_t id = 0; id < gridSize; ++id) {
				ids.push_back(id);
			}
			break;

		case chunkChecker::down:
			for (size_t id = gridSize * gridSize; id > gridSize * (gridSize - 1); --id) {
				ids.push_back(id - 1);
			}
			break;

		case chunkChecker::left:
			for (size_t id = 0; id < gridSize; ++id) {
				ids.push_back(id * gridSize);
			}
			break;

		case chunkChecker::right:
			for (size_t id = 0; id < gridSize; ++id) {
				ids.push_back(id * gridSize + gridSize - 1);
			}
			break;
		}

		// Multithreading
		for (auto id : ids)
		{
			std::thread t(&ChunkHandler::generateChunk, this, cc, id);	
			t.detach();
		}
	}


	while (!renderQ.empty()) {

		// 0: Vertex, 1: unsigned int, 2: vec3, 3: chunkChecker, 4: size_t
		chunkInfo ci = renderQ.front();
		renderQ.pop();
		++renderCounter;

		size_t id = std::get<size_t>(ci);
		Chunk* newChunk = new Chunk{ std::get<0>(ci), std::get<1>(ci), std::get<2>(ci), nrVertices };
		newChunk->id = id;
		Chunk* temp;

		switch (std::get<chunkChecker>(ci))
		{
		case chunkChecker::up:
			temp = chunks[id + gridSize * (gridSize - 1)];	// Chunk to be deleted

			for (size_t i = 1; i < gridSize; ++i) {

				chunks[id + gridSize * (gridSize - i)] = chunks[id + gridSize * (gridSize - i - 1)];
				chunks[id + gridSize * (gridSize - i)]->id += gridSize;
			}

			chunks[id] = newChunk;
			delete temp;
			temp = nullptr;

			break;

		case chunkChecker::down:
			temp = chunks[id - gridSize * (gridSize - 1)];	// Chunk to be deleted

			for (size_t i = 1; i < gridSize; ++i) {

				chunks[id - gridSize * (gridSize - i)] = chunks[id - gridSize * (gridSize - i - 1)];
				chunks[id - gridSize * (gridSize - i)]->id -= gridSize;
			}

			chunks[id] = newChunk;
			delete temp;
			temp = nullptr;

			break;

		case chunkChecker::left:
			temp = chunks[id + gridSize - 1];	// Chunk to be deleted

			for (size_t i = 1; i < gridSize; ++i) {

				chunks[id + gridSize - i] = chunks[id + gridSize - i - 1];
				++chunks[id + gridSize - i]->id;
			}

			chunks[id] = newChunk;
			delete temp;
			temp = nullptr;

			break;

		case chunkChecker::right:
			temp = chunks[id - gridSize + 1];	// Chunk to be deleted

			for (size_t i = 1; i < gridSize; ++i) {

				chunks[id - gridSize + i] = chunks[id - gridSize + i + 1];
				--chunks[id - gridSize + i]->id;
			}

			chunks[id] = newChunk;
			delete temp;
			temp = nullptr;

			break;

		default:
			break;
		}


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
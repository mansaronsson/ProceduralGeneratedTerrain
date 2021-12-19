#include "..\header\ChunkHandler.h"
#include <iostream>

ChunkHandler::ChunkHandler(unsigned int _gridSize, unsigned int _nrVertices, float _spacing, float _yscale)
	: gridSize{ (_gridSize % 2 == 0 ? (_gridSize + 1) : _gridSize) }, nrVertices{ _nrVertices }, spacing{ _spacing }, yscale{ _yscale }, currentChunk{ nullptr }
{
	//unsigned int size = nrVertices; //two extra rows / columns for the skirts
	unsigned int maxLOD{ 16 };
	float width = (nrVertices - 1) * spacing; //width of 1 chunk, -3 due to extra skirts

	std::vector<std::future<Chunk*>> f;
	std::vector<std::thread> t;
	for (int row = 0; row < gridSize; ++row) {
		float zpos = -width * (static_cast<float>(gridSize) / 2.0f) + row * width;
		for (int col = 0; col < gridSize; ++col) {
			float xpos = -width * (static_cast<float>(gridSize) / 2.0f) + col * width;

			// Threads the creation of the grid
			for (int lod = maxLOD; lod > 0; lod /= 2) {

				std::promise<Chunk*> p;
				f.push_back(p.get_future());
				std::thread th = std::thread(&ChunkHandler::generateLOD, this, std::move(p), nrVertices, lod, xpos, zpos, spacing, index(col, row, gridSize));
				t.push_back(std::move(th));
			}
		}
	}

	// Join all threads
	for (auto& th : t) {
		th.join();
	}

	for (auto& fu : f) {
		Chunk* c = fu.get();
		c->bakeMeshes();
		initializeChunk(c);
	}

	currentChunk = chunks[gridSize * gridSize / 2];
}

// Threaded Chunk creation for every LOD
void ChunkHandler::generateLOD(std::promise<Chunk*>&& p, unsigned int nrVerticies, unsigned int _lod, float xpos, float zpos, float _spacing, unsigned int id) {
	Chunk* chunkLOD = new Chunk{ nrVerticies, _lod, xpos, zpos, _spacing, id };
	p.set_value(chunkLOD);
}

void ChunkHandler::initializeChunk(Chunk* c) {
	// Empty vector
	if (chunks.empty()) {
		chunks.push_back(c);
	}
	else {
		insertLOD(c, true);
	}
}

/// <summary>
/// Inserts new LOD Chunks in the correct position in the LOD-list. If init == true, any Chunk that can't find its parent will be inserted instead of deleted.
/// </summary>
/// <param name="c"></param>
/// <param name="init"></param>
void ChunkHandler::insertLOD(Chunk* c, bool init) {

	float constexpr epsilon{ 1e-3 };

	auto it = chunks.begin();
	while (it != chunks.end()) {
		// Finds chunks in the same position
		if (glm::distance(c->getPostition(), (*it)->getPostition()) < epsilon) {

			c->id = (*it)->id;
			Chunk* temp = (*it);

			// If c is new lowest lod, places it in chunks vector
			if (temp->lod < c->lod) {
				c->higherLod = temp;
				chunks[c->id] = c;
			}
			else {

				// All LOD gets in correct order
				Chunk* parent = nullptr;
				while (temp != nullptr && c->lod < temp->lod) {
					c->higherLod = temp->higherLod;
					temp->higherLod = c;

					if (parent) parent->higherLod = temp;

					parent = temp;
					temp = c->higherLod;

				}
			}
			break;
		}
		// If this is the initilization of the grid, insert Chunk into chunks vector
		else if (init && c->id < (*it)->id) {
			chunks.insert(it, c);
			break;
		}
		++it;
	}
	// If this is the initilization of the grid, insert Chunk into chunks vector
	if (init && it == chunks.end()) {
		chunks.push_back(c);
	}
	// The created chunk is now out of scope and is deleted instead
	else if(it == chunks.end()) {
		delete c;
		c = nullptr;
	}
}
 
glm::vec3 ChunkHandler::Chunk::createPointWithNoise(float x, float z, float* minY, float* maxY ) const {
	/*** Apply noise to the height ie. y component using fbm ***/
	int octaves = 8;
	float noiseSum = 0.0f;
	float seed = 0.1f;
	float amplitude = 6.0f;
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
	float groundlevel = -1.51f;
	if (noiseY < groundlevel)
		noiseY = groundlevel;

	glm::vec3 pos{ x, noiseY, z };

	if (minY != nullptr && maxY != nullptr) {
		*minY = *minY > noiseY ? noiseY : *minY;
		*maxY = *maxY < noiseY ? noiseY : *maxY;
	}
	return pos;
}

std::pair<float, float> ChunkHandler::Chunk::computeXZpos(int width, int depth) const {
	float x = XPOS + (width - 1) * SPACING;
	float z = ZPOS + (depth - 1) * SPACING;
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
	normal += glm::cross(e2, e3);
	//t5
	e1 = e - v0;
	e2 = n - v0;
	normal += glm::cross(e1, e2);
	//Divide by the 6 normals we summed together and normalize vector 
	normal /= 6;
	normal = glm::normalize(normal);

	return normal;
}

void ChunkHandler::Chunk::bakeMeshes() {
	mesh = Mesh{ vertices, indices };
	boundingBox = BoundingBox{ points };

	//if (higherLod)
	//	higherLod->bakeMeshes();
}

glm::vec3 ChunkHandler::Chunk::setColorFromLOD() {
	switch (lod)
	{
	case 1:
		return glm::vec3{ 0.1f, 0.6f, 0.1f }; //green
	case 2:
		return glm::vec3{ 0.5f, 1.0f, 0.0f }; //yellowgreenish
	case 4: 
		return glm::vec3{ 1.0f, 1.0f, 0.0f }; //yellow
	case 8:
		return glm::vec3{ 1.0f, 0.5f, 0.0f }; //orange
	case 16:
		return glm::vec3{ 1.0f, 0.2f, 0.2f }; //red
	default:
		return glm::vec3{ 0.7f, 0.7f, 0.7f };
		break;
	}
}

ChunkHandler::Chunk::Chunk(unsigned int _nrVertices, unsigned int _lod, float xpos, float zpos, float _spacing, unsigned int _id) :
	lod{ _lod }, nrVertices { (_nrVertices - 1) / _lod + 3 }, XPOS{ xpos }, ZPOS{ zpos }, SPACING{ _spacing * _lod }, id{ _id } {

	vertices.reserve(nrVertices * nrVertices);
	indices.reserve(6 * (nrVertices - 2) * (nrVertices - 2) + 3 * nrVertices * 4 - 18); // se notes in lecture 6 
	
	color = setColorFromLOD();
	//Need min and max height of this chunk to compute the bounding box
	float minY = std::numeric_limits<float>::max();
	float maxY = std::numeric_limits<float>::min();

	/*** Compute vertex positions and indices ***/
	for (int depth = 0; depth < nrVertices; ++depth)
	{
		for (int width = 0; width < nrVertices; ++width) {
			float x = xpos + (width - 1) * SPACING;
			float z = zpos + (depth - 1) * SPACING;

			/*** Skirts should be at the same x and z position as the next / previous vertex ***/
			if (depth == 0) {
				z = zpos + (depth + 0) * SPACING;
			}
			if (depth == nrVertices - 1) {
				z = zpos + (depth - 2) * SPACING;
			}
			if (width == 0) {
				x = xpos + (width + 0) * SPACING;
			}
			if (width == nrVertices - 1) {
				x = xpos + (width - 2) * SPACING;
			}

			if (depth == 0 || depth == nrVertices - 1 || width == 0 || width == nrVertices - 1) //edges of grid ie. skirts
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
			vertices.back().color = color;

			//add indices to create triangle list
			if (width < nrVertices - 1 && depth < nrVertices - 1) {
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
	for (int width = 1; width < nrVertices - 1; ++width) {
		glm::vec3 v0 = vertices[index(width, depth)].position; //current vertex
		glm::vec3 ne, n, nw, w, sw, s, se, e;
		//Find all surrounding vertices 

		//NE och SW behövs ej
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
		if (width == nrVertices - 2) { //top rightmost vertex 
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
		if (width == nrVertices -2) {
			vertices[index(width + 1, depth - 1)].normal = normal; //ne skirt
			vertices[index(width + 1, depth)].normal = normal; //e skirt
		}
	} //End of top row

	//Bottom row visit each column
	depth = nrVertices - 2;
	for (int width = 1; width < nrVertices - 1; ++width) {
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
		if (width == nrVertices - 2) { //bottom rightmost vertex 
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
		if (width == nrVertices - 2) {
			vertices[index(width + 1, depth + 1)].normal = normal; //se skirt
			vertices[index(width + 1, depth)].normal = normal; //e skirt
		}
	}//End of bottom row

	//Left column visit every row except top and bottom
	int width = 1;
	for (int depth = 2; depth < nrVertices - 2; ++depth) { //+2 - 2 range skips top and bottom row since they are already computed
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
	width = nrVertices - 2;
	for (int depth = 2; depth < nrVertices - 2; ++depth) { //+2 - 2 range skips top and bottom row since they are already computed
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
	for (int depth = 2; depth < nrVertices - 2; ++depth)
	{
		for (int width = 2; width < nrVertices -2; ++width) {
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

	//mesh = Mesh{ vertices, indices }; 

	//create boundingbox ignoring the extra row and column added by the skirts
	//max x and z already had size - 1 before skirts were added 
	float minX = xpos;
	float maxX = xpos + (nrVertices - 3) * SPACING;
	float minZ = zpos;
	float maxZ = zpos + (nrVertices - 3) * SPACING;

	//Important theese are given in correct order -> see BoundingBox.h ctor
	points = std::vector<glm::vec3>{ { minX, maxY, minZ }, { maxX, maxY, minZ }, { maxX, maxY, maxZ }, { minX, maxY, maxZ },
				{ minX, minY, minZ }, { maxX, minY, minZ }, { maxX, minY, maxZ }, { minX, minY, maxZ } };
}

chunkChecker ChunkHandler::Chunk::checkMovement(const glm::vec3& pos)
{
	chunkChecker cc = inside;
	auto v1 = mesh.vertices[0].position; //first vertex in chunk
	auto v2 = mesh.vertices[mesh.vertices.size() - 1].position; // last vertex in chunk

	//Check if position is within chunk borders spanned by v1 and v2 in the x,z plane
	if (pos.z < v1.z) {
		cc = up;
		return cc;
	}
	if (pos.z > v2.z) {
		cc = down;
		return cc;
	}
	if (pos.x < v1.x) {
		cc = left;
		return cc;
	}
	if (pos.x > v2.x) {
		cc = right;
		return cc;
	}
	//Position is inside borders
	return cc;
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

chunkChecker ChunkHandler::checkChunk(const glm::vec3& camPos)
{
	return currentChunk->checkMovement(camPos);
}

/// <summary>
/// Constructs a new chunk (with all LOD) and adds it together with the movement (chunkChecker) that triggered the generation to the render queue. The chunk mesh is not initiated so it can be multi-threaded.
/// </summary>
/// <param name="newPos"></param>
/// <param name="nrVeritices"></param>
/// <param name="_spacing"></param>
/// <param name="id"></param>
/// <param name="inside"></param>
/// 
/// unsigned int _nrVertices, unsigned int _lod, float xpos, float zpos, float _spacing, unsigned int _id
void ChunkHandler::generateChunk(const std::pair<float, float>& newPos, unsigned int nrVeritices, float _spacing, unsigned int id, chunkChecker cc)
{
	// First create and render the lowest LOD
	unsigned int lod = 16;
	renderQ.push({ new Chunk{ nrVertices, lod, newPos.first, newPos.second, _spacing, id }, cc });

	// Create other LOD after
	for (lod /= 2; lod > 0; lod /= 2) {
			lodQ.push(new Chunk{ nrVertices, lod, newPos.first, newPos.second, _spacing, id });
	}
}

/// <summary>
/// Updates which chunks that are rendered based on camera position. New chunks are genereted by multi-threading.
/// </summary>
/// <param name="camPos"></param>
void ChunkHandler::updateChunks(const glm::vec3& camPos)
{
	// Checks movement and adds it to the move queue if outside the current chunk. Updates current chunk according to the camera position.
	chunkChecker cc{ this->checkChunk(camPos) };	// Enum that hold if and which direction the camera moved 
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

	// MoveQ evaluator, only adds chunks to the render queue if all chunks from the previous move have been generated.
	if (!moveQ.empty() && renderCounter == gridSize)
	{
		std::lock_guard<std::mutex> lock(mu);	// Thread safe

		renderCounter = 0;
		std::vector<unsigned int> ids;

		chunkChecker cc = moveQ.front();
		moveQ.pop();
		
		int row, col;
		switch (cc)
		{
		case chunkChecker::up:
			row = 0;
			for (col = 0; col < gridSize; ++col) {
				ids.push_back(index(col, row, gridSize));
			}
			break;

		case chunkChecker::down:
			row = gridSize - 1;
			for(col = 0; col < gridSize; ++col) {
				ids.push_back(index(col, row, gridSize));
			}
			break;

		case chunkChecker::left:
			col = 0;
			for(row = 0; row < gridSize; ++row) {
				ids.push_back(index(col, row, gridSize));
			}
			break;

		case chunkChecker::right:
			col = gridSize - 1;
			for(row = 0; row < gridSize; ++row) {
				ids.push_back(index(col, row, gridSize));
			}
			break;
		}

		// Multi-threading
		for (auto id : ids)
		{
			auto newPos = newChunkPosition(cc, id);
			std::thread t(&ChunkHandler::generateChunk, this, newPos, nrVertices, spacing, id, cc);
			t.detach();
		}
	}

	// Swaps chunks in the chunks grid when a new chunk has been rendered. Updates all chunk ids to match it's position in the chunks grid.
	while (!renderQ.empty()) 
	{
		std::lock_guard<std::mutex> lock(mu);	// Thread safe

		chunkInfo ci = renderQ.front();	// <Chunk*, chunkChecker>
		renderQ.pop();
		++renderCounter;
		
		Chunk* newChunk = std::get<Chunk*>(ci);
		newChunk->bakeMeshes();

		Chunk* temp;
		unsigned int id = newChunk->id;

		int row, col;
		switch (std::get<chunkChecker>(ci))
		{
		case chunkChecker::up:
			temp = chunks[id + gridSize * (gridSize - 1)];	// Chunk to be deleted

			col = id;
			for (row = gridSize - 1; row > 0; --row) {

				chunks[index(col, row, gridSize)] = chunks[index(col, row - 1, gridSize)];
				chunks[index(col, row, gridSize)]->id += gridSize;
			}

			chunks[id] = newChunk;
			delete temp;
			temp = nullptr;

			break;

		case chunkChecker::down:
			col = id % gridSize;
			temp = chunks[col];	// Chunk to be deleted

			for (row = 0; row < gridSize - 1; ++row) {

				chunks[index(col, row, gridSize)] = chunks[index(col, row + 1, gridSize)];
				chunks[index(col, row, gridSize)]->id -= gridSize;
			}

			chunks[id] = newChunk;
			delete temp;
			temp = nullptr;

			break;

		case chunkChecker::left:
			temp = chunks[id + gridSize - 1];	// Chunk to be deleted

			row = id / gridSize;
			for (col = gridSize - 1; col > 0; --col) {

				chunks[index(col, row, gridSize)] = chunks[index(col - 1, row, gridSize)];
				++chunks[index(col, row, gridSize)]->id;
			}

			chunks[id] = newChunk;
			delete temp;
			temp = nullptr;

			break;

		case chunkChecker::right:
			temp = chunks[id - gridSize + 1];	// Chunk to be deleted

			row = id / gridSize;
			for (col = 0; col < gridSize - 1; ++col) {

				chunks[index(col, row, gridSize)] = chunks[index(col + 1, row, gridSize)];
				--chunks[index(col, row, gridSize)]->id;
			}

			chunks[id] = newChunk;
			delete temp;
			temp = nullptr;

			break;

		default:
			break;
		}
	}
	
	// Insert newly created lod chunks into correct lod-list.
	while (!lodQ.empty()) {

		std::lock_guard<std::mutex> lock(mu);	// Thread safe

		Chunk* c = lodQ.front();
		lodQ.pop();
		c->bakeMeshes();
		insertLOD(c);
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

std::pair<float, float> ChunkHandler::newChunkPosition(chunkChecker cc, unsigned int gridId) const
{
	glm::vec3 prevPos = chunks[gridId]->getPostition();
	std::pair<float, float> newPos;

	switch (cc) {
	case chunkChecker::up:
		newPos.first = prevPos.x;
		newPos.second = prevPos.z - (nrVertices - 1) * spacing;
		break;
	case chunkChecker::down:
		newPos.first = prevPos.x;
		newPos.second = prevPos.z + (nrVertices - 1) * spacing;
		break;
	case chunkChecker::left:
		newPos.first = prevPos.x - (nrVertices - 1) * spacing;
		newPos.second = prevPos.z;
		break;
	case chunkChecker::right:
		newPos.first = prevPos.x + (nrVertices - 1) * spacing;
		newPos.second = prevPos.z;
		break;
	default:
		break;
	}

	return newPos;
}

#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "Matrix.h"
#include "ShaderProgram.h"
#include <vector>
#include <istream>
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;
void copy(float* a1, float* a2, int size) {
	for (int i = 0; i < size; ++i) {
		a2[i] = a1[i];
	}
}
enum entityType{ STATIC_LADDER, STATIC_LADDERTOP, STATIC_END, ENTITY_PLAYER, ENTITY_COIN, ENTITY_ENEMY};
enum inputAction{ JUMP, DUCK, LEFT, RIGHT, IDLE};
class entity {
public:
	entity(float x, float y, entityType t, GLuint tex[]) : type(t){
		position[0] = x;
		position[1] = y;
		velocity[0] = 0.0f;
		velocity[1] = 0.0f;
		for (int i = 0; i < 5; ++i) {
			textures[i] = tex[i];
		}
		float u, v;
		float tSize;
		if (type == ENTITY_COIN) {
			texture = textures[4];
			float vert[12] = { -0.25f, 0.25f, -0.25f, -0.25f, 0.25f, 0.25f, 0.25f, -0.25f, -0.25f, -0.25f, 0.25f, 0.25f };
			float s[2] = { 0.25f, 0.25f };
			float tex[12] = { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f };
			copy(vert, verticies, 12);
			copy(s, size, 2);
			copy(tex, textureCoordinates, 12);
		}
		else if (type == ENTITY_PLAYER) {//verterticies and textures will change 
			texture = textures[2];
			float s[2] = { 0.5f, 0.7f };
			copy(s, size, 2);
		}
		else if (type == ENTITY_ENEMY) {//texture coordinates will change;
			texture = textures[3];
			float vert[12] = { -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f,-0.5f, 0.5f, 0.5f };
			float s[2] = { 0.39f, 0.155f };
			copy(vert, verticies, 12);
			copy(s, size, 2);
			velocity[0] = 1.0f;
		}
		else if (type == STATIC_LADDER) {
			tSize = 1024.0f;
			u = 7 * tilesize;
			v = 1 * tilesize;
			texture = textures[1];
			float vert[12] = { -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f,-0.5f, 0.5f, 0.5f };
			float s[2] = { 0.5f, 0.5f };
			float tex[12] = { u / tSize, v / tSize, u / tSize, (v + tilesize) / tSize, (u + tilesize) / tSize, v / tSize,
			(u + tilesize) / tSize, (v + tilesize) / tSize, u / tSize, (v + tilesize) / tSize, (u + tilesize) / tSize, v / tSize };
			copy(vert, verticies, 12);
			copy(s, size, 2);
			copy(tex, textureCoordinates, 12);
		}
		else if (type == STATIC_LADDERTOP) {
			tSize = 1024.0f;
			u = 7 * tilesize;
			v = 1 * tilesize;
			texture = textures[1];
			float vert[12] = { -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f,-0.5f, 0.5f, 0.5f };
			float s[2] = { 0.5f, 0.5f };
			float tex[12] = { u / tSize, v / tSize, u / tSize, (v + tilesize) / tSize, (u + tilesize) / tSize, v / tSize,
				(u + tilesize) / tSize, (v + tilesize) / tSize, u / tSize, (v + tilesize) / tSize, (u + tilesize) / tSize, v / tSize };
			copy(vert, verticies, 12);
			copy(s, size, 2);
			copy(tex, textureCoordinates, 12);
		}
		else if (type == STATIC_END) {
			tSize = 1024.0f;
			u = 7 * tilesize;
			v = 1 * tilesize;
			texture = textures[1];
			float vert[12] = { -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f,-0.5f, 0.5f, 0.5f };
			float s[2] = { 0.5f, 0.5f };
			float tex[12] = { u / tSize, v / tSize, u / tSize, (v + tilesize) / tSize, (u + tilesize) / tSize, v / tSize,
				(u + tilesize) / tSize, (v + tilesize) / tSize, u / tSize, (v + tilesize) / tSize, (u + tilesize) / tSize, v / tSize };
			copy(vert, verticies, 12);
			copy(s, size, 2);
			copy(tex, textureCoordinates, 12);
		}
	}
	void render(ShaderProgram* program, Matrix* modelviewMatrix, Matrix* projectionMatrix) {
		modelviewMatrix->Identity();
		modelviewMatrix->Translate(position[0], position[1], 0.0f);
		changeAnimation(modelviewMatrix);//set u,v coordinates & vertex coordinates for textures based on animation state.
		program->SetProjectionMatrix(*projectionMatrix);
		program->SetModelviewMatrix(*modelviewMatrix);
		glBindTexture(GL_TEXTURE_2D, texture);
		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, verticies);
		glEnableVertexAttribArray(program->positionAttribute);
		glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, textureCoordinates);
		glEnableVertexAttribArray(program->texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	void changeAnimation(Matrix* modelviewMatrix) {
		float u, v, width, height, texWidth, texHeight;
		switch (type) {
		case ENTITY_PLAYER:
			texWidth = 508.0f;
			texHeight = 288.0f;
			if (state == JUMP) {
				texture = textures[2];
				width = 67.0f;
				height = 94.0f;
				u = 438.0f;
				v = 93.0f;
			}
			else if (state == LEFT || state == RIGHT) {
				texture = textures[2];
				width = 72.0f;
				height = 97.0f;
				if (animationTime > 0.1f) {
					animationTime = 0;
					animationState = (animationState + 1) % 11;
				}
				if (state == LEFT) {
					modelviewMatrix->Scale(-1.0f, 1.0f, 1.0f);
				}
				switch (animationState) {
				case 0:
					u = 0.0f;
					v = 0.0f;
					break;
				case 1:
					u = 73.0f;
					v = 0.0f;
					break;
				case 2:
					u = 146.0f; 
					v = 0.0f;
					break;
				case 3:
					u = 0.0f;
					v = 98.0f;
					break;
				case 4:
					u = 73.0f;
					v = 98.0f;
					break;
				case 5:
					u = 146.0f;
					v = 98.0f;
					break;
				case 6:
					u = 219.0f;
					v = 0.0f;
					break;
				case 7:
					u = 292.0f;
					v = 0.0f;
					break;
				case 8:
					u = 219.0f;
					v = 98.0f;
					break;
				case 9:
					u = 365.0f;
					v = 0.0f;
					break;
				case 10:
					u = 292.0f;
					v = 98.0f;
					break;
				}
			}
			else if (state == IDLE) {
				texture = textures[2];
				width = 66.0f;
				height = 92.0f;
				u = 0.0f;
				v = 196.0f;
			}
			else if (state == DUCK) {
				texture = textures[2];
				width = 69.0f;
				height = 71.0f;
				u = 365.0f;
				v = 98.0f;
			}
			size[0] = width / 140.0f;
			size[1] = height / 140.0f;
			break;
		case ENTITY_ENEMY:
			texWidth = 353.0f;
			texHeight = 153.0f;
			if (animationTime > 0.5f) {
				animationTime = 0;
				animationState = (animationState + 1) % 2;
			}
			if (state == RIGHT) {
				modelviewMatrix->Scale(-1.0f, 1.0f, 1.0f);
			}
			if (animationState == 0) {
				width = 54.0f;
				height = 31.0f;
				u = 143.0f;
				v = 35.0f;
			}
			else {
				width = 57.0f;
				height = 31.0f;
				u = 67.0f;
				v = 87.0f;
			}
			size[0] = width / 140.0f;
			size[1] = height / 140.0f;
			break;
		default:
			return;
		}
		float vert[12] = { -size[0] , size[1] , -size[0] , -size[1] ,
			size[0] , size[1] , size[0] , -size[1] ,
			-size[0] , -size[1] , size[0] , size[1] };
		copy(vert, verticies, 12);
		float t[12] = { u / texWidth, v / texHeight , u / texWidth , (v + height) / texHeight,
			(u + width) / texWidth, v / texHeight , (u + width) / texWidth , (v + height) / texHeight,
			u / texWidth, (v + height) / texHeight, (u + width) / texWidth , v / texHeight };
		copy(t, textureCoordinates, 12);
	}
	//need to take in account of states to check if moving is appropriate.
	void updateX(float elapsed, Matrix* projectionMatrix) {
		velocity[0] = lerp(velocity[0], 0.0f, elapsed*0.9f);
		velocity[0] += acceleration[0]*elapsed;
		position[0] += velocity[0]*elapsed;
		if (type == ENTITY_PLAYER) {
			projectionMatrix->Identity();
			projectionMatrix->SetOrthoProjection(-10.0f, 10.0f, -7.0f, 7.0f, -1.0f, 1.0f);
			if (position[0] > 10.0f && position[0] < 40.0f) {
				projectionMatrix->Translate(-position[0], 7.0f, 0.0f);
			}
			else  if (position[0] < 10.0f){
				projectionMatrix->Translate(-10.0f, 7.0f, 0.0f);
			}
			else if (position[0] > 40.0f) {
				projectionMatrix->Translate(-40.0f, 7.0f, 0.0f);
			}
		}
	}
	void updateY(float elapsed) {
		velocity[1] = lerp(velocity[1], -8.0f, elapsed* 0.95f);
		velocity[1] += acceleration[1] * elapsed;
		if (fabs(velocity[1]) > 10.0f) {
			velocity[1] = -5.0f;
		}
		position[1] += velocity[1] * elapsed;
		animationTime += elapsed;
	}
	void input(inputAction input, float amount) {
		if (input == DUCK) {
			if (bottomF) {
				velocity[0] = 0.0f;
				state = DUCK;
				animationState = 0;
				animationTime = 0.0f;
			}
		}
		else if (input == LEFT) {
			acceleration[0] = amount;
			if (bottomF && state != LEFT) {
				state = LEFT;
				animationState = 0;
				animationTime = 0.0f;
			}
		}
		else if (input == RIGHT) {
			acceleration[0] = amount;
			if (bottomF && state != RIGHT) {
				state = RIGHT;
				animationState = 0;
				animationTime = 0.0f;
			}
		}
		else if (input == JUMP) {
			if (bottomF) {
				velocity[1] = amount;
				state = JUMP;
				animationState = 0;
				animationTime = 0.0f;
			}
		}
		else {
			if (state == LEFT || state == RIGHT || state == DUCK) {
				velocity[0] = velocity[0] / 2;
				acceleration[0] = 0.0f;
				state = IDLE;
				animationState = 0;
				animationTime = 0.0f;
			}
			if (state == JUMP && bottomF) {
				state = IDLE;
				acceleration[0] = 0.0f;
				velocity[0] = velocity[0] / 2;
				animationState = 0;
				animationTime = 0.0f;
			}
		}
	}
	void changeCollisionFlags(bool t, bool b, bool r, bool l) {
		topF = t;
		bottomF = b;
		rightF = r;
		leftF = l;
		if (bottomF && type == ENTITY_COIN) {
			input(JUMP, 3.0f);
			if (position[1] > -2.0f) {
				velocity[1] = -1.0f;
			}
			bottomF = false;
		}
		if (type == ENTITY_ENEMY) {
			if (rightF) {
				input(LEFT, -1.0f);
			}
			if (leftF) {
				input(RIGHT, 1.0f);
			}
		}
	}
	void translate(float x, float y) {
		position[0] += x;
		position[1] += y;
	}
	bool entityCollision(entity* other) const {
		if (this->top() < other->bottom() || this->bottom() > other->top() ||
			this->right() < other->left() || this->left() > other->right()) {
			return false;
		}
		else {
			return true;
		}
	}
	float lerp(float v0, float v1, float t) {
		return (1.0 - t)*v0 + t*v1;
	}
	void ladderCollision(bool l) {
		nextToLadder = l;
	}
	float bottom()const { return position[1] - size[1]; }
	float top() const { return position[1] + size[1]; }
	float left() const { return position[0] - size[0]; }
	float right() const { return position[0] + size[1]; }
	float xPosition() const { return position[0]; }
	float yPosition() const { return position[1]; }
	float getType() const { return type; }
private:
	float position[2];
	float velocity[2];
	float acceleration[2] = { 0.0f, 0.0f };
	float size [2];
	float verticies [12];
	float textureCoordinates [12];

	float tilesize = 70.0f;
	GLuint texture;
	GLuint textures[5];
	entityType type;
	inputAction state = JUMP;
	int animationState = 0;
	float animationTime = 0.0f;
	bool nextToLadder = false;

	//contact flags
	bool topF = false;
	bool bottomF = false;
	bool rightF = false;
	bool leftF= false;
};
class Gamestate {
public:
	Gamestate(std::ifstream* map, GLuint tex[]) {
		for (int i = 0; i < 5; ++i) {
			textures[i] = tex[i];
		}
		std::string line;
		while (getline(*map, line)) {
			if (line == "[header]") {
				if (!readHeader(map)) {
					assert(false);
				}
			}
			else if (line == "[layer]") {
				readLayer(map);
			}
			else if (line == "[Entities]") {
				readObjects(map, tex);
			}
		}
	}
	bool readHeader(std::ifstream* map) {
		std::string line;
		width = -1;
		height = -1;
		while (getline(*map, line)) {
			if (line == "") { break; }
			std::istringstream sStream(line);
			std::string key, value;
			getline(sStream, key, '=');
			getline(sStream, value);
			if (key == "width") {
				width = atoi(value.c_str());
			}
			else if (key == "height") {
				height = atoi(value.c_str());
			}
			else if (key == "tilewidth") {
				tilesize = atoi(value.c_str());
			}
		}
		if (width == -1 || height == -1) {
			return false;
		}
		else { // allocate our map data
			int** tiles = new int*[height];
			for (int i = 0; i < height; ++i) {
				tiles[i] = new int[width];
			}
			tilemap = tiles;
			return true;
		}
	}
	bool readLayer(std::ifstream* map) {
		std::string line;
		while (getline(*map, line)) {
			if (line == "") { break; }
			std::istringstream sStream(line);
			std::string key, value;
			getline(sStream, key, '=');
			getline(sStream, value);
			if (value == "Terrain"){
				getline(*map, line);
				createTileMap(map);
			}
		}
		return true;
	}
	void readObjects(std::ifstream* map, GLuint textures[]) {
		std::string line;
		entityType type;
		while (getline(*map, line)) {
			if (line == "") { break; }
			std::istringstream sStream(line);
			std::string key, value;
			getline(sStream, key, '=');
			getline(sStream, value);
			if (key == "type") {
				if (value == "coin") {
					type = ENTITY_COIN;
				}
				else if (value == "player") {
					type = ENTITY_PLAYER;
				}
				else if (value == "enemy") {
					type = ENTITY_ENEMY;
				}
			}
			else if (key == "location") {
				std::istringstream lineStream(value);
				std::string xPosition, yPosition;
				getline(lineStream, xPosition, ',');
				getline(lineStream, yPosition, ',');
				float x = atoi(xPosition.c_str());
				float y= atoi(yPosition.c_str())*-1;
				dynamics.push_back(new entity(x, y, type, textures));
			}
		}
	}
	void createTileMap(std::ifstream* map) {
		std::string line;
		for (int y = 0; y < height; ++y) {
			getline(*map, line);
			std::istringstream lineStream(line);
			std::string tile;
			for (int x = 0; x < width; ++x) {
				getline(lineStream, tile, ',');
				int val = atoi(tile.c_str());
				if (val > 0) {  //  the tiles in this format are indexed from 1 not 0
					tilemap[y][x] = val;
				}
				else {
					tilemap[y][x] = 0;
					}
				}
			}
	}
	void input(const inputAction input) {
		if (input == DUCK) {
			dynamics[0]->input(DUCK, 0.0f);
		}
		else if (input == LEFT) {
			dynamics[0]->input(LEFT, -5.0f);
		}
		else if (input == RIGHT) {
			dynamics[0]->input(RIGHT, 5.0f);
		}
		else if (input == JUMP) {
			dynamics[0]->input(JUMP, 7.0f);
		}
		else { //idle
			dynamics[0]->input(IDLE, 0.0f);
		}
	}
	void render(ShaderProgram* program, Matrix* modelviewMatrix, Matrix* projectionMatrix) {
		dynamics[0]->render(program, modelviewMatrix, projectionMatrix);	//render player and set projectionMatrix
		if (!runOnce){
			renderStatics(program, modelviewMatrix, projectionMatrix);			//renders terrain
			runOnce = true;
		}
		else {
			modelviewMatrix->Identity();
			program->SetModelviewMatrix(*modelviewMatrix);
			glVertexAttribPointer(program->vertexShader, 2, GL_FLOAT, false, 0, staticVerticies.data());
			glEnableVertexAttribArray(program->vertexShader);
			GLuint textureMap = textures[1];
			glBindTexture(GL_TEXTURE_2D, textureMap);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, staticTextures.data());
			glEnableVertexAttribArray(program->texCoordAttribute);
			glDrawArrays(GL_TRIANGLES, 0, staticVerticies.size() / 2);
		}
		for (int i = 1; i < dynamics.size(); ++i) {						//render dynamic objects except player
			dynamics[i]->render(program, modelviewMatrix, projectionMatrix);
		}
		float x, y;
		y = -1.0f;
		if (dynamics[0]->xPosition() < 10.0f) {
			x = 1.0f;
		}
		else if (dynamics[0]->xPosition() > 40.0f) {
			x = 31.0f;
		}
		else {
			x = dynamics[0]->xPosition() - 9.0f;
		}
		modelviewMatrix->Identity();
		modelviewMatrix->Translate(x, y, 0.0f);
		DrawText(program, modelviewMatrix, projectionMatrix, "SCORE:", 0.5f, 0.2f);
		modelviewMatrix->Identity();
		modelviewMatrix->Translate(x+4.0f, y, 0.0f);
		std::stringstream sScore;
		sScore << score;
		DrawText(program, modelviewMatrix, projectionMatrix, sScore.str(), 0.5f, 0.0f);
	}
	bool setStaticTextures(int xCoordinate, int yCoordinate, ShaderProgram* program, Matrix* modelviewMatrix, Matrix* projectionMatrix, std::vector<float>* tex) {
		//binds the correct texture to draw;
		float textureSize = 350.0f;
		float x, y;
		switch (tilemap[yCoordinate][xCoordinate]) {
		case 0:
			return false;
		case 26: //flat grass
			x = 2 * tilesize;
			y = 1 * tilesize;
			break;
		case 204: //wooden crate:
			x = 0 * tilesize;
			y = 0 * tilesize;
			break;
		case 31: //right sharp grass edge
			x = 3 * tilesize;
			y = 2 * tilesize;
			break;
		case 38: //left sharp grass edge
			x = 2 * tilesize;
			y = 2 * tilesize;
			break;
		case 30: //dirt block
			x = 1 * tilesize;
			y = 3 * tilesize;
			break;
		case 3: //right grass flat
			x = 0 * tilesize;
			y = 3 * tilesize;
			break;
		case 10: //right grass flat
			x = 4 * tilesize;
			y = 2 * tilesize;
			break;
		case 168://waves
			x = 4 * tilesize;
			y = 0 * tilesize;
			break;
		case 99: //water block
			x = 3 * tilesize;
			y = 0 * tilesize;
			break;
		case 124: //exit sign
			x = 0 * tilesize;
			y = 1 * tilesize;
			break;
		case 96: //go right sign
			x = 1 * tilesize;
			y = 1 * tilesize;
			break;
		}
		tex->insert(tex->end(), { x / textureSize, y / textureSize,
			x / textureSize, (y + tilesize) / textureSize, 
			(x + tilesize) / textureSize, y/textureSize, 
			(x + tilesize) / textureSize, (y + tilesize) / textureSize,
			x / textureSize, (y + tilesize) / textureSize, 
			(x + tilesize) / textureSize, y / textureSize });
		return true;
	}
	void renderStatics(ShaderProgram* program, Matrix* modelviewMatrix, Matrix* projectionMatrix) {
		std::vector<float> vertexData;
		std::vector<float> texData;
		for (int y = 0; y < height; ++y) {									//render static tiles
			for (int x = 0; x < width; ++x) {
				if (nearPlayer(x) && setStaticTextures(x, y, program, modelviewMatrix, projectionMatrix, &texData)) {
					vertexData.insert(vertexData.end(), {
						(float)x, (float)-y,
						(float)x, (float)-y - 1,
						(float)x + 1, (float)-y,
						(float)x + 1, (float)-y - 1,
						(float)x, (float)-y - 1,
						(float)x + 1, (float)-y });
				}
			}
		}
		staticVerticies = vertexData;
		staticTextures = texData;
		modelviewMatrix->Identity();
		program->SetModelviewMatrix(*modelviewMatrix);
		glVertexAttribPointer(program->vertexShader, 2, GL_FLOAT, false, 0, vertexData.data());
		glEnableVertexAttribArray(program->vertexShader);
		GLuint textureMap = textures[1];
		glBindTexture(GL_TEXTURE_2D, textureMap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texData.data());
		glEnableVertexAttribArray(program->texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, vertexData.size() / 2);
	}
	void entityCollisions(float elapsed) {
		//coin collisions
		for (int i = 1; i < dynamics.size(); ++i) {
			if (dynamics[0]->entityCollision(dynamics[i])) {
				score += 10;
				if (i < dynamics.size()-1) {
					dynamics[i] = dynamics[dynamics.size() - 1];
					dynamics.pop_back();
				}
				else {
					dynamics.pop_back();
				}
			}
		}
	}
	void staticCollisions(float elapsed, Matrix* projectionMatrix) {
		int x, y;
		bool t, b, r, l;
		float buffer = 0.01f;
		float destination;
		for (entity* d : dynamics) { //dynamic - static entity collisions
			//check if the entity has fallen within a static tile and pushes them out.
			d->updateY(elapsed);//check y collisions
			t = false;
			b = false; 
			r = false;
			l = false;
			worldToTileCoordinates(d->xPosition(), d->bottom(), &x, &y);
			if (checkStatic(tilemap[y][x])) {
				b = true;
				destination = fabs(d->bottom() + y);
				d->translate(0.0f, destination + buffer);
			}
			worldToTileCoordinates(d->xPosition(), d->top(), &x, &y);
			if (checkStatic(tilemap[y][x])) {
				t = true;
				destination = fabs(d->top() + (y + 1));
				d->translate(0.0f, -destination - buffer);
			}
			d->updateX(elapsed, projectionMatrix);//check x collisions
			worldToTileCoordinates(d->right(), d->yPosition(), &x, &y);
			if (checkStatic(tilemap[y][x])) {
				r = true;
				destination = fabs(d->right() - x);
				d->translate(-destination - buffer, 0.0f);
			}
			worldToTileCoordinates(d->left(), d->yPosition(), &x, &y);
			if (checkStatic(tilemap[y][x])) {
				l = true;
				destination = fabs(d->left() - (x + 1));
				d->translate(destination + buffer, 0.0f);
			}
			d->changeCollisionFlags(t, b, r, l);
		}
	}
	void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY) {
		*gridX = floor(worldX);
		*gridY = floor(-worldY);
	}
	bool checkStatic(int n) { //checks if it is collidable
		if (n == 124 || n == 96 || n== 0) {
			return false;
		}
		else if (n == 168 || n == 99) {
			dynamics[0]->translate(-dynamics[0]->xPosition() + 2, -dynamics[0]->yPosition() - 6);
			return false;
		}
		return true;
	}
	void DrawText(ShaderProgram *program, Matrix* modelviewMatrix, Matrix*projectionMatrix, std::string text, float size, float spacing) {
		float texture_size = 1.0 / 16.0f;
		std::vector<float> vertexData;
		std::vector<float> texCoordData;
		for (int i = 0; i < text.size(); i++) {
			int spriteIndex = (int)text[i];
			float texture_x = (float)(spriteIndex % 16) / 16.0f;
			float texture_y = (float)(spriteIndex / 16) / 16.0f;
			vertexData.insert(vertexData.end(), {
				((size + spacing) * i) + (-0.5f * size), 0.5f * size,
				((size + spacing) * i) + (-0.5f * size), -0.5f * size,
				((size + spacing) * i) + (0.5f * size), 0.5f * size,
				((size + spacing) * i) + (0.5f * size), -0.5f * size,
				((size + spacing) * i) + (0.5f * size), 0.5f * size,
				((size + spacing) * i) + (-0.5f * size), -0.5f * size,
			});
			texCoordData.insert(texCoordData.end(), {
				texture_x, texture_y,
				texture_x, texture_y + texture_size,
				texture_x + texture_size, texture_y,
				texture_x + texture_size, texture_y + texture_size,
				texture_x + texture_size, texture_y,
				texture_x, texture_y + texture_size,
			});
		}
		glBindTexture(GL_TEXTURE_2D, textures[0]);
		program->SetModelviewMatrix(*modelviewMatrix);
		program->SetProjectionMatrix(*projectionMatrix);
		glVertexAttribPointer(program->vertexShader, 2, GL_FLOAT, false, 0, vertexData.data());
		glEnableVertexAttribArray(program->vertexShader);
		glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
		glEnableVertexAttribArray(program->texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, vertexData.size() / 2);
	}
	void update(float elapsed, Matrix* projectionMatrix) {
		staticCollisions(elapsed, projectionMatrix);
		entityCollisions(elapsed);
	}
private:
	int width;
	int height;
	float verticies[12] = { -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f, -0.5f, 0.5f, 0.5f };
	int tilesize;
	int** tilemap;
	std::vector<entity*> dynamics;
	int score;
	GLuint textures[5];
	bool runOnce = false;
	std::vector<float> staticVerticies;
	std::vector<float> staticTextures;
};

void input(Gamestate* state) {
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	if (keys[SDL_SCANCODE_SPACE]) {
		state->input(JUMP);
	}
	else if (keys[SDL_SCANCODE_A]){
		state->input(LEFT);
		}
	else if (keys[SDL_SCANCODE_S]){
		state->input(DUCK);
		}
	else if (keys[SDL_SCANCODE_D]){
		state->input(RIGHT);
		}
	else{
		state->input(IDLE);
	}
}
GLuint LoadTexture(const char *filePath) {
	int w, h, comp;
	unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
	if (image == NULL) {
		std::cout << "Unable to load image. Make sure the path is correct\n";
		assert(false);
	}
	GLuint retTexture;
	glGenTextures(1, &retTexture);
	glBindTexture(GL_TEXTURE_2D, retTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_image_free(image);
	return retTexture;
}

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1000,700, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif

	glViewport(0, 0, 1000,700);
	ShaderProgram program(RESOURCE_FOLDER"Vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	Matrix projectionMatrix;
	projectionMatrix.SetOrthoProjection(-10.0f, 10.0f, -7.0f, 7.0f, -1.0f, 1.0f);
	Matrix modelviewMatrix;

	GLuint coin = LoadTexture("hud_coins.png");
	GLuint font = LoadTexture("font.png");
	GLuint player = LoadTexture("player.png");
	GLuint terrain = LoadTexture("terrain.png");
	GLuint enemies = LoadTexture("enemies.png");
	GLuint textures[] = { font, terrain, player, enemies, coin };
	std::ifstream map("map.txt");
	Gamestate state(&map, textures);

	float time;
	float elapsed;
	float last = 0.0f;
	float fps = 120.0f;
	glClearColor(0.52f, 0.81f, 0.92f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}
		glUseProgram(program.programID);
		program.SetModelviewMatrix(modelviewMatrix);
		program.SetProjectionMatrix(projectionMatrix);
		time = SDL_GetTicks() *0.001f;
		elapsed = time - last;
		if (elapsed > 1 / fps) {
			input(&state);
			state.update(elapsed, &projectionMatrix);
			glClear(GL_COLOR_BUFFER_BIT);
			state.render(&program, &modelviewMatrix, &projectionMatrix);
			last = time;
		}
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);
		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}

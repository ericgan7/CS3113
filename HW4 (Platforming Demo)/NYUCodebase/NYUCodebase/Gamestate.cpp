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

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
using namespace Platformer;

namespace Platformer {

	enum entityType { STATIC_LADDER, STATIC_LADDERTOP, STATIC_END, ENTITY_PLAYER, ENTITY_COIN, ENTITY_ENEMY };
	enum inputAction { JUMP, CLIMB, DUCK, LEFT, RIGHT, IDLE };
	class Gamestate {
	public:
		Gamestate(std::ifstream* map, GLuint tex[]) {
			textures = tex;
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
				else if (line == "[entity]") {
					readObjects(map);
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
				if (value == "Terrain") {
					getline(*map, line);
					createTileMap(map);
				}
				if (value == "Interactable") {
					getline(*map, line);
					createStatics(map);
				}
			}
			return true;
		}
		void readObjects(std::ifstream* map, GLuint textures[]) {
			std::string line;
			while (getline(*map, line)) {
				if (line == "") { break; }
				std::istringstream sStream(line);
				std::string key, value;
				entityType type;
				getline(sStream, key, '=');
				getline(sStream, value);
				if (key == "type") {
					if (value == "ENTITY_COIN") {
						type = ENTITY_COIN;
					}
					else if (value == "ENTITY_PLAYER") {
						type = ENTITY_PLAYER;
					}
					else if (value == "ENTITY_ENEMY") {
						type = ENTITY_ENEMY;
					}
				}
				else if (key == "location") {
					std::istringstream lineStream(value);
					std::string xPosition, yPosition;
					getline(lineStream, xPosition, ',');
					getline(lineStream, yPosition, ',');
					float x = atoi(xPosition.c_str())*tilesize;
					float y = atoi(yPosition.c_str())*-tilesize;
					dynamics.push_back(&entity(x, y, 0.0f, 0.0f, type, textures));
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
						tilemap[y][x] = val - 1;
					}
					else {
						tilemap[y][x] = 0;
					}
				}
			}
		}
		void createStatics(std::ifstream* map) {
			float verticies[12] = { -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f, -0.5f, 0.5f, 0.5f };
			std::string line;
			for (int y = 0; y < height; ++y) {
				getline(*map, line);
				std::istringstream lineStream(line);
				std::string tile;
				for (int x = 0; x < width; ++x) {
					int val = atoi(tile.c_str());
					if (val > 0) {
						if (val == 71) {
							statics.push_back(&entity(x*tilesize, -y*tilesize, 0.0f, 0.0f, LADDERTOP));
						}
						else if (val == 85) {
							statics.push_back(&entity(x*tilesize, -y*tilesize, 0.0f, 0.0f, LADDER));
						}
					}
				}
			}
		}
		void getVerticies(entityType* type, inputAction* state, float** vert, float** size, GLuint* texture) {
			if (*type == ENTITY_COIN) {
				float v[12] = { -0.25f, 0.25f, -0.25f, -0.25f, 0.25f, 0.25f, 0.25f, -0.25f, -0.25f, -0.25f, 0.25f, 0.25f };
				float s[2] = { 0.25f, 0.25f };
				*vert = v;
				*size = s;
				*texture = textures[5];
			}
			else if (*type == ENTITY_PLAYER) {
				float v[12] = { -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f, -0.5f, 0.5f, 0.5f };
				float s[2] = { 0.5f, 0.5f };
				*vert = v;
				*size = s;
				*texture = textures[3];
			}
			else if (*type == ENTITY_ENEMY) {
				float v[12] = { -0.25f, 0.15f, -0.25f, -0.15f, 0.25f, 0.15f, 0.25f, -0.15f, -0.25f,-0.15f, 0.25f, 0.15f };
				float s[2] = { 0.25f, 0.15f };
				*vert = v;
				*size = s;
				*texture = textures[4];
			}
		}
		void input(const inputAction input) {
			if (input == CLIMB) {
				dynamics[0]->input(input, 1.0f);
			}
			else if (input == DUCK) {
				dynamics[0]->input(input, 0.0f);
			}
			else if (input == LEFT) {
				dynamics[0]->input(input, -2.0f);
			}
			else if (input == RIGHT) {
				dynamics[0]->input(input, 2.0f);
			}
			else if (input == JUMP) {
				dynamics[0]->input(input, 5.0f);
			}
		}
		void render(ShaderProgram* program, Matrix* modelviewMatrix, Matrix* projectionMatrix) {
			std::vector<float> vertexData;
			dynamics[0]->render(program, modelviewMatrix, projectionMatrix, textures);	//render player and set projectionMatrix
			for (int y = 0; y < height; ++y) {									//render static tiles
				for (int x = 0; x < width; ++x) {
					renderStatics(x, y, program, modelviewMatrix, projectionMatrix);
					vertexData.insert(vertexData.end(), {
						(float)tilesize * x, -(float)tilesize * y,
						(float)tilesize * x, (-(float)tilesize * y) - (float)tilesize,
						((float)tilesize * x) + (float)tilesize, (-(float)tilesize * y) - (float)tilesize,
						(float)tilesize * x, -(float)tilesize * y,
						((float)tilesize * x) + (float)tilesize, (-(float)tilesize * y) - (float)tilesize,
						((float)tilesize * x) + (float)tilesize, -(float)tilesize * y
					});
				}
			}
			glVertexAttribPointer(program->vertexShader, 2, GL_FLOAT, false, 0, vertexData.data());
			glEnableVertexAttribArray(program->vertexShader);
			glDrawArrays(GL_TRIANGLES, 0, vertexData.size() / 2);
			for (int i = 1; i < dynamics.size(); ++i) {						//render dynamic objects except player
				dynamics[i]->render(program, modelviewMatrix, projectionMatrix);
			}
			for (entity* s : statics) {										//render interactable objects;
				s->render(program, modelviewMatrix, projectionMatrix, textures);
			}
		}
		void renderStatics(int x, int y, ShaderProgram* program, Matrix* modelviewMatrix, Matrix* projectionMatrix) {
			GLuint textureMap;
			float textureSize;
			float x, y;
			switch (tilemap[y][x]) {
			case 25: //left rounded grass edge
				textureMap = textures[1];
				x = 3 * tilesize;
				y = 3 * -tilesize;
				break;
			case 26: //flat grass
				textureMap = textures[1];
				x = 4 * tilesize;
				y = 3 * -tilesize;
				break;
			case 18: //right rounded grass edge
				textureMap = textures[1];
				x = 3 * tilesize;
				y = 2 * -tilesize;
				break;
			case 204: //wooden crate:
				textureMap = textures[2];
				x = 0 * tilesize;
				y = 11 * -tilesize;
				break;
			case 31: //right sharp grass edge
				textureMap = textures[1];
				x = 2 * tilesize;
				y = 4 * -tilesize;
				break;
			case 38: //left sharp grass edge
				textureMap = textures[1];
				x = 1 * tilesize;
				y = 5 * -tilesize;
				break;
			case 30: //dirt block
				textureMap = textures[1];
				x = 0 * tilesize;
				y = 5 * -tilesize;
				break;
			case 3: //right grass flat
				textureMap = textures[1];
				x = 2 * tilesize;
				y = 0 * -tilesize;
				break;
			case 10: //right grass flat
				textureMap = textures[1];
				x = 2 * tilesize;
				y = 1 * -tilesize;
				break;
			case 168://waves
				textureMap = textures[2];
				x = 6 * tilesize;
				y = 8 * -tilesize;
				break;
			case 99: //water block
				textureMap = textures[2];
				x = 7 * tilesize;
				y = 3 * -tilesize;
				break;
			case 4: //grass top right corner
				textureMap = textures[1];
				x = 3 * tilesize;
				y = 0 * -tilesize;
				break;
			case 5: //slanted grass right
				textureMap = textures[1];
				x = 4 * tilesize;
				y = 0 * -tilesize;
				break;
			case 124: //exit sign
				textureMap = textures[2];
				x = 4 * tilesize;
				y = 5 * -tilesize;
				break;
			case 96: //go right sign
				textureMap = textures[2];
				x = 4 * tilesize;
				y = 3 * -tilesize;
				break;
			}
			if (textureMap = textures[1]) {
				textureSize = 512.0f;
			}
			else if (textureMap = textures[2]) {
				textureSize = 1024.0f;
			}
			float tex[12] = { x / textureSize, y / textureSize, x / textureSize, (y - tilesize) / textureSize, (x + tilesize) / textureSize, (x + tilesize) / textureSize,
				(y - tilesize) / textureSize, x / textureSize, (y - tilesize) / textureSize, (x + tilesize) / textureSize, y / textureSize };
			glBindTexture(GL_TEXTURE_2D, textureMap);
			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tex);
			glEnableVertexAttribArray(program->texCoordAttribute);
		}
		void entityCollisions(float elapsed) {
			//coin collisions
			for (int i = 1; i < dynamics.size(); ++i) {
				if (dynamics[0]->entityCollision(dynamics[i])) {
					score += 10;
					if (i < dynamics.size() - 1) {
						dynamics[i] = dynamics[dynamics.size() - 1];
						dynamics.pop_back();
					}
					else {
						dynamics.pop_back();
					}
				}
			}
			//ladder collisions
		}
		void staticCollisions(float elapsed) {
			int x, y;
			bool t, b, l, r = false;
			float buffer = 0.01f;
			for (entity* d : dynamics) { //dynamic - static entity collisions
										 //checsk if the entity has fallen within a static tile and pushes them out.
				d->updateY(elapsed);//check y collisions
				worldToTileCoordinates(d->xPosition(), d->bottom(), &x, &y);
				if (checkStatic(tilemap[y][x])) {
					b = true;
					d->translate(0.0f, d->penetration(d->bottom(), y*-tilesize, tilesize / 2, 1)* +buffer);
				}
				worldToTileCoordinates(d->xPosition(), d->top(), &x, &y);
				if (checkStatic(tilemap[y][x])) {
					t = true;
					d->translate(0.0f, d->penetration(d->top(), y*-tilesize, tilesize / 2, 1)*-1 - buffer);
				}
				d->updateX(elapsed);//check x collisions
				worldToTileCoordinates(d->right(), d->yPosition(), &x, &y);
				if (checkStatic(tilemap[y][x])) {
					r = true;
					d->translate(d->penetration(d->right(), x, tilesize / 2, 0)*-1 - buffer, 0.0f);
				}
				worldToTileCoordinates(d->left(), d->yPosition(), &x, &y);
				if (checkStatic(tilemap[y][x])) {
					l = true;
					d->translate(d->penetration(d->left(), x + tilesize, tilesize / 2, 0) + buffer, 0.0f);
				}
				d->changeCollisionFlags(t, b, r, l);
			}
		}
		void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY) {
			*gridX = (int)(worldX / tilesize);
			*gridY = (int)(-worldY / tilesize);
		}
		bool checkStatic(int n) { //checks if it is collidable
			if (n == 168 || n == 99 || n == 124 || n == 96) {
				return false;
			}
			return true;
		}
		void update(float elapsed) {
			staticCollisions(elapsed);
			entityCollisions(elapsed);
		}
	private:
		int width;
		int height;
		float verticies[12] = { -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f, -0.5f, 0.5f, 0.5f };
		int tilesize;
		int** tilemap;
		std::vector<entity*> statics;
		std::vector<entity*> dynamics;
		int score;
		GLuint* textures;
	};
}
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
				if (value == "Terrain") {
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
					float y = atoi(yPosition.c_str())*-1;
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
			if (!runOnce) {
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
			modelviewMatrix->Translate(x + 4.0f, y, 0.0f);
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
				(x + tilesize) / textureSize, y / textureSize,
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
					if (i < dynamics.size() - 1) {
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
			if (n == 124 || n == 96 || n == 0) {
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
}
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
#include "Entity.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

namespace Platformer {
	class entity {
	public:
		entity(float x, float y, entityType t, GLuint tex[]) : type(t) {
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
			velocity[0] += acceleration[0] * elapsed;
			position[0] += velocity[0] * elapsed;
			if (type == ENTITY_PLAYER) {
				projectionMatrix->Identity();
				projectionMatrix->SetOrthoProjection(-10.0f, 10.0f, -7.0f, 7.0f, -1.0f, 1.0f);
				if (position[0] > 10.0f && position[0] < 40.0f) {
					projectionMatrix->Translate(-position[0], 7.0f, 0.0f);
				}
				else  if (position[0] < 10.0f) {
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
		float size[2];
		float verticies[12];
		float textureCoordinates[12];

		float tilesize = 70.0f;
		GLuint texture;
		GLuint textures[5];
		entityType type;
		inputAction state = JUMP;
		int animationState = 0;
		float animationTime = 0.0f;

		//contact flags
		bool topF = false;
		bool bottomF = false;
		bool rightF = false;
		bool leftF = false;
	};
}
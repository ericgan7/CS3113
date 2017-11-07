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

namespace Platformer {
	class entity {
	public:
		entity(float x, float y, float xVelocity, float yVelocity, entityType t) : type(t) {
			position[0] = x;
			position[1] = y;
			velocity[0] = xVelocity;
			velocity[1] - yVelocity;
		}
		void render(ShaderProgram* program, Matrix* modelviewMatrix, Matrix* projectionMatrix) const {
			program->SetProjectionMatrix(*projectionMatrix);
			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, verticies);
			glEnableVertexAttribArray(program->positionAttribute);
			GLuint tex;
			//set u,v coordinates for textures based on animation state.
			if (type == ENTITY_PLAYER) {

			}
			else if (type == ENTITY_COIN) {}
			else if (type == ENTITY_PLAYER) {}
			else if (type == STATIC_END) {}
			else if (type == STATIC_LADDER) {}
			else if (type == STATIC_LADDERTOP) {}
			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texture);
			glEnableVertexAttribArray(program->texCoordAttribute);
			modelviewMatrix->Identity();
			modelviewMatrix->Translate(position[0], position[1], 0.0f);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		//need to take in account of states to check if moving is appropriate.
		void updateX(float elapsed) {
			lerp(velocity[0], 0.0f, elapsed*0.75f);
			velocity[0] += acceleration[0] * elapsed;
			position[0] += velocity[0] * elapsed;
		}
		void updateY(float elapsed) {
			velocity[1] += acceleration[1] * elapsed - 3.0f*elapsed;//gravity is present
			position[1] += velocity[1] * elapsed;
		}
		void input(inputAction input, float amount) {
			switch (input) {
			case CLIMB:
				if (nextToLadder) {
					velocity[1] = amount;
					state = CLIMB;
				}
				break;
			case DUCK:
				state = DUCK;
				break;
			case LEFT:
				if (leftF) {
					acceleration[0] = amount;
					state = LEFT;
				}
				break;
			case RIGHT:
				if (leftF) {
					acceleration[0] = amount;
					state = RIGHT;
				}
				break;
			case JUMP:
				if (bottomF) {
					velocity[1] = amount;
					state = JUMP;
				}
				break;
			}
		}
		void changeCollisionFlags(bool t, bool b, bool r, bool l) {
			topF = t;
			bottomF = b;
			rightF = r;
			leftF = l;
			if (type == ENTITY_COIN && bottomF) {
				input(JUMP, 1.0f);
			}
			else if (type == ENTITY_ENEMY) {
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
			return (1.0 - t)*v0 + t *v1;
		}
		float penetration(float entityPos, float staticPos, float staticDistance, int axis) {
			return fabs(entityPos - staticPos - staticDistance - size[axis]);
		}
		float bottom()const { return position[1] - size[1]; }
		float top() const { return position[1] + size[1]; }
		float left() const { return position[0] - size[0]; }
		float right() const { return position[0] + size[1]; }
		float xPosition() const { return position[0]; }
		float yPosition() const { return position[1]; }
	private:
		float position[2];
		float velocity[2];
		float acceleration[2] = { 0.0f, 9.0f };
		float size[2];
		float verticies[12];
		float texture[12];

		GLuint texture;
		entityType type;
		inputAction state = IDLE;
		int animationState = 0;
		bool nextToLadder = false;

		//contact flags
		bool topF = false;
		bool bottomF = false;
		bool rightF = false;
		bool leftF = false;
	};
}
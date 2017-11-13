#ifndef PLATFORMER_ENTITY
#define PLATFORMER_ENTITY

namespace Platformer {
	void copy(float* a1, float* a2, int size) {
		for (int i = 0; i < size; ++i) {
			a2[i] = a1[i];
		}
	}
	enum entityType { STATIC_LADDER, STATIC_LADDERTOP, STATIC_END, ENTITY_PLAYER, ENTITY_COIN, ENTITY_ENEMY };
	enum inputAction { JUMP, CLIMB, DUCK, LEFT, RIGHT, IDLE };
	class entity {
	public:
		entity(float x, float y, entityType t, GLuint tex[]);
		void render(ShaderProgram* program, Matrix* modelviewMatrix, Matrix* projectionMatrix);
		void changeAnimation(Matrix* modelviewMatrix);
		void updateX(float elapsed, Matrix* projectionMatrix);
		void updateY(float elapsed);
		void input(inputAction input, float amount);
		void changeCollisionFlags(bool t, bool b, bool r, bool l);
		void translate(float x, float y);
		bool entityCollision(entity* other) const;
		float lerp(float v0, float v1, float t);
		float bottom()const;
		float top() const;
		float left() const;
		float right() const;
		float xPosition() const;
		float yPosition() const;
		float getType() const;
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
#endif

#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "Matrix.h"
#include "ShaderProgram.h"
#include <vector>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

enum entityType {ENTITY_ENEMYBULLET1, ENTITY_ENEMYBULLET2, ENTITY_PLAYERBULLET,
	ENTITY_ENEMY1, ENTITY_ENEMY2, ENTITY_ENEMY3, ENTITY_PLAYER};
enum Gamemode { MODE_MENU, MODE_GAME, MODE_END};

void getTexture(entityType type, std::vector<float>& result) {
	float x, y, w, h;
	float size = 1024.0f;
	switch (type) {
	case ENTITY_ENEMYBULLET1:
		x = 858.0f / size;
		y = 230.0f / size;
		w = 9.0f / size;
		h = 54.0f / size;
		break;
	case ENTITY_ENEMYBULLET2:
		x = 843.0f / size;
		y = 940.0f / size;
		w = 13.0f / size;
		h = 37.0f / size;
		break;
	case ENTITY_ENEMY1:
		x = 423.0f / size;
		y = 738.0f / size;
		w = 93.0f / size;
		h = 84.0f / size;
	case ENTITY_ENEMY2:
		x = 144.0f / size;
		y = 156.0f / size;
		w = 103.0f / size;
		h = 84.0f / size;
		break;
	case ENTITY_ENEMY3:
		x = 518.0f / size;
		y = 325.0f / size;
		w = 82.0f / size;
		h = 84.0f / size;
		break;
	case ENTITY_PLAYERBULLET:
		x = 842.0f / size;
		y = 206.0f / size;
		w = 13.0f / size;
		h = 57.0f / size;
		break;
	case ENTITY_PLAYER:
		x = 112.0f / size;
		y = 791.0f / size;
		w = 112.0f / size;
		h = 75.0f / size;
		break;
	}
	std::vector<float> coordinates;
	coordinates.insert(coordinates.end(), { x, y, x, y + h, x + w, y, x + w, y + h, x + w, y, x, y + h });
	for (int i = 0; i < coordinates.size(); ++i) {
		result[i] = coordinates[i];
	}
}

class entity {
public:
	entity(float vertex[], float p[], float v[], float s[], int health, float time, entityType type) 
		: health(health),time(time), alive(true), type(type) {
		for (int i = 0; i < 12; ++i) {
			verticies[i] = vertex[i];
		}
		for (int i = 0; i < 3; ++i) {
			position[i] = p[i];
			velocity[i] = v[i];
		}
		size[0] = s[0];
		size[1] = s[1];
		yDestination = position[1];
		switch (type) {
		case ENTITY_ENEMYBULLET1:
		case ENTITY_ENEMYBULLET2:
		case ENTITY_PLAYERBULLET:
			animationTime = 3;
			break;
		case ENTITY_PLAYER:
		case ENTITY_ENEMY1:
		case ENTITY_ENEMY2:
		case ENTITY_ENEMY3:
			animationTime = 2;
			break;
		}
	}
	entity(const entity& other) {
		for (int i = 0; i < 12; ++i) {
			this->verticies[i] = other.verticies[i];
		}
		for (int i = 0; i < 3; ++i) {
			this->position[i] = other.position[i];
			this->velocity[i] = other.velocity[i];
		}
		for (int i = 0; i < 2; ++i) {
			this->size[i] = other.size[i];
		}
		this->health = other.health;
		this->time = other.time;
		this->alive = other.alive;
		this->type = other.type;
		this->animationTime = other.animationTime;
		this->yDestination = other.yDestination;
	}
	void draw(ShaderProgram& program, Matrix& modelviewMatrix, GLuint texture, GLuint death) {
		if (health > 0) { //draw ship
			glBindTexture(GL_TEXTURE_2D, texture);
			glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, verticies);
			glEnableVertexAttribArray(program.positionAttribute);
			std::vector<float> coordinates(12);
			getTexture(type, coordinates);
			glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, coordinates.data());
			glEnableVertexAttribArray(program.texCoordAttribute);
			modelviewMatrix.Identity();
			modelviewMatrix.Translate(position[0], position[1], position[2]);
			if (type == ENTITY_ENEMYBULLET1) {
				modelviewMatrix.Rotate((float)atan(1) * 4);
			}
			program.SetModelviewMatrix(modelviewMatrix);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		else {
			deathAnimation(program, modelviewMatrix, type, texture, death);
			++currentAnimation;
			if (currentAnimation%animationTime == 0) {
				--health;
			}
		}
	}
	void deathAnimation(ShaderProgram& program, Matrix& modelviewMatrix, entityType type, GLuint texture, GLuint death) {
		switch (type) {
		case ENTITY_ENEMYBULLET1:
		case ENTITY_ENEMYBULLET2:
		case ENTITY_PLAYERBULLET:
			if (health > -5) { //draw bullet explosion
				glBindTexture(GL_TEXTURE_2D, death);
				float x = abs(health)*32.0f / 576.0f;
				float y = 0.0f;
				float height = 1.0f;
				float width = 32.0f / 576.0f;
				float texCoord[12] = { x,y,x,y + height,x + width,y,x + width,y + height, x + width, y, x, y + height };
				float newVerticies[12] = { -0.2f, 0.2f, -0.2f, -0.2f, 0.2f, 0.2f, 0.2f, -0.2f, -0.2f, -0.2f, 0.2f, 0.2f };
				glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, newVerticies);
				glEnableVertexAttribArray(program.positionAttribute);
				glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoord);
				glEnableVertexAttribArray(program.texCoordAttribute);
				modelviewMatrix.Identity();
				modelviewMatrix.Translate(position[0], position[1], position[2]);
				program.SetModelviewMatrix(modelviewMatrix);
				glDrawArrays(GL_TRIANGLES, 0, 6);
			}
			else { alive = false; } //will not draw if dead
			break;
		case ENTITY_PLAYER:
		case ENTITY_ENEMY1:
		case ENTITY_ENEMY2:
		case ENTITY_ENEMY3:
			if (health > -13) {
				if (health > -4) {		//draw ship
					glBindTexture(GL_TEXTURE_2D, texture);
					glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, verticies);
					glEnableVertexAttribArray(program.positionAttribute);
					std::vector<float> coordinates(12);
					getTexture(type, coordinates);
					glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, coordinates.data());
					modelviewMatrix.Identity();
					modelviewMatrix.Translate(position[0], position[1], position[2]);
					program.SetModelviewMatrix(modelviewMatrix);
					glDrawArrays(GL_TRIANGLES, 0, 6);
				}	//draw explosion
				glBindTexture(GL_TEXTURE_2D, death);
				float x = (abs(health) + 5)*32.0f / 576.0f;
				float y = 0.0f;
				float height = 1.0f;
				float width = 32.0f / 576.0f;
				float texCoord[12] = { x,y,x,y + height,x + width,y,x + width,y + height, x + width, y, x, y + height };
				glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, verticies);
				glEnableVertexAttribArray(program.positionAttribute);
				glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoord);
				glEnableVertexAttribArray(program.texCoordAttribute);
				modelviewMatrix.Identity();
				modelviewMatrix.Translate(position[0], position[1], position[2]);
				program.SetModelviewMatrix(modelviewMatrix);
				glDrawArrays(GL_TRIANGLES, 0, 6);
			}
			else { alive = false; } //will not draw if dead
			break;
		}
	}
	int checkHealth() const {
		return health;
	}
	float getxPosition()const {
		return position[0];
	}
	float getyPosition() const {
		return position[1];
	}
	void input(float v[], bool destination) {
		if (destination) {
			yDestination = position[1] - 1.0f;
			velocity[0] = 0.0f;
		}
		for (int i = 0; i < 3; ++i){
			velocity[i] = v[i];
		}
	}
	float lerp(float v0, float v1, float t) {
		return (1.0f - t)*v0 + t*v1;
	}
	void update(float elapsed) {
		position[0] += velocity[0] * elapsed;
		position[1] += velocity[1] * elapsed;
		if (type == ENTITY_ENEMY1 || type == ENTITY_ENEMY2
			|| type == ENTITY_ENEMY3 || type == ENTITY_PLAYER) {
			if (position[1] < yDestination) {
				velocity[1] = 0.0f;
				position[1] = yDestination;
				if (travelRight) {
					velocity[0] = 0.75f;
				}
				else {
					velocity[0] = -0.75f;
				}
			}
		}
		time += elapsed;
	}
	void changeDirection(float dir) {
		travelRight = dir;
	}
	bool shoot() {
		switch (type) {
		case ENTITY_PLAYER:
			if (time > 0.75f) {
				time = 0.0f;
				return true;
			}
			return false;
			break;
		case ENTITY_ENEMY1:
		case ENTITY_ENEMY2:
		case ENTITY_ENEMY3:
			if (time > 2.0f) {
				time = 0.0f;
				return true;
			}
			return false;
			break;
		}
	}
	void hit() {
		--health;
	}
	void setDead() {
		alive = false;
	}
	operator bool() {
		if (type == ENTITY_PLAYERBULLET || type == ENTITY_ENEMYBULLET1 || type == ENTITY_ENEMYBULLET2) {
			return time < 15.0f && alive;
		}
		else {
			return alive;
		}
	}
	float bottom() const{ return position[1] - size[1];}
	float top() const { return position[1] + size[1]; }
	float right() const { return position[0] + size[0]; }
	float left() const{ return position[0] - size[0]; }
	entityType getType() const { return type; }
	void reset() {
		position[0] = 0.0f;
		velocity[0] = 0.0f;
		health = 5;
	}
private:
	float verticies[12];
	float position[3];
	float velocity[3];

	float size[2];
	int health;
	bool alive = true;
	float time = 0.0f;
	int animationTime;
	int currentAnimation = 0;

	entityType type;
	float yDestination;
	float travelRight = true;
};

void DrawText(ShaderProgram &program, int fontTexture, std::string text, float size,
	float spacing, float position[], Matrix& modelviewMatrix) {

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
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program.positionAttribute);
	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program.texCoordAttribute);
	modelviewMatrix.Identity();
	modelviewMatrix.Translate(position[0], position[1], position[2]);
	program.SetModelviewMatrix(modelviewMatrix);
	glDrawArrays(GL_TRIANGLES, 0, vertexData.size()/2);
}

class Gamestate {
public:
	Gamestate(const entity& p, GLuint textures, GLuint fonts, GLuint death) : texture(textures), font(fonts), death(death), player(p) {}
	void draw(ShaderProgram& program, Matrix& modelviewMatrix) {
		glUseProgram(program.programID);
		player.draw(program, modelviewMatrix, texture, death);
		for (int i = 0; i < enemies.size(); ++i) {
			if (enemies[i]) {
				enemies[i].draw(program, modelviewMatrix, texture, death);
			}
		}
		for (int i = 0; i < enemyBullets.size(); ++i) {
			if (enemyBullets[i]) {
				enemyBullets[i].draw(program, modelviewMatrix, texture, death);
			}
		}
		for (int i = 0; i < playerBullets.size(); ++i) {
			if (playerBullets[i]) {
				playerBullets[i].draw(program, modelviewMatrix, texture, death);
			}
		}
		std::stringstream myScore; 
		myScore << score;
		float pos[3] = {-9.0f, 7.0f, 1.0f};
		DrawText(program, font, "SCORE:", 0.5f, 0.25f, pos, modelviewMatrix);
		pos[0] = -3.0f;
		DrawText(program, font, myScore.str(), 0.5f, 0.25f, pos, modelviewMatrix);
		pos[0] = 0.0f;
		DrawText(program, font, "HEALTH:", 0.5f, 0.25f, pos, modelviewMatrix);
		pos[0] = 9.0f;
		std::stringstream myHealth;
		myHealth << player.checkHealth();
		DrawText(program, font, myHealth.str(), 0.5f, 0.25f, pos, modelviewMatrix);
	}
	void checkCollisions() {
		for (int i = 0; i < enemyBullets.size(); ++i) {
			if (enemyBullets[i]) {
				for (int j = 0; j < playerBullets.size(); ++j) {
					if (playerBullets[j]) {
						if(collision(enemyBullets[i], playerBullets[j])){ //bullets collide
							enemyBullets[i].hit();
							enemyBullets[i].setDead();
							playerBullets[j].hit();
							break;
						}
					}
				}
				if (collision(player, enemyBullets[i])) { //player gets hit by by enemy
					enemyBullets[i].hit();
					enemyBullets[i].setDead();
					player.hit();
				}
			}
		}
		for (int i = 0; i < playerBullets.size(); ++i) {
			if (playerBullets[i]) {
				for (int j = 0; j < enemies.size(); ++j) {
					if (enemies[j]) {
						if (collision(playerBullets[i], enemies[j])) { //enemy gets hit by player
							playerBullets[i].hit();
							playerBullets[i].setDead();
							enemies[j].hit();
							score += 10;
							break;
						}//enemy, player collisions?
					}
				}
			}
		}
	}
	bool checkMovement() {
		bool change = false;
		int direction = 0;
		for (int i = 0; i < enemies.size(); ++i) {
			if (enemies[i]) {
				if (goingRight) {
					if (enemies[i].right() > 9.0f) {
						direction = -1;
						change = true;
					}
				}
				else {
					if (enemies[i].left() < -9.0f) {
						change = true;
						direction = 1;
					}
				}
			}
		}
			if (change) {
				if (direction < 0) {
					goingRight = false;
				}
				else {
					goingRight = true;
				}
				return true;
			}
			return false;
	}
	void update(float elapsed, Gamemode& mode) {
		int remaining = 0;
		for (int i = 0; i < enemies.size(); ++i) {//enemy movement
			if (enemies[i]) {
				remaining += 1;
				enemies[i].update(elapsed);
				if (fabs(enemies[i].getxPosition() - player.getxPosition()) < 1.0f) {
					enemyShoot(i);
				}
			}
		}
		if (remaining == 0) {
			spawnEnemies();
		}
		for (int i = 0; i < enemyBullets.size(); ++i) {//enemy bullet movement
			enemyBullets[i].update(elapsed);
		}
		if (checkMovement()) {	//if enemies need to change direction
			float v[3] = { 0.0f, -1.0f, 0.0f };
			for (int i = 0; i < enemies.size(); ++i) {
				enemies[i].input(v, true);
				enemies[i].changeDirection(goingRight);
			}
		}
		for (int i = 0; i < playerBullets.size(); i++) { //player bullet movement
			playerBullets[i].update(elapsed);
		}
		if (player) {
			player.update(elapsed);//player movement
		}
		else {
			mode = MODE_END;
		}
		
		checkCollisions(); //check collisions
	}
	void playerInput(float velocity[]) {
		player.input(velocity, false);
	}
	void playerShoot() {
		if (player.shoot()) {
			float vertex[12] = { -0.1f, 0.2f, -0.1f, -0.2f, 0.1f, 0.2f, 0.1f, -0.2f, 0.1f, 0.2f, -0.1f, -0.2f };
			float pos[3] = { player.getxPosition(), player.getyPosition() + 0.5f, 1.0f };
			float velocity[3] = { 0.0f, 4.0f, 0.0f };
			float size[2] = {0.1f, 0.2f};
			entity bullet(vertex, pos, velocity, size, 1, 0.0f, ENTITY_PLAYERBULLET);
			addBullets(bullet, true);
		}
	}
	void enemyShoot(int i) {
		if (enemies[i].shoot() && bottomRow(i)) {
			float vertex[12] = { -0.1f, 0.2f, -0.1f, -0.2f, 0.1f, 0.2f, 0.1f, -0.2f, 0.1f, 0.2f, -0.1f, -0.2f };
			float pos[3] = { 0.0f, 0.0f, 0.0f };
			float velocity[3] = { 0.0f, -4.0f, 0.0f };
			float size[2] = { 0.1f, 0.2f };
			entity bullet1(vertex, pos, velocity, size, 1, 0.0f, ENTITY_ENEMYBULLET1);
			entity bullet2(vertex, pos, velocity, size, 1, 0.0f, ENTITY_ENEMYBULLET1);
			switch (enemies[i].getType()) {
			case ENTITY_ENEMY1:
				pos[0] = enemies[i].getxPosition() - 0.3f;
				pos[1] = enemies[i].getyPosition() - 0.5f;
				bullet1 = entity(vertex, pos, velocity, size, 1, 0.0f, ENTITY_ENEMYBULLET1);
				pos[0] = enemies[i].getxPosition() + 0.3f;
				bullet2 = entity(vertex, pos, velocity, size, 1, 0.0f, ENTITY_ENEMYBULLET1);
				addBullets(bullet1, false);
				addBullets(bullet2, false);
				break;
			case ENTITY_ENEMY2:
				pos[0] = enemies[i].getxPosition();
				pos[1] = enemies[i].getyPosition() - 0.5f;
				bullet1 = entity(vertex, pos, velocity, size, 1, 0.0f, ENTITY_ENEMYBULLET1);
				pos[1] = enemies[i].getyPosition();
				bullet2 = entity(vertex, pos, velocity, size, 1, 0.0f, ENTITY_ENEMYBULLET1);
				addBullets(bullet1, false);
				addBullets(bullet2, false);
				break;
			case ENTITY_ENEMY3:
				pos[0] = enemies[i].getxPosition();
				pos[1] = enemies[i].getyPosition() - 0.5f;
				bullet1 = entity(vertex, pos, velocity, size, 1, 0.0f, ENTITY_ENEMYBULLET2);
				addBullets(bullet1, false);
				break;
			}
		}
	}
	void addBullets(entity bullet, bool player) {
		/*
		if (two) {//shoots two enemy bullets
			for (int i = 0; i < enemyBullets.size(); ++i) {
				if (!enemyBullets[i] && !first) {
					enemyBullets[i] = bullets[0];
				}
				else if (!enemyBullets[i] && first) {
					enemyBullets[i] = bullets[1];
					return;
				}
			}
			if (first) {
				enemies.push_back(bullets[0]);
				enemies.push_back(bullets[1]);
			}
		}
		*/
		if (player) {//shoots one player bullet
			for (int i = 0; i < playerBullets.size(); ++i) {
				if (!playerBullets[i]) {
					playerBullets[i] = bullet;
					return;
				}
			}
			playerBullets.push_back(bullet);
		}
		else { //shoots one enemy bullet
			for (int i = 0; i < enemyBullets.size(); ++i) {
				if (!enemyBullets[i]) {
					enemyBullets[i] = bullet;
					return;
				}
			}
			enemyBullets.push_back(bullet);
		}
	}
	bool bottomRow(int x) {
		for (int i = 0; i < enemies.size(); ++i) {
			if (enemies[i] && fabs(enemies[i].getxPosition() - enemies[x].getxPosition()) < 1.0f
				&& enemies[i].getyPosition() < enemies[x].getyPosition()) {
				return false;
			}
		}
		return true;
	}
	void resetGamestate() {
		player.reset();
		std::vector<entity> pBullets;
		playerBullets = pBullets;
		std::vector<entity> eBullets;
		enemyBullets = eBullets;
		score = 0;
		/*
		for (int i = 0; i < playerBullets.size(); ++i) {
			playerBullets[i].setDead();
		}
		for (int i = 0; i < enemyBullets.size(); ++i) {
			enemyBullets[i].setDead();
		}
		*/
		spawnEnemies();
	}
	void spawnEnemies() {
		float v[12] = { -0.5f, 0.5f, -0.5f, -0.5f, 0.5, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f };
		float velocity[3] = { 0.25f, 0.0f, 1.0f };
		float size[2] = { 0.5f, 0.5f };
		int enemyType;
		float position[3] = { 1.0f, 1.0f, 1.0f };
		std::vector<entity> nEnemies;
		for (int i = 0; i < 28; ++i) {
			position[0] = (float)-5.0f + (i % 7)*2.0f;
			position[1] = (float)5.0f - floor(i / 7.0f)*1.0f;
			enemyType = rand() % 3;
			switch (enemyType) {
			case 0:
				nEnemies.push_back(entity(v, position, velocity, size, 1, 0.0f, ENTITY_ENEMY1));
				break;
			case 1:
				nEnemies.push_back(entity(v, position, velocity, size, 1, 1.0f, ENTITY_ENEMY2));
				break;
			case 2:
				nEnemies.push_back(entity(v, position, velocity, size, 1, 2.0f, ENTITY_ENEMY3));
				break;
			}
		}
		enemies = nEnemies;
	}
	int getScore() {
		return score;
	}
private:
	bool collision(const entity& bullet, const entity& other) {
		if (bullet.bottom() > other.top()) { return false;}
		else if (bullet.top() < other.bottom()) { return false; }
		else if (bullet.left() > other.right()) { return false; }
		else if (bullet.right() < other.left()) { return false; }
		else { return true; }
	}
	//constants
	int score = 0;
	GLuint texture;
	GLuint font;
	GLuint death;
	bool goingRight = true;
	float travelDown = 0.0f;

	//objects 
	std::vector<entity> enemies;
	std::vector<entity> enemyBullets;
	std::vector<entity> playerBullets;
	entity player;

};

class Gamemenu {
public:
	Gamemenu(const float pPos[], const float qPos[], const GLuint& font) :play(pPos, true), quit(qPos, false), font(font) {}
	void draw(ShaderProgram& program, ShaderProgram& menuProgram, Matrix& modelviewMatrix, Gamemode& mode, int score) {
		/* I wanted to draw boxes around the selected button, but I cannot get it to draw. 
		glUseProgram(menuProgram.programID);
		float border[48] = { -4.0f, 1.0f, -4.0f, 0.9f, 4.0f, 1.0f, 4.0f, 0.9f, 4.0f, 1.0f, -4.0f, 0.9f,
			-4.0f, -0.9f, -4.0f, -1.0f, 4.0f, -0.9f, 4.0f, -1.0f, 4.0f, -0.9f, -4.0f, -1.0f,
			-4.0f, 1.0f, -4.0f, -1.0f, -3.9f, 1.0f, -3.9f, -1.0f, 3.9f, 1.0f, -4.0f, -1.0f,
			3.9f, 1.0f, 3.9f, -1.0f, 4.0f, 1.0f, 4.0f, -1.0f, 4.0f, 1.0f, 3.9f, -1.0f };
		glVertexAttribPointer(menuProgram.positionAttribute, 2, GL_FLOAT, false, 0, border);
		glEnableVertexAttribArray(menuProgram.positionAttribute);
		modelviewMatrix.Identity();
		modelviewMatrix.Translate(play.position[0], play.position[1], play.position[2]);
		menuProgram.SetModelviewMatrix(modelviewMatrix);
		glDrawArrays(GL_TRIANGLES, 0, 24);

		glVertexAttribPointer(menuProgram.positionAttribute, 2, GL_FLOAT, false, 0, border);
		glEnableVertexAttribArray(menuProgram.positionAttribute);
		modelviewMatrix.Identity();
		modelviewMatrix.Translate(quit.position[0], quit.position[1], quit.position[2]);
		menuProgram.SetModelviewMatrix(modelviewMatrix);
		glDrawArrays(GL_TRIANGLES, 0, 24);
		*/
		glUseProgram(program.programID);
		if (mode == MODE_MENU) {
			DrawText(program, font, "SPACE", 1.5f, 0.5f, spacePos, modelviewMatrix);
			DrawText(program, font, "INVADERS", 1.5f, 0.5, invaderPos, modelviewMatrix);
		}
		else if (mode == MODE_END) {
			std::stringstream myscore;
			myscore << "SCORE: " << score;
			DrawText(program, font, "DEFEAT", 1.5f, 0.f, spacePos, modelviewMatrix);
			DrawText(program, font, myscore.str(), 1.5f, 0.0f, invaderPos, modelviewMatrix);
		}
		DrawText(program, font, "PLAY", 1.0f, 0.5f, playPos, modelviewMatrix);
		DrawText(program, font, "QUIT", 1.0f, 0.5f, quitPos, modelviewMatrix);
	}
	int checkCollisions(const float x, const float y) const {
		if (x > play.position[0] - play.size[0] && x < play.position[0] + play.size[0] &&
			y > play.position[1] - play.size[1] && y < play.position[1] + play.size[1]) {
			return 1;
		}
		else if (x > quit.position[0] - quit.size[0] && x < quit.position[0] + quit.size[0] &&
			y > quit.position[1] - quit.size[1] && y < quit.position[1] + quit.size[1]) {
			return 2;
		}
		return 0;
	}
	void setSelected(int selection) {
		if (selection == 1) {
			play.selected = true;
			quit.selected = false;
		}
		else if (selection == 2) {
			play.selected = false;
			quit.selected = true;
		}
		else {
			play.selected = false;
			quit.selected = false;
		}
	}
	void select(Gamemode& mode, bool& done) {
		if (play.selected) {
			mode = MODE_GAME;
		}
		else if (quit.selected){
			done = true;
		}
	}
private:
	struct button {
		button(const float pos[], const bool sel): selected(sel) {
			for (int i = 0; i < 3; ++i) {
				position[i] = pos[i];
			}
		}
		float position[3];
		float size[2] = { 4.0f, 1.0f };
		bool selected;
	};
	button quit;
	button play;
	GLuint font;
	float spacePos[3] = { -4.0f, 6.0f, 1.0f };
	float invaderPos[3] = { -6.0f, 4.5f, 1.0f };
	float playPos[3] = { -2.75f, -3.0f, 1.0f };
	float quitPos[3] = { -2.75f, -6.0f, 1.0f };
};

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

void processInputs(Gamemode& mode, Gamestate& state, Gamemenu& menu, Gamemenu& end, SDL_Event& event, bool& done) {
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	switch (mode) {
	case MODE_MENU:
		//mouse controlls
		if (event.type == SDL_MOUSEMOTION) {
			float inputx = ((float)event.motion.x / 1000.0f*20.0f) - 10.0f;
			float inputy = (((float)800.0f - event.motion.y) / 800.0f*16.0f) - 8.0f;
			int selection = menu.checkCollisions(inputx, inputy);
			menu.setSelected(selection);
		}
		if (event.type == SDL_MOUSEBUTTONUP) {
			float inputx = ((float)event.button.x / 1000.0f*20.0f) - 10.0f;
			float inputy = (((float)800.0f - event.button.y) / 800.0f*16.0f) - 8.0f;
			int selection = menu.checkCollisions(inputx, inputy);
			menu.select(mode, done);
			state.resetGamestate();
		}
		//keyboard menu controls
		if (event.key.keysym.scancode == SDL_SCANCODE_UP) {
			menu.setSelected(1);
		}
		if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
			menu.setSelected(2);
		}
		if (event.key.keysym.scancode == SDL_SCANCODE_SPACE || event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
			menu.select(mode, done);
			state.resetGamestate();
		}
		break;
	case MODE_GAME:
		if (keys[SDL_SCANCODE_LEFT]) { //move left
			float velocity[3] = { -3.0f, 0.0f, 0.0f };
			state.playerInput(velocity);
		}
		else if (keys[SDL_SCANCODE_RIGHT]) { //move right
			float velocity[3] = { 3.0f, 0.0f, 0.0f };
			state.playerInput(velocity);
		}
		else {
			float velocity[3] = { 0.0f, 0.0f, 0.0f };
			state.playerInput(velocity);
		}
		if (keys[SDL_SCANCODE_SPACE]) {//shoot
			state.playerShoot();
		}
		if (keys[SDL_SCANCODE_ESCAPE]) {//end game
			mode = MODE_MENU;
			state.resetGamestate();
		}
		break;
	case MODE_END:
		//mouse controlls
		if (event.type == SDL_MOUSEMOTION) {
			float inputx = ((float)event.motion.x / 1000.0f*20.0f) - 10.0f;
			float inputy = (((float)800.0f - event.motion.y) / 800.0f*16.0f) - 8.0f;
			int selection = end.checkCollisions(inputx, inputy);
			end.setSelected(selection);
		}
		if (event.type == SDL_MOUSEBUTTONUP) {
			float inputx = ((float)event.button.x / 1000.0f*20.0f) - 10.0f;
			float inputy = (((float)800.0f - event.button.y) / 800.0f*16.0f) - 8.0f;
			int selection = end.checkCollisions(inputx, inputy);
			end.select(mode, done);
			state.resetGamestate();
		}
		//keyboard end controls
		if (event.key.keysym.scancode == SDL_SCANCODE_UP) {
			end.setSelected(1);
		}
		if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
			end.setSelected(2);
		}
		if (event.key.keysym.scancode == SDL_SCANCODE_SPACE || event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
			end.select(mode, done);
			state.resetGamestate();
		}
		break;
	}
}
void Update(Gamemode& mode, Gamestate& state, float elapsed) {
	if (mode == MODE_GAME) {
		state.update(elapsed, mode);
	}
		
}
void Render(Gamemode& mode, Gamemenu& menu, Gamestate& state, Gamemenu& end,
	ShaderProgram& program, ShaderProgram menuProgram, Matrix& modelviewMatrix, Matrix& projectionMatrix) {

	glClear(GL_COLOR_BUFFER_BIT);
	program.SetModelviewMatrix(modelviewMatrix);
	program.SetProjectionMatrix(projectionMatrix);
	menuProgram.SetModelviewMatrix(modelviewMatrix);
	menuProgram.SetProjectionMatrix(projectionMatrix);
	switch (mode) {
	case MODE_MENU:
		menu.draw(program, menuProgram, modelviewMatrix, mode, state.getScore());
		break;
	case MODE_GAME:
		state.draw(program, modelviewMatrix);
		break;
	case MODE_END:
		end.draw(program, menuProgram, modelviewMatrix, mode, state.getScore());
		break;
	}
}
SDL_Window* displayWindow;

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1000, 800, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif
	glViewport(0, 0, 1000, 800);
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	ShaderProgram menuProgram(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
	Matrix projectionMatrix;
	projectionMatrix.SetOrthoProjection(-10.0f, 10.0f, -8.0f, 8.0f, -1.0f, 1.0f);
	Matrix modelviewMatrix;
	
	//textures
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	GLuint fonts = LoadTexture(RESOURCE_FOLDER "font.png");
	GLuint ships = LoadTexture(RESOURCE_FOLDER "ship.png");
	GLuint death = LoadTexture(RESOURCE_FOLDER "explosion.png");

	//objects
	float playPos[3] = { 0.0f, -3.0f, 0.0f };
	float quitPos[3] = { 0.0f, -6.0f, 0.0f };
	float playerV[12] = { -0.5f, 0.5f, -0.5f, -0.5f, 0.5, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f };
	float playerPos[3] = { 0.0f, -7.0f, 1.0f };
	float playerVel[3] = { 0.0f, 0.0f, 0.0f };
	float playerSize[2] = { 0.5f, 0.5f };
	entity player(playerV, playerPos, playerVel, playerSize, 5, 0.0f, ENTITY_PLAYER);
	Gamemenu menu (playPos, quitPos, fonts);
	Gamemenu end (playPos, quitPos, fonts);
	Gamestate state(player, ships, fonts, death);
	Gamemode mode = MODE_MENU;

	//time
	float elapsed = 0.0f;
	float current;
	float last = 0.0f;
	float framerate = 150.0f;

	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
			//inputs
			processInputs(mode, state, menu, end, event, done);
		}
		//update
		current = SDL_GetTicks()*0.001f;
		elapsed = current - last;
		if (elapsed > (float)1.0f / framerate) {
			last = current;
			Update(mode, state, elapsed);
			//render
			Render(mode, menu, state, end, program, menuProgram, modelviewMatrix, projectionMatrix);
		}
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}

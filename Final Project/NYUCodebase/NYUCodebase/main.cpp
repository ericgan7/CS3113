#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "Matrix.h"
#include "ShaderProgram.h"
#include <vector>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <SDL_mixer.h>
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

static const int WIDTH = 50;
static const int HEIGHT = 50;
static const float ORTHOW = 15.0f;
static const float ORTHOH = 10.0f;
enum terrainType {T_DIRT, T_GRASS, T_MOUNTAIN, T_TREE, T_WATER};
enum quadrant {Q_TOPRIGHT, Q_TOPLEFT, Q_BOTTOMLEFT, Q_BOTTOMRIGHT};
enum mode {M_MENU, M_GAME, M_END};
enum entityType {E_ARCHER, E_ENEMY, E_CC, E_TENT, E_WALL, E_FOOD, E_LUMBER, E_STONE, E_TORCH, E_GATE, E_TURRET};
enum buttonType {
	B_DESTROY, B_NONE, B_BACK,  //building icons
	B_INDUSTRY, B_FOOD, B_DEFENSE, B_MILITARY, B_COLONY, //building types
	B_LUMBER, B_STONE, //INDUSTRY
	B_FARM, //food
	B_TURRET, B_WALL, B_GATE, //defense
	B_ARCHER, //military
	B_TENT, B_TORCH,
	B_PLAY, B_QUIT, B_RETURN, //menu
	B_NAME, B_ATTACK, B_PATROL, B_ATTACKMOVE, B_MOVE, B_HALT, //unit commands
	B_HEALTH, B_DAMAGE, B_ATTACKRANGE, B_ATTACKSPEED, B_MOVESPEED, B_RANGE, //unit stats
	B_GOLD, B_WORKER, B_FOODUSED, B_RESOURCEL, B_RESOURCES, B_RESOURCEF, B_BUILDSTATE, B_BUILDQUEUE, //building stats
	B_W, B_F, B_G, B_L, B_S, //resources (worker, food, power, gold,lumber, stone
	B_TIME, B_PAUSE, B_RUNNING,
	B_ARCHERCOUNT //selecting multiple units
};
enum commandState {S_DEFAULT, S_ATTACK, S_PATROL, S_ATTACKMOVE, S_BUILD, S_TIME, S_DESTROY, S_RECRUIT};
enum entityStatus {STATUS_BUILD, STATUS_DEFAULT, STATUS_ATTACK, STATUS_MOVE, STATUS_DEAD, STATUS_ATTACKMOVE, STATUS_PATROL};
enum direction{D_N, D_S, D_E, D_W, D_NW, D_SW, D_NE, D_SE};

float mapValue(float value, float srcMin, float srcMax, float dstMin, float dstMax) {
	float retVal = dstMin + ((value - srcMin) / (srcMax - srcMin) * (dstMax - dstMin));
	if (retVal < dstMin) {
		retVal = dstMin;
	}
	if (retVal > dstMax) {
		retVal = dstMax;
	}
	return retVal;
}
void DrawHealthbar(ShaderProgram& program, GLuint barTexture, int current, int max, float p1, float p2, float s1, float s2) {
	float position[2] = { p1,p2 };
	float size[2] = { s1,s2 };
	std::vector<float> ver;
	std::vector<float> tex;
	float u, v, w, h;
	ver.insert(ver.end(), {
		position[0], position[1],
		position[0], position[1] - size[1],
		position[0] + size[0], position[1],
		position[0] + size[0], position[1] - size[1],
		position[0] + size[0], position[1],
		position[0], position[1] -size[1],
	});
	u = 20.0f / 161.0f;
	v = 20.0f / 141.0f; 
	w = 120.0f / 161.0f;
	h = 40.0f / 141.0f;
	tex.insert(tex.end(), {
		u, v, 
		u, v+h, 
		u+w, v,
		u+w, v+h,
		u+w, v,
		u, v+h
	});
	float midSize = mapValue(current, 0, max, position[0], position[0] + size[0]);
	ver.insert(ver.end(), {
		position[0], position[1],
		position[0], position[1] - size[1],
		midSize, position[1],
		midSize, position[1] - size[1],
		midSize, position[1],
		position[0], position[1] - size[1],
	});
	u = 20.0f / 161.0f;
	v = 80.0f / 141.0f;
	tex.insert(tex.end(), {
		u, v,
		u, v + h,
		u + w, v,
		u + w, v + h,
		u + w, v,
		u, v + h
	});
	glBindTexture(GL_TEXTURE_2D, barTexture);
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, ver.data());
	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, tex.data());
	glDrawArrays(GL_TRIANGLES, 0, ver.size() / 2);
}
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
			((size + spacing) * i) + (position[0]), position[1] ,
			((size + spacing) * i) + (position[0] ), (position[1]-size) ,
			((size + spacing) * i) + ((position[0]+size) ), position[1] ,
			((size + spacing) * i) + ((position[0]+size) ), (position[1]-size) ,
			((size + spacing) * i) + ((position[0]+size)),position[1] ,
			((size + spacing) * i) + (position[0] ), (position[1]-size) ,
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
	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glDrawArrays(GL_TRIANGLES, 0, vertexData.size() / 2);
}
void DrawIcon(ShaderProgram& program, GLuint texture, float position[], float size[], buttonType type) {
	std::vector<float> vertexData;
	std::vector<float> textureData;
	float w, h, u, v;
	bool flag = true;
	switch (type) {
		//time
	case B_PAUSE:
		u = 0.0f;
		v = 0.0f;
		w = 1.0f;
		h = 100.0f/200.0f;
		break;
	case B_RUNNING:
		u = 0.0f;
		v = 100.0f/200.0f;
		w = 1.0f;
		h = 100.0f/200.0f;
		break;
		//buildings
	case B_TENT:
		u = 390.0f / 633.0f;
		v = 0.0f / 657.0f;
		w = 75.0f / 633.0f;
		h = 70.0f / 657.0f;
		break;
	case B_FARM:
		u = 100.0f / 633.0f;
		v = 270.0f / 657.0f;
		w = 100.0f / 633.0f;
		h = 90.0f / 657.0f;
		break;
	case B_TURRET:
		u = 65.0f / 330.0f;
		v = 0.0f;
		w = 65.0f / 330.0f;
		h = 65.0f / 300.0f;
		break;
	case B_WALL:
		u = 465.0f / 633.0f;
		v = 135.0f / 657.0f;
		w = 65.0f / 633.0f;
		h = 62.0f / 657.0f;
		break;
	case B_GATE:
		u = 0.0f;
		v = 0.0f;
		w = 130.0f / 633.0f;
		h = 130.0f / 657.0f;
		break;
	case B_ARCHER:
		u = 120.0f / 292.0f;
		v = 370.0f / 637.0f;
		w = 60.0f / 292.0f;
		h = 60.0f / 637.0f;
		break;
	case B_TORCH:
		u = 95.0f / 192.0f;
		v = 155.0f / 256.0f;
		w = 30.0f / 192.0f;
		h = 45.0f / 256.0f;
		break;
	case B_LUMBER:
		u = 305.0f / 633.0f;
		v = 260.0f / 657.0f;
		w = 100.0f / 633.0f;
		h = 105.0f / 657.0f;
		break;
	case B_STONE:
		u = 210.0f / 633.0f;
		v = 553.0f / 657.0f;
		w = 93.0f / 633.0f;
		h = 97.0f / 657.0f;
		break;
		//commands
	//stats
	case B_HEALTH:
		u = (float)13 * 34.0f / 544.0f;
		v = 0.0f;
		w = 34.0f / 544.0f;
		h = 1.0f;
		break;
	case B_DAMAGE:
		u = (float)7 * 34.0f / 544.0f;
		v = 0.0f;
		w = 34.0f / 544.0f;
		h = 1.0f;
		break;
	case B_MOVESPEED:
		u = (float) 10 * 34.0f / 544.0f;
		v = 0.0f;
		w = 34.0f / 544.0f;
		h = 1.0f;
		break;
	case B_ATTACKSPEED:
		u = (float)9 * 34.0f / 544.0f;
		v = 0.0f;
		w = 34.0f / 544.0f;
		h = 1.0f;
		break;
	case B_ATTACKRANGE:
		u = (float)8 * 34.0f / 544.0f;
		v = 0.0f;
		w = 34.0f / 544.0f;
		h = 1.0f;
		break;
	case B_RANGE:
		u = (float)5 * 34.0f / 544.0f;
		v = 0.0f;
		w = 34.0f / 544.0f;
		h = 1.0f;
		break;
	case B_GOLD:
		u = (float)3 * 34.0f / 544.0f;
		v = 0.0f;
		w = 34.0f / 544.0f;
		h = 1.0f;
		break;
	case B_W:
	case B_WORKER:
		u = (float)11* 34.0f / 544.0f;
		v = 0.0f;
		w = 34.0f / 544.0f;
		h = 1.0f;
		break;
	//resources
	case B_FOODUSED:
	case B_RESOURCEF:
	case B_F:
		u = (float) 1* 34.0f / 544.0f;
		v = 0.0f;
		w = 34.0f / 544.0f;
		h = 1.0f;
		break;
	case B_G:
		u = (float)2 * 34.0f / 544.0f;
		v = 0.0f;
		w = 34.0f / 544.0f;
		h = 1.0f;
		break;
	case B_RESOURCEL:
	case B_L:
		u = (float)0 * 34.0f / 544.0f;
		v = 0.0f;
		w = 34.0f / 544.0f;
		h = 1.0f;
		break;
	case B_RESOURCES:
	case B_S:
		u = (float)4 * 34.0f / 544.0f;
		v = 0.0f;
		w = 34.0f / 544.0f;
		h = 1.0f;
		break;
	case B_ARCHERCOUNT:
		u = 130.0f / 292.0f;
		v = 380.0f / 637.0f;
		w = 50.0f / 292.0f;
		h = 60.0f / 637.0f;
		flag = false;
		break;
	}
	if (flag) {
		vertexData.insert(vertexData.end(), {
			position[0], position[1],
			position[0], position[1] - size[1],
			position[0] + size[0], position[1],
			position[0] + size[0], position[1] - size[1],
			position[0] + size[0], position[1],
			position[0], position[1] - size[1]
		});
	}
	else {
		vertexData.insert(vertexData.end(), {
			position[0], (position[1] + size[1]*2),
			position[0], position[1] - size[1],
			position[0] + size[0], (position[1] + size[1]*2),
			position[0] + size[0], position[1] - size[1],
			position[0] + size[0], (position[1] + size[1]*2),
			position[0], position[1] - size[1]
		});
	}
	textureData.insert(textureData.end(), {
		u, v, 
		u, v+h, 
		u+w, v,
		u+w, v+h,
		u+w, v, 
		u, v+h
	});
	
	glBindTexture(GL_TEXTURE_2D, texture);
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, textureData.data());
	glDrawArrays(GL_TRIANGLES, 0, 6);
}
struct feature {
	std::vector<std::pair<int, int>> points;
	terrainType type;
	int size[2];
	int amount;
};
struct soundAssets {
	void quit() {
		for (int i = 0; i < sounds.size(); ++i) {
			Mix_FreeChunk(sounds[i]);
		}
		for (int i = 0; i < musics.size(); ++i) {
			Mix_FreeMusic(musics[i]);
		}
	}
	std::vector<Mix_Chunk*> sounds;
	std::vector<Mix_Music*> musics;
};

class entity {
public:
	entity(entityType type, float x, float y, std::vector<GLuint> textures, soundAssets& sounds) : type(type), textures(textures), sounds(sounds) {
		position[0] = x; //entities are defined by center
		position[1] = y;
		position[2] = 0.0f;
		float u, v, w, h;
		switch (type) {
		case E_CC:
			name = "Command Center";
			statistics.insert(statistics.end(), {1500, 200, 8, 12, 0, 0, 0, 8}); //health, gold, worker, food, none, none, none, range
			size[0] = 3.0f;
			size[1] = 3.0f;
			tex = textures[5];
			u = 130.0f / 633.0f;
			v = 0.0f;
			w = 140.0f / 633.0f;
			h = 130.0f / 657.0f;
			break;
		case E_ARCHER:
			name = "Ranger";
			statistics.insert(statistics.end(), {60, 0, 2, 5, 15, 2, 8});//health, none, attack speed, range, damage, movespeed, vision
			size[0] = 0.75f;
			size[1] = 1.0f;
			tex = textures[6];
			u = 240.0f / 292.0f;
			v = 0.0f;
			w = 50.0f / 292.0f;
			h = 60.0f / 637.0f;
			unitDirection = D_S;
			break;
		case E_TENT:
			name = "Cottage";
			statistics.insert(statistics.end(), { 50, 100, 2, -4, 0, 0, 0, 8 });
			size[0] = 2.0f;
			size[1] = 2.0f;
			tex = textures[5];
			u = 390.0f / 633.0f;
			v = 0.0f / 657.0f;
			w = 75.0f / 633.0f;
			h = 70.0f / 657.0f;
			break;
		case E_FOOD:
			name = "Farm";
			statistics.insert(statistics.end(), { 50, -50, -2, 4, 0, 0, 0, 8 });
			size[0] = 2.0f;
			size[1] = 2.0f;
			tex = textures[5];
			u = 100.0f / 633.0f;
			v = 270.0f / 657.0f;
			w = 100.0f / 633.0f;
			h = 90.0f / 657.0f;
			break;
		case E_TURRET:
			name = "Ballista";
			statistics.insert(statistics.end(), { 200, -50, 2, 8, 100, 0, 8 });
			size[0] = 1.0f;
			size[1] = 1.0f;
			tex = textures[7];
			u = 65.0f / 330.0f;
			v = 0.0f;
			w = 65.0f / 330.0f;
			h = 65.0f / 300.0f;
			break;
		case E_WALL:
			name = "Wall";
			statistics.insert(statistics.end(), { 150, -10, 0, 0, 0, 0, 0, 8 });
			size[0] = 1.0f;
			size[1] = 1.0f;
			tex = textures[1];
			u = 16.0f / 112.0f;
			v = 3 * 16.0f / 832.0f;
			w = 16.0f / 112.0f;
			h = 16.0f / 832.0f;
			break;
		case E_GATE:
			name = "Gate";
			statistics.insert(statistics.end(), { 100, -10, 0, 0, 0, 0, 0, 8 });
			size[0] = 1.0f;
			size[1] = 1.0f;
			tex = textures[1];
			u = 6 * 16.0f / 112.0f;
			v = 4 * 16.0f / 832.0f;
			w = 16.0f / 112.0f;
			h = 16.0f / 832.0f;
			break;
		case E_TORCH:
			name = "Torch";
			statistics.insert(statistics.end(), { 20, 0, -1, 0, 0, 0, 0, 8 });
			size[0] = 1.0f;
			size[1] = 1.0f;
			tex = textures[9];
			u = 95.0f / 192.0f;
			v = 155.0f / 256.0f;
			w = 30.0f / 192.0f;
			h = 45.0f / 256.0f;
			break;
		case E_LUMBER:
			name = "Lumbermill";
			statistics.insert(statistics.end(), { 50, -50, -4, 0, 0, 0, 0, 8 });
			tex = textures[5];
			size[0] = 2.0f;
			size[1] = 2.0f;
			u = 305.0f / 633.0f;
			v = 260.0f / 657.0f;
			w = 100.0f / 633.0f;
			h = 105.0f / 657.0f;
			break;
		case E_STONE:
			name = "Quarry";
			statistics.insert(statistics.end(), { 50, -50, -4, 0, 0, 0, 0, 8 });
			tex = textures[5];
			size[0] = 2.0f;
			size[1] = 2.0f;
			u = 210.0f / 633.0f;
			v = 553.0f / 657.0f;
			w = 93.0f / 633.0f;
			h = 97.0f / 657.0f;
			break;
		case E_ENEMY:
			name = "Skeleton";
			statistics.insert(statistics.end(), {100, 0, 1, 1, 20, 1, 8 });//health, none, attack speed, range, damage, movespeed, vision
			size[0] = 0.75f;
			size[1] = 1.0f;
			u = 0.0f;
			v = 0.0f;
			w = 23.0f / 264.0f;
			h = 1.0f;
			break;
		}
		if (type == E_ARCHER || type == E_CC || type == E_ENEMY) {
			health = statistics[0];
		}
		else {
			unitState = STATUS_BUILD;
			health = 1;
			Mix_PlayChannel(-1, sounds.sounds[3], 0);
		}
		textureCoordinates.insert(textureCoordinates.end(), {
			u, v,
			u, v + h,
			u + w, v,
			u + w, v + h,
			u + w, v,
			u, v + h
		});
		vertexCoordinates.insert(vertexCoordinates.end(), {-size[0]/2, size[1]/2,
			-size[0]/2, -size[1]/2, size[0]/2, size[1]/2,
			size[0]/2, -size[1]/2, size[0]/2, size[1]/2, -size[0]/2, -size[1]/2
		});
	}
	bool buildUnit(bool newUnit) {
		if (newUnit) {
			buildQueue += 1;
		}
		else if (buildNewUnit) {
			buildNewUnit = false;
			return true;
		}
		return false;
	}

	//ai
	void setTarget(entity* t) {
		target = t;
	}
	void update() {
		if (health < 1) {
			unitState = STATUS_DEAD;
			switch (type) {
			case E_ARCHER:
				Mix_PlayChannel(-1, sounds.sounds[7], 0);
				break;
			case E_ENEMY:
				Mix_PlayChannel(-1, sounds.sounds[6], 0);
				break;
			default:
				Mix_PlayChannel(-1, sounds.sounds[5], 0);
			}
		}
		if (unitState == STATUS_DEFAULT && health < statistics[0]) {
			if (animationState == 0) {
				++health;
			}
		}
		if (buildQueue > 0) {
			if (animationState < 900) {
				++animationState;
			}
			else {
				buildQueue -= 1;
				buildNewUnit = true;
				animationState = 0;
			}
		}
		else if (type == E_ARCHER) {
			switch (unitState) {
			case STATUS_DEFAULT:
				if (target != nullptr && target->health > 0) {
					unitState = STATUS_ATTACK;
					previous = STATUS_DEFAULT;
					animationState = 0;
				}
				else {
					acceleration[0] = 0;
					acceleration[1] = 0;
					animationState = (animationState + 1) % 150;
				}
				break;
			case STATUS_DEAD:
				++animationState;
				if (animationState > 80) {
					isDead = true;
				}
				break;
			case STATUS_MOVE:
				if (path.size() == 0 || pathLocation == -1) {
					unitState = STATUS_DEFAULT;
					animationState = 0;
				}
				else if (distance(position[0], position[1], path[pathLocation].first, path[pathLocation].second) < 0.25) {
					move(path[pathLocation].first, path[pathLocation].second);
					pathLocation += pathDirection;
					setDirection();
					animationState = (animationState + 1) % 60;
				}
				else {
					move(path[pathLocation].first, path[pathLocation].second);
					setDirection();
					animationState = (animationState + 1) % 60;
				}
				break;
			case STATUS_ATTACK:
				if (target->health < 0) { //no target, return to default
					unitState = previous;
					animationState = 0;
					target = nullptr;
				}
				else if (distance(position[0], position[1], target->position[0], target->position[1]) < statistics[3]) {
					animationState = (animationState + 1) % 80;
					setDirection();
					if (animationState == 60) {//shoot
						target->hit(statistics[4]);
						Mix_PlayChannel(-1, sounds.sounds[1], 0);
					}
				}
				else if (path.size() > 0) { //move towards target until in range
					move(path.back().first, path.back().second);
					setDirection();
					animationState = (animationState + 1) % 60;
				}
				else {
					unitState = previous;
					animationState = 0;
				}
				break;
			case STATUS_PATROL:
				if (target != nullptr) {
					previous = STATUS_PATROL;
					unitState = STATUS_ATTACK;
					animationState = 0;
				}
				else if ((pathLocation == -1 && pathDirection == -1) || (pathLocation == path.size() && pathDirection == 1)) {
					pathDirection *= -1;
					pathLocation += pathDirection * 2;
				}
				else if (distance(position[0], position[1], path[pathLocation].first, path[pathLocation].second) < 0.1f) {
					move(path[pathLocation].first, path[pathLocation].second);
					pathLocation += pathDirection;
					setDirection();
					animationState = (animationState + 1) % 60;
				}
				else {
					move(path[pathLocation].first, path[pathLocation].second);
					setDirection();
					animationState = (animationState + 1) % 60;
				}
				break;
			case STATUS_ATTACKMOVE:
				if (target != nullptr && distance(target->position[0], target->position[1], position[0], position[1])< statistics[3]) {
					previous = STATUS_ATTACKMOVE;
					unitState = STATUS_ATTACK;
					animationState = 0;
				}
				else if (pathLocation == -1) {
					unitState = STATUS_DEFAULT;
				}
				else if (distance(position[0], position[1], path[pathLocation].first, path[pathLocation].second) < 0.1f) {
					move(path[pathLocation].first, path[pathLocation].second);
					pathLocation += pathDirection;
					setDirection();
					animationState = (animationState + 1) % 60;
				}
				else {
					move(path[pathLocation].first, path[pathLocation].second);
					setDirection();
					animationState = (animationState + 1) % 60;
				}
				break;
			}
		}
		else if (type == E_ENEMY) {
			switch (unitState) {
			case STATUS_DEFAULT:
				animationState = (animationState + 1) % 150;
				break;
			case STATUS_ATTACKMOVE:
				if (target == nullptr || target->getState() == STATUS_DEAD) {
					unitState = STATUS_DEFAULT;
					animationState = 0;
				}
				else if (pathLocation  < 0) {
					unitState = STATUS_DEFAULT;
				}
				else if (distance(position[0], position[1], target->position[0], target->position[1]) < 1.0f) {
					animationState = 0;
					unitState = STATUS_ATTACK;
				}
				else if (distance(position[0], position[1], path[pathLocation].first, path[pathLocation].second) < 0.2f) {
					move(path[pathLocation].first, path[pathLocation].second);
					--pathLocation;
					setDirection();
					animationState = (animationState + 1) % 100;
				}
				else {
					move(path[pathLocation].first, path[pathLocation].second);
					setDirection();
					animationState = (animationState + 1) % 100;
				}
				break;
			case STATUS_ATTACK:
				if (target == nullptr || target->getState() == STATUS_DEAD) {
					animationState = 0;
					unitState = STATUS_DEFAULT;
				}
				else if (distance(position[0], position[1], target->position[0], target->position[1]) > 1.5f) {
					animationState = 0;
					unitState = STATUS_DEFAULT;
				}
				else {
					animationState = (animationState + 1) % 200;
					if (animationState == 80) {
						target->hit(statistics[4]);
						Mix_PlayChannel(-1, sounds.sounds[4], 0);
					}
				}
				break;
			case STATUS_DEAD:
				++animationState;
				if (animationState > 140) {
					isDead = true;
				}
			}
		}
		else if (type == E_TURRET && unitState != STATUS_BUILD && unitState != STATUS_DEAD) {
			if (target == nullptr || target->health == 0 || distance(target->position[0], target->position[1], position[0], position[1])) {
				target = nullptr;
				unitState = STATUS_DEFAULT;
				animationState = 0;
			}
			else if (unitState == STATUS_ATTACK) {
				animationState = (animationState + 1) % 60;
				setDirection();
				if (animationState == 75) {
					target->health -= 50;
					Mix_PlayChannel(-1, sounds.sounds[0], 0);
				}
			}
		}
		else if (unitState == STATUS_BUILD){
			if (health > statistics[0]) {
				health = statistics[0];
				unitState = STATUS_DEFAULT;
			}
			animationState = (animationState + 1) % 30;
			switch (type) {
			case E_GATE:
			case E_WALL:
				if (animationState == 0) {
					health += 10;
				}
				break;
			default:
				if (animationState == 0) {
					++health;
				}
			}
		}
	}
	float distance(float x1, float y1, float x2, float y2) {
		return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
	}
	void move(float xDestination, float yDestination) { //returns any change in entity Tile
		std::pair<float, float> targetV = normalize(position[0], position[1], xDestination, yDestination);
		acceleration[0] = targetV.first/2 - velocity[0];
		acceleration[1] = targetV.second/2 - velocity[1];
		velocity[0] += acceleration[0] * statistics[5]/ 20;
		velocity[1] += acceleration[1] * statistics[5]/ 20;
		if (velocity[0] > 0.02f) {
			velocity[0] = 0.02f;
		}
		else if (velocity[0] < -0.02f) {
			velocity[0] = -0.02f;
		}
		if (velocity[1] > 0.02f) {
			velocity[1] = 0.02f;
		}
		else if (velocity[1] < -0.02f) {
			velocity[1] = -0.02f;
		}
		position[0] += velocity[0];
		position[1] += velocity[1];
	}
	void setDestination(std::vector<std::pair<float, float>> p, entityStatus status, entity* t) {
		path = p;
		pathLocation = p.size() - 2;
		pathDirection = -1;
		unitState = status;
		target = t;
	}
	std::pair<float,float> normalize(float x1, float y1, float x2, float y2) {
		float d = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
		return std::pair<float, float>((x2 - x1) / d, (y2 - y1) / d);
	}
	void setDirection() {
		float x, y, angle;
		switch (unitState) {
		case STATUS_ATTACK:
			x = target->position[0] - position[0];
			y = target->position[1] - position[1];
			angle = atan2(-y, x)*180/M_PI;
			break;
		default:
			angle = atan2(-velocity[1], velocity[0]) * 180 / M_PI;
		}
		if (angle > -23.0f && angle < 23) {
			unitDirection = D_E;
		}
		else if (angle > 23.0f && angle < 68.0f) {
			unitDirection = D_NE;
		}
		else if (angle > 68.0f && angle < 113.0f) {
			unitDirection = D_N;
		}
		else if (angle > 113.0f && angle < 158.0f) {
			unitDirection = D_NW;
		}
		else if (angle > -158.0f && angle < -113.0f) {
			unitDirection = D_SW;
		}
		else if (angle > -113.0f && angle < -68.0f) {
			unitDirection = D_S;
		}
		else if (angle > -68.0f && angle < -23.0f) {
			unitDirection = D_SE;
		}
		else {
			unitDirection = D_W;
		}
	}
	void hit(int damage) {
		health -= damage;
	}
	void setResource(int r) {
		if (type == E_FOOD) {
			statistics[3] = r;
		}
		else{
			statistics[4] = r;
		}
	}
	void setState(entityStatus state) {
		unitState = state;
	}

	//collision
	bool boxCollision(float x1, float y1, float x2, float y2) {
		float top, bottom, left, right;
		if (y1 < y2) {
			top = y2;
			bottom = y1;
		}
		else {
			top = y1;
			bottom = y2;
		}
		if (x1 < x2) {
			right = x2;
			left = x1;
		}
		else {
			right = x1;
			left = x2;
		}
		if (position[1]-(size[1]/2) > top ||
			position[1]+(size[1]/2) < bottom ||
			position[0]-(size[0]/2) > right ||
			position[0]+(size[0]/2) < left) {
			return false;
		}
		return true;
	}//box collision
	bool circleCollision(float x, float y) {
		return distance(position[0], position[1], x, y) > size[1] / 2;
	}
	void checkCollision(entity* other) { //entity-entity collision
		if (distance(other->position[0], other->position[1], position[0], position[1]) < 0.75) {
			if (other->type == E_ARCHER) {
				if (other->unitState == STATUS_DEFAULT && unitState != STATUS_DEFAULT) {
					std::pair<float, float> targetV = normalize(position[0], position[1], other->position[0], other->position[1]);
					other->move(other->position[0] + targetV.first, other->position[0] + targetV.second);
				}
				else if (other->unitState == STATUS_MOVE && unitState == STATUS_DEFAULT) {
					std::pair<float, float> targetV = normalize(other->position[0], other->position[1], position[0], position[1]);
					move(position[0] + targetV.first, position[0] + targetV.second);
				}
			}
			else {
				std::pair<float, float> selfV = normalize(other->position[0], other->position[1], position[0], position[1]);
				std::pair<float, float> otherV = normalize(position[0], position[1], other->position[0], other->position[1]);
				other->move(other->position[0] + otherV.first, other->position[0] + otherV.second);
				move(position[0] + selfV.first, position[1] + selfV.second);
			}
		}
	}

	//render
	void setVerticies() {
		float xSize = size[0] / 2;
		float ySize = size[1] / 2;
		if (type == E_ENEMY && (unitState == STATUS_ATTACK || unitState ==  STATUS_DEAD)) {
			if (unitState == STATUS_ATTACK && animationState > 77 && animationState < 143) {
				xSize = 0.5f;
			}
			else if (unitState == STATUS_DEAD && animationState > 110) {
				xSize = 0.5f;
			}
		}
		else {
			return;
		}
		std::vector<float> v = { -xSize, ySize,
			-xSize, -ySize, xSize, ySize,
			xSize, -ySize, xSize, ySize, -xSize, -ySize
		};
		vertexCoordinates = v;
	}
	void setTextures() {
		float u, v, w, h;
		int animation;
		flipped = false;
		if (type == E_ARCHER) {
			w = 50.0f / 292.0f;
			h = 60.0f / 637.0f;
			switch (unitState) {
			case STATUS_DEFAULT:
				v = 0.0f;
				switch (unitDirection) {
				case D_N:
					u = 0.0f;
					break;
				case D_S:
					u = 240.0f / 292.0f;
					break;
				case D_W:
					flipped = true;
				case D_E:
					u = 120.0f / 292.0f;
					break;
				case D_NW:
					flipped = true;
				case D_NE:
					u = 60.0f;
					break;
				case D_SW:
					flipped = true;
				case D_SE:
					u = 190.0f;
					break;
				}
				break;
			case STATUS_MOVE:
			case STATUS_ATTACKMOVE:
				animation = mapValue(animationState, 0, 60, 0, 5);
				switch (unitDirection) {
				case D_N:
					u = 0.0f;
					break;
				case D_NW:
					flipped = true;
				case D_NE:
					u = 60.0f / 292.0f;;
					break;
				case D_W:
					flipped = true;
				case D_E:
					u = 120.0f / 292.0f;
					break;
				case D_SW:
					flipped = true;
				case D_SE:
					u = 180.0f / 292.0f;
					break;
				case D_S:
					u = 240.0f / 292.0f;
					break;
				}
				switch (animation) {
				case 0:
					v = 0.0f;
					break;
				case 1:
					v = 80.0f / 637.0f;
					break;
				case 2:
					v = 150.0f / 637.0f;
					break;
				case 3:
					v = 230.0f / 637.0f;
					break;
				default:
					v = 300.0f / 637.0f;
				}
				break;
			case STATUS_ATTACK:
				animation = mapValue(animationState, 0, 80, 0, 5);
				switch (unitDirection) {
				case D_N:
					u = 0.0f;
					break;
				case D_NW:
					flipped = true;
				case D_NE:
					u = 60.0f / 292.0f;;
					break;
				case D_W:
					flipped = true;
				case D_E:
					u = 120.0f / 292.0f;
					break;
				case D_SW:
					flipped = true;
				case D_SE:
					u = 190.0f;
					break;
				case D_S:
					u = 240.0f / 292.0f;
					break;
				}
				switch (animation) {
				case 0:
				case 1:
				case 2:
					v = 300.0f / 637.0f;
					break;
				case 3:
					v = 370.0f / 637.0f;
					break;
				default:
					v = 445.0f / 637.0f;
					break;
				}
				break;
			case STATUS_DEAD:
				animation = mapValue(animationState, 0, 60, 0, 3);
				switch (unitDirection) {
				case D_NW:
					flipped = true;
				case D_N:
				case D_NE:
					v = 520 / 637.0f;
					if (animation == 0) {
						u = 5.0f / 292.0f;
					}
					else if (animation == 1) {
						u = 120.0f / 292.0f;
					}
					else {
						u = 230.0f;
					}
					break;
				case D_W:
				case D_SW:
					flipped = true;
				case D_S:
				case D_SE:
				case D_E:
					if (animation == 0) {
						u = 60.0f / 292.0f;
						v = 520.0f / 637.0f;
					}
					else if (animation == 1) {
						u = 180.0f / 292.0f;
						v = 520.0f / 637.0f;
					}
					else {
						u = 5.0f / 292.0f;
						v = 575.0f / 637.0f;
					}
					break;
				}
				break;
			}
			std::vector<float> texCoord;
			texCoord.insert(texCoord.end(), {
				u, v,
				u, v + h,
				u + w, v,
				u + w, v + h,
				u + w, v,
				u, v + h
			});
			textureCoordinates = texCoord;
		}
		else if (type == E_ENEMY) {
			v = 0.0f;
			h = 1.0f;
			if (unitDirection == D_E || unitDirection == D_S || unitDirection == D_SE || unitDirection == D_NE) {
				flipped = false;
			}
			else {
				flipped = true;
			}
			switch (unitState) {
			case STATUS_DEFAULT:
				tex = textures[12];
				animation = mapValue(animationState, 0, 150, 0, 11);
				switch (animation) {
				case 0:
					u = 0.0f;
					w = 23.0f / 264.0f;
					break;
				case 1:
					u = 24.0f / 264.0f;
					w = 23.0f / 264.0f;
					break;
				case 2:
					u = 48.0f / 264.0f;
					w = 23.0f / 264.0f;
					break;
				case 3:
					u = 72.0f / 264.0f;
					w = 23.0f / 264.0f;
					break;
				case 4:
					u = 97.0f / 264.0f;
					w = 23.0f / 264.0f;
					break;
				case 5:
					u = 122.0f / 264.0f;
					w = 23.0f / 264.0f;
					break;
				case 6:
					u = 146.0f / 264.0f;
					w = 23.0f / 264.0f;
					break;
				case 7:
					u = 169.0f / 264.0f;
					w = 23.0f / 264.0f;
					break;
				case 8:
					u = 192.0f / 264.0f;
					w = 23.0f / 264.0f;
					break;
				case 9:
					u = 216.0f / 264.0f;
					w = 23.0f / 264.0f;
					break;
				default:
					u = 240.0f / 264.0f;
					w = 23.0f / 264.0f;
					break;
				}
				break;
			case STATUS_ATTACKMOVE:
			case STATUS_MOVE:
				tex = textures[13];
				animation = mapValue(animationState, 0, 100, 0, 14);
				switch (animation) {
				case 0:
					u = 0.0f / 286.0f;
					w = 20.0f / 286.0f;
					break;
				case 1:
					u = 22.0f / 286.0f;
					w = 20.0f / 286.0f;
					break;
				case 2:
					u = 45.0f / 286.0f;
					w = 20.0f / 286.0f;
					break;
				case 3:
					u = 66.0f / 286.0f;
					w = 20.0f / 286.0f;
					break;
				case 4:
					u = 88.0f / 286.0f;
					w = 20.0f / 286.0f;
					break;
				case 5:
					u = 110.0f / 286.0f;
					w = 22.0f / 286.0f;
					break;
				case 6:
					u = 132.0f / 286.0f;
					w = 22.0f / 286.0f;
					break;
				case 7:
					u = 154.0f / 286.0f;
					w = 20.f / 286.0f;
					break;
				case 8:
					u = 176.0f / 286.0f;
					w = 22.0f / 286.0f;
					break;
				case 9:
					u = 198.0f / 286.0f;
					w = 22.0f / 286.0f;
					break;
				case 10:
					u = 221.0f /286.0f;
					w = 20.0f / 286.0f;
					break;
				case 11:
					u = 242.0f / 286.0f;
					w = 20.0f / 286.0f;
					break;
				default:
					u = 264.0f / 286.0f;
					w = 20.0 / 286.0f;
					break;
				}
				break;
			case STATUS_ATTACK:
				tex = textures[14];
				animation = mapValue(animationState, 0, 200, 0, 18);
				switch (animation) {
				case 0:
					u = 0.0f;
					w = 25.0f / 774.0f;
					break;
				case 1:
					u = 45.0f / 774.0f;
					w = 25.0f / 774.0f;
					break;
				case 2:
					u = 90.0f / 774.0f;
					w = 25.0f / 774.0f;
					break;
				case 3:
					u = 130.0f / 774.0f;
					w = 25.0f / 774.0f;
					break;
				case 4:
					u = 175.0f;
					w = 25.0f / 774.0f;
					break;
				case 5:
					u = 215.0f / 774.0f;
					w = 25.0f / 774.0f;
					break;
				case 6:
					u = 255.0f / 774.0f;
					w = 25.0f / 774.0f;
					break;
				case 7:
					u = 305.0f / 774.0f;
					w = 35.0f / 774.0f;
					break;
				case 8:
					u = 350.0f / 774.0f;
					w = 35.0f / 774.0f;
					break;
				case 9:
					u = 395.0f / 774.0f;
					w = 35.0f / 774.0f;
					break;
				case 10:
					u = 435.0f / 774.0f;
					w = 35.0f / 774.0f;
					break;
				case 11:
					u = 480.0f / 774.0f;
					w = 35.0f / 774.0f;
					break;
				case 12:
					u = 525.0f / 774.0f;
					w = 35.0f / 774.0f;
					break;
				case 13:
					u = 565.0f / 774.0f;
					w = 35.0f / 774.0f;
					break;
				case 14:
					u = 605.0f / 774.0f;
					w = 30.0f / 774.0f;
					break;
				case 15:
					u = 645.0f / 774.0f;
					w = 25.0f / 774.0f;
					break;
				case 16:
					u = 690.0f / 774.0f;
					w = 25.0f / 774.0f;
					break;
				default:
					u = 732.0f / 774.0f;
					w = 25.0f / 774.0f;
					break;
				}
				break;
			case STATUS_DEAD:
				tex = textures[15];
				animation = mapValue(animationState, 0, 140, 0, 14);
				switch (animation) {
				case 0:
					u = 41.0f / 495.0f;
					w = 24.0f / 495.0f;
					break;
				case 1:
					u = 73.0f / 495.0f;
					w = 24.0f / 495.0f;
					break;
				case 2:
					u = 104.0f / 495.0f;
					w = 24.0f / 495.0f;
					break;
				case 3:
					u = 137.0f / 495.0f;
					w = 24.0f / 495.0f;
					break;
				case 4:
					u = 169.0f / 495.0f;
					w = 24.0f / 495.0f;
					break;
				case 5:
					u = 203.0f / 495.0f;
					w = 24.0f / 495.0f;
					break;
				case 6:
					u = 236.0f / 495.0f;
					w = 24.0f / 495.0f;
					break;
				case 7:
					u = 269.0f / 495.0f;
					w = 24.0f / 495.0f;
					break;
				case 8:
					u = 301.0f / 495.0f;
					w = 24.0f / 495.0f;
					break;
				case 9:
					u = 334.0f / 495.0f;
					w = 24.0f / 495.0f;
					break;
				case 10:
					u = 367.0f / 495.0f;
					w = 24.0f / 495.0f;
					break;
				case 11:
					u = 396.0f / 495.0f;
					w = 30.0f / 495.0f;
					break;
				case 12:
					u = 429.0f / 495.0f;
					w = 30.0f / 495.0f;
					break;
				default:
					u = 462.0f / 495.0f;
					w = 30.0f / 495.0f;
					break;
				}
				break;
			}
			std::vector<float> texCoord;
			texCoord.insert(texCoord.end(), {
				u, v,
				u, v + h,
				u + w, v,
				u + w, v + h,
				u + w, v,
				u, v + h
			});
			textureCoordinates = texCoord;
		}
		else if (type == E_TURRET) {
			if (unitState == STATUS_BUILD) {
				animation = 0;
			}
			else {
				animation = mapValue(animationState, 0, 160, 0, 6);
			}
			w = 65.0f / 330.0f;
			h = 60.0f / 300.0f;
			flipped = false;
			switch (animation) {
			case 0:
			case 1:
			case 2:
				v = 0.0f;
				break;
			case 3:
				v = 65.0f / 300.0f;
				break;
			case 4:
				v = 130.0f / 300.0f;
				break;
			default:
				v = 195.0 / 300.0f;
				break;
			}
			switch (unitDirection) {
			case D_N:
				u = 0.0f;
				break;
			case D_NW:
				flipped = true;
			case D_NE:
				u = 65.0f / 330.0f;
				break;
			case D_W:
				flipped = true;
			case D_E:
				u = 130.0f / 330.0f;
				break;
			case D_SW:
				flipped = true;
			case D_SE:
				u = 200.0f / 330.0f;
				break;
			case D_S:
				u = 265.0f / 330.0f;
				break;
			}
			std::vector<float> texCoord;
			texCoord.insert(texCoord.end(), {
				u, v,
				u, v + h,
				u + w, v,
				u + w, v + h,
				u + w, v,
				u, v + h
			});
			textureCoordinates = texCoord;
		}
	}
	void render(ShaderProgram& program, Matrix& modelviewMatrix) {
		modelviewMatrix.Identity();
		modelviewMatrix.Translate(position[0], -position[1], position[2]);
		float fullColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		if (flipped) {
			modelviewMatrix.Scale(-1.0f, 1.0f, 1.0f);
		}
		program.SetModelviewMatrix(modelviewMatrix);
		if (unitState == STATUS_BUILD) {
			float partialColor[4] = { 0.8f, 0.8f, 0.8f, 0.5f };
			program.SetColor(partialColor);
		}
		else {
			program.SetColor(fullColor);
		}
		setTextures();
		setVerticies();
		glBindTexture(GL_TEXTURE_2D, tex);
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexCoordinates.data());
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, textureCoordinates.data());
		glDrawArrays(GL_TRIANGLES, 0, textureCoordinates.size() / 2);
		program.SetColor(fullColor);
	}

	//gettors
	std::string getStatistics(int i) {
		 std::stringstream s;
		 s << statistics[i];
		 return s.str();
	}
	std::vector<int> getStatistics() {
		return statistics;
	}
	entityType getType() {
		return type;
	}
	std::string getName() {
		return name;
	}
	std::pair<int, int> getHP() {
		return std::pair<int, int>(health, statistics[0]);
	}
	std::pair<float, float> getPosition() const {
		return std::pair<float, float>(position[0], position[1]);
	}
	entityStatus getState() {
		return unitState;
	}
	int getBuild() {
		return animationState;
	}
	int getBuildQueue() {
		return buildQueue;
	}
	bool dead() {
		return isDead;
	}
private:
	std::vector <int> statistics;
	entityType type;
	std::string name;
	int health;
	bool isDead = false;
	int animationState = 0;

	entityStatus unitState = STATUS_DEFAULT;
	entityStatus previous = STATUS_DEFAULT;
	entity* target;

	float position[3];
	float size[2];
	float velocity[2] = { 0.0f, 0.0f };
	float acceleration[2] = { 0.0f, 0.0f };

	std::vector<std::pair<float, float>> path;
	int pathLocation;
	int pathDirection;
	direction unitDirection;
	bool flipped;

	int buildQueue = 0;
	bool buildNewUnit = false;

	std::vector<GLuint> textures;
	soundAssets sounds;
	GLuint tex;
	std::vector<float> textureCoordinates;
	std::vector<float> vertexCoordinates;
};
struct button {
	button(float p[], float s[], std::vector<GLuint>& texture, bool c): text(text), textures(texture), clickable(c) {
		copy(position, p, 3);
		copy(size, s, 2);
	}
	void copy(float array1[], float array2[], int size) {
		for (int i = 0; i < size; ++i) {
			array1[i] = array2[i];
		}
	}
	bool collision(float x, float y) {
		if (x > position[0] && x < position[0] + size[0] &&
			y > position[1] - size[1] && y < position[1]) {
			return true;
		}
		return false;
	}
	void drawBackground(ShaderProgram& program) {
		std::vector<float> verticies;
		std::vector<float> tex;
		verticies.insert(verticies.end(), {
			position[0], position[1],
			position[0], position[1] - size[1],
			position[0] + size[0], position[1],
			position[0] + size[0], position[1] - size[1],
			position[0] + size[0], position[1],
			position[0], position[1] - size[1]
		});
		float w, h;
		float s = 512;
		float u, v;
		if (type == B_NONE) {
			u = 293.0f /512.0f;
			v = 392.0f / 512.0f;
			w = 45.0f / 512.0f;
			h = 45.0f / 512.0f;
		}
		else {
			u = 293.0f / 512.0f;
			v = 343.0f / 512.0f;
			w = 45.0f / 512.0f;
			h = 49.0f / 512.0f;
		}
		tex.insert(tex.end(), {
			u, v,
			u, v + h,
			u + w, v,
			u + w, v + h,
			u + w, v,
			u, v + h
		});
		glBindTexture(GL_TEXTURE_2D, textures[2]);
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, verticies.data());
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, tex.data());
		glDrawArrays(GL_TRIANGLES, 0, verticies.size()/2);
	}
	void render(ShaderProgram& program, Matrix& modelviewMatrix, Matrix& projectionMatrix) {
		if (clickable) {
			drawBackground(program);
		}
		float s[2] = { size[1], size[1] };
		float p[2] = { position[0] + size[1], position[1] };
		switch (type) {
			//cc buildings
		case B_TIME:
			DrawText(program, textures[3], text, size[1] * 4 / 5, -0.45f, position, modelviewMatrix);
			break;
		case B_RUNNING:
		case B_PAUSE:
			DrawIcon(program, textures[10], position, size, type);
			break;
		case B_INDUSTRY:
			DrawText(program, textures[3], "I", size[1] / 2, 0.0f, position, modelviewMatrix);
			break;
		case B_FOOD:
			DrawText(program, textures[3], "F", size[1] / 2, 0.0f, position, modelviewMatrix);
			break;
		case B_MILITARY:
			DrawText(program, textures[3], "M", size[1] / 2, 0.0f, position, modelviewMatrix);
			break;
		case B_DEFENSE:
			DrawText(program, textures[3], "D", size[1] / 2, 0.0f, position, modelviewMatrix);
			break;
		case B_COLONY:
			DrawText(program, textures[3], "C", size[1] / 2, 0.0f, position, modelviewMatrix);
			break;
		case B_TENT:	
		case B_WALL:
		case B_GATE:
		case B_FARM:			
		case B_LUMBER:
		case B_STONE:
			DrawIcon(program, textures[5], position, size, type);
			break;
		case B_TURRET:
			DrawIcon(program, textures[7], position, size, type);
			break;
		case B_ARCHER:
			DrawIcon(program, textures[6], position, size, type);
			break;
		case B_TORCH:
			DrawIcon(program, textures[9], position, size, type);
			break;
			//unit commands
		case B_HALT:
			DrawText(program, textures[3], "S", size[1] / 2, 0.0f, position, modelviewMatrix);
			break;
		case B_MOVE:
			DrawText(program, textures[3], "M", size[1] / 2, 0.0f, position, modelviewMatrix);
			break;
		case B_ATTACK:
			DrawText(program, textures[3], "A", size[1] / 2, 0.0f, position, modelviewMatrix);
			break;
		case B_ATTACKMOVE:
			DrawText(program, textures[3], "X", size[1] / 2, 0.0f, position, modelviewMatrix);
			break;
		case B_PATROL:
			DrawText(program, textures[3], "P", size[1] / 2, 0.0f, position, modelviewMatrix);
			break;
			//building UI
		case B_DESTROY:
			DrawText(program, textures[3], "D", size[1] / 2, 0.0f, position, modelviewMatrix);
			break;
			//menu UI
		case B_NONE:
			break;
			//statistics
		case B_NAME:
			DrawText(program, textures[3], text, size[1]*4/5, -0.5f, position, modelviewMatrix);
			break;
		case B_ARCHERCOUNT:
			s[0] = size[0]*2/3;
			s[1] = size[1];
			DrawIcon(program, textures[6], position, s, type);
			p[0] = position[0];
			p[1] = position[1] - size[1];
			DrawText(program, textures[3], text, size[1] * 4 / 5, -0.5f, p, modelviewMatrix);
			break;
		case B_BACK:
			DrawText(program, textures[3], "B", size[1]/2, 0.0f, position, modelviewMatrix);
			break;
		case B_BUILDSTATE:
			if (value > 0) {
				DrawHealthbar(program, textures[8], value, maxValue, position[0], position[1], size[0] * 2, size[1]);
				DrawText(program, textures[3], text, size[1] * 4 / 5, -0.5f, p, modelviewMatrix);
			}
			break;
		case B_BUILDQUEUE:
			if (value > 0) {
				DrawText(program, textures[3], text, size[1] * 4 / 5, -0.5f, p, modelviewMatrix);
			}
			break;
		case B_HEALTH:
			DrawHealthbar(program, textures[8], value, maxValue, position[0] + size[1], position[1], size[0] - size[1], size[1]);
		default:
			DrawIcon(program, textures[4], position, s, type);
			DrawText(program, textures[3], text, size[1]*4/5, -0.5f, p, modelviewMatrix);
		}
	}
	void setType(buttonType t) {
		type = t;
	}
	void setValue(std::vector<entity*> selected, buttonType t) {
		type = t;
		std::stringstream s;
		switch (t) {
			//cc
		case B_INDUSTRY:
			tooltip = "Resource Production";
			break;
		case B_FOOD:
			tooltip = "Food production";
			break;
		case B_MILITARY:
			tooltip = "Combat Units";
			break;
		case B_DEFENSE:
			tooltip = "Defensive Structures";
			break;
		case B_COLONY:
			tooltip = "Colonial Structures";
			break;

			//building UI
		case B_DESTROY:
			tooltip = "Demolish building";
			//statics
		case B_NAME:
			text = selected[0]->getName();
			break;
		case B_HEALTH:
			tooltip = "Health";
			value = selected[0]->getHP().first;
			maxValue = selected[0]->getHP().second;
			s << value << "/" << maxValue;
			text = s.str();
			break;
		case B_BUILDSTATE:
			tooltip = "Training";
			value = selected[0]->getBuild()/60;
			maxValue = 15;
			s << value << "/" << maxValue;
			text = s.str();
			break;
		case B_BUILDQUEUE:
			tooltip = "Build queue";
			value = selected[0]->getBuildQueue();
			s << "x" << value;
			text = s.str();
			break;
		case B_DAMAGE:
			tooltip = "Damage";
			text = selected[0]->getStatistics(4);
			break;
		case B_MOVESPEED:
			tooltip = "Movespeed";
			text = selected[0]->getStatistics(5);
			break;
		case B_ATTACKSPEED:
			tooltip = "Attack intervals";
			text = selected[0]->getStatistics(2);
			break;
		case B_ATTACKRANGE:
			tooltip = "Attack range";
			text = selected[0]->getStatistics(6);
			break;
		case B_RANGE:
			tooltip = "Vision range";
			text =  selected[0]->getStatistics(6);
			break;
		case B_GOLD:
			tooltip = "Gold";
			text =  selected[0]->getStatistics(1);
			break;
		case B_WORKER:
			tooltip = "Workers";
			text =  selected[0]->getStatistics(2);
			break;
		case B_FOODUSED:
			tooltip = "Food consumed";
			text = selected[0]->getStatistics(3);
			break;
		case B_RESOURCES:
		case B_RESOURCEF:
		case B_RESOURCEL:
			tooltip = "Resources produced";
			text = selected[0]->getStatistics(4);
			break;
		case B_ARCHERCOUNT:
			s << selected.size();
			text = s.str();
			break;
		default:
			text = "";
			break;
		}
	}
	void setValue(std::string newValue, std::string newValue2) {
		text = newValue;
		if (type != B_W && type != B_F) {
			text += "+" + newValue2;
		}
	}
	void setValue(int hour, int day) {
		std::stringstream s;
		s << "DAY " << day << "        " << "HOUR " << hour;
		text = s.str();
	}
	void setValue(buttonType t) {
		type = t;
	}
	buttonType getType() {
		return type;
	}
	buttonType type;
	float position[3];
	float size[2];

	std::string text;
	std::string tooltip;
	int value;
	int maxValue;
	std::vector<GLuint> textures;
	bool clickable;

	std::vector<float> verticies;
	std::vector<float> textureV;
};
class UI {
public:
	UI(std::vector<GLuint>& t) :textures(t) {
		//initalize background
		float width1 = ORTHOW * 9/10;
		float height = ORTHOW * 5/10;
		float tSize = 100.0f / 512.0f;
		backgroundV.insert(backgroundV.end(), {//minimap
			-ORTHOW, -ORTHOH + height,
			-ORTHOW, -ORTHOH,
			-ORTHOW + width1, -ORTHOH + height,
			-ORTHOW + width1, -ORTHOH,
			-ORTHOW + width1, -ORTHOH + height,
			-ORTHOW, -ORTHOH
		});
		float u = 0.0f;
		float v = 376.0f / 512.0f;
		backgroundT.insert(backgroundT.end(), {//512x512
			u,v,
			u, v + tSize,
			u + tSize, v,
			u + tSize, v + tSize,
			u + tSize, v,
			u, v + tSize
		}); 
		float width2 = ORTHOW * 6/10;
		backgroundV.insert(backgroundV.end(), {//statistics
			-ORTHOW + width1, -ORTHOH + height,
			-ORTHOW + width1, -ORTHOH,
			-ORTHOW + width1 + width2, -ORTHOH + height,
			-ORTHOW + width1 + width2, -ORTHOH,
			-ORTHOW + width1 + width2, -ORTHOH + height,
			-ORTHOW + width1, -ORTHOH
		});
		u = 190.0f / 512.0f;
		v = 0.0f;
		backgroundT.insert(backgroundT.end(), {
			u,v,
			u, v + tSize,
			u + tSize, v,
			u + tSize, v + tSize,
			u + tSize, v,
			u, v + tSize
		});
		float width3 = ORTHOW * 5 / 10;
		backgroundV.insert(backgroundV.end(), {//controls
			-ORTHOW + width2 + width1, -ORTHOH + height,
			-ORTHOW + width2 + width1, -ORTHOH,
			-ORTHOW + width2 + width1 +width3, -ORTHOH + height,
			-ORTHOW + width2 + width1 +width3, -ORTHOH,
			-ORTHOW + width2 + width1 +width3, -ORTHOH + height,
			-ORTHOW + width2 + width1, -ORTHOH
		});
		u = 0.0f;
		v = 376.0f / 512.0f;
		backgroundT.insert(backgroundT.end(), {
			u,v,
			u, v + tSize,
			u + tSize, v,
			u + tSize, v + tSize,
			u + tSize, v,
			u, v + tSize
		});
		initResources();
		initStatistics();
		initClickable();
		initTime();
	}
	void initResources() {
		float position[3] = { 0.0f, 0.0f, 0.0f };
		float size[2] = { ORTHOW * 1/4, ORTHOW * 7/100 };
		float buffer = ORTHOW * 1 / 40;
		float xStart = -ORTHOW + ORTHOW * 9/10 + buffer;
		float yStart = -ORTHOH + ORTHOW * 5/10 - buffer*2 - size[1];
		for (int x = 0; x < 2; ++x) {
			position[0] = xStart + buffer*x + size[0] * x;
			position[1] = yStart;
			resources.push_back(new button(position, size, textures, false));
		}
		size[0] = ORTHOW * 11/20;
		yStart -= (buffer + size[1]);
		for (int y = 0; y < 3; ++y) {
			position[0] = xStart;
			position[1] = yStart - buffer * y - size[1] * y;
			resources.push_back(new button(position, size, textures, false));
		}
		buttonType resourceType[5] = { B_W, B_F, B_G, B_L, B_S };
		for (int i = 0; i < 5; ++i) {
			resources[i]->setType(resourceType[i]);
		}
	}
	void initClickable() {
		float position[3] = { 0.0f, 0.0f, 0.0f };
		float size[2] = { ORTHOW * 2/15, ORTHOW * 2/15 };
		float buffer = ORTHOW * 1 / 40;
		float xStart = -ORTHOW + ORTHOW * 15/10 + buffer;
		float yStart = -ORTHOH + ORTHOH * 3 / 4 - buffer;
		for (int y = 0; y < 3; ++y) {
			for (int x = 0; x < 3; ++x) { //build from top left to bottom right
				position[0] = xStart + buffer*x + size[0] * x;
				position[1] = yStart - buffer*y - size[1] * y;
				clickable.push_back(new button(position, size, textures, true));
			}
		}
	}
	void initStatistics() {
		float position[3] = { 0.0f, 0.0f, 0.0f };
		float size[2];
		float buffer = ORTHOW * 1 / 40;
		float xStart = -ORTHOW + buffer;
		float yStart = -ORTHOH + ORTHOH * 3 / 4 - buffer;
		position[0] = xStart;
		position[1] = yStart;
		size[0] = ORTHOW * 17/20;
		size[1] = ORTHOW * 3/32;
		statistics.push_back(new button(position, size, textures, false));//name
		position[1] = yStart - buffer - size[1];
		statistics.push_back(new button(position, size, textures, false));//health
		yStart -= (buffer * 2 + size[1] * 2);
		size[0] = ORTHOW * 4/15;
		for (int y = 0; y < 2; ++y) {
			for (int x = 0; x < 3; ++x) {
				position[0] = xStart + buffer*x + size[0] * x;
				position[1] = yStart - buffer*y - size[1] * y;
				statistics.push_back(new button(position, size, textures, false));//unit stats
			}
		}
	}
	void initTime() {
		float position[3] = { 0.0f, 0.0f, 0.0f };
		float size[2] = { ORTHOW * 11/20, ORTHOW * 7 / 100 };
		float buffer = ORTHOW * 1 / 40;
		float xStart = -ORTHOW + ORTHOW * 9 / 10 + buffer;
		float yStart = -ORTHOH + ORTHOW * 5 / 10 - buffer;
		position[0] = xStart + buffer;
		position[1] = yStart;
		time.push_back(new button(position, size, textures, false));
		time[0]->setType(B_TIME);
		position[0] += size[0] / 2 - size[1] / 2 - buffer;
		size[0] = size[1];
		time.push_back(new button(position, size, textures, true));
		time[1]->setType(B_RUNNING);
	}
	std::pair<commandState, buttonType> input(std::vector<entity*> selected, float x, float y) {
		buttonType t;
		bool flag = false;
		for (int i = 0; i < clickable.size(); ++i) {
			if (clickable[i]->collision(x,y)){
				t = clickable[i]->getType();
				flag = true;
				break;
			}
		}
		if (flag) {
			std::vector<buttonType> uiChange;
			switch (t) {
			case B_COLONY:
				uiChange.insert(uiChange.end(), {
					B_TENT, B_NONE, B_NONE, B_TORCH, B_NONE, B_NONE, B_NONE, B_NONE, B_BACK
				});
				break;
			case B_FOOD:
				uiChange.insert(uiChange.end(), {
					B_FARM, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_BACK
				});
				break;
			case B_INDUSTRY:
				uiChange.insert(uiChange.end(), {
					B_LUMBER, B_STONE, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_BACK
				});
				break;
			case B_MILITARY:
				uiChange.insert(uiChange.end(), {
					B_ARCHER, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_BACK
				});
				break;
			case B_DEFENSE:
				uiChange.insert(uiChange.end(), {
					B_WALL, B_GATE, B_TURRET, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_BACK
				});
				break;
			case B_BACK:
				uiChange.insert(uiChange.end(), {
					B_COLONY, B_FOOD, B_INDUSTRY, B_DEFENSE, B_MILITARY, B_NONE, B_NONE, B_NONE, B_NONE
				});
				break;
			case B_ARCHER:
				return std::pair<commandState, buttonType>(S_RECRUIT, t);
			case B_NONE:
			case B_MOVE:
				return std::pair<commandState, buttonType>(S_DEFAULT, t);
			case B_DESTROY:
				return std::pair<commandState, buttonType>(S_DESTROY, t);
			case B_PATROL:
				return std::pair<commandState, buttonType>(S_PATROL, t);
			case B_ATTACK:
				return std::pair<commandState, buttonType>(S_ATTACK, t);
			case B_ATTACKMOVE:
				return std::pair<commandState, buttonType>(S_ATTACKMOVE, t);
			default:
				return std::pair<commandState, buttonType>(S_BUILD, t);
			}
			for (int i = 0; i < 9; ++i) {
				clickable[i]->setValue(uiChange[i]);
			}
			return std::pair<commandState, buttonType> (S_DEFAULT, t);
		}
		else if (time[1]->collision(x, y)) {
			buttonType t = time[1]->getType();
			if (t == B_PAUSE) {
				time[1]->setType(B_RUNNING);
			}
			else {
				time[1]->setType(B_PAUSE);
			}
			return std::pair<commandState, buttonType>(S_TIME, t);
		}
	}
	void setUI(std::vector<entity*>& selected) {
		if (UIstate == selected) {
			if (selected.size() == 1) {
				updateBar(selected[0]);
				return;
			}
		}
		UIstate = selected;
		std::vector<buttonType> statisticTypes;
		std::vector<buttonType> clickableTypes;
		if (selected.size() > 1) {
			switch (selected[0]->getType()) {
				//military units
			case E_ARCHER:
				statisticTypes.insert(statisticTypes.end(), {
					B_NONE, B_NONE, B_NONE, B_ARCHERCOUNT, B_NONE, B_NONE, B_NONE, B_NONE });
				clickableTypes.insert(clickableTypes.end(), {
					B_HALT, B_MOVE, B_ATTACK, B_PATROL, B_ATTACKMOVE, B_NONE, B_NONE, B_NONE, B_NONE });
			}
		}
		else {
			switch (selected[0]->getType()) {
			case E_CC:
				statisticTypes.insert(statisticTypes.end(), {
					B_NAME, B_HEALTH, B_GOLD, B_WORKER, B_FOODUSED, B_BUILDSTATE, B_NONE,  B_BUILDQUEUE });
				clickableTypes.insert(clickableTypes.end(), {
					B_COLONY, B_FOOD, B_INDUSTRY, B_DEFENSE, B_MILITARY,
					B_NONE, B_NONE, B_NONE, B_NONE, });
				break;
			case E_ARCHER:
				statisticTypes.insert(statisticTypes.end(), {
					B_NAME, B_HEALTH, B_NONE, B_ATTACKSPEED, B_ATTACKRANGE, B_DAMAGE, B_MOVESPEED, B_RANGE });
				clickableTypes.insert(clickableTypes.end(), {
					B_HALT, B_MOVE, B_ATTACK, B_PATROL, B_ATTACKMOVE, B_NONE, B_NONE, B_NONE, B_NONE });
				break;
			case E_TENT:
				statisticTypes.insert(statisticTypes.end(), {
					B_NAME, B_HEALTH, B_GOLD, B_WORKER, B_FOODUSED, B_NONE, B_NONE,  B_RANGE });
				clickableTypes.insert(clickableTypes.end(), {
					B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_DESTROY });
				break;
			case E_WALL:
				statisticTypes.insert(statisticTypes.end(), {
					B_NAME, B_HEALTH, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE,  B_RANGE });
				clickableTypes.insert(clickableTypes.end(), {
					B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_DESTROY });
				break;
			case E_LUMBER:
				statisticTypes.insert(statisticTypes.end(), {
					B_NAME, B_HEALTH, B_GOLD, B_WORKER, B_RESOURCEL, B_NONE, B_NONE,  B_RANGE });
				clickableTypes.insert(clickableTypes.end(), {
					B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_DESTROY });
				break;
			case E_STONE:
				statisticTypes.insert(statisticTypes.end(), {
					B_NAME, B_HEALTH, B_GOLD, B_WORKER, B_RESOURCES, B_NONE, B_NONE,  B_RANGE });
				clickableTypes.insert(clickableTypes.end(), {
					B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_DESTROY });
				break;
			case E_TURRET:
				statisticTypes.insert(statisticTypes.end(), {
					B_NAME, B_HEALTH, B_GOLD, B_ATTACKSPEED, B_ATTACKRANGE, B_DAMAGE, B_NONE, B_RANGE });
				clickableTypes.insert(clickableTypes.end(), {
					B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_DESTROY });
				break;
			case E_GATE:
				statisticTypes.insert(statisticTypes.end(), {
					B_NAME, B_HEALTH, B_NONE , B_NONE, B_NONE, B_NONE, B_NONE,  B_RANGE });
				clickableTypes.insert(clickableTypes.end(), {
					B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_DESTROY });
				break;
			case E_FOOD:
				statisticTypes.insert(statisticTypes.end(), {
					B_NAME, B_HEALTH, B_FOOD, B_WORKER, B_RESOURCEF, B_NONE, B_NONE,  B_RANGE });
				clickableTypes.insert(clickableTypes.end(), {
					B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_DESTROY });
				break;
			case E_TORCH:
				statisticTypes.insert(statisticTypes.end(), {
					B_NAME, B_HEALTH, B_GOLD, B_WORKER, B_NONE, B_NONE, B_NONE,  B_RANGE });
				clickableTypes.insert(clickableTypes.end(), {
					B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_NONE, B_DESTROY });
				break;
			}
		}
			for (int i = 0; i < statistics.size(); ++i) {
				statistics[i]->setValue(selected, statisticTypes[i]);
			}
			for (int i = 0; i < clickable.size(); ++i) {
				clickable[i]->setValue(selected, clickableTypes[i]);
			}
	}
	void updateResources(std::vector<int> r, std::vector<int> p) {
		for (int i = 0; i < resources.size(); ++i) {
			std::stringstream s1;
			std::stringstream s2;
			s1 << r[i];
			s2 << p[i];
			resources[i]->setValue(s1.str(), s2.str());
		}
	}
	void updateTime(int hour, int day, bool running) {
		time[0]->setValue(hour, day);
		if (running) {
			time[1]->setType(B_PAUSE);
		}
		else {
			time[1]->setType(B_RUNNING);
		}
	}
	void updateBar(entity* selected) {
		std::stringstream s;
		std::pair<int, int> hp = selected->getHP();
		statistics[1]->value = hp.first;
		s << hp.first << "/" << hp.second;
		statistics[1]->text = s.str();
		if (selected->getType() == E_CC) {
			int progress = selected->getBuild()/60;
			statistics[5]->value = progress;
			s.clear();
			s << progress << "/" << statistics[5]->maxValue;
			statistics[5]->text = s.str();
			statistics[7]->value = selected->getBuildQueue();
			s.clear();
			s << statistics[7]->value;
			statistics[7]->text = s.str();
		}
	}
	void render(ShaderProgram& program, Matrix& modelviewMatrix, Matrix& projectionMatrix, float cameraPos[]) {
		modelviewMatrix.Identity();
		modelviewMatrix.Translate(cameraPos[0], cameraPos[1], 0.0f);//change by projection matrix
		program.SetModelviewMatrix(modelviewMatrix);
		glBindTexture(GL_TEXTURE_2D, textures[2]);
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, backgroundV.data());
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, backgroundT.data());
		glDrawArrays(GL_TRIANGLES, 0, backgroundV.size() / 2);
		for (int i = 0; i < clickable.size(); ++i) {
			clickable[i]->render(program, modelviewMatrix, projectionMatrix);
		}
		for (int i = 0; i < statistics.size(); ++i) {
			statistics[i]->render(program, modelviewMatrix, projectionMatrix);
		}
		for (int i = 0; i < resources.size(); ++i) {
			resources[i]->render(program, modelviewMatrix, projectionMatrix);
		}
		for (int i = 0; i < time.size(); ++i) {
			time[i]->render(program, modelviewMatrix, projectionMatrix);
		}
	}
private:
	commandState commands = S_DEFAULT;
	std::vector<entity*> UIstate;
	//animationState;
	std::vector<float> backgroundV;
	std::vector<float> backgroundT;
	std::vector<button*> clickable;
	std::vector<button*> statistics;
	std::vector<button*> resources;
	std::vector<button*> time;
	std::vector<GLuint> textures;
};

class Gamestate {
public:
	Gamestate(std::vector<GLuint> tex, UI* gameUI, soundAssets& sounds) : textures(tex), gameUI(gameUI), sounds(sounds) {
		setGameState();
	}
	void setGameState() {
		buildings.clear();
		playerUnits.clear();
		enemyUnits.clear();
		selected.clear();
		resources.insert(resources.end(), { 0, 0, 500, 20, 20 });// workers, food,  gold, lumber, stone
		resourceProduction.insert(resourceProduction.end(), { 0,0,0,0,0 });
		stats.insert(stats.end(), { 0, 0, 0, 0 }); //gold earned, enemykilled, playerkilled, built
												   //map
		map = std::vector <std::vector<int> >(mapHeight, std::vector<int>(mapWidth, 0));
		terrainMap = std::vector <std::vector<int> >(mapHeight, std::vector<int>(mapWidth, 0));
		powerMap = std::vector <std::vector<int> >(mapHeight, std::vector<int>(mapWidth, 0));

		mouseState.first = S_DEFAULT;
		mouseState.second = B_NONE;

		srand(time(0));
		generateTiles(map, 4, 4); //grass & dirt
		generateTiles(terrainMap, 5, 4); //generate terrain features - lake, mountain, forests
		std::vector<std::vector<std::pair<int, int>>> mapArea;
		std::vector<std::vector<std::pair<int, int>>> terrainArea;
		//clear center area for player spawn
		int xCenter = ceil(mapWidth / 2);
		int yCenter = ceil(mapHeight / 2);
		for (int y = -2; y < 3; ++y) {
			for (int x = -2; x < 3; ++x) {
				terrainMap[yCenter + y][xCenter + x] = 0;
			}
		}

		cullArea(mapArea, 0, 5, 1);
		cullArea(terrainArea, 1, 3, 0);
		connectTerrain(mapArea);
		generateFeatures(terrainArea);

		//entities
		buildingMap = terrainMap;
		generateEnemies();
		spawnPlayer();
		fogMap = std::vector< std::vector<fowVertex>>(mapHeight + 1, std::vector<fowVertex>(mapWidth + 1, fowVertex()));
		updateFog();

		//navigationMap && playerunitMapp - player unit map makes use of buildings
		for (int y = 0; y < mapHeight; ++y) { //initialize map with nodes
			navigationMap.push_back(std::vector<node*>());
			playerUnitMap.push_back(std::vector <node*>());
			for (int x = 0; x < mapWidth; ++x) {
				/*
				if (terrainMap[y][x] == 0) {
				navigationMap[y].push_back(new node(x + 0.5f, y + 0.5f, 1));
				}
				else {
				navigationMap[y].push_back(new node(x + 0.5f, y + 0.5f, mapHeight));
				}
				*/
				if (buildingMap[y][x] < 2) {
					playerUnitMap[y].push_back(new node(x + 0.5f, y + 0.5f, 1));
				}
				else {
					playerUnitMap[y].push_back(new node(x + 0.5f, y + 0.5f, mapHeight));
				}
			}
		}
		for (int y = 0; y < mapHeight; ++y) { //initialize edges
			for (int x = 0; x < mapWidth; ++x) {
				for (int i = -1; i < 2; ++i) {
					for (int j = -1; j < 2; ++j) {//includes diagonal members
						if (y + i > 0 && y + i < mapHeight && x + j > 0 && x + j < mapWidth) {
							//navigationMap[y][x]->neighbors.push_back(navigationMap[y + i][x + j]);
							playerUnitMap[y][x]->neighbors.push_back(playerUnitMap[y + i][x + j]);
						}
					}
				}
			}
		}
		//breadthFirstSearch(); //initialize paths to center. all enemy units will follows these paths

		//rendering setup
		int i, j, t;
		float tile, xMap, yMap;
		for (int y = 0; y < map.size(); ++y) {  //fills the map verticies and textures
			for (int x = 0; x < map[0].size(); ++x) {
				switch (map[y][x]) {
				case 0:
					i = 320;
					j = 0;
					tile = 64;
					xMap = 550;
					yMap = 550;
					break;
				case 1:
					i = 128;
					j = 414;
					tile = 64;
					xMap = 550;
					yMap = 550;
					break;
				}
				mapTextures.insert(mapTextures.end(), {
					i / xMap, j / yMap,
					i / xMap, (j + tile) / yMap,
					(i + tile) / xMap, j / yMap,
					(i + tile) / xMap, (j + tile) / yMap,
					(i + tile) / xMap, j / yMap,
					i / xMap, (j + tile) / yMap
				});
				mapVerticies.insert(mapVerticies.end(), {
					x*tileSize, -y*tileSize,
					x*tileSize, -y*tileSize - tileSize,
					x*tileSize + tileSize, -y*tileSize,
					x*tileSize + tileSize, -y*tileSize - tileSize,
					x*tileSize + tileSize, -y*tileSize,
					x*tileSize, -y*tileSize - tileSize
				});
			}
		}
		bool skipFlag;
		for (int y = 0; y < terrainMap.size(); ++y) {
			for (int x = 0; x < terrainMap[0].size(); ++x) {
				skipFlag = false;
				switch (terrainMap[y][x]) {
				case 2:
					i = 80;
					j = 16;
					tile = 16;
					xMap = 112;
					yMap = 832;
					break;
				case 3:
					i = 80;
					j = 0;
					tile = 16;
					xMap = 112;
					yMap = 832;
					break;
				case 4:
					i = 48;
					j = 384;
					tile = 16;
					xMap = 112;
					yMap = 832;
					break;
				default:
					skipFlag = true;
					break;
				}
				if (!skipFlag) {
					terrainVerticies.insert(terrainVerticies.end(), {
						x*tileSize, -y*tileSize,
						x*tileSize, -y*tileSize - tileSize,
						x*tileSize + tileSize, -y*tileSize,
						x*tileSize + tileSize, -y*tileSize - tileSize,
						x*tileSize + tileSize, -y*tileSize,
						x*tileSize, -y*tileSize - tileSize
					});
					terrainTextures.insert(terrainTextures.end(), {
						i / xMap, j / yMap,
						i / xMap, (j + tile) / yMap,
						(i + tile) / xMap, j / yMap,
						(i + tile) / xMap, (j + tile) / yMap,
						(i + tile) / xMap, j / yMap,
						i / xMap, (j + tile) / yMap
					});
				}
			}
		}
	}
	//cellular automata - modified game of life
	void generateTiles(std::vector<std::vector<int>>& mapType, int revive, int death) {
		//seed the map with live and dead
		for (size_t x = 0; x < mapWidth; ++x) {
			for (size_t y = 0; y < mapHeight; ++y) {
				mapType[y][x] = rand() % 2;
			}
		}
		int count = 2; //run the simulation three times
		std::vector<std::vector<int>> tempMap(mapHeight, std::vector<int>(mapWidth, 0));
		while (count > 0) {
			for (size_t x = 0; x < mapWidth; ++x) {
				for (size_t y = 0; y < mapHeight; ++y) {
					int neighbors = getNeighbors(x, y, mapType);
					if (mapType[y][x] == 0 && neighbors > revive) { //revive
						tempMap[y][x] = 1;
					}
					else if (mapType[y][x] == 1 && neighbors < death) {
						tempMap[y][x] = 0;
					}
					else {
						tempMap[y][x] = mapType[y][x];
					}
				}
			}
			mapType = tempMap;
			--count;
		}
	}
	int getNeighbors(int x, int y, std::vector<std::vector<int>> mapType) {
		int count = 0;
		for (int i = -1; i < 2; ++i) {
			for (int j = -1; j < 2; ++j) {
				if (x + i > -1 && x + i < mapWidth && y + j  > -1 && y + j < mapHeight) { //check within boundaries
					if (!(i == 0 && j == 0) && mapType[y + j][x + i] == 1) { //not center and cell is alive, count it
						++count;
					}
				}
			}
		}
		return count;
	}
	void cullArea(std::vector<std::vector<std::pair<int, int>>>& area, int target, int buffer, int replacement) {
		auto copy(terrainMap);
		for (int y = 0; y < mapHeight; ++y) { //get areas
			for (int x = 0; x < mapWidth; ++x) {
				if (copy[y][x] == target) {
					area.push_back(std::vector<std::pair<int, int>>());
					floodFill(copy, x, y, target, area.size() + target, area[area.size() - 1]);
				}
			}
		}
		//fill in small sections
		for (int i = area.size() - 1; i > -1; --i) {
			if (area[i].size() < buffer) {
				for (std::pair<int, int> tile : area[i]) {
					terrainMap[tile.second][tile.first] = replacement;
				}
				area[i] = area.back();
				area.pop_back();
			}
		}
	}
	void connectTerrain(std::vector<std::vector<std::pair<int, int>>>& area) {
		if (area.size() > 1) {
			int max = 0;
			int index = 0;
			for (int i = 0; i < area.size(); ++i) {
				if (area[i].size() > max) {
					max = area[i].size();
					index = i;
				}
			}
			for (int i = 0; i < area.size(); ++i) {//create paths between seperate areas
				if (i != index) {
					auto points = getMinimumDistance(area[i], area[index]);
					int xdistance = (points.first.first - points.second.first);
					int ydistance = (points.first.second - points.second.second);
					int steps;
					if (xdistance > ydistance) {
						steps = xdistance;
					}
					else {
						steps = ydistance;
					}
					float xStep = xdistance / steps;
					float yStep = ydistance / steps;
					int b;
					int x;
					int y;
					for (int a = 0; a < 2; ++a) {//top left and bottom right lines to ensure that a walkable path is formed.
						b = 0;
						while (b < steps) {
							++b;
							x = floor(points.first.first + a + xStep*b);
							y = floor(points.first.second + a + yStep*b);
							if (y < mapHeight && y < 0 && x < mapWidth && x > 0) {
								terrainMap[y][x] = 0;
							}

						}
					}
				}
			}
		}
	}
	std::pair<std::pair<int, int>, std::pair<int, int>> getMinimumDistance(
		std::vector<std::pair<int, int>>& disconnectedArea, std::vector<std::pair<int, int>>& mainArea) {
		float minimum = mapWidth + mapHeight;
		float current;
		std::pair<int, int> p1;
		std::pair<int, int> p2;
		for (int i = 0; i < disconnectedArea.size(); ++i) {
			for (int j = 0; j < mainArea.size(); ++j) {
				current = distance(disconnectedArea[i], mainArea[j]);
				if (current < minimum) {
					minimum = current;
					p1 = disconnectedArea[i];
					p2 = mainArea[i];
				}
			}
		}
		return std::pair<std::pair<int, int>, std::pair<int, int>>(p1, p2);
	}
	void generateFeatures(std::vector<std::vector<std::pair<int, int>>> area) {
		std::vector <feature> featureMap;
		//divide terrain into features if they are too large
		float x;
		int variance;
		int variance2;
		terrainType terrain;
		int size;
		for (auto featureSize : area) {
			x = (rand() % featureSize.size()) / featureSize.size();//probability to split
			if (featureSize.size() < 6) {	//too small to split
				fillTerrain(featureSize, 0, featureSize.size(), T_TREE);
			}
			else if (featureSize.size() < 20) {
				x = 0.1;
			}
			else if (featureSize.size() > 90) {
				x = 0.9;
			}
			if (x < 0.2) {	//1 terrain feautre
				generateTerrainType(terrain, mountain, lake, forest);
				fillTerrain(featureSize, 0, featureSize.size(), terrain);
			}
			else if (x < 0.8) {	//divide into 2 terrain features
				size = featureSize.size() / 2;
				variance = rand() % (size / 3);
				generateTerrainType(terrain, mountain, lake, 0);
				fillTerrain(featureSize, 0, size + variance, terrain);
				generateTerrainType(terrain, mountain, lake, forest);
				fillTerrain(featureSize, size + variance, featureSize.size(), terrain);
			}
			else {	//divide into 3 terrain features
				size = featureSize.size() / 3;
				variance = rand() % (size / 3) - size / 6;
				generateTerrainType(terrain, mountain, lake, forest);
				fillTerrain(featureSize, 0, size + variance, terrain);
				variance2 = rand() % (size / 3) - size / 6;
				generateTerrainType(terrain, mountain, lake, forest);
				fillTerrain(featureSize, size + variance, size * 2 + variance + variance2, terrain);
				generateTerrainType(terrain, mountain, lake, forest);
				fillTerrain(featureSize, size * 2 + variance + variance2, featureSize.size(), terrain);
			}

		}
	}
	void floodFill(std::vector<std::vector<int>>& tempMap, int x, int y, int tileType, int replacement, std::vector<std::pair<int, int>>& area) {
		/* recursion results in overflow
		if (tempMap[y][x] != tileType) {
			return;
		}
		tempMap[y][x] == replacement;
		std::pair<int, int> point(x, y);
		area.push_back(point);
		if (x - 1 > 0) {
			fill(tempMap, x - 1, y, tileType, replacement, area);
		}
		if (x + 1 < mapWidth - 1) {
			fill(tempMap, x + 1, y, tileType, replacement, area);
		}
		if (y - 1 > 0) {
			fill(tempMap, x, y - 1, tileType, replacement, area);
		}
		if (y + 1 < mapHeight - 1) {
			fill(tempMap, x, y + 1, tileType, replacement, area);
		}
		*/
		std::queue<std::pair<int, int>> queue;
		std::pair<int, int> current;
		queue.push(std::pair<int, int>(x, y));
		tempMap[y][x] = replacement;
		while (queue.size() != 0) {
			current = queue.front();
			area.push_back(current);
			queue.pop();
			if (current.first - 1 > -1 && tempMap[current.second][current.first - 1] == tileType) {
				tempMap[current.second][current.first - 1] = replacement;
				queue.push(std::pair<int, int>(current.first - 1, current.second));
			}
			if (current.first + 1 < mapWidth && tempMap[current.second][current.first + 1] == tileType) {
				tempMap[current.second][current.first + 1] = replacement;
				queue.push(std::pair<int, int>(current.first + 1, current.second));
			}
			if (current.second - 1 > -1 && tempMap[current.second - 1][current.first] == tileType) {
				tempMap[current.second - 1][current.first] = replacement;
				queue.push(std::pair<int, int>(current.first, current.second - 1));
			}
			if (current.second + 1 < mapHeight && tempMap[current.second + 1][current.first] == tileType) {
				tempMap[current.second + 1][current.first] = replacement;
				queue.push(std::pair<int, int>(current.first, current.second + 1));
			}
		}
	}
	void fillTerrain(std::vector<std::pair<int, int>>& tiles, int start, int end, terrainType terrain) {
		for (int i = start; i < end; ++i) {
			terrainMap[tiles[i].second][tiles[i].first] = terrain;
		}
	}
	void generateTerrainType(terrainType& terrain, int mountain, int water, int forest) {
		int t = rand() % (mountain + water + forest);
		if (t < mountain) {
			terrain = T_MOUNTAIN;
		}
		else if (t < mountain + water) {
			terrain = T_WATER;
		}
		else {
			terrain = T_TREE;
		}
	}
	
	//naviagtion
	void breadthFirstSearch() {
		std::queue<node*> nodes;
		std::unordered_set<node*> visited;
		node* current;
		nodes.push(navigationMap[floor(mapHeight / 2) - 1][floor(mapWidth / 2) - 1]);
		visited.insert(nodes.front());
		while (nodes.size() > 0) { //while not empty
			current = nodes.front();
			nodes.pop();
			for (int i = 0; i < current->neighbors.size(); ++i) {
				if (visited.count(current->neighbors[i]) == 0) {
					nodes.push(current->neighbors[i]); //add to queue if not checked
					current->neighbors[i]->previous = current;
					visited.insert(current->neighbors[i]);
				}
			}
		}
	}
	std::vector<std::pair<float, float>> enemyPath(float xStart, float yStart) {
		std::vector<std::pair<float, float>> path;
		node* current = navigationMap[yStart][xStart];
		while (current != nullptr) {
			current = current->previous;
			path.insert(path.end(), std::pair<float, float>(current->x, current->y));
		}
	}
	std::vector<std::pair<float, float>> playerPathfinding(float xStart, float yStart, float xDestination, float yDestination) {
		if (distance(xStart, yStart, xDestination, yDestination) < 2.0f) {
			std::vector<std::pair<float, float>> trivialPath;
			trivialPath.insert(trivialPath.end(), { 
				std::pair<float, float>(xDestination, yDestination), std::pair<float, float>(xStart, yStart) });
			return trivialPath;
		}
		std::set<node*, compareNode> minHeap;
		std::unordered_set<node*> visited;
		std::unordered_map<node*, float> cost;
		node* neighbor;
		node* current = playerUnitMap[yStart][xStart];
		int iterations = 0;
		minHeap.insert(current);
		visited.insert(current);
		cost[current] = 0;
		while (minHeap.size() > 0) { //while not empty
			++iterations;
			current = *minHeap.begin();
			minHeap.erase(*minHeap.begin());
			if (distance(current->x, current->y, xDestination, yDestination) < 1.0f) {
				break;
			}
			for (int i = 0; i < current->neighbors.size(); ++i) {
				neighbor = current->neighbors[i];
				if (visited.count(neighbor) == 0 || cost[current] + neighbor->cost < cost[neighbor]) {
					neighbor->previous = current;
					cost[neighbor] = neighbor->cost + cost[current];
					neighbor->pathCost = cost[neighbor];
					neighbor->distance = distance((float)neighbor->x, (float)neighbor->y, xDestination, yDestination);
					minHeap.insert(neighbor); //push neighbors onto heap
					visited.insert(neighbor); // to visited
				}
			}
			if (iterations > 200) {
				return std::vector<std::pair<float, float>>();
			}
		}
		std::vector<std::pair<float, float>> path;
		node* next = current->previous;
		node* before = next;
		bool walkable = true;
		float increment, x, y;
		path.insert(path.end(), std::pair<float, float>(xDestination, yDestination));
		while (next != playerUnitMap[yStart][xStart]) { //skip uneccessary nodes;
			x = next->x - current->x;
			y = next->y - current->y;
			increment = normalize(x, y);
			for (float i = 0; i < increment; i += 0.1f) {
				if (tileIntersection(current->x + x*i, current->y + y*i)) {
					walkable = false;
					break;
				}
			}
			if (walkable) {
				before = next;
				next = next->previous;
			}
			else {
				path.insert(path.end(), std::pair<float, float>(before->x, before->y));
				current = before;
				next = current->previous;
				before = next;
				walkable = true;
			}
		}
		path.insert(path.end(), std::pair<float, float>(xStart, yStart));
		return path;
	}
	bool tileIntersection(float x, float y) { //check corners to see if they intersect with unpassable terrain
		float top, bot, left, right;
		top = y + 0.5f;
		bot = y - 0.5f;
		left = x - 0.4f;
		right = x + 0.4f;
		if (buildingMap[top][left] > 1 || buildingMap[top][right] > 1 ||
			buildingMap[bot][left] > 1 || buildingMap[bot][right] > 1 || buildingMap[y][x]) {
			return true;
		}
		return false;
	}
	float normalize(float& x, float& y) {
		float d = sqrt(pow(x, 2) + pow(y, 2));
		x = x / d;
		y = y / d;
		return d;
	}

	//entities
	void generateEnemies() {
		int count = 80;
		int attempts = 0;
		float xCenter = (mapWidth / 2) + 0.5f;
		float yCenter = (mapHeight / 2) - 0.5f;
		float x, y, xDeviation, yDeviation;
		float xSize = 0.75f;
		float ySize = 1.0f;
		while (count > 0) {
			attempts = 0;
			while (attempts < 20) {
				++attempts;
				x = rand() % mapHeight;
				y = rand() % mapWidth;
				xDeviation = rand() / RAND_MAX;
				yDeviation = rand() / RAND_MAX;
				if (terrainMap[y][x] == 0 && distance(x, y, xCenter, yCenter) > 15.0f) { //valid location
					for (entity* other : enemyUnits) {
						//check if it collides with other entities;
						if (!other->boxCollision(x + xDeviation, y + yDeviation, x + xDeviation + xSize, y + yDeviation + ySize)) {
							break;
						}
					}
					enemyUnits.push_back(new entity(E_ENEMY, x + xDeviation, y + yDeviation, textures, sounds));
					break;
				}
			}
			--count;
		}
	}
	void spawnPlayer() {
		float x = (mapWidth / 2) + 0.5f;
		float y = (mapHeight / 2) - 0.5f;
		buildings.push_back(new entity(E_CC, x, y, textures, sounds));
		selected.push_back(buildings[0]);
		for (int i = -1; i < 2; ++i) {
			for (int j = -1; j < 2; ++j) {
				buildingMap[y + i][x + j] = 2;
			}
		}
		for (int i = 0; i < 4; ++i) {
			playerUnits.push_back(new entity(E_ARCHER, x + 0.8f* (i-1), y + 2, textures, sounds));
		}
	}
	void unitCommand(commandState command, float xDestination, float yDestination) {
		std::vector<std::pair<float, float>> path;
		std::pair<float, float> pos;
		switch (command) {
		case S_PATROL:
			for (int i = 0; i < selected.size(); ++i) {
				pos = selected[i]->getPosition();
				path = playerPathfinding(pos.first, pos.second, xDestination, yDestination);
				selected[i]->setDestination(path, STATUS_PATROL, nullptr);
			}
			break;
		case S_ATTACKMOVE:
			for (int i = 0; i < selected.size(); ++i) {
				pos = selected[i]->getPosition();
				path = playerPathfinding(pos.first, pos.second, xDestination, yDestination);
				selected[i]->setDestination(path, STATUS_ATTACKMOVE, nullptr);
			}
			break;
		default:
			for (int i = 0; i < enemyUnits.size(); ++i) { //if selected an enemy units
				if (enemyUnits[i]->boxCollision(xDestination, yDestination, xDestination, yDestination)) {
					for (int j = 0; j < selected.size(); ++j) {
						pos = selected[j]->getPosition();
						path = playerPathfinding(pos.first, pos.second, xDestination, yDestination);
						selected[j]->setDestination(path, STATUS_ATTACKMOVE, enemyUnits[i]);
					}
					return;
				}
			}
			for (int i = 0; i < selected.size(); ++i) {	//if selected ground
				pos = selected[i]->getPosition();
				path = playerPathfinding(pos.first, pos.second, xDestination, yDestination);
				selected[i]->setDestination(path, STATUS_MOVE, nullptr);
			}
		}
	}
	void spawnHorde(int amount) {
		int direction = rand() % 4;
		float xLocation, yLocation;
		float xSpawn, ySpawn;
		int attempts = 0;
		switch (direction) {
		case 0: //north
			yLocation = 0;
			while (true) {
				++attempts;
				ySpawn = -50;
				xSpawn = mapWidth / 2;
				xLocation = rand() % mapWidth;
				if (terrainMap[yLocation][xLocation] == 0) {
					break;
				}
			}
			break;
		case 1: //south
			yLocation = mapHeight - 1;
			while (true) {
				++attempts;
				ySpawn = mapHeight + 50;
				xSpawn = mapWidth / 2;
				xLocation = rand() % mapWidth;
				if (terrainMap[yLocation][xLocation] == 0) {
					break;
				}
			}
			break;
		case 2: //east
			xLocation = mapWidth - 1;
			while (true) {
				++attempts;
				ySpawn = mapHeight/2;
				xSpawn = mapWidth + 50;
				yLocation = rand() % mapHeight;
				if (terrainMap[yLocation][xLocation] == 0) {
					break;
				}
			}
			break;
		case 3: //west
			xLocation = 0;
			while (true) {
				++attempts;
				ySpawn = mapWidth/2;
				xSpawn = -50;
				yLocation = rand() % mapHeight;
				if (terrainMap[yLocation][xLocation] == 0) {
					break;
				}
			}
		}
		auto path = std::vector<std::pair<float, float>>(1, std::pair<float, float>(xLocation, yLocation));
		for (int i = 0; i < amount/2; ++i) {
			for (int j = 0; j < amount / 2; ++j) {
				horde.push_back(new entity(E_ENEMY, xSpawn + i, ySpawn + j, textures, sounds));
				horde.back()->setDestination(path, STATUS_ATTACKMOVE, nullptr);
			}
		}
	}
	void updateHorde() {
		std::pair<float, float> p;
		for (entity* e : horde) {
			if (e->getState() == STATUS_DEFAULT) {
				p = e->getPosition();
			}
		}
	}

	//input
	void scrolling(ShaderProgram& program, Matrix& projectionMatrix) {
		int x, y;
		SDL_GetMouseState(&x, &y);
		float xCoord = (float)((x / 1200.0f) * ORTHOW * 2) - ORTHOW;
		float yCoord = (((float)(800.0f - y) / 800.0f) * ORTHOH * 2) - ORTHOH;
		if (xCoord > ORTHOW - 1.5f && cameraPos[0] < mapWidth - ORTHOW) {
			cameraPos[0] += 0.1f;
		}
		else if (xCoord < -ORTHOW + 1.5f && cameraPos[0] > 0 + ORTHOW) {
			cameraPos[0] -= 0.1f;
		}
		if (yCoord > ORTHOH - 1.5f && cameraPos[1] < 0 - ORTHOH) {
			cameraPos[1] += 0.1f;
		}
		else if (yCoord < -ORTHOH + 1.5f && cameraPos[1] > -mapHeight + ORTHOH) {
			cameraPos[1] -= 0.1f;
		}
		projectionMatrix.Identity();
		projectionMatrix.SetOrthoProjection(-ORTHOW, ORTHOW, -ORTHOH, ORTHOH, -1.0f, 1.0f);
		projectionMatrix.Translate(-cameraPos[0], -cameraPos[1], 0.0f);
		program.SetProjectionMatrix(projectionMatrix);
	}
	void mouseInput(bool mouseButtonDown, bool rightClick) {
		if (rightClick && mouseButtonDown && mouseState.first != S_DEFAULT) { //reset mouse
			mouseState.first = S_DEFAULT;
		}
		//ui buttons
		else if ((mouseState.first == S_DEFAULT || mouseState.first == S_BUILD) && mousePos.second < -ORTHOH + ORTHOW * 5 / 10) {
			if (mouseButtonDown) {//prevents double clicking
				mouseState = gameUI->input(selected, mousePos.first, mousePos.second);
				if (mouseState.first == S_TIME) {
					if (mouseState.second == B_PAUSE) {
						running = false;
					}
					else {
						running = true;
					}
					mouseState.first = S_DEFAULT;
				}
				else if (mouseState.first == S_RECRUIT) {
					if (resourceCost(E_ARCHER)) {
						buildings[0]->buildUnit(true);
						mouseState.first = S_DEFAULT;
					}
				}
				else if (mouseState.first == S_DESTROY) {
					std::pair<float, float> temp;
					entityType t;
					for (int i = 0; i < selectedPos.size(); ++i) {
						t = buildings[selectedPos[i]]->getType();
						temp = buildings[selectedPos[i]]->getPosition();
						switch (t) {
						case E_GATE:
						case E_WALL:
						case E_TURRET:
						case E_TORCH:
							buildingMap[floor(temp.second)][floor(temp.first)] = 0;
							playerUnitMap[floor(temp.second)][floor(temp.first)]->cost = 1;
							break;
						default:
							for (int y = -1; y < 2; ++y) {
								for (int x = -1; x < 2; ++x) {
									buildingMap[floor(temp.second + y)][floor(temp.first + x)] = 0;
									playerUnitMap[floor(temp.second + y)][floor(temp.first + x)]->cost = 1;
								}
							}
						}
						buildings[selectedPos[i]] = buildings[buildings.size() - 1];
						buildings.pop_back();
						delete selected[i];
					}
					selected = std::vector<entity*>(1, buildings[0]);
					mouseState.first = S_DEFAULT;
				}
			}
		}
		//unit commands
		else if (rightClick && mouseButtonDown && mouseState.first == S_DEFAULT && selected[0]->getType() == E_ARCHER) {
			if (buildingMap[floor(tilePos.second)][floor(tilePos.first)] < 2) {
				unitCommand(S_DEFAULT, tilePos.first, tilePos.second);
			}
		}
		else if (!rightClick && mouseButtonDown && mouseState.first == S_PATROL && selected[0]->getType() == E_ARCHER) {
			if (buildingMap[floor(tilePos.second)][floor(tilePos.first)] < 2) {
				unitCommand(S_PATROL, tilePos.first, tilePos.second);
				mouseState.first = S_DEFAULT;
			}
		}
		else if (rightClick && mouseButtonDown && mouseState.first == S_ATTACKMOVE && selected[0]->getType() == E_ARCHER) {
			if (buildingMap[floor(tilePos.second)][floor(tilePos.first)] < 2) {
				unitCommand(S_ATTACKMOVE, tilePos.first, tilePos.second);
				mouseState.first = S_DEFAULT;
			}
		}
		else if (!rightClick && mouseState.first == S_DEFAULT) { //select
			if (mouseButtonDown) {
				select1.first = tilePos.first;
				select1.second = tilePos.second;
			}
			else {
				setSelected(tilePos.first, tilePos.second);
				select1.first = tilePos.first;
				select1.second = tilePos.second;
			}
		}
		//build 
		else if (mouseButtonDown && !rightClick && mouseState.first == S_BUILD && mousePos.second > -ORTHOH + ORTHOW*1/2) {//building
			float pos[2] = { floor(tilePos.first) + 0.5f, floor(tilePos.second) +0.5f }; //center of the tile
			float size = 0;
			if (checkValidBuilding(floor(tilePos.first), floor(tilePos.second))) {
				switch (mouseState.second) {
				case B_TENT:
					if (resourceCost(E_TENT)) {
						pos[0] = pos[0] + 0.5f; //4 tile size building
						pos[1] = pos[1] + 0.5f;
						buildings.push_back(new entity(E_TENT, pos[0], pos[1], textures, sounds));
						size = 2;
					}
break;
				case B_STONE:
					if (resourceCost(E_STONE)) {
						pos[0] = pos[0] + 0.5f; //4 tile size building
						pos[1] = pos[1] + 0.5f;
						buildings.push_back(new entity(E_STONE, pos[0], pos[1], textures, sounds));
						size = 2;
					}
					break;
				case B_LUMBER:
					if (resourceCost(E_LUMBER)) {
						pos[0] = pos[0] + 0.5f; //4 tile size building
						pos[1] = pos[1] + 0.5f;
						buildings.push_back(new entity(E_LUMBER, pos[0], pos[1], textures, sounds));
						size = 2;
					}
					break;
				case B_FARM:
					if (resourceCost(E_FOOD)) {
						pos[0] = pos[0] + 0.5f; //4 tile size building
						pos[1] = pos[1] + 0.5f;
						buildings.push_back(new entity(E_FOOD, pos[0], pos[1], textures, sounds));
						size = 2;
					}
					break;
				case B_TORCH:
					if (resourceCost(E_TORCH)) {
						buildings.push_back(new entity(E_TORCH, pos[0], pos[1], textures, sounds));
						size = 1;
					}
					break;
				case B_WALL:
					if (resourceCost(E_WALL)) {
						buildings.push_back(new entity(E_WALL, pos[0], pos[1], textures, sounds));
						size = 1;
					}
					break;
				case B_GATE:
					if (resourceCost(E_GATE)) {
						buildings.push_back(new entity(E_GATE, pos[0], pos[1], textures, sounds)); //gates are 1x1 tiles because I dont want to do rotation
						size = 1;
					}
					break;
				case B_TURRET:
					if (resourceCost(E_TURRET)) {
						playerUnits.push_back(new entity(E_TURRET, pos[0], pos[1], textures, sounds));
						size = 1;
					}
					break;
				}
				stats[3] += 1;
				if (size == 2) {
					for (int i = 0; i < 2; ++i) {
						for (int j = 0; j < 2; ++j) {
							buildingMap[tilePos.second + j][tilePos.first + i] = 2;
							playerUnitMap[tilePos.second + j][tilePos.first + i]->cost = mapHeight;
						}
					}
				}
				else if (size == 1) {
					buildingMap[tilePos.second][tilePos.first] = 2;
					playerUnitMap[tilePos.second][tilePos.first]->cost = mapHeight;
				}
			}
		}
		else if (mouseState.first < 0 || mouseState.first > 8) {
			mouseState.first = S_DEFAULT;
		}
	}
	float distance(float x1, float y1, float x2, float y2) {
		return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
	}
	float distance(std::pair<int, int>& p1, std::pair<int, int>& p2) {
		return sqrt(pow(abs(p1.first - p2.first), 2) - pow(abs(p1.second - p2.second), 2));
	}
	void setSelected(float x, float y) {
		std::vector<entity*> newSelected;
		std::vector<int> pos;
		//prioritize selecting units
		for (int i = 0; i < playerUnits.size(); ++i) {
			if (playerUnits[i]->boxCollision(select1.first, select1.second, x, y)) {
				newSelected.push_back(playerUnits[i]);
				pos.push_back(i);
			}
		}
		if (newSelected.size() == 0) {//else check the buildings
			for (int i = 0; i < buildings.size(); ++i) {
				if (buildings[i]->boxCollision(select1.first, select1.second, x, y)) {
					newSelected.push_back(buildings[i]);
					pos.push_back(i);
					break;
				}
			}
		}
		if (newSelected.size() > 0) {
			selected = newSelected;
			selectedPos = pos;
		}
	}
	std::vector<float> getStatistics() {
		return stats;
	}
	bool checkGameOver() {
		return enemyUnits.size() == 0 || buildings[0]->getHP().first == 0;
	}

	//updating
	void update(float elapsed) {
		if (transition) {
			--transitionState;
			if (transitionState < 0) {
				transition = false;
				Mix_HaltMusic();
				Mix_VolumeMusic(30);
				Mix_PlayMusic(sounds.musics[1], -1);
			}
		}
		gameUI->setUI(selected);
		gameUI->updateResources(resources, resourceProduction);
		gameUI->updateTime(gameTime.hour, gameTime.day, running);
		updatePower();
		//commands
		if (running) {
			//calculate gold and food
			updateFog();
			if (gameTime.update(elapsed)) {
				//play sound?
				updateResources(true);
			}
			//playerunits
			std::vector<std::pair<float, float>> path;
			float d;
			for (int i = 0; i < playerUnits.size(); ++i) {
				if (playerUnits[i]->getState() == STATUS_DEFAULT || playerUnits[i]->getState() == STATUS_ATTACKMOVE ||
					playerUnits[i]->getState() == STATUS_PATROL) { //automaticaly attack enemies in range
					std::pair<float, float> p = playerUnits[i]->getPosition();
					for (auto enemy : enemyUnits) {
						std::pair<float, float> e = enemy->getPosition();
						if (enemy->getState() != STATUS_DEAD && fogMap[e.second][e.first].inVision) {
							d = distance(p.first, p.second, e.first, e.second);
							if (d < playerUnits[i]->getStatistics()[3]) {//enemy in range;
								playerUnits[i]->setTarget(enemy);
								if (enemy->getState() == STATUS_DEFAULT) {
									path = playerPathfinding(e.first, e.second, p.first, p.second);
									enemy->setDestination(path, STATUS_ATTACKMOVE, playerUnits[i]);
								}
								break;
							}
						}
					}
				}
				playerUnits[i]->update();
				if (playerUnits[i]->dead()) {
					stats[2] += 1;
					auto temp = playerUnits[i];
					playerUnits[i] = playerUnits.back();
					playerUnits.pop_back();
					for (int j = 0; j < selected.size(); ++j) {
						if (selected[j] == temp) {
							selected[j] = selected.back();
							selected.pop_back();
							if (selected.size() == 0) {
								selected.push_back(buildings[0]);
							}
						}
					}
				}
			}
			//enemies
			std::pair<float, float> pos;
			std::pair<float, float> targetPos;
			std::vector<entity*> targets;
			targets.insert(targets.end(), buildings.begin(), buildings.end());
			targets.insert(targets.end(), playerUnits.begin(), playerUnits.end());
			float minDistance;
			entity* target;
			for (int i = 0; i < enemyUnits.size(); ++i) {
				pos = enemyUnits[i]->getPosition();
				if (fogMap[pos.second][pos.first].inVision) {
					if (enemyUnits[i]->getState() == STATUS_DEFAULT || enemyUnits[i]->getState() == STATUS_ATTACKMOVE) {
						target = nullptr;
						minDistance = mapHeight;
						for (entity* e : targets) {
							targetPos = e->getPosition();
							d = distance(pos.first, pos.second, targetPos.first, targetPos.second);
							if (d < minDistance) {
								minDistance = d;
								if (d < 5.0f) {
									target = e;
								}
							}
						}
						if (target != nullptr) {
							targetPos = target->getPosition();
							pos = enemyUnits[i]->getPosition();
							path = std::vector<std::pair<float, float>>(2, std::pair<float, float>(targetPos.first, targetPos.second));
							enemyUnits[i]->setDestination(path, STATUS_ATTACKMOVE, target);
						}
					}
					enemyUnits[i]->update();
				}
				if (enemyUnits[i]->dead()) {
					stats[1] += 1;
					auto temp = enemyUnits[i];
					enemyUnits[i] = enemyUnits.back();
					enemyUnits.pop_back();
					delete temp;
				}
			}
			for (int i = 0; i < buildings.size(); ++i) {
				buildings[i]->update();
				if (buildings[i]->dead()) {
					auto temp = buildings[i];
					buildings[i] = buildings.back();
					buildings.pop_back();
					auto p = temp->getPosition();
					switch (temp->getType()) {
					case E_TORCH:
					case E_TURRET:
					case E_WALL:
					case E_GATE:
						buildingMap[floor(p.second)][floor(p.first)] = 0;
						playerUnitMap[floor(p.second)][floor(p.first)]->cost = 1;
					default:
						for (int y = -1; y < 2; ++y) {
							for (int x = -1; x < 2; ++x) {
								buildingMap[floor(p.second + y)][floor(p.first + x)] = 0;
								playerUnitMap[floor(p.second + y)][floor(p.first + x)]->cost = 1;
							}
						}
					}
					delete temp;
				}
			}
			//buildings
			if (buildings[0]->buildUnit(false)) {
				std::pair<float, float> pos = buildings[0]->getPosition();
				playerUnits.push_back(new entity(E_ARCHER, pos.first, pos.second + 1.5f, textures, sounds));
			}
			checkCollisions();
		}
		else {
			updateResources(false);
		}
	}
	void updateResources(bool time) {
		std::vector<int> temp;
		std::vector<int> r;
		r.insert(r.end(), { 0, 0, 0, 0, 0 });
		for (int i = 0; i < buildings.size(); ++i) {
			if (buildings[i]->getType() == E_LUMBER || buildings[i]->getType() == E_STONE || buildings[i]->getType() == E_FOOD) {
				updateResourceBuilding(buildings[i]);
			}
			temp = buildings[i]->getStatistics();
			r[2] += temp[1];
			r[0] += temp[2];
			r[1] += temp[3];
			r[3] += temp[4];
			r[4] += temp[5];
		}
		resourceProduction = r;
		if (time) {
			for (int i = 2; i < 5; ++i) {
				resources[i] += resourceProduction[i];
			}
		}
		resources[0] = r[0];
		resources[1] = r[1];
		stats[0] += r[2];
	}
	void updateResourceBuilding(entity* b) {
		int resource = 0;
		std::pair<float, float> p = b->getPosition();
		switch (b->getType()) {
		case E_STONE:
			for (int x = -2; x < 3; ++x) {
				for (int y = -2; y < 3; ++y) {
					if (terrainMap[p.second+y][p.first+x] == T_MOUNTAIN) {
						++resource;
					}
				}
			}
			break;
		case E_LUMBER:
			for (int x = -2; x < 3; ++x) {
				for (int y = -2; y < 3; ++y) {
					if (terrainMap[p.second + y][p.first + x] == T_TREE) {
						resource += 2;
					}
				}
			}
			break;
		case E_FOOD:
			for (int x = -2; x < 3; ++x) {
				for (int y = -2; y < 3; ++y) {
					if (buildingMap[p.second + y][p.first + x] == 0) {
						if (terrainMap[p.second + y][p.first + x] == T_GRASS) {
							resource += 2;
						}
						else {
							++resource;
						}
					}
					
				}
			}
		}
		b->setResource(resource);
	}
	void checkCollisions() {
		std::pair<float, float> pos;
		for (int i = 0; i < playerUnits.size(); ++i) {
			for (entity* unit : playerUnits) {
				if (unit != playerUnits[i]) {
					playerUnits[i]->checkCollision(unit);
				}
			}
			for (entity* unit : enemyUnits) {
				playerUnits[i]->checkCollision(unit);
			}
		}
		for (int i = 0; i < enemyUnits.size(); ++i) {
			for (entity* unit : enemyUnits) {
				if (unit != enemyUnits[i]) {
					enemyUnits[i]->checkCollision(unit);
				}
			}
		}
	}
	bool checkValidBuilding(float x, float y) {
		switch (mouseState.second) {
		case B_STONE:
		case B_LUMBER:
		case B_FARM:
		case B_TENT:
			for (int i = 0; i < 2; ++i) {
				for (int j = 0; j < 2; ++j) {
					if (y + j < 0 || y + j > mapHeight || x + i > mapWidth || x + i < 0
						|| buildingMap[y + j][x + i] != 0 || powerMap[y + j][x + i] == 0) {
						return false;
					}
				}
			}
			break;
		default:
			if (buildingMap[y][x] != 0 || powerMap[y][x] == 0) {
				return false;
			}
			buildingMap[y][x] = 2;
			playerUnitMap[y][x]->cost = mapHeight;
			break;
		}
		return true;
	}
	bool checkPower(float x, float y) {

	}
	bool resourceCost(entityType type) {
		std::vector<int> cost;
		switch (type) {
		case E_TENT:
			cost.insert(cost.end(), { 0, 4, 50, 0, 0 }); // workers, food, gold, lumber, stone
			break;
		case E_STONE:
			cost.insert(cost.end(), { 4, 0, 300, 20, 0 });
			break;
		case E_LUMBER:
			cost.insert(cost.end(), { 4, 0, 200, 0, 0 });
			break;
		case E_FOOD:
			cost.insert(cost.end(), { 2, 0, 200, 0, 0 });
			break;
		case E_TORCH:
			cost.insert(cost.end(), { 1, 0, 50, 1, 0 });
			break;
		case E_WALL:
			cost.insert(cost.end(), { 0, 0, 10, 2, 0 });
			break;
		case E_GATE:
			cost.insert(cost.end(), { 0, 0, 20, 3, 0 });
			break;
		case E_TURRET:
			cost.insert(cost.end(), { 4, 0, 300, 10, 20 });
			break;
		case E_ARCHER:
			cost.insert(cost.end(), { 1, 0, 60, 5, 0 });
			break;
		}
		for (int i = 0; i < 5; ++i) {
			if (cost[i] > resources[i]) {
				return false;
			}
		}
		for (int i = 0; i < 5; ++i) {
			resources[i] -= cost[i];
		}
		return true;
	}
	void updatePower() { //update only when buildingsize changes
		std::vector<std::vector<int>> tempMap(mapHeight, std::vector<int>(mapWidth, 0));
		for (int i = 0; i < buildings.size(); ++i) {
			if (buildings[i]->getType() == E_TORCH || buildings[i]->getType() == E_CC) {
				std::pair<float, float> temp = buildings[i]->getPosition();
				int xPos = floor(temp.first);
				int yPos = floor(temp.second);
				for (int x = -8; x < 9; ++x) {
					for (int y = -8; y < 9; ++y) {
						if (yPos + y > 0 && yPos + y < mapHeight && xPos + x < mapWidth && xPos + x > 0 && (abs(x) + abs(y)) < 9) {
							tempMap[yPos + y][xPos + x] = 1;
						}
					}
				}
			}
		}
		powerMap.swap(tempMap);
	}
	void updateFog() {
		for (int i = 0; i < mapHeight + 1; ++i) { //rests all the vision
			for (int j = 0; j < mapHeight + 1; ++j){
				fogMap[i][j].inVision = 0;
			}
		}
		std::vector<entity*> entities; // holds all player entities
		entities.insert(entities.end(), buildings.begin(), buildings.end());
		entities.insert(entities.end(), playerUnits.begin(), playerUnits.end());
		std::pair<float, float> position;
		float xPos;
		float yPos;
		for (int i = 0; i < entities.size(); ++i) {
			position = entities[i]->getPosition();
			for (int y = -10; y < 11; ++y) { //check a range of tiles
				for (int x = -10; x < 11; ++x) {
					xPos = position.first + x;
					yPos = position.second + y;
					if (xPos > 0 && xPos < mapWidth + 1 && yPos > 0 && yPos < mapHeight + 1) {//within size
						if (distance(xPos, yPos, position.first, position.second) < 9.0f) {
							fogMap[floor(yPos)][floor(xPos)].explored = 1;
							fogMap[floor(yPos)][floor(xPos)].inVision = 1;
						}
					}
				}
			}
		}
	}
	std::vector<int> getProduction() {
		return resourceProduction;
	}

	//render
	void render(ShaderProgram& program, ShaderProgram& fog, Matrix& modelviewMatrix, Matrix& projectionMatrix) {
		modelviewMatrix.Identity();
		program.SetModelviewMatrix(modelviewMatrix);
		program.SetProjectionMatrix(projectionMatrix);
		glEnableVertexAttribArray(program.positionAttribute);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glEnableVertexAttribArray(program.exploredAttribute);
		renderMap(program, fog, modelviewMatrix, projectionMatrix);
		glDisableVertexAttribArray(program.exploredAttribute);
		renderEntities(program, modelviewMatrix);
		gameUI->render(program, modelviewMatrix, projectionMatrix, cameraPos);
		renderCursor(program, modelviewMatrix);
		if (transition) {
			float a = mapValue(transitionState, 0, 180, 0, 1);
			float box[12] = { -ORTHOW, ORTHOH, -ORTHOW, -ORTHOH, ORTHOW, ORTHOH, ORTHOW, -ORTHOH, ORTHOW, ORTHOH, -ORTHOW, -ORTHOH };
			float tex[12] = { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f };
			float black[4] = { 0.0f, 0.0f, 0.0f, a };
			program.SetColor(black);
			glBindTexture(GL_TEXTURE_2D, textures[16]);
			glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, box);
			glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, tex);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
	}
	//render tile map;
	void renderMap(ShaderProgram& program, ShaderProgram& fog, Matrix& modelviewMatrix, Matrix& projectionMatrix) {
		float fullColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		fog.SetModelviewMatrix(modelviewMatrix);
		fog.SetProjectionMatrix(projectionMatrix);
		glBindTexture(GL_TEXTURE_2D, textures[0]);
		glEnableVertexAttribArray(fog.positionAttribute);
		glEnableVertexAttribArray(fog.texCoordAttribute);
		glEnableVertexAttribArray(fog.exploredAttribute);
		fowVertex TL, TR, BL, BR;
		std::vector<float> state;
		for (int y = 0; y < mapHeight; ++y) {
			for (int x = 0; x < mapWidth; ++x) {
				TL = fogMap[y][x];
				TR = fogMap[y][x+1];
				BL = fogMap[y+1][x];
				BR = fogMap[y+1][x+1];
				state.insert(state.end(), {
					TL.getState(), BL.getState(),
					TR.getState(), BR.getState(),
					TR.getState(), BL.getState()
				});
			}
		}
		glVertexAttribPointer(fog.positionAttribute, 2, GL_FLOAT, false, 0, mapVerticies.data());
		glVertexAttribPointer(fog.texCoordAttribute, 2, GL_FLOAT, false, 0, mapTextures.data());
		glVertexAttribPointer(fog.exploredAttribute, 1, GL_FLOAT, false, 0, state.data());
		glDrawArrays(GL_TRIANGLES, 0, mapVerticies.size() / 2);

		std::vector<float> terrainState;
		for (int y = 0; y < mapHeight; ++y) {
			for (int x = 0; x < mapWidth; ++x) {
				if (terrainMap[y][x] != 0) {
					TL = fogMap[y][x];
					TR = fogMap[y][x + 1];
					BL = fogMap[y + 1][x];
					BR = fogMap[y + 1][x + 1];
					terrainState.insert(terrainState.end(), {
						TL.getState(), BL.getState(),
						TR.getState(), BR.getState(),
						TR.getState(), BL.getState()
					});
				}
			}
		}
		glBindTexture(GL_TEXTURE_2D, textures[1]);
		glVertexAttribPointer(fog.positionAttribute, 2, GL_FLOAT, false, 0, terrainVerticies.data());
		glVertexAttribPointer(fog.texCoordAttribute, 2, GL_FLOAT, false, 0, terrainTextures.data());
		glVertexAttribPointer(fog.exploredAttribute, 1, GL_FLOAT, false, 0, terrainState.data());
		glDrawArrays(GL_TRIANGLES, 0, terrainVerticies.size() / 2);
		if (mouseState.first == S_BUILD) {
			float partialColor[4] = { 1.0f, 1.0f, 1.0f, 0.5f };
			program.SetColor(partialColor);
			program.SetModelviewMatrix(modelviewMatrix);
			program.SetProjectionMatrix(projectionMatrix);
			std::vector<float> powerVerticies;
			std::vector<float> powerTextures;
			//white tiles show where you can build
			float u = 190.0f / 302.0f;
			float v = 50.0f / 160.0f;
			float w = 50.0f / 302.0f;
			float h = 50.0f / 160.0f;
			for (int y = 0; y < mapHeight; ++y) {
				for (int x = 0; x < mapWidth; ++x) {
					if (powerMap[y][x] == 1) {
						powerVerticies.insert(powerVerticies.end(), {
							(float)x, (float)-y,
							(float)x, (float)-y - 1,
							(float)x + 1, (float)-y,
							(float)x + 1, (float)-y - 1,
							(float)x + 1, (float)-y,
							(float)x, (float)-y - 1
						});
						powerTextures.insert(powerTextures.end(), {
							u, v,
							u, v + h,
							u + w, v,
							u + w, v + h,
							u + w, v,
							u, v + h
						});
					}
				}
			}
			glBindTexture(GL_TEXTURE_2D, textures[11]);
			glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, powerVerticies.data());
			glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, powerTextures.data());
			glDrawArrays(GL_TRIANGLES, 0, powerVerticies.size() / 2);
		}
		
		program.SetColor(fullColor);
	}
	void renderEntities(ShaderProgram& program, Matrix& modelviewMatrix) {
		for (int i = 0; i < buildings.size(); ++i) {
			buildings[i]->render(program, modelviewMatrix);
		}
		for (int i = 0; i < playerUnits.size(); ++i) {
			playerUnits[i]->render(program, modelviewMatrix);
		}
		std::pair<float, float> p;
		for (int i = 0; i < enemyUnits.size(); ++i) {
			p = enemyUnits[i]->getPosition();
			if (fogMap[p.second][p.first].getState() > 0.7f) {
				enemyUnits[i]->render(program, modelviewMatrix);
			}
		}
	}
	void renderCursor(ShaderProgram& program, Matrix& modelviewMatrix) {
		if (mouseState.first == S_BUILD && mousePos.second > -ORTHOH + ORTHOW * 1 / 2) {
			float pos[2] = { floor(tilePos.first) + 0.5f , -floor(tilePos.second) - 0.5f };
			float size[2] = { 0.5f, 0.5 };
			float u, v, w, h;
			GLuint tex = textures[5];
			switch (mouseState.second) {
			case B_TENT:
				size[0] = 1.0f;
				size[1] = 1.0f;
				pos[0] = pos[0] + 0.5f;
				pos[1] = pos[1] - 0.5f;
				u = 390.0f / 633.0f;
				v = 0.0f / 657.0f; 
				w = 75.0f / 633.0f;
				h = 70.0f / 657.0f;
				break;
			case B_TORCH:
				tex = textures[9];
				u = 95.0f / 192.0f;
				v = 155.0f / 256.0f;
				w = 30.0f / 192.0f;
				h = 45.0f / 256.0f;
				break;
			case B_FARM:
				size[0] = 1.0f;
				size[1] = 1.0f;
				pos[0] = pos[0] + 0.5f;
				pos[1] = pos[1] - 0.5f;
				u = 100.0f / 633.0f;
				v = 270.0f / 657.0f;
				w = 100.0f / 633.0f;
				h = 90.0f / 657.0f;
				break;
			case B_TURRET:
				tex = textures[7];
				u = 65.0f / 330.0f;
				v = 0.0f;
				w = 65.0f / 330.0f;
				h = 65.0f / 300.0f;
				break;
			case B_WALL:
				tex = textures[1];
				u = 16.0f / 112.0f;
				v = 3 * 16.0f / 832.0f;
				w = 16.0f / 112.0f;
				h = 16.0f / 832.0f;
				break;
			case B_GATE:
				tex = textures[1];
				u = 6 * 16.0f / 112.0f;
				v = 4 * 16.0f / 832.0f;
				w = 16.0f / 112.0f;
				h = 16.0f / 832.0f;
				break;
			case B_LUMBER:
				size[0] = 1.0f;
				size[1] = 1.0f;
				pos[0] = pos[0] + 0.5f;
				pos[1] = pos[1] - 0.5f;
				u = 305.0f / 633.0f;
				v = 260.0f / 657.0f;
				w = 100.0f / 633.0f;
				h = 105.0f / 657.0f;
				break;
			case B_STONE:
				size[0] = 1.0f;
				size[1] = 1.0f;
				pos[0] = pos[0] + 0.5f;
				pos[1] = pos[1] - 0.5f;
				u = 210.0f / 633.0f;
				v = 553.0f / 657.0f;
				w = 93.0f / 633.0f;
				h = 97.0f / 657.0f;
				break;
			}
			float verticies[12] = {
				-size[0], size[1],
				-size[0], -size[1],
				size[0], size[1],
				size[0], -size[1],
				size[0], size[1],
				-size[0], -size[1]
			};
			float texCoords[12] = {
				u, v,
				u, v + h,
				u + w, v,
				u + w, v + h,
				u + w, v,
				u, v + h
			};
			std::vector<float> tileVerticies;
			std::vector<float> tileTextures;
			if (size[0] < 1.0f) {
				tileVerticies.insert(tileVerticies.end(), {
					-size[0], size[1],
					-size[0], -size[1],
					size[0], size[1],
					size[0], -size[1],
					size[0], size[1],
					-size[0], -size[1] });
				if (-pos[1] > 0 && -pos[1] < mapHeight && pos[0] > 0 && pos[0] < mapWidth &&
					buildingMap[floor(-pos[1])][floor(pos[0])] > 0) {
					u = 120.0f / 302.0f;
					v = 50.0f / 160.0f;
					w = 50.0f / 302.0f;
					h = 50.0f / 160.0f;
				}
				else {
					u = 50.0f / 302.0f;
					v = 50.0f / 160.0f;
					w = 50.0f / 302.0f;
					h = 50.0f / 160.0f;
				}
				tileTextures.insert(tileTextures.end(), {
					u, v, u, v + h, u + w, v,
					u + w, v + h, u + w, v, u, v + h });
			}
			else {
				for (float y = 0; y < 2; ++y) {
					for (float x = -1; x < 1; ++x) {
						tileVerticies.insert(tileVerticies.end(), {
							x , y, x, y - 1.0f, x + 1.0f, y,
							x + 1.0f, y - 1.0f, x + 1.0f, y, x, y - 1.0f
						});
						if (tilePos.first + x > 0 && tilePos.first + x < mapHeight &&
							tilePos.second + y > 0 && tilePos.second + y < mapWidth &&
							buildingMap[ceil(tilePos.second - y)][ceil(tilePos.first + x)] > 0 ||
							powerMap[ceil(tilePos.second - y)][ceil(tilePos.first + x)] == 0) {
							u = 120.0f / 302.0f;
							v = 50.0f / 160.0f;
							w = 50.0f / 302.0f;
							h = 50.0f / 160.0f;
						}
						else {
							u = 50.0f / 302.0f;
							v = 50.0f / 160.0f;
							w = 50.0f / 302.0f;
							h = 50.0f / 160.0f;
						}
						tileTextures.insert(tileTextures.end(), {
							u, v, u, v + h, u + w, v,
							u + w, v + h, u + w, v, u, v + h });
					}
				}
			}
			glBindTexture(GL_TEXTURE_2D, textures[11]);
			modelviewMatrix.Identity();
			modelviewMatrix.Translate(pos[0], pos[1], 0.0f);
			program.SetModelviewMatrix(modelviewMatrix);
			float partialColor[4] = { 1.0f, 1.0f, 1.0f, 0.8f };
			float fullColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
			program.SetColor(partialColor);
			glBindTexture(GL_TEXTURE_2D, tex);
			glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, verticies);
			glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			partialColor[3] = 0.4f;
			program.SetColor(partialColor);
			glBindTexture(GL_TEXTURE_2D, textures[11]);
			glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, tileVerticies.data());
			glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, tileTextures.data());
			glDrawArrays(GL_TRIANGLES, 0, tileVerticies.size() / 2);
			program.SetColor(fullColor);
		}
	}
	void updateCursor(float xCoord, float yCoord) {
		if (transition) { return; }
		mousePos.first = (xCoord / 1200.0f*ORTHOW * 2) - ORTHOW; //orthographic coordinates
		mousePos.second = ((800.0f - yCoord) / 800.0f *ORTHOH * 2) - ORTHOH;
		tilePos.first = mousePos.first + cameraPos[0]; //tile coordinates
		tilePos.second = -mousePos.second - cameraPos[1];

	}
private:
	struct gTime {
		bool update(float elapsed) {
			accumulator += elapsed;
			if (accumulator > 3.0f) {
				++hour;
				accumulator -= 5.0f;
				if (hour > 23) {
					++day;
					hour = 0;
				}
				if (hour == 0 || hour == 8 || hour == 16) {
					return true;
				}
			}
			return false;
		}
		float accumulator = 0;
		float hour;
		float day;
	};
	struct fowVertex { //fog of war tile
		float getState() {
			if (inVision == 1) { //in vision currently
				return 0.8f;
			}
			else if (explored == 1) { //has been explored;
				return 0.3f;
			}
			else {
				return 0.0f; //not explored at all
			}
		}
		int explored = 0;
		int inVision = 0;
	};
	struct node {
		node(float x, float y, int cost) :x(x), y(y), cost(cost) {}
		bool operator < (const node* other) const {
			return pathCost + distance < other->pathCost + other->distance;
		}
		bool operator > (const node* other) const {
			return pathCost + distance > other->pathCost + other->distance;
		}
		bool operator == (const node* other) const {
			return x == other->x && y == other->y;
		}
		float x;
		float y;
		float distance = 0.0f;
		node* previous = nullptr;
		std::vector<node*> neighbors;
		int cost;
		int pathCost = 0;
	};
	struct compareNode {
		bool operator() (const node* lhs, const node* rhs) const {
			return lhs->operator<(rhs);
		}
	};
	struct compareEntity {
		bool operator() (const entity* lhs, const entity* rhs) {
			return lhs->getPosition().second < rhs->getPosition().first;
		}
	};
	//map
	int mapWidth = WIDTH;
	int mapHeight = HEIGHT;
	int mountain = 10;
	int lake = 9;
	int forest = 7;
	std::vector < std::vector<int> > map;
	std::vector < std::vector<int> > terrainMap;
	std::vector < std::vector<int> > buildingMap;
	std::vector < std::vector<int> > powerMap;
	std::vector < std::vector<fowVertex> > fogMap;
	std::vector < std::vector<node*> > navigationMap;
	std::vector < std::vector<node*> > playerUnitMap;
	std::vector <float> mapVerticies;
	std::vector <float> mapTextures;
	std::vector <float> terrainVerticies;
	std::vector <float> terrainTextures;
	float tileSize = 1.0f;

	//misc
	bool running = false;
	gTime gameTime;
	std::vector<entity*> selected;
	std::vector<int> selectedPos;
	std::pair<commandState, buttonType> mouseState;
	std::vector<GLuint> textures;
	soundAssets sounds;
	std::vector<float> stats;

	UI* gameUI = nullptr;
	std::pair<float, float> select1;
	float cameraPos[3] = { (float) mapWidth / 2.0f, (float) -mapHeight / 2.0f, 0.0f };//default to center
	bool transition = true;
	int transitionState = 200;

	//lists
	std::vector<entity*> playerUnits;
	std::vector<entity*> buildings;
	std::vector<entity*> enemyUnits;
	std::vector<entity*> horde;
	std::pair<float, float> tilePos;
	std::pair<float, float> mousePos;
	std::vector<int> resources;
	std::vector<int> resourceProduction;
};
class Gamemenu {
	public:
		Gamemenu(std::vector<GLuint> texture, mode type, soundAssets& s):textures(texture), type(type), sounds(s) {
			verticies.insert(verticies.end(), {
				-ORTHOW, ORTHOH,
				-ORTHOW, -ORTHOH, 
				ORTHOW, ORTHOH,
				ORTHOW, -ORTHOH,
				ORTHOW, ORTHOH,
				-ORTHOW, -ORTHOH
			});
			texCoords.insert(texCoords.end(), {
				0.0f, 0.0f, 
				0.0f, 1.0f, 
				1.0f, 0.0f, 
				1.0f, 1.0f, 
				1.0f, 0.0f,
				0.0f, 1.0f
			});
			if (type == M_MENU) {
				for (int i = 0; i < 3; ++i) {
					buttons.push_back(new menuButton(i, textures));
				}
			}
			else {
				for (int i = 3; i < 8; ++i) {
					buttons.push_back(new menuButton(i, textures));
				}
			}
		}
		int select(float x, float y, bool s){
			if (transition && animationState > 200) {
				return 1;
			}
			for (menuButton* b : buttons) {
				b->collision(x, y);
				if (s && b->selected) {
					if (b->type == 1) {
						//play sound;
						transition = true;
					}
					else {
						return b->type;
					}
				}
			}
			return 0;
		}
		void setStats(std::vector<float> stat) {
			Mix_HaltMusic();
			Mix_VolumeMusic(30);
			Mix_PlayMusic(sounds.musics[0], -1);
			for (int i = 0; i < stat.size(); ++i) {
				buttons[i]->setStats(stat[i]);
			}
		}
		bool change() {
			if (transition && animationState > 200) {
				transition = false;
				animationState = 0;
				return true;
			}
			return false;
		}
		void render(ShaderProgram& program, Matrix& modelviewMatrix, Matrix& projectionMatrix) {
			float color[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
			glBindTexture(GL_TEXTURE_2D, textures[0]);
			modelviewMatrix.Identity();
			program.SetModelviewMatrix(modelviewMatrix);
			projectionMatrix.Identity();
			projectionMatrix.SetOrthoProjection(-ORTHOW, ORTHOW, -ORTHOH, ORTHOH, -1.0f, 1.0f);
			program.SetProjectionMatrix(projectionMatrix);
			program.SetColor(color);
			glEnableVertexAttribArray(program.positionAttribute);
			glEnableVertexAttribArray(program.texCoordAttribute);
			glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, verticies.data());
			glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords.data());
			glDrawArrays(GL_TRIANGLES, 0, verticies.size() / 2);

			for (int i = 0; i < buttons.size(); ++i) {
				buttons[i]->render(program, modelviewMatrix);
			}
			if (transition) {
				float box[12] = { -ORTHOW, ORTHOH, -ORTHOW, -ORTHOH, ORTHOW, ORTHOH, ORTHOW, -ORTHOH, ORTHOW, ORTHOH, -ORTHOW, -ORTHOH };
				float tex[12] = { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f };
				++animationState;
				float a = mapValue(animationState, 0, 180, 0, 1);
				int volume = mapValue(animationState, 0, 180, 0, 30);
				Mix_VolumeMusic(30 - volume);
				float black[4] = { 0.0f, 0.0f, 0.0f, a };
				program.SetColor(black);
				glBindTexture(GL_TEXTURE_2D, textures[2]);
				glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, box);
				glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, tex);
				glDrawArrays(GL_TRIANGLES, 0, 6);
			}
		}
	private:
		struct menuButton {
			menuButton(int type, std::vector<GLuint> tex):type(type) {
				textures = tex;
				selected = false;
				ySize = 1.0f;
				switch(type){
				case 0:
					xPos = -ORTHOW /3;
					yPos = ORTHOH * 4/5;
					text = "THE DEAD MARCH!";
					clickable = false;
					xSize = 15.0f;
					break;
				case 1:
					xPos = ORTHOW / 3;
					yPos = ORTHOH / 2;
					text = "SURVIVE";
					clickable = true;
					xSize = 6.0f;
					break;
				case 2:
					xPos = ORTHOW / 3;
					yPos = ORTHOH / 3;
					text = "QUIT";
					clickable = true;
					xSize = 4.0f;
					break;
				case 3:
					xPos = -ORTHOW * 3/ 4;
					yPos = 0.0f;
					text = "GOLD EARNED";
					xSize = 11.0f;
					clickable = false;
					break;
				case 4:
					xPos = -ORTHOW * 3 / 4;
					yPos = -ORTHOH / 2;
					text = "ENEMIES KILLED";
					xSize = 14.0f;
					clickable = false;
					break;
				case 5:
					xPos = ORTHOW / 4;
					yPos = -ORTHOH / 2;
					text = "SOLDERS KILLED";
					xSize = 14.0f;
					clickable = false;
					break;
				case 6:
					xPos = ORTHOW / 4;
					yPos = 0.0f;
					text = "BUILDINGS MADE";
					xSize = 14.0f;
					clickable = false;
					break;
				case 7:
					xPos = -ORTHOW / 5;
					yPos = -ORTHOH * 3/4;
					text = "RETURN TO MENU";
					xSize = 14.0f;
					clickable = true;
					break;
				}
			}
			bool collision(float x, float y) {
				if (x > xPos && x < xPos + xSize && y > yPos - ySize && y < yPos) {
					if (clickable) { selected = true; }
					return true;
				}
				selected = false;
				return false;
			}
			void setVerticies() {
				std::string printText;
				if (selected) {
					printText = "{" + text + "}";
				}
				else {
					printText = text;
				}
				float xTexSize = 15.0f / 241.0f;
				float yTexSize = 15.0f / 103.0f;
				float font_size;
				if (type < 3) {
					font_size = 1.0f;
				}
				else {
					font_size = 0.5f;
				}
				float spacing = 0.0f;
				float texture_x ,texture_y;
				int spriteIndex, column;
				std::vector<float> vertexData;
				std::vector<float> textureData;
				for (int i = 0; i < printText.size(); i++) {
					spriteIndex = ((int)printText[i]) - 32;
					texture_x = (float)(spriteIndex % 16) / 16.0f;
					column = (float)(spriteIndex / 16);
					switch (column) {
					case 0:
						texture_y = 0.0f;
						break;
					case 1:
						texture_y = 18.0f / 103.0f;
						break;
					case 2:
						texture_y = 35.0f / 103.0f;
						break;
					case 3:
						texture_y = 52.0f / 103.0f;
						break;
					case 4:
						texture_y = 69.0f / 103.0f;
						break;
					case 5:
						texture_y = 86.0f / 103.0f;
						break;
					}
					vertexData.insert(vertexData.end(), {
						((font_size + spacing) * i) + (xPos), yPos,
						((font_size + spacing) * i) + (xPos), (yPos - font_size) ,
						((font_size + spacing) * i) + ((xPos + font_size)), yPos ,
						((font_size + spacing) * i) + ((xPos + font_size)), (yPos - font_size) ,
						((font_size + spacing) * i) + ((xPos + font_size)),yPos ,
						((font_size + spacing) * i) + (xPos), (yPos - font_size) ,
					});
					textureData.insert(textureData.end(), {
						texture_x, texture_y,
						texture_x, texture_y + yTexSize,
						texture_x + xTexSize, texture_y,
						texture_x + xTexSize, texture_y + yTexSize,
						texture_x + xTexSize, texture_y,
						texture_x, texture_y + yTexSize,
					});
				}
				verticies = vertexData;
				texCoords = textureData;
			}
			void setStats(float i) {
				std::stringstream s;
				s << i << text;
				text = s.str();
			}
			void render(ShaderProgram& program, Matrix& modelviewMatrix) {
				float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
				program.SetColor(color);
				setVerticies();
				glBindTexture(GL_TEXTURE_2D, textures[1]);
				glEnableVertexAttribArray(program.positionAttribute);
				glEnableVertexAttribArray(program.texCoordAttribute);
				glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, 0, false, verticies.data());
				glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, 0, false, texCoords.data());
				glDrawArrays(GL_TRIANGLES, 0, verticies.size() / 2);
			}
			std::string text;
			int type;
			float xPos;
			float yPos;
			float xSize;
			float ySize;
			bool clickable;
			std::vector<float> verticies;
			std::vector<float> texCoords;
			std::vector<GLuint> textures;
			bool selected;
		};
		mode type;
		std::vector<float> verticies;
		std::vector<float> texCoords;
		std::vector<GLuint> textures;
		std::vector<menuButton*> buttons;
		bool transition =false;
		int animationState = 0;
		soundAssets sounds;
	};
void processInput(mode& state, Gamestate& game, Gamemenu& menu, Gamemenu& end, SDL_Event& event, bool& done) {
		switch (state) {
		case M_MENU:
			if (menu.change()) {
				state = M_GAME;
			}
			//mouse controlls
			if (event.type == SDL_MOUSEMOTION) {
				float inputx = ((float)event.motion.x / 1200.0f*2*ORTHOW) - ORTHOW;
				float inputy = (((float)800.0f - event.motion.y) / 800.0f*2*ORTHOH) - ORTHOH;
				int i = menu.select(inputx, inputy, false);
			}
			if (event.type == SDL_MOUSEBUTTONUP) {
				float inputx = ((float)event.button.x / 1200.0f*2*ORTHOW) - ORTHOW;
				float inputy = (((float)800.0f - event.button.y) / 800.0f*2*ORTHOH) - ORTHOH;
				int i = menu.select(inputx, inputy, true);
				if (i == 2) {
					done = true;
				}
			}
			break;
		case M_GAME:
			if (event.type == SDL_MOUSEMOTION) {
				game.updateCursor(event.motion.x, event.motion.y);
			}
			if (event.type == SDL_MOUSEBUTTONDOWN) {
				if (event.button.button == SDL_BUTTON_LEFT) {
					game.mouseInput(true, false);
				}
				else {
					game.mouseInput(true, true);
				}
			}
			if (event.type == SDL_MOUSEBUTTONUP) {
				if (event.button.button == SDL_BUTTON_LEFT) {
					game.mouseInput(false, false);
				}
				else {
					game.mouseInput(false, true);
				}
			}
			if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
				state = M_END;
				end.setStats(game.getStatistics());
			}
			if (event.key.keysym.scancode == SDL_SCANCODE_1) {}
			if (event.key.keysym.scancode == SDL_SCANCODE_2) {}
			if (event.key.keysym.scancode == SDL_SCANCODE_3) {}
			if (event.key.keysym.scancode == SDL_SCANCODE_4) {}
			if (event.key.keysym.scancode == SDL_SCANCODE_5) {}
			if (event.key.keysym.scancode == SDL_SCANCODE_6) {}
			if (event.key.keysym.scancode == SDL_SCANCODE_7) {}
			if (event.key.keysym.scancode == SDL_SCANCODE_8) {}
			if (event.key.keysym.scancode == SDL_SCANCODE_9) {}
			break;
		case M_END:
			if (event.type == SDL_MOUSEMOTION) {
				float inputx = ((float)event.motion.x / 1200.0f * 2 * ORTHOW) - ORTHOW;
				float inputy = (((float)800.0f - event.motion.y) / 800.0f * 2 * ORTHOH) - ORTHOH;
				end.select(inputx, inputy, false);
			}
			if (event.type == SDL_MOUSEBUTTONUP) {
				float inputx = ((float)event.motion.x / 1200.0f * 2 * ORTHOW) - ORTHOW;
				float inputy = (((float)800.0f - event.motion.y) / 800.0f * 2 * ORTHOH) - ORTHOH;
				if (end.select(inputx, inputy, true) == 7) {
					state = M_MENU;
				}
			}
			break;
		}
	}
void render(ShaderProgram& program, ShaderProgram& fog, Matrix& modelviewMatrix, Matrix& projectionMatrix, 
		mode& gameMode, Gamemenu& menu, Gamestate& game, Gamemenu& end) {
		switch (gameMode) {
		case M_MENU:
			menu.render(program, modelviewMatrix, projectionMatrix);
			break;
		case M_GAME:
			game.render(program, fog, modelviewMatrix, projectionMatrix);
			break;
		case M_END:
			end.render(program, modelviewMatrix, projectionMatrix);
			break;
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

SDL_Window* displayWindow;
int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1200, 800, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif

	glViewport(0, 0, 1200, 800);
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	ShaderProgram fogOfWar(RESOURCE_FOLDER"vertex_fogOfWar.glsl", RESOURCE_FOLDER"fragment_fogOfWar.glsl");
	Matrix projectionMatrix;
	projectionMatrix.SetOrthoProjection(-ORTHOW, ORTHOW, -ORTHOH, ORTHOH, -1.0f, 1.0f);
	program.SetProjectionMatrix(projectionMatrix);
	Matrix modelviewMatrix;
	program.SetModelviewMatrix(modelviewMatrix);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//art assets
	std::vector<GLuint> textures;
	GLuint font = LoadTexture("art_font.png");
	GLuint terrain = LoadTexture("art_medievaltiles.png");
	GLuint ground = LoadTexture("art_dirtgrass.png");
	GLuint enemyWalk = LoadTexture("art_SkeletonWalk.png");
	GLuint enemyIdle = LoadTexture("art_SkeletonIdle.png");
	GLuint enemyHit = LoadTexture("art_SkeletonHit.png");
	GLuint enemyAttack = LoadTexture("art_SkeletonAttack.png");
	GLuint enemyDead = LoadTexture("art_SkeletonDead.png");
	GLuint uiBackground = LoadTexture("art_uiBackground.png");
	GLuint icons = LoadTexture("art_icons.png");
	GLuint buildings = LoadTexture("art_wbuilding.png");
	GLuint archer = LoadTexture("art_warcher.png");
	GLuint healthbars = LoadTexture("art_health.png");
	GLuint turret = LoadTexture("art_wballista.png");
	GLuint torch = LoadTexture("art_torch.png");
	GLuint time = LoadTexture("art_time.png");
	GLuint tiles = LoadTexture("art_tiles.png");
	GLuint box = LoadTexture("art_box.png");
	textures.insert(textures.end(), { ground, terrain, uiBackground, font, icons,
		buildings, archer, turret, healthbars, torch, time, tiles,
		enemyIdle, enemyWalk, enemyAttack, enemyDead, box});
	GLuint mainMenu = LoadTexture("art_background.png");
	GLuint menuFont = LoadTexture("art_font2.png");
	std::vector<GLuint> menuTextures;
	menuTextures.insert(menuTextures.end(), { mainMenu, menuFont, box});
	GLuint endMenu = LoadTexture("art_background1.png");
	std::vector<GLuint> endTextures;
	endTextures.insert(endTextures.end(), { endMenu, menuFont });

	//sound assets
	soundAssets sounds;
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 4, 4096);
	Mix_Chunk* ballista, *bow, *step, *place, *slash, *destroy, *eDeath, *pDeath;
	ballista = Mix_LoadWAV("sounds_ballista.wav");
	bow = Mix_LoadWAV("sounds_bow.wav");
	step = Mix_LoadWAV("sounds_step.wav");
	place = Mix_LoadWAV("sounds_place.wav");
	slash = Mix_LoadWAV("sounds_slash1.wav");
	destroy = Mix_LoadWAV("sounds_break1.wav");
	eDeath = Mix_LoadWAV("sounds_bones.wav");
	pDeath = Mix_LoadWAV("sounds_death.wav");
	sounds.sounds.insert(sounds.sounds.end(), {
		ballista, bow, step, place, slash, destroy, eDeath, pDeath
	});
	Mix_Music* menuMusic, *gameMusic;
	menuMusic = Mix_LoadMUS("music_action.ogg");
	gameMusic = Mix_LoadMUS("music_suspense.ogg");
	sounds.musics.insert(sounds.musics.end(), {
		menuMusic, gameMusic
	});

	//gamestates
	Gamemenu menu(menuTextures, M_MENU, sounds);
	Gamemenu end(endTextures, M_END, sounds);
	UI gameUI (textures);
	Gamestate game(textures, &gameUI, sounds);
	mode gameMode = M_MENU;

	//time
	float elapsed;
	float current = 0.0f;
	float ticks;
	float frames = 1.0f / 60.0f;
	float accumulator = 0.0f;
	Mix_VolumeMusic(30);
	Mix_PlayMusic(sounds.musics[0], -1);

	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
			processInput(gameMode, game, menu, end, event, done);
		}
	
		ticks = SDL_GetTicks() *0.001f;
		elapsed = ticks - current;
		current = ticks;
		elapsed += accumulator;
		if (elapsed < frames) {
			accumulator = elapsed;
			continue;
		}
		while (elapsed > frames) {
			elapsed -= frames;
			if (gameMode == M_GAME) {
				game.update(frames);
				game.scrolling(program, projectionMatrix);
				if (game.checkGameOver()) {
					end.setStats(game.getStatistics());
				}
			}
		}
		accumulator = elapsed;
		glClear(GL_COLOR_BUFFER_BIT);
		render(program, fogOfWar, modelviewMatrix, projectionMatrix, gameMode, menu, game, end);
		SDL_GL_SwapWindow(displayWindow);
	}
	sounds.quit();
	SDL_Quit();
	return 0;
}


#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "Matrix.h"
#include "ShaderProgram.h"
#include <vector>

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

static const int WIDTH = 100;
static const int HEIGHT = 100;
enum terrainType {T_DIRT, T_GRASS, T_MOUNTAIN, T_TREE, T_WATER, T_CLEARING};
enum quadrant {Q_TOPRIGHT, Q_TOPLEFT, Q_BOTTOMLEFT, Q_BOTTOMRIGHT};
struct feature {
	std::vector<std::pair<int, int>> points;
	terrainType type;
	int size[2];
	int amount;
};

class Gamestate {
public:
	Gamestate() {
		map = std::vector <std::vector<int> > (mapHeight, std::vector<int>(mapWidth, 0));
		terrainMap = std::vector <std::vector<int> >(mapHeight, std::vector<int>(mapWidth, 0));
		generateTiles(map); //grass vs. dirt
		generateTiles(terrainMap); //generate terrain features - lake, mountain, forests
		connectTerrain();
		generateFeatures();
		generateVerticies();
	}
	//cellular automata
	void generateTiles(std::vector<std::vector<int>> mapType) {
		//seed the map with live and dead
		for (size_t x = 0; x < mapWidth; ++x) {
			for (size_t y = 0; y < mapHeight; ++y) {
				mapType[y][x] = rand() % 2;
			}
		}
		int count = 3; //run the simulation three times
		std::vector<std::vector<int>> tempMap (mapHeight, std::vector<int>(mapWidth, 0));
		while (count > 0) {
			for (size_t x = 0; x < mapWidth; ++x) {
				for (size_t y = 0; y < mapHeight; ++y) {
					int neighbors = getNeighbors(x, y);
					if (mapType[y][x] == 0 && neighbors > 3){ //cell is dead and will be revived
						tempMap[y][x] = 1;
					}
					else if (mapType[y][x] == 1 && (neighbors == 2 || neighbors == 3)) { // cell will stay alive
						tempMap[y][x] = 1;
					}
					else {	//cell will be dead
						tempMap[y][x] = 0;
					}
				}
			}
			mapType = tempMap;
		}
	}
	int getNeighbors(int x, int y) {
		int count = 0;
		for (int i = -1; i < 2; ++i) {
			for (int j = -1; j < 2; ++j) {
				if (x + i > -1 && x + i < mapWidth && y + j  > -1 && y + j < mapHeight) { //check within boundaries
					if (map[y][x] == 1) {
						++count;
					}
				}
			}
		}
		return count;
	}
	void connectTerrain() {
		auto copy(terrainMap);
		int count = 0;
		int amount = 0;
		std::vector <std::vector<std::pair<int, int>> >area;
		for (int y = 0; y < mapWidth; ++y) { //get areas
			for (int x = 0; x < mapWidth; ++x) {
				if (copy[y][x] == 0) {
					++count;
					area.push_back(std::vector<std::pair<int, int>>());
					fill(copy, x, y, 0, count, amount, area[count - 1]);
				}
			}
		}
		//fill in small sections
		for (int i = 0; i < area.size(); ++i) {
			if (area[i].size() < 20) {
				for (std::pair<int, int> tile : area[i]) {
					terrainMap[tile.second][tile.first] = 1;
					area[i] = area[area.size() - 1];
					area.pop_back();
				}
			}
		}
		for (int i = 0; i < area.size(); ++i) {
			for (int j = 0; j < area.size(); ++j) {
				if (i != j) {
					auto points = getMinimumDistance(area[i], area[j]);
					int xdistance = (points.first.first - points.second.first);
					int ydistance = (points.first.second - points.second.second);
					int steps;
					if (xdistance > ydistance) {
						steps = xdistance;
					}
					else {
						steps = ydistance;
					}
					float xStep = xdistance/steps;
					float yStep = ydistance/steps;
					int b;
					int x;
					int y;
					for (int a = 0; a < 2; ++a) {
						b = 0;
						while (b < steps) {
							++b;
							x = floor(points.first.first + a + xStep*b);
							y = floor(points.first.second + a + yStep*b);
							terrainMap[y][x] = 0;
						}
					}
				}
			}
		}
	}
	std::pair<std::pair<int,int>,std::pair<int,int>> getMinimumDistance(std::vector<std::pair<int, int>> a1, std::vector<std::pair<int, int>> a2) {
		float minimum = mapWidth+mapHeight;
		float current;
		std::pair<int, int> p1;
		std::pair<int, int> p2;
		for (int i = 0; i < a1.size(); ++i) {
			for (int j = 0; j < a2.size(); ++j) {
				current = distance(a1[i], a2[j]);
				if (current < minimum) {
					minimum = current;
					p1 = a1[i];
					p2 = a2[j];
				}
			}
		}
		return std::pair<std::pair<int, int>, std::pair<int, int>>(p1, p2);
	}
	float distance(std::pair<int, int> p1, std::pair<int, int> p2) {
		return sqrt(pow(abs(p1.first - p2.first), 2) - pow(abs(p1.second - p2.second), 2));
	}
	void generateFeatures() {
		std::vector <feature> featureMap;
		auto copy(terrainMap);
		int count = 0;
		int amount = 0;
		std::vector <std::vector<std::pair<int, int>> >area;
		for (int y = 0; y < mapWidth; ++y) {// finds number of distinct terrain features
			for (int x = 0; x < mapWidth; ++x) {
				if (copy[y][x] == 1) {
					++count;
					area.push_back(std::vector<std::pair<int, int>>());
					fill(copy, x, y, 1, count, amount, area[count-1]);
				}
			}
		}
		//divide terrain into features if they are too large
		for (auto featureSize : area) {
			if (featureSize.size() < 30) {

			}
		}
	}
	int fill(std::vector<std::vector<int>>& tempMap, int x, int y, int tileType, int replacement, int& amount, std::vector<std::pair<int,int>>& area) {
		if (tempMap[y][x] != tileType) {
			return;
		}
		tempMap[y][x] == replacement;
		std::pair<int, int> point(x, y);
		area.push_back(point);
		++amount;
		if (x - 1 > 0) {
			fill(tempMap, x - 1, y, tileType, replacement, amount, area);
		}
		if (x + 1> mapWidth - 1) {
			fill(tempMap, x + 1, y, tileType, replacement, amount, area);
		}
		if (y - 1 > 0) {
			fill(tempMap, x, y - 1, tileType, replacement, amount, area);
		}
		if (y + 1> mapWidth - 1) {
			fill(tempMap, x, y + 1, tileType, replacement, amount, area);
		}
		return amount;
	}
	quadrant getQuadrant(std::vector<std::pair<int, int>> area) {
		int topLeft;
		int topRight;
		int bottomLeft;
		int bottomRight;
		for (auto point : area) {
			if (point.first < mapWidth/2) {
				if (point.second < mapHeight / 2) {
					++topLeft;
				}
				else {
					++bottomLeft;
				}
			}
			else {
				if (point.second < mapHeight / 2) {
					++topRight;
				}
				else {
					++bottomRight;
				}
			}
		}
		//return maximum
	}
	//generate enemies;
	void generateEnemies() {

	}
	//generate verticies for drawing - static
	void generateVerticies() {
		for (int x = 0; x < mapWidth; ++x) {
			for (int y = 0; y < mapHeight; ++y) {
				mapVerticies.insert(mapVerticies.end(), {
					tileSize * x, -tileSize * y,
					tileSize * x, (-tileSize * y) - tileSize,
					(tileSize * x) + tileSize, (-tileSize * y) - tileSize,
					tileSize * x, -tileSize * y,
					(tileSize * x) + tileSize, (-tileSize * y) - tileSize,
					(tileSize * x) + tileSize, -tileSize * y
				});
				//mapTextures
			}
		}
	}
	//rendertile map;
	void renderMap(ShaderProgram* program){
		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, mapVerticies.data());
		glEnableVertexAttribArray(program->positionAttribute);
		glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, mapVerticies.data());
		glEnableVertexAttribArray(program->texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, mapVerticies.size()/2);
		glDisableVertexAttribArray(program->positionAttribute);
	}
	class Gamemenu {
	public:

	private:

	};
private:
	static const int mapWidth = WIDTH;
	static const int mapHeight = HEIGHT;
	std::vector < std::vector<int> > map;
	std::vector <std::vector<int> > terrainMap;
	std::vector <float> mapVerticies;
	std::vector <float> mapTextures;
	float tileSize = 16.0f;
};
SDL_Window* displayWindow;
int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif

	glViewport(0, 0, 640, 360);
	ShaderProgram program(RESOURCE_FOLDER"Vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
	Matrix projectionMatrix;
	projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
	Matrix modelviewMatrix;

	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(program.programID);
		program.SetModelviewMatrix(modelviewMatrix);
		program.SetProjectionMatrix(projectionMatrix);

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}

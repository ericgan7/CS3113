//Eric Gan
//Basic Pong implementation
//Introduction to input capture and collision detection.

#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "Matrix.h"
#include "ShaderProgram.h"

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

struct paddle {
	paddle(float pos[]){
		for (int i = 0; i < 3; ++i) {
			position[i] = pos[i];
		}
	}
	float paddleVerticies[12] = { -0.1f, 0.5f, -0.1f, -0.5f, 0.1f, 0.5f, 0.1f, -0.5f, -0.1f, -0.5f, 0.1f, 0.5f };
	float position[3];
	float yVelocity = 0.0f;
};

struct ball{
	ball() {
		yVelocity = (float) (rand() % 40)/10.f;
	}
	float ballVerticies[12] = { -0.1f, 0.1f, -0.1f, -0.1f, 0.1f, 0.1f, 0.1f, -0.1f, -0.1f, -0.1f, 0.1f, 0.1f };
	float bPosition[3] = { 0.0f, 0.0f, 1.0f };
	float xVelocity = 5.5f;
	float yVelocity;
};

struct background {
	float walls[36] = { -10.0f, 6.0f, -10.0f, 5.9f, 10.0f, 6.0f, 10.0f, 5.9f, 10.0f, 6.0f, -10.0f, 5.9f, //top wall
		-10.0f, -6.0f, -10.0f, -5.9f, 10.0f, -6.0f, 10.0f, -5.9f, 10.0f, -6.0f, -10.0f, -5.9f,			 //bottom wal
		-0.05f, 6.0f, -0.05f, -6.0f, 0.05f, 6.0f, 0.05f, -6.0f, -0.05f, -6.0f, 0.05f, 6.0f };			 //divider
};

void drawPaddle(ShaderProgram& program, Matrix& modelviewMatrix, paddle& info) {
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, info.paddleVerticies);
	glEnableVertexAttribArray(program.positionAttribute);

	modelviewMatrix.Identity();
	modelviewMatrix.Translate(info.position[0], info.position[1], info.position[2]);
	program.SetModelviewMatrix(modelviewMatrix);

	glDrawArrays(GL_TRIANGLES, 0, 6);
}
void drawBall(ShaderProgram& program, Matrix& modelviewMatrix, ball& info) {
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, info.ballVerticies);
	glEnableVertexAttribArray(program.positionAttribute);

	modelviewMatrix.Identity();
	modelviewMatrix.Translate(info.bPosition[0], info.bPosition[1], info.bPosition[2]);
	program.SetModelviewMatrix(modelviewMatrix);

	glDrawArrays(GL_TRIANGLES, 0, 6);
}
void drawBackground(ShaderProgram& program, Matrix& modelviewMatrix, background& back, bool clear) {
	if (clear) {
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, back.walls);
		glEnableVertexAttribArray(program.positionAttribute);
		modelviewMatrix.Identity();
		program.SetModelviewMatrix(modelviewMatrix);
		glDrawArrays(GL_TRIANGLES, 0, 18);
	}
}

void paddleCollision(paddle& p1, paddle& p2, ball& b, float elapsed) {
	//walls
	if (fabs(p1.position[1] + p1.yVelocity*elapsed) > 5.4f) {
		if (p1.yVelocity > 0) {
			p1.position[1] = 5.4f;
		}
		else { p1.position[1] = -5.4f; }
		p1.yVelocity = 0.0f;
	}
	if (fabs(p2.position[1] + p2.yVelocity*elapsed) > 5.4f) {
		if (p2.yVelocity > 0) {
			p2.position[1] = 5.4f;
		}
		else { p2.position[1] = -5.4f; }
		p2.yVelocity = 0.0f;
	}
}

bool boxboxCollision(paddle& p, ball& b) {
	//left, right, top, bottom
	float pBounds[] = { p.position[0] - 0.1f, p.position[0] + 0.1f, p.position[1] + 0.5f, p.position[1] - 0.5f };
	float bBounds[] = { b.bPosition[0] - 0.1f, b.bPosition[0] + 0.1f, b.bPosition[1] + 0.1f, b.bPosition[1] - 0.1f };
	return !(bBounds[3] > pBounds[2] || bBounds[2] < pBounds[3] || bBounds[0] > pBounds[1] || bBounds[1] < pBounds[0]);
}

void ballCollision(paddle& p1, paddle& p2, ball& b, float elapsed) {
	//wall
	if (fabs(b.bPosition[1] + b.yVelocity*elapsed) > 5.9f) {
		b.yVelocity = b.yVelocity *-1.0f;
	}
	//paddle & ball
	if (b.bPosition[0] > 0.0f) { //right side
		if (boxboxCollision(p2, b)) {
			b.xVelocity = b.xVelocity *-1.0f;
		}
	}
	else { //left side
		if (boxboxCollision(p1, b)) {
			b.xVelocity = b.xVelocity * -1.0f;
		}
	}
}

bool setMovement(paddle& p1, paddle& p2, ball& b, float elapsed) {
	if (elapsed > 1.0f / 60.0f) {
		p1.position[1] += p1.yVelocity*elapsed;
		p2.position[1] += p2.yVelocity*elapsed;
		b.bPosition[0] += b.xVelocity*elapsed;
		b.bPosition[1] += b.yVelocity*elapsed;
		return true;
	}
	return false;
}

int checkGoal(ball& b) {
	if (fabs(b.bPosition[0]) > 10.0f) {
		if (b.bPosition[0] > 0.0f) {
			return -1;
		}
		return 1;
	}
	return 0;
}

void reset(paddle& p1, paddle& p2, ball& b) {
	p1.position[1] = 0.0f;
	p2.position[1] = 0.0f;
	b.bPosition[0] = 0.0f;
	b.bPosition[1] = 0.0f;
	b.xVelocity = 0.0f;
	b.yVelocity = 0.0f;
}


int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1000, 600, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif
	//setup
	glViewport(0, 0, 1000, 600);
	ShaderProgram program(RESOURCE_FOLDER"Vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
	Matrix projectionMatrix;
	projectionMatrix.SetOrthoProjection(-10.0f, 10.0f, -6.0f, 6.0f, -1.0f, 1.0f);
	Matrix modelviewMatrix;
	const Uint8* keyboard;
	bool wait = false;
	srand(SDL_GetTicks()%100);

	//Object info
	float p1[] = {-9.7f, 0.0f, 1.0f };
	float p2[] = { 9.7f, 0.0f, 1.0f };
	paddle playerOne(p1);
	paddle playerTwo(p2);
	ball ball;
	background back;
	float time;
	int goal;
	float direction;

	//time
	float ticks = 0.0f;
	float elapsed;
	float lastFrameTicks = 0.0f;

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

		goal = checkGoal(ball);
		if (goal != 0) {
			reset(playerOne, playerTwo, ball);
			wait = true;
			time = (float)SDL_GetTicks();
			direction = (float)goal;
		}

		keyboard = SDL_GetKeyboardState(NULL);
		if (keyboard[SDL_SCANCODE_S]) {
			playerOne.yVelocity = -5.0f;
		}
		else if (keyboard[SDL_SCANCODE_W]) {
			playerOne.yVelocity = 5.0f;
		}
		else { playerOne.yVelocity = 0.0f; }
		if (keyboard[SDL_SCANCODE_UP]) {
			playerTwo.yVelocity = 5.0f;
		}
		else if (keyboard[SDL_SCANCODE_DOWN]) {
			playerTwo.yVelocity = -5.0f;
		}
		else { playerTwo.yVelocity = 0.0f; }

		ticks = (float)SDL_GetTicks() * 0.001f;
		elapsed = ticks - lastFrameTicks;

		paddleCollision(playerOne, playerTwo, ball, elapsed);
		ballCollision(playerOne, playerTwo, ball, elapsed);
		if (setMovement(playerOne, playerTwo, ball, elapsed)) {
			lastFrameTicks = ticks;
		}

		drawPaddle(program, modelviewMatrix, playerOne);
		drawPaddle(program, modelviewMatrix, playerTwo);
		drawBall(program, modelviewMatrix, ball);
		drawBackground(program, modelviewMatrix, back, !wait);
		glDisableVertexArrayAttrib;

		if (wait) {
			if ((float)SDL_GetTicks() > time + 1000.0f) {
				ball.xVelocity = direction * 4.5f;
				ball.yVelocity = direction * (float)(rand() % 40) / 10.f;
				wait = false;
			}
		}

		SDL_GL_SwapWindow(displayWindow);
	}
	SDL_Quit();
	return 0;
}

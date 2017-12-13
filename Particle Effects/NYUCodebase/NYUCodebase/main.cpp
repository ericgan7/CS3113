#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "Matrix.h"
#include "ShaderProgram.h"
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

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
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1000,800, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif

	glViewport(0, 0, 1000,800);
	ShaderProgram program(RESOURCE_FOLDER"Vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
	Matrix projectionMatrix;
	projectionMatrix.SetOrthoProjection(-25.0f, 25.0f, -20.0f, 20.0f, -1.0f, 1.0f);
	projectionMatrix.Translate(-25.0f, 20.0f, 0.0f);
	Matrix modelviewMatrix;
	modelviewMatrix.Translate(25.0f, -20.0f, 0.0f);
	program.SetModelviewMatrix(modelviewMatrix);
	program.SetProjectionMatrix(projectionMatrix);

	GLuint t = LoadTexture("art_uiBackground.png");

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
		float ORTHOW = 25.0f;
		float ORTHOH = 20.0f;
		float width = ORTHOW / 3;
		float height = ORTHOH / 3;
		float tSize = 100.0f / 512.0f;
		std::vector<float> backgroundV;
		std::vector<float> backgroundT;
		backgroundV.insert(backgroundV.end(), {//minimap
			-ORTHOW, -ORTHOH + height,
			-ORTHOW, -ORTHOH,
			-ORTHOW + width, -ORTHOH + height,
			-ORTHOW + width, -ORTHOH,
			-ORTHOW + width, -ORTHOH + height,
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
		glBindTexture(GL_TEXTURE_2D, t);
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, backgroundV.data());
		glEnableVertexAttribArray(program.positionAttribute);
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, backgroundT.data());
		glEnableVertexAttribArray(program.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, backgroundV.size() / 2);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);
		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}

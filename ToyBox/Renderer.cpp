#include "Renderer.h"

#include <iostream>
#include <random>

#define MAX_THREADSNUM 6

GLubyte* Renderer::outputImage = NULL;
GLuint Renderer::outputTexID = -1;
float Renderer::progress = 0;
clock_t Renderer::start = -1;
clock_t Renderer::end = -1;

bool Renderer::PrepareVFB() {
	// delete old array
	if (Renderer::outputTexID != -1) free(Renderer::outputImage);

	// malloc space for new output image
	Renderer::outputImage = (GLubyte *)malloc(Renderer::OUTPUT_SIZE.x * Renderer::OUTPUT_SIZE.y * 3 * sizeof(GLubyte));

	// initiallize new output image


	// load init blank image
	bool ret = LoadTextureFromMemory(Renderer::OUTPUT_SIZE, Renderer::outputImage, &Renderer::outputTexID);
	//IM_ASSERT(ret);

	return ret;
}

void* Renderer::rendering(void* args) {
	std::default_random_engine seed;
	std::uniform_int_distribution<unsigned> randGen(0, 255);
	int currLine = 0;
	int width = OUTPUT_SIZE.x;
	int height = OUTPUT_SIZE.y;

	// start rendering
	start = clock();
	std::cout << "start rendering!" << std::endl;
	std::cout << "output size: " << OUTPUT_SIZE.x << " * " << OUTPUT_SIZE.y << std::endl;

	#pragma omp parallel for num_threads(MAX_THREADSNUM)
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int stp = (i * width + j) * 3;
			for (int k = 0; k < 1000; k++) int sss = pow(randGen(seed), 1000);
			outputImage[stp] = (GLubyte)randGen(seed);
			outputImage[stp + 1] = (GLubyte)randGen(seed);
			outputImage[stp + 2] = (GLubyte)randGen(seed);
		}
		currLine++;
		progress = currLine / OUTPUT_SIZE.y;
	}

	// finished
	end = clock();
	progress = -1;

	return 0;
}
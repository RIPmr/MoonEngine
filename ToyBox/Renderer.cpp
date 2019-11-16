#include "Renderer.h"

#include <iostream>
#include <random>

using namespace moon;

#define MAX_THREADSNUM 6

GLubyte* Renderer::outputImage = NULL;
GLuint Renderer::outputTexID = -1;
float Renderer::progress = 0;
clock_t Renderer::start = -1;
clock_t Renderer::end = -1;
bool Renderer::isAbort = false;

bool Renderer::PrepareVFB() {
	// delete old array
	if (outputTexID != -1) free(outputImage);
	// malloc space for new output image
	outputImage = (GLubyte *)malloc(OUTPUT_SIZE.x * OUTPUT_SIZE.y * 3 * sizeof(GLubyte));
	// initiallize new output image

	// load init blank image
	bool ret = LoadTextureFromMemory(OUTPUT_SIZE, outputImage, &outputTexID);
	//IM_ASSERT(ret);

	return ret;
}

bool Renderer::PrepareRendering() {
	start = end = -1;
	isAbort = false;
	if (outputTexID != -1) glDeleteTextures(1, &outputTexID);
	PrepareVFB();
	return true;
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
		if (isAbort) continue;
		for (int j = 0; j < width; j++) {
			if (isAbort) continue;
			int stp = (i * width + j) * 3;
			// make it slower
			for (int k = 0; k < 1000; k++) {
				if (isAbort) continue;
				int sss = pow(randGen(seed), 1000);
			}
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
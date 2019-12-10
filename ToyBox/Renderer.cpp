#include <iostream>
#include <iomanip>
#include <random>

#include "Renderer.h"
#include "Material.h"
#include "MathUtils.h"
#include "SceneMgr.h"
#include "Model.h"
#include "Camera.h"

using namespace MOON;

#define MAX_THREADSNUM 6

Camera Renderer::matCamera = Camera("matCamera", Vector3(0, 0, 4));
Camera* Renderer::targetCamera = MOON_UNSPECIFIEDID;
GLubyte* Renderer::outputImage = MOON_UNSPECIFIEDID;
GLubyte* Renderer::matPrevImage = MOON_UNSPECIFIEDID;
GLuint Renderer::outputTexID = MOON_AUTOID;
float Renderer::progress = 0;
clock_t Renderer::start = -1;
clock_t Renderer::end = -1;
bool Renderer::isAbort = false;
bool Renderer::prevInQueue = false;

void Renderer::SetOutputSize(unsigned int width, unsigned int height) {
	OUTPUT_SIZE.setValue(width, height);
	aspect = (float)width / height;
}

bool Renderer::PrepareVFB() {
	// delete old texture
	if (outputTexID != -1) free(outputImage);
	// malloc space for new output image
	outputImage = (GLubyte *)malloc(OUTPUT_SIZE.x * OUTPUT_SIZE.y * 3 * sizeof(GLubyte));
	// TODO: initialize new output image

	// load init blank image
	bool ret = LoadTextureFromMemory(OUTPUT_SIZE, outputImage, outputTexID);
	//IM_ASSERT(ret);

	return ret;
}

bool Renderer::PrepareRendering() {
	start = end = -1;
	isAbort = false;
	if (outputTexID != -1) glDeleteTextures(1, &outputTexID);
	targetCamera->InitRenderCamera();
	PrepareVFB();
	return true;
}

void* Renderer::renderingTestImage(void* args) {
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


bool Renderer::PrepareMatPrevRendering(Texture* target) {
	start = end = -1;
	matCamera.fov = 20.0f;
	matCamera.InitRenderCamera();
	if (target->localID != -1) {
		free(matPrevImage);
		glDeleteTextures(1, &target->localID);
	}
	matPrevImage = (GLubyte *)malloc(Material::PREVSIZE.x * Material::PREVSIZE.y * 3 * sizeof(GLubyte));

	// load init blank image
	return LoadTextureFromMemory(Material::PREVSIZE, matPrevImage, target->localID);
}
Vector3 Renderer::SamplingColor_Simple(const Ray &r, int depth, const Sphere* ball, const Sphere* ground) {
	HitRecord recB, recG;
	if (ball->Hit(r, recB) || ground->Hit(r, recG)) {
		Ray scattered;
		Vector3 attenuation;
		HitRecord& rec = recB.t < recG.t ? recB : recG;
		if (depth < maxReflectionDepth && rec.mat->scatter(r, rec, attenuation, scattered))
			return attenuation * SamplingColor_Simple(scattered, depth + 1, ball, ground);
		else return Vector3::ZERO();
	} else return SimpleSky(r);
}
void* Renderer::renderingMatPreview(void* args) {
	int currLine = 0;
	int width = Material::PREVSIZE.x;
	int height = Material::PREVSIZE.y;

	// mat ball
	Sphere matBall(Vector3(0, 0, 0), 0.5, (Material*)args);
	Sphere ground(Vector3(0, -100.5, 0), 100, new Lambertian(Vector3(0.8, 0.8, 0.8)));
	
	// start rendering
	std::cout << "rendering preview..." << std::endl;

#pragma omp parallel for num_threads(MAX_THREADSNUM)
	for (int i = height - 1; i >= 0; i--) {
		for (int j = 0; j < width; j++) {
			int stp = ((height - 1 - i) * width + j) * 3;

			// MonteCarlo Sampling [BruteForce] ----------
			Vector3 col = Vector3::ZERO();
			for (int s = 0; s < 20; s++) {
				float u = float(j + MoonMath::drand48()) / width;
				float v = float(i + MoonMath::drand48()) / height;
				Ray ray = matCamera.GetRay(u, v, 1.0f);
				col += SamplingColor_Simple(ray, 0, &matBall, &ground);
			}
			// -------------------------------------------

			col /= 20.0f;
			MoonMath::GammaCorrection(col);

			matPrevImage[stp] = (GLubyte)(255.99 * col.x);
			matPrevImage[stp + 1] = (GLubyte)(255.99 * col.y);
			matPrevImage[stp + 2] = (GLubyte)(255.99 * col.z);
		}
		currLine++;
		progress = currLine / OUTPUT_SIZE.y;
	}

	// finished
	progress = -1;
	delete ground.mat;
	std::cout << "done." << std::endl;

	return 0;
}

void* Renderer::rendering(void* args) {
	int currLine = 0;
	int width = OUTPUT_SIZE.x;
	int height = OUTPUT_SIZE.y;

	// start rendering
	start = clock();
	std::cout << std::endl;
	std::cout << "===================== [start rendering] =====================" << std::endl;
	std::cout << "- output size: " << OUTPUT_SIZE.x << " * " << OUTPUT_SIZE.y << std::endl;
	std::cout << "- output Texture ID: " << outputTexID << std::endl;
	std::cout << "- sampling rate: " << samplingRate << std::endl;
	std::cout << "- max reflection depth: " << maxReflectionDepth << std::endl;

#pragma omp parallel for num_threads(MAX_THREADSNUM)
	for (int i = height - 1; i >= 0; i--) {
		if (isAbort) continue;
		for (int j = 0; j < width; j++) {
			if (isAbort) continue;
			int stp = ((height - 1 - i) * width + j) * 3;
			
			// MonteCarlo Sampling [BruteForce] ----------
			Vector3 col = Vector3::ZERO();

			for (int s = 0; s < samplingRate; s++) {
				if (isAbort) continue;
				float u = float(j + MoonMath::drand48()) / width;
				float v = float(i + MoonMath::drand48()) / height;
				Ray ray = targetCamera->GetRay(u, v, aspect);
				//Vector3 p = ray.PointAtParameter(2.0);
				col += SamplingColor(ray, 0);

				// break
				/*if (_kbhit()) {
					if (_getch() == 27) {
						std::cout << "cancled by user!" << std::endl;
						isAbort = true;
					}
				}*/
			}
			// -------------------------------------------

			col /= float(samplingRate);
			MoonMath::GammaCorrection(col);

			outputImage[stp] = (GLubyte)(255.99 * col.x);
			outputImage[stp + 1] = (GLubyte)(255.99 * col.y);
			outputImage[stp + 2] = (GLubyte)(255.99 * col.z);
		}
		currLine++;
		progress = currLine / OUTPUT_SIZE.y;
	}

	// finished
	end = clock();
	progress = -1;

	return 0;
}

Vector3 Renderer::SamplingColor(const Ray &r, int depth) {
	HitRecord rec;
	if (MOON_ModelManager::Hit(r, rec)) {
		Ray scattered;
		Vector3 attenuation;
		if (depth < maxReflectionDepth && rec.mat->scatter(r, rec, attenuation, scattered))
			return attenuation * SamplingColor(scattered, depth + 1);
		else return Vector3::ZERO();
	} else return SimpleSky(r);
}

Vector3 Renderer::SimpleSky(const Ray &r) {
	Vector3 unit_direction = Vector3::Normalize(r.dir);
	float t = 0.5 * (unit_direction.y + 1.0);
	return (1.0 - t) * Vector3::ONE() + t * Vector3(0.5, 0.7, 1.0);
}
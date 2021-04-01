#include <iostream>
#include <iomanip>
#include <random>

#include "Renderer.h"
#include "Material.h"
#include "MathUtils.h"
#include "SceneMgr.h"
#include "Model.h"
#include "Camera.h"
#include "ThreadPool.h"
#include "Graphics.h"

namespace MOON {

#pragma region renderer_params
	Camera		 Renderer::matCamera = Camera("matCamera", Vector3(0, 0, -4));
	Camera*		 Renderer::targetCamera = MOON_UNSPECIFIEDID;
	GLfloat*	 Renderer::outputRAW = MOON_UNSPECIFIEDID;
	GLfloat*	 Renderer::matPrevRaw = MOON_UNSPECIFIEDID;
	FrameBuffer* Renderer::output = MOON_UNSPECIFIEDID;
	//GLuint	 Renderer::outputTexID = MOON_AUTOID;
	clock_t		 Renderer::start = -1;
	clock_t		 Renderer::end = -1;
	float		 Renderer::progress = 0;
	int			 Renderer::bucketSize = 35;
	unsigned int Renderer::prevSampleRate = 20;
	unsigned int Renderer::samplingRate = 5;
	unsigned int Renderer::maxReflectionDepth = 5;
	bool		 Renderer::useIS = false;
	bool		 Renderer::depth = true;
	bool		 Renderer::motion = false;
	bool		 Renderer::isAbort = false;
	bool		 Renderer::prevInQueue = false;

	std::string				Renderer::timeStamp = "";
	TexFilter				Renderer::filter = Nearest;
	Renderer::AccStruct		Renderer::acc = acc_AABB;
	ToneMappingMethod		Renderer::tone = Tone_Reinhard;
	Renderer::RenderMode	Renderer::renderMode = bucket;
	Renderer::SamplingMode	Renderer::sampleMode = BruteForce;
#pragma endregion

#pragma region initialization
	void Renderer::UpdateTimeStamp() {
		timeStamp = MOON_Clock::GetTimeStamp();
	}

	void Renderer::SetOutputSize(unsigned int width, unsigned int height) {
		OUTPUT_SIZE.setValue(width, height);
		aspect = (float)width / height;
	}

	void Renderer::StartRendering() {
		if (!Renderer::prevInQueue) if (Renderer::PrepareRendering())
			ThreadPool::CreateThread(Renderer::rendering, NULL);
	}

	bool Renderer::PrepareVFB() {
		// delete old texture
		//if (outputTexID != -1) free(outputRAW);
		if (output == nullptr) {
			output = new FrameBuffer(
				MOON_OutputSize.x, MOON_OutputSize.y,
				"output", MOON_UNSPECIFIEDID, GL_RGB16F, GL_FLOAT
			);
		} else if (output->width != MOON_OutputSize.x || output->height != MOON_OutputSize.y) {
			output->Reallocate(MOON_OutputSize.x, MOON_OutputSize.y);
		}
		free(outputRAW);
		// malloc space for new output image
		outputRAW = (GLfloat *)malloc(OUTPUT_SIZE.x * OUTPUT_SIZE.y * 3 * sizeof(GLfloat));
		// TODO: initialize new output image

		// load init blank image
		//bool ret = Utility::LoadTextureFromMemory(OUTPUT_SIZE, outputRAW, outputTexID);
		//IM_ASSERT(ret);

		return true;
	}

	bool Renderer::PrepareRendering() {
		start = end = -1;
		isAbort = false;
		//if (outputTexID != -1) glDeleteTextures(1, &outputTexID);
		targetCamera->InitRenderCamera();
		if (acc == acc_BVH) SceneManager::BuildBVH();
		PrepareVFB();
		return true;
	}

	bool Renderer::PrepareMatPrevRendering(Texture* target) {
		start = end = -1;
		matCamera.fov = 20.0f;
		matCamera.InitRenderCamera();
		if (!target->localID) {
			matPrevRaw = (GLfloat *)malloc(Material::PREVSIZE.x * Material::PREVSIZE.y * 3 * sizeof(GLfloat));
			// load init blank image
			Utility::LoadTextureFromMemory(Material::PREVSIZE, matPrevRaw, target->localID);
		}
		//free(matPrevRaw);
		//glDeleteTextures(1, &target->localID);

		return true;
	}
#pragma endregion

#pragma region rendering
	void* Renderer::rendering(void* args) {
		int width = OUTPUT_SIZE.x;
		int height = OUTPUT_SIZE.y;

		// start rendering
		start = clock();
		std::cout << std::endl;
		std::cout << "===================== [start rendering] =====================" << std::endl;
		std::cout << "- output size: " << OUTPUT_SIZE.x << " * " << OUTPUT_SIZE.y << std::endl;
		std::cout << "- output Texture ID: " << output->localID << std::endl;
		std::string acs = "None"; if (acc == acc_AABB) acs = "AABB"; else if (acc == acc_BVH) acs = "BVH";
		std::cout << "- acc structure: " << acs << std::endl;
		std::string filt = "Nearest"; if (filter == Bilinear) filt = "Bilinear"; else if (filter == Trilinear) filt = "Trilinear";
		std::cout << "- texture filter: " << filt << std::endl;
		std::cout << "- sampling rate: " << samplingRate << std::endl;
		std::cout << "- max reflection depth: " << maxReflectionDepth << std::endl;
		
		// Scanline mode --------------------------------------------------------------------------
		if (renderMode == scanline) {
			int currLine = 0;
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

						if (!depth && !s) {
							HitRecord tmp;
							if (!MOON_ModelManager::Hit(ray, tmp, acc)) {
								Ray sray = targetCamera->GetRay((float)j / width, (float)i / height, aspect);
								if (MOON_Enviroment == env_hdri) col = SampleSphericalMap(sray) * samplingRate;
								else if (MOON_Enviroment == env_pure_color) col = Graphics::clearColor * samplingRate;
								else col = ProceduralSky(sray) * samplingRate;
								break;
							}
						}

						col += useIS ? SamplingColor_IS(ray, 0) : SamplingColor(ray, 0);
					}
					// -------------------------------------------

					col /= float(samplingRate);

					//MoonMath::GammaCorrection(col);
					//col.setValue(MoonMath::clamp(col, 0.0f, 1.0f));
					//outputRAW[stp] = (GLubyte)(255.0f * col.x);
					//outputRAW[stp + 1] = (GLubyte)(255.0f * col.y);
					//outputRAW[stp + 2] = (GLubyte)(255.0f * col.z);

					outputRAW[stp] = col.x;
					outputRAW[stp + 1] = col.y;
					outputRAW[stp + 2] = col.z;
				}
				currLine++;
				progress = currLine / OUTPUT_SIZE.y;
			}
		}

		// Bucket mode ----------------------------------------------------------------------------
		else if (renderMode == bucket) {
			// prepare bucket set
			std::vector<Vector4> buckets;

			int u, v, cnt = 0;
			for (u = 0; u < width - 1; u += bucketSize) {
				for (v = 0; v < height - 1; v += bucketSize)
					buckets.push_back(Vector4(
						u, v, 
						std::min(bucketSize, width - 1 - u), 
						std::min(bucketSize, height - 1 - v)
					));
			}
			if (u < width - 1) {
				for (int vv = 0; vv < height - 1; vv += bucketSize)
					buckets.push_back(Vector4(u, vv, width - 1 - u, bucketSize));
			}
			if (v < height - 1) {
				for (int uu = 0; uu < width - 1; uu += bucketSize)
					buckets.push_back(Vector4(uu, v, bucketSize, height - 1 - v));
				if (u < width - 1) buckets.push_back(Vector4(u, v, width - 1 - u, height - 1 - v));
			}

			Utility::ShuffleVector(buckets);

			#pragma omp parallel for num_threads(MAX_THREADSNUM)
			for (int k = 0; k < buckets.size(); k++) {
				if (isAbort) continue;
				for (int i = buckets[k].y + buckets[k].w; i >= buckets[k].y; i--) {
					if (isAbort) continue;
					for (int j = buckets[k].x; j <= buckets[k].x + buckets[k].z; j++) {
						if (isAbort) continue;
						int stp = ((height - 1 - i) * width + j) * 3;

						// MonteCarlo Sampling [BruteForce] ----------
						Vector3 col = Vector3::ZERO();

						for (int s = 0; s < samplingRate; s++) {
							if (isAbort) continue;
							float u = float(j + MoonMath::drand48()) / width;
							float v = float(i + MoonMath::drand48()) / height;
							Ray ray = targetCamera->GetRay(u, v, aspect);

							if (!depth && !s) {
								HitRecord tmp;
								if (!MOON_ModelManager::Hit(ray, tmp, acc)) {
									Ray sray = targetCamera->GetRay((float)j / width, (float)i / height, aspect);
									if (MOON_Enviroment == env_hdri) col = SampleSphericalMap(sray) * samplingRate;
									else if (MOON_Enviroment == env_pure_color) col = Graphics::clearColor * samplingRate;
									else col = ProceduralSky(sray) * samplingRate;
									break;
								}
							}

							col += useIS ? SamplingColor_IS(ray, 0) : SamplingColor(ray, 0);
						}
						// -------------------------------------------
						col /= float(samplingRate);

						outputRAW[stp] = col.x;
						outputRAW[stp + 1] = col.y;
						outputRAW[stp + 2] = col.z;
					}
				}
				cnt++;
				progress = (float)cnt / (buckets.size() - 1);
			}
			
		}

		// Progressive mode -----------------------------------------------------------------------
		else if (renderMode == progressive) {
			// TODO
		}

		// finished
		end = clock();
		progress = -1;
		UpdateTimeStamp();

		return 0;
	}

	void* Renderer::renderingMatPreview(void* args) {
		int currLine = 0;
		int width = Material::PREVSIZE.x;
		int height = Material::PREVSIZE.y;
		//float samplingRate = MOON_Enviroment == env_hdri ? 100 : 20;
		float samplingRate = prevSampleRate;

		// mat ball
		MSphere matBall(Vector3(0, 0, 0), 0.5, (Material*)args);
		MSphere ground(Vector3(0, -100.5, 0), 100, MOON_MaterialManager::defaultMat);

		// start rendering
		std::cout << "rendering preview..." << std::endl;

		#pragma omp parallel for num_threads(MAX_THREADSNUM)
		for (int i = height - 1; i >= 0; i--) {
			for (int j = 0; j < width; j++) {
				int stp = ((height - 1 - i) * width + j) * 3;

				// MonteCarlo Sampling [BruteForce] ----------
				Vector3 col = Vector3::ZERO();
				for (int s = 0; s < samplingRate; s++) {
					float u = float(j + MoonMath::drand48()) / width;
					float v = float(i + MoonMath::drand48()) / height;
					Ray ray = matCamera.GetRay(u, v, 1.0f);
					col += SamplingColor_MatPrev(ray, 0, &matBall, &ground);
				}
				// -------------------------------------------

				col /= samplingRate;
				MoonMath::ReinhardTonemapping(col);

				matPrevRaw[stp] = col.x;
				matPrevRaw[stp + 1] = col.y;
				matPrevRaw[stp + 2] = col.z;
			}
			currLine++;
			progress = currLine / OUTPUT_SIZE.y;
		}

		// finished
		progress = -1;
		std::cout << "done." << std::endl;

		return 0;
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
				outputRAW[stp] = (GLubyte)randGen(seed);
				outputRAW[stp + 1] = (GLubyte)randGen(seed);
				outputRAW[stp + 2] = (GLubyte)randGen(seed);
			}
			currLine++;
			progress = currLine / OUTPUT_SIZE.y;
		}

		// finished
		end = clock();
		progress = -1;

		return 0;
	}
#pragma endregion

#pragma region sampling_functions
	Vector3 Renderer::SamplingColor_MatPrev(const Ray &r, int depth, const MSphere* ball, const MSphere* ground) {
		HitRecord recB, recG;
		if (ball->Hit(r, recB) || ground->Hit(r, recG)) {
			Ray scattered; Vector3 attenuation;
			HitRecord& rec = recB.t < recG.t ? recB : recG;
			Vector3 emitted = rec.mat->Emitted(rec.uv);
			if (depth < maxReflectionDepth && rec.mat->Scatter(r, rec, attenuation, scattered))
				return emitted + attenuation * SamplingColor_MatPrev(scattered, depth + 1, ball, ground);
			else return emitted;
		} else {
			if (MOON_Enviroment == env_hdri) return SampleSphericalMap(r);
			else if (MOON_Enviroment == env_pure_color) return Graphics::clearColor;
			else return ProceduralSky(r);
		}
	}

	Vector3 Renderer::SamplingColor(const Ray &r, int depth) {
		HitRecord rec;
		if (MOON_ModelManager::Hit(r, rec, acc)) {
			Ray scattered; Vector3 attenuation;
			Vector3 emitted = rec.mat->Emitted(rec.uv);
			if (depth < maxReflectionDepth && rec.mat->Scatter(r, rec, attenuation, scattered))
				return emitted + attenuation * SamplingColor(scattered, depth + 1);
			else return emitted;
		} else {
			if (MOON_Enviroment == env_hdri) return SampleSphericalMap(r);
			else if (MOON_Enviroment == env_pure_color) return Graphics::clearColor;
			else return ProceduralSky(r);
		}
	}

	Vector3 Renderer::SamplingColor_IS(const Ray &r, int depth) {
		HitRecord rec;
		if (MOON_ModelManager::Hit(r, rec, acc)) {
			Ray scattered; Vector3 albedo; float pdf;
			Vector3 emitted = rec.mat->Emitted(rec.uv);
			if (depth < maxReflectionDepth && rec.mat->Scatter_IS(r, rec, albedo, scattered, pdf))
				return emitted + albedo * rec.mat->PDF(r, rec, scattered) * SamplingColor_IS(scattered, depth + 1) / pdf;
			else return emitted;
		} else {
			if (MOON_Enviroment == env_hdri) return SampleSphericalMap(r);
			else if (MOON_Enviroment == env_pure_color) return Graphics::clearColor;
			else return ProceduralSky(r);
		}
	}

	Vector3 Renderer::ProceduralSky(const Ray &r) {
		Vector3 unit_direction = Vector3::Normalize(r.dir);
		float t = 0.5 * (unit_direction.y + 1.0);
		auto skyCol = (1.0 - t) * Vector3::ONE() + t * Vector3(0.5, 0.7, 1.0);
		MoonMath::GammaCorrection(skyCol, 1.0f / 2.2f);
		return skyCol;
	}

	Vector3 Renderer::SampleSphericalMap(const Ray &r) {
		const Vector2 invAtan(0.1591f, 0.3183f);

		auto dir = Vector3::Normalize(r.dir);
		Vector2 uv(std::atan2(dir.z, dir.x), std::asin(dir.y));
		uv *= invAtan; uv.x += 0.5f; uv.y += 0.5f;

		return MOON_TextureManager::HDRI->SamplingColor(uv, filter);
	}
#pragma endregion

}

/*
*NOTE: about the 'invAtan' constant:
The invAtan constant is the reciprocal(or multiplicative inverse) of 2PI and PI:
	0.1591 = 1/6.28319(=>2PI) = 360 degrees in radians
	0.3183 = 1/3.14159(=>PI) = 180 degrees in radians
So you go from cartesian coordinates to polar angles to uvs,
see this great resource(headline: Direct Polar). In more practical terms,
assuming that given direction is normalized(hence mapped to the unit-sphere)
multiplying by invAtan transformes the values into the [-0.5, 0.5] range,
adding 0.5 results in uv lookup coordinates in the range of [0, 1].
*/
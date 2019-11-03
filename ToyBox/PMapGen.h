#pragma once
#include <GLFW/glfw3.h>

#include "Vector2.h"

class ProceduralMapGenerator {
public:
	static void MakeCheckImage(const Vector2 &PMapSize, GLubyte *checkImage) {
		for (int i = 0, c, cnt = 0; i < PMapSize.x; i++) {
			for (int j = 0; j < PMapSize.y; j++) {
				c = ((((i & 0x8) == 0) ^ ((j & 0x8)) == 0)) * 255;
				checkImage[cnt++] = (GLubyte)c;
				checkImage[cnt++] = (GLubyte)c;
				checkImage[cnt++] = (GLubyte)c;
			}
		}
	}

	static void MakeBlankImage(const Vector2 &PMapSize, GLubyte *checkImage) {
		for (int i = 0, cnt = 0; i < PMapSize.x; i++) {
			for (int j = 0; j < PMapSize.y; j++) {
				checkImage[cnt++] = (GLubyte)0;
				checkImage[cnt++] = (GLubyte)0;
				checkImage[cnt++] = (GLubyte)0;
			}
		}
	}
};
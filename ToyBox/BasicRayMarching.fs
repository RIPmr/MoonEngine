#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform mat4 invPVMat;
uniform sampler2D screenBuffer;
uniform sampler2D depthBuffer;

uniform vec2 bufferSize;
uniform vec3 bboxMin;
uniform vec3 bboxMax;

vec3 viewPos, viewDir;

vec2 RayBoxDst(vec3 boundsMin, vec3 boundsMax, vec3 rayOrigin, vec3 invRaydir) {
	vec3 t0 = (boundsMin - rayOrigin) * invRaydir;
	vec3 t1 = (boundsMax - rayOrigin) * invRaydir;
	vec3 tmin = min(t0, t1);
	vec3 tmax = max(t0, t1);

	// enter point
	float dstA = max(max(tmin.x, tmin.y), tmin.z);
	// exit point
	float dstB = min(tmax.x, min(tmax.y, tmax.z));

	float dstToBox = max(0, dstA);
	float dstInsideBox = max(0, dstB - dstToBox);
	return vec2(dstToBox, dstInsideBox);
}

void CalculatePixelRay(vec2 n) {
	vec4 ray_start = invPVMat * vec4(n.x, n.y, -1.0, 1.0);
	vec4 ray_end = invPVMat * vec4(n.x, n.y, 1.0, 1.0);

	ray_start.xyz /= ray_start.w;
	ray_end.xyz /= ray_end.w;

	viewPos = ray_start.xyz;
	viewDir = normalize((ray_end - ray_start).xyz);
}

vec4 CalculateWorldSpacePosition(float depth, vec2 n) {
	return invPVMat * vec4(n.x, n.y, depth, 1.0);
}

float CloudRayMarching(vec3 startPoint, vec3 direction, vec3 worldPos) {
	vec3 testPoint = startPoint;

	float depthEyeLinear = length(worldPos - testPoint);
	vec2 intersectInfo = RayBoxDst(bboxMin, bboxMax, testPoint, 1.0 / direction);
	float dstLimit = min(depthEyeLinear - intersectInfo.x, intersectInfo.y);
	
	float sum = 1.0;
	if (dstLimit > 0) {
		testPoint = startPoint + intersectInfo.x * direction;
		for (int i = 0; i < 256; i++) {
			testPoint += direction * 0.1;
			if (0.1 * i > dstLimit) break;
			if (testPoint.x < bboxMax.x && testPoint.x > bboxMin.x &&
				testPoint.z < bboxMax.z && testPoint.z > bboxMin.z &&
				testPoint.y < bboxMax.y && testPoint.y > bboxMin.y)
				sum += 0.1;
			//if (sum >= 1.0) break;
		}
	}
	return sum;
}

void main() {
    vec3 color = texture(screenBuffer, TexCoords).rgb;
	float depth = 2 * texture(depthBuffer, TexCoords).r - 1;
	vec4 worldPos = CalculateWorldSpacePosition(depth, TexCoords * 2.0 - vec2(1.0));
	worldPos.xyz /= worldPos.w;

	// cloud ray marching
	CalculatePixelRay(TexCoords * 2.0 - vec2(1.0));
	float cloud = CloudRayMarching(viewPos, viewDir, worldPos.xyz);

    FragColor = vec4(color * pow(cloud, 2.2), 1.0);

	// debug
	//float depthEyeLinear = length(worldPos.xyz - viewPos);
	//vec2 intersectInfo = RayBoxDst(bboxMin, bboxMax, viewPos, 1.0 / viewDir);
	//float dstLimit = min(depthEyeLinear - intersectInfo.x, intersectInfo.y);
    //FragColor = vec4(clamp(dstLimit, 0, 1), pow(depthEyeLinear / 10, 2.0) / 20, pow(depthEyeLinear / 10, 2.0), 1);
}
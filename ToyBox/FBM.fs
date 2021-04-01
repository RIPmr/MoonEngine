// GLSL中没有函数指针，但是可以定义带有函数名称列表的结构：
const struct functions_list {
    int sin;
    int cos;
    int tan;
    int fract;
};
functions_list functions = functions_list(1,2,3,4);

// 使用此函数名列表，可以模拟函数指针：
float callback(int func,float arg){
    if(func == functions.sin)
        return sin(arg);
    else if(func == functions.cos)
        return cos(arg);
    else if(func == functions.tan)
        return tan(arg);
    else if (func == functions.fract)
        return fract(arg);
    else
        return 0.0;
}

// FBM function -------------------------------------------------------------------------------------
float fbm(vec3 uv) {
	float f = 0, a = 1;
	for(int i = 0; i < 3; i++) {
		f += a * PerlinNoise3(uv);
		uv *= 2; a /= 2;
	}
	return f;
}

#define OCTAVES 3
float turbulence (vec2 st) {
    // Initial values
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 0.0;

    // Loop of octaves
    for (int i = 0; i < OCTAVES; i++) {
    	//snoise : 2D simplex noise function
        value += amplitude * abs(Simplex2(st));
        st *= 2.0; amplitude *= 0.5;
    }
    return value;
}
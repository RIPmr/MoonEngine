#define time iTime

// INNER ATMOS PROPIETIES:
// inner atmos inner strenght
#define in_inner 0.2
// inner atmos outer strenght
#define in_outer 0.2

// OUTER ATMOS PROPIETIES:
// inner atmos inner strenght
#define out_inner 0.2 
// inner atmos outer strenght
#define out_outer 0.4 // 0.01 is nice too

float noise3D(vec3 p) {
	return fract(sin(dot(p ,vec3(12.9898,78.233,128.852))) * 43758.5453) * 2.0 - 1.0;
}

float simplex3D(vec3 p) {
	float f3 = 1.0 / 3.0;
	float s = (p.x + p.y + p.z) * f3;
	int i = int(floor(p.x + s));
	int j = int(floor(p.y + s));
	int k = int(floor(p.z + s));
	
	float g3 = 1.0 / 6.0;
	float t = float(i + j + k) * g3;
	float x0 = float(i) - t;
	float y0 = float(j) - t;
	float z0 = float(k) - t;
	x0 = p.x - x0;
	y0 = p.y - y0;
	z0 = p.z - z0;
	int i1,j1,k1;
	int i2,j2,k2;
	if (x0 >= y0) {
		if (y0 >= z0){ i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 1; k2 = 0; } // X Y Z order
		else if	(x0 >= z0){ i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 0; k2 = 1; } // X Z Y order
		else { i1 = 0; j1 = 0; k1 = 1; i2 = 1; j2 = 0; k2 = 1; } // Z X Z order
	} else { 
		if (y0 < z0) { i1 = 0; j1 = 0; k1 = 1; i2 = 0; j2 = 1; k2 = 1; } // Z Y X order
		else if	(x0 < z0) { i1 = 0; j1 = 1; k1 = 0; i2 = 0; j2 = 1; k2 = 1; } // Y Z X order
		else { i1 = 0; j1 = 1; k1 = 0; i2 = 1; j2 = 1; k2 = 0; } // Y X Z order
	}
	float x1 = x0 - float(i1) + g3; 
	float y1 = y0 - float(j1) + g3;
	float z1 = z0 - float(k1) + g3;
	float x2 = x0 - float(i2) + 2.0 * g3; 
	float y2 = y0 - float(j2) + 2.0 * g3;
	float z2 = z0 - float(k2) + 2.0 * g3;
	float x3 = x0 - 1.0 + 3.0 * g3; 
	float y3 = y0 - 1.0 + 3.0 * g3;
	float z3 = z0 - 1.0 + 3.0 * g3;			 
	vec3 ijk0 = vec3(i, j, k);
	vec3 ijk1 = vec3(i + i1, j + j1, k + k1);	
	vec3 ijk2 = vec3(i + i2, j + j2, k + k2);
	vec3 ijk3 = vec3(i + 1, j + 1,k + 1);	     
	vec3 gr0 = normalize(vec3(noise3D(ijk0),noise3D(ijk0 * 2.01),noise3D(ijk0 * 2.02)));
	vec3 gr1 = normalize(vec3(noise3D(ijk1),noise3D(ijk1 * 2.01),noise3D(ijk1 * 2.02)));
	vec3 gr2 = normalize(vec3(noise3D(ijk2),noise3D(ijk2 * 2.01),noise3D(ijk2 * 2.02)));
	vec3 gr3 = normalize(vec3(noise3D(ijk3),noise3D(ijk3 * 2.01),noise3D(ijk3 * 2.02)));
	float n0 = 0.0;
	float n1 = 0.0;
	float n2 = 0.0;
	float n3 = 0.0;
	float t0 = 0.5 - x0 * x0 - y0 * y0 - z0 * z0;
	if(t0 >= 0.0) {
		t0 *= t0;
		n0 = t0 * t0 * dot(gr0, vec3(x0, y0, z0));
	}
	float t1 = 0.5 - x1 * x1 - y1 * y1 - z1 * z1;
	if(t1 >= 0.0) {
		t1 *= t1;
		n1 = t1 * t1 * dot(gr1, vec3(x1, y1, z1));
	}
	float t2 = 0.5 - x2 * x2 - y2 * y2 - z2 * z2;
	if(t2 >= 0.0) {
		t2 *= t2;
		n2 = t2 * t2 * dot(gr2, vec3(x2, y2, z2));
	}
	float t3 = 0.5 - x3 * x3 - y3 * y3 - z3 * z3;
	if(t3 >= 0.0) {
		t3 *= t3;
		n3 = t3 * t3 * dot(gr3, vec3(x3, y3, z3));
	}
	return 96.0 * (n0 + n1 + n2 + n3);
}

float fbm(vec3 p) {
	float f;
    f  = 0.50000 * (simplex3D(p)); p = p * 2.01;
    f += 0.25000 * (simplex3D(p)); p = p * 2.02;
    f += 0.12500 * (simplex3D(p)); p = p * 2.03;
    f += 0.06250 * (simplex3D(p)); p = p * 2.04;
    f += 0.03125 * (simplex3D(p)); p = p * 2.05;
    f += 0.015625 * (simplex3D(p));
	return f;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {    
    vec3 pos = vec3(gl_FragCoord.xy - iResolution.xy/2.0,0.0); // planet center
    
    // LIGHT
    vec3 l = normalize(vec3(sin(time), sin(time*0.5), (cos(time))));
    
    // PLANET
    float r = iResolution.y/3.0; // radius
    float z_in = sqrt(r*r - pos.x*pos.x - pos.y*pos.y);
    float z_out = sqrt(-r*r + pos.x*pos.x + pos.y*pos.y);
    
    // NORMALS
    vec3 norm = normalize(vec3(pos.x, pos.y, z_in)); // normals from sphere
    vec3 norm_out = normalize(vec3(pos.x, pos.y, z_out)); // normals from outside sphere
    float e = 0.05; // planet rugosity
    float nx = fbm(vec3(norm.x+e, norm.y,   norm.z  ))*0.5+0.5; // x normal displacement
    float ny = fbm(vec3(norm.x,   norm.y+e, norm.z  ))*0.5+0.5; // y normal displacement
    float nz = fbm(vec3(norm.x,   norm.y,   norm.z+e))*0.5+0.5; // z normal displacement
    norm = normalize(vec3(norm.x*nx, norm.y*ny, norm.z*nz));
    //norm = (norm+1.)/2.; // for normals visualization
	
    // TEXTURE
    float n = 1.0-(fbm(vec3(norm.x, norm.y, norm.z))*0.5+0.5); // noise for every pixel in planet
    
    // ATMOS
    float z_in_atm  = (r * in_outer)  / z_in - in_inner;   // inner atmos
    float z_out_atm = (r * out_inner) / z_out - out_outer; // outer atmos
    z_in_atm = max(0.0, z_in_atm);
    z_out_atm = max(0.0, z_out_atm);
    
    // DIFFUSE LIGHT
    float diffuse = max(0.0, dot(norm, l));
    float diffuse_out = max(0.0, dot(norm_out, l)+0.3); // +0.3 because outer atmosphere stills when inner doesn't
    
	//fragColor = vec4(vec3(n * diffuse),1.0);
    //fragColor = vec4(vec3(z_in_atm * diffuse),1.0);
    //fragColor = vec4(vec3(z_out_atm * diffuse_out),1.0);*/
    
    fragColor = vec4(vec3(n * diffuse + z_in_atm * diffuse + z_out_atm * diffuse_out),1.0);
}
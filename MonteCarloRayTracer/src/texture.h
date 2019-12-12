#pragma once
#include "vec3.h"

class texture {
public:
	virtual vec3 value(float u, float v, const vec3& p) const = 0;
};

class constant_texture : public texture {
public:
	constant_texture() { }
	constant_texture(vec3 c) : color(c) { }
	virtual vec3 value(float u, float v, const vec3& p) const override;

private:
	vec3 color;
};

class image_texture : public texture {
public:
	image_texture() {}
	image_texture(unsigned char* pixels, int A, int B) : data(pixels), nx(A), ny(B) {}
	virtual vec3 value(float u, float v, const vec3& p) const override;

private:
	unsigned char* data;
	int nx, ny;
};

// constant texture
// ----------------
vec3 constant_texture::value(float u, float v, const vec3& p) const {
	return color;
}

// image texture
// -------------
vec3 image_texture::value(float u, float v, const vec3& p) const {
	int i = (u)*nx;
	int j = (1 - v) * ny - 0.001;
	if (i < 0) i = 0;
	if (j < 0) j = 0;
	if (i > nx - 1) i = nx - 1;
	if (j > ny - 1) j = ny - 1;
	float r = int(data[3 * i + 3 * nx * j]) / 255.0;
	float g = int(data[3 * i + 3 * nx * j + 1]) / 255.0;
	float b = int(data[3 * i + 3 * nx * j + 2]) / 255.0;
	return vec3(r, g, b);
}
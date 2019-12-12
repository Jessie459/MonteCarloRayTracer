#pragma once
#include <cstdlib>

double random_double() {
	return rand() / (RAND_MAX + 1.0);
}

vec3 random_in_unit_disk() {
	vec3 p;
	do {
		p = 2.0 * vec3(random_double(), random_double(), 0) - vec3(1, 1, 0);
	} while (dot(p, p) >= 1.0);
	return p;
}

vec3 random_in_unit_sphere() {
	vec3 p;
	do {
		p = 2.0 * vec3(random_double(), random_double(), random_double()) - vec3(1, 1, 1);
	} while (dot(p, p) >= 1.0);
	return p;
}

inline vec3 random_cosine_direction() {
	float r1 = random_double();
	float r2 = random_double();
	float z = sqrt(1 - r2);
	float phi = 2 * M_PI * r1;
	float x = cos(phi) * sqrt(r2);
	float y = sin(phi) * sqrt(r2);
	return vec3(x, y, z);
}

inline vec3 random_to_sphere(float radius, float distance_squared) {
	float r1 = random_double();
	float r2 = random_double();
	float z = 1 + r2 * (sqrt(1 - radius * radius / distance_squared) - 1);
	float phi = 2 * M_PI * r1;
	float x = cos(phi) * sqrt(1 - z * z);
	float y = sin(phi) * sqrt(1 - z * z);
	return vec3(x, y, z);
}
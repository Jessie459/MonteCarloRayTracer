#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <vector>
#include "vertex.h"
using namespace std;

enum MATERIAL_TYPE {
	DIFFUSE, SPECULAR, REFRACTIVE, PLASTIC
};

class Mesh {
public:
	vector<Vertex> vertices;
	vector<int> indices;
	vec3 ka, kd, ks;
	float shininess, refracti, opacity;
	MATERIAL_TYPE type;

	Mesh(
		const vector<Vertex>& vertices,
		const vector<int>& indices,
		const vec3& ka,
		const vec3& kd,
		const vec3& ks,
		float shininess,
		float refracti,
		float opacity,
		MATERIAL_TYPE type
	) {
		this->vertices = vertices;
		this->indices = indices;
		this->ka = ka;
		this->kd = kd;
		this->ks = ks;
		this->shininess = shininess;
		this->refracti = refracti;
		this->opacity = opacity;
		this->type = type;
	}
};
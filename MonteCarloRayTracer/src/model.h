#pragma once
#include "Mesh.h"
#include "vec3.h"

class Model {
public:
	vector<Mesh> meshes;
	string directory;

	Model(const string& path) {
		loadModel(path);
	}

private:
	void loadModel(const string& path) {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
			return;
		}
		directory = path.substr(0, path.find_last_of('/'));
		processNode(scene->mRootNode, scene);
	}

	void processNode(aiNode* node, const aiScene* scene) {
		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));
		}
		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			processNode(node->mChildren[i], scene);
		}
	}

	Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
		// data to fill
		vector<Vertex> vertices;
		vector<int> indices;
		vec3 ka, kd, ks;
		float shininess, refracti, opacity;
		MATERIAL_TYPE type;

		ka = vec3(0.0f, 0.0f, 0.0f);
		kd = vec3(0.0f, 0.0f, 0.0f);
		ks = vec3(0.0f, 0.0f, 0.0f);
		shininess = 0.0f; refracti = 0.0f; opacity = 0.0f;
		type = DIFFUSE;

		// fill vertices
		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			Vertex vertex;
			vec3 vector;
			// positions
			vector.x() = mesh->mVertices[i].x;
			vector.y() = mesh->mVertices[i].y;
			vector.z() = mesh->mVertices[i].z;
			vertex.position = vector;
			// normals
			vector.x() = mesh->mNormals[i].x;
			vector.y() = mesh->mNormals[i].y;
			vector.z() = mesh->mNormals[i].z;
			vertex.normal = vector;
			vertices.push_back(vertex);
		}

		// fill indices
		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		if (mesh->mMaterialIndex >= 0) {
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			aiColor4D diffuse;
			aiColor4D specular;
			aiColor4D ambient;

			// fill ka, kd, ks
			if (aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &ambient) == AI_SUCCESS) {
				ka = vec3(ambient.r, ambient.g, ambient.b) * 100.0f;
			}
			if (aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse) == AI_SUCCESS) {
				kd = vec3(diffuse.r, diffuse.g, diffuse.b);
			}
			if (aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &specular) == AI_SUCCESS) {
				ks = vec3(specular.r, specular.g, specular.b);
			}

			// fill shininess, refracti, opacity
			unsigned int  max;
			aiGetMaterialFloatArray(material, AI_MATKEY_SHININESS, &shininess, &max);
			aiGetMaterialFloatArray(material, AI_MATKEY_REFRACTI, &refracti, &max);
			aiGetMaterialFloatArray(material, AI_MATKEY_OPACITY, &opacity, &max);

			// fill material type
			if (shininess > 0.0f) {
				type = SPECULAR;
				kd = vec3(1.0f, 1.0f, 1.0f);
			}
			if (refracti > 1.0f) {
				type = REFRACTIVE;
			}
			if (opacity < 1.0f) {
				type = PLASTIC;
			}
		}
		return Mesh(vertices, indices, ka, kd, ks, shininess, refracti, opacity, type);
	}
};
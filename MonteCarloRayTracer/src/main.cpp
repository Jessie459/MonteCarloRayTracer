#include "rect.h"
#include "box.h"
#include "bvh.h"
#include "camera.h"
#include "hittable_list.h"
#include "material.h"
#include "mesh.h"
#include "model.h"
#include "pdf.h"
#include "random.h"
#include "sphere.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "texture.h"
#include "transform.h"
#include "triangle.h"
#include "vertex.h"

#include <float.h>
#include <time.h>
#include <iostream>
#include <fstream>
using namespace std;

inline vec3 de_nan(const vec3& c) {
	vec3 temp = c;
	if (!(temp[0] == temp[0])) temp[0] = 0;
	if (!(temp[1] == temp[1])) temp[1] = 0;
	if (!(temp[2] == temp[2])) temp[2] = 0;
	return temp;
}

vec3 color(const ray& r, hittable* scene, hittable* light_shape, int depth) {
	hit_record hrec;
	if (scene->hit(r, 0.001, FLT_MAX, hrec)) {
		vec3 emitted = hrec.mat_ptr->emitted(r, hrec, hrec.u, hrec.v, hrec.p);

		scatter_record srec;
		if (depth < 50 && hrec.mat_ptr->scatter(r, hrec, srec)) {
			if (srec.is_specular) {
				return srec.attenuation * color(srec.specular_ray, scene, light_shape, depth + 1);
			}
			else {
				hittable_pdf plight(light_shape, hrec.p);
				mixture_pdf p(&plight, srec.pdf_ptr);
				ray scattered = ray(hrec.p, p.generate(), r.time());
				float pdf_val = p.value(scattered.direction());
				delete srec.pdf_ptr;
				return emitted
					+ srec.attenuation * hrec.mat_ptr->scattering_pdf(r, hrec, scattered)
					* color(scattered, scene, light_shape, depth + 1)
					/ pdf_val;
			}
		}
		else
			return emitted;
	}
	else
		return vec3(0, 0, 0);
}

hittable* import_model(string path, material* mat) {
	Model model(path);
	vector<Triangle*> triangles;
	for (unsigned int i = 0; i < model.meshes.size(); i++) {
		Mesh mesh = model.meshes[i];
		for (unsigned int j = 0; j < mesh.indices.size() - 2; j += 3) {
			Triangle* triangle = new Triangle(
				mesh.vertices[mesh.indices[j]],
				mesh.vertices[mesh.indices[j + 1]],
				mesh.vertices[mesh.indices[j + 2]],
				mat
			);
			triangles.push_back(triangle);
		}
	}
	unsigned int triangles_size = triangles.size();
	hittable** triangles_list = new hittable* [triangles_size];
	for (unsigned int i = 0; i < triangles_size; i++) {
		triangles_list[i] = triangles[i];
	}
	return new bvh_node(triangles_list, triangles_size, 0, 1);
}

void cornell_box(hittable** scene) {
	// materials
	material* red = new lambertian(new constant_texture(vec3(0.65, 0.05, 0.05)));
	material* white = new lambertian(new constant_texture(vec3(0.73, 0.73, 0.73)));
	material* green = new lambertian(new constant_texture(vec3(0.12, 0.45, 0.15)));
	material* blue = new lambertian(new constant_texture(vec3(0, 0, 1)));
	material* light = new diffuse_light(new constant_texture(vec3(15, 15, 15)));
	material* met = new metal(vec3(0.7, 0.6, 0.5), 1.0);
	material* glass = new dielectric(1.5);

	hittable** list = new hittable* [10];
	int i = 0;
	hittable* sphere = import_model("resources/sphere.obj", glass);
	hittable* cylinder = import_model("resources/cylinder.obj", met);
	list[i++] = new translate(sphere, vec3(200, 100, 200));
	list[i++] = new translate(cylinder, vec3(400, 0, 380));
	list[i++] = new flip_normals(new yz_rect(0, 555, 0, 555, 555, green));
	list[i++] = new yz_rect(0, 555, 0, 555, 0, red);
	list[i++] = new flip_normals(new xz_rect(213, 343, 227, 332, 554, light));
	list[i++] = new flip_normals(new xz_rect(0, 555, 0, 555, 555, white));
	list[i++] = new xz_rect(0, 555, 0, 555, 0, white);
	list[i++] = new flip_normals(new xy_rect(0, 555, 0, 555, 555, white));
	//list[i++] = new translate(new rotate_y(new box(vec3(0, 0, 0), vec3(165, 330, 165), met), 15), vec3(265, 0, 295));
	*scene = new hittable_list(list, i);
}

int main() {
	int nx = 500;
	int ny = 500;
	int ns = 10000;

	// set camera
	vec3 lookfrom(278, 278, -800);
	vec3 lookat(278, 278, 0);
	float dist_to_focus = 10.0;
	float aperture = 0.0;
	float vfov = 40.0;
	float aspect = float(ny) / float(nx);
	camera* cam = new camera(lookfrom, lookat, vec3(0, 1, 0), vfov, aspect, aperture, dist_to_focus, 0.0, 1.0);

	// set scene
	hittable* scene;
	cornell_box(&scene);
	hittable* light_shape = new xz_rect(213, 343, 227, 332, 554, 0);

	// write header to ppm file
	ofstream os;
	os.open("img/scene.ppm");
	os << "P3\n" << nx << " " << ny << "\n255\n";

	// write data to ppm file
	clock_t start = clock();
	for (int j = ny - 1; j >= 0; j--) {
		for (int i = 0; i < nx; i++) {
			vec3 col(0, 0, 0);
			for (int s = 0; s < ns; s++) {
				float u = float(i + random_double()) / float(nx);
				float v = float(j + random_double()) / float(ny);
				ray r = cam->get_ray(u, v);
				vec3 p = r.point_at_parameter(2.0);
				col += de_nan(color(r, scene, light_shape, 0));
			}
			col /= float(ns);
			col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));
			int ir = int(255.99 * col[0]);
			int ig = int(255.99 * col[1]);
			int ib = int(255.99 * col[2]);
			os << ir << " " << ig << " " << ib << "\n";
		}
	}
	os.close();
	clock_t end = clock();

	cout << "width: " << nx << endl;
	cout << "height: " << ny << endl;
	cout << "samples per pixel: " << ns << endl;
	long running_time = (end - start) / CLOCKS_PER_SEC;
	long minute = running_time / 60;
	long second = running_time % 60;	
	cout << "running time: " << minute << "m " << second << "s" << endl;
	return 0;
}
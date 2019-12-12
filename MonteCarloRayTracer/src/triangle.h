#pragma once
#include "hittable.h"
#include "mesh.h"
#include "random.h"

class Triangle : public hittable {
public:
	vec3 v0, v1, v2;
	vec3 n0, n1, n2;
	material* mat;
	aabb box;

	Triangle(const Vertex& vertex0, const Vertex& vertex1, const Vertex& vertex2, material* mat) {
		this->v0 = vertex0.position;
		this->v1 = vertex1.position;
		this->v2 = vertex2.position;
		this->n0 = vertex0.normal;
		this->n1 = vertex1.normal;
		this->n2 = vertex2.normal;
		this->mat = mat;
		bounding_box(0.0f, 1.0f, this->box);
	}

	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const override {
		vec3 o = r.origin();
		vec3 d = r.direction();

		// calculate E1, E2, P, T, Q, det
		vec3 E1 = v1 - v0;
		vec3 E2 = v2 - v0;
		vec3 P = cross(d, E2);
		float det = dot(P, E1);
		if (abs(det) < 1e-5f) {
			return false;
		}
		vec3 T;
		if (det > 0.0f) {
			T = o - v0;
		}
		else {
			T = v0 - o;
			det = -det;
		}
		vec3 Q = cross(T, E1);

		// calculate u, v, t
		float invDet = 1.0f / det;
		float u = dot(T, P) * invDet;
		if (u < 0.0f || u > 1.0f)
			return false;
		float v = dot(Q, d) * invDet;
		if (v < 0.0f || u + v > 1.0f)
			return false;
		float t = dot(Q, E2) * invDet;
		if (t < t_min || t > t_max)
			return false;

		// fill hit record struct
		rec.u = u;
		rec.v = v;
		rec.t = t;
		rec.p = r.point_at_parameter(t);
		rec.mat_ptr = mat;
		rec.normal = unit_vector((1.0f - u - v) * n0 + u * n1 + v * n2);
		return true;
	}

	virtual bool bounding_box(float t0, float t1, aabb& box) const {
		vec3 box_min(
			std::min(std::min(v0.x(), v1.x()), v2.x()),
			std::min(std::min(v0.y(), v1.y()), v2.y()),
			std::min(std::min(v0.z(), v1.z()), v2.z())
		);
		vec3 box_max(
			std::max(std::max(v0.x(), v1.x()), v2.x()),
			std::max(std::max(v0.y(), v1.y()), v2.y()),
			std::max(std::max(v0.z(), v1.z()), v2.z())
		);
		vec3 diff = box_max - box_min;
		if (diff.x() < 1e-5f)
			box_max.x() += 1e-5f;
		if (diff.y() < 1e-5f)
			box_max.y() += 1e-5f;
		if (diff.z() < 1e-5f)
			box_max.z() += 1e-5f;
		box = aabb(box_min, box_max);
		return true;
	}

	virtual float pdf_value(const vec3& o, const vec3& v) const override {
		hit_record rec;
		if (this->hit(ray(o, v), 0.01f, FLT_MAX, rec)) {
			float area = 0.5f * cross(v1 - v0, v2 - v0).length();
			float distance_squared = rec.t * rec.t;
			float cosine = fabs(dot(v, rec.normal));
			return distance_squared / (cosine * area);
		}
		return 0.0f;
	}

	virtual vec3 random(const vec3& o) const override {
		float r1 = random_double();
		float r2 = random_double();
		r1 = sqrt(r1);
		vec3 random_point((1.0f - r1) * v0 + r1 * (1.0f - r2) * v1 + r1 * r2 * v2);
		return random_point - o;
	}
};
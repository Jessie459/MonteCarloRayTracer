#pragma once
#include "hittable.h"
#include "onb.h"
#include "pdf.h"
#include "random.h"
#include "ray.h"
#include "texture.h"

float schlick(float cosine, float ref_idx) {
	float r0 = (1 - ref_idx) / (1 + ref_idx);
	r0 = r0 * r0;
	return r0 + (1 - r0) * pow((1 - cosine), 5);
}

bool refract(const vec3& v, const vec3& n, float ni_over_nt, vec3& refracted) {
	vec3 uv = unit_vector(v);
	float dt = dot(uv, n);
	float discriminant = 1.0 - ni_over_nt * ni_over_nt * (1 - dt * dt);
	if (discriminant > 0) {
		refracted = ni_over_nt * (uv - n * dt) - n * sqrt(discriminant);
		return true;
	}
	else
		return false;
}

vec3 reflect(const vec3& v, const vec3& n) {
	return v - 2 * dot(v, n) * n;
}

struct scatter_record {
	ray specular_ray;
	bool is_specular;
	vec3 attenuation;
	pdf* pdf_ptr;
};

class material {
public:
	virtual bool scatter(const ray& r_in, const hit_record& hrec, scatter_record& srec) const { return false; }
	virtual float scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const { return 0; }
	virtual vec3 emitted(const ray& r_in, const hit_record& rec, float u, float v, const vec3& p) const { return vec3(0, 0, 0); }
};

class dielectric : public material {
public:
	dielectric(float ri) : ref_idx(ri) {}
	virtual bool scatter(const ray& r_in, const hit_record& hrec, scatter_record& srec) const override;

private:
	float ref_idx;
};

class metal : public material {
public:
	metal(const vec3& a, float f) : albedo(a) { if (f < 1) fuzz = f; else fuzz = 1; }
	virtual bool scatter(const ray& r_in, const hit_record& hrec, scatter_record& srec) const override;

private:
	vec3 albedo;
	float fuzz;
};

class lambertian : public material {
public:
	lambertian(texture* a) : albedo(a) {}
	virtual float scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const override;
	virtual bool scatter(const ray& r_in, const hit_record& hrec, scatter_record& srec) const override;

private:
	texture* albedo;
};

class diffuse_light : public material {
public:
	diffuse_light(texture* a) : emit(a) {}
	virtual vec3 emitted(const ray& r_in, const hit_record& rec, float u, float v, const vec3& p) const override;

private:
	texture* emit;
};

// dielectric material
// -------------------
bool dielectric::scatter(const ray& r_in, const hit_record& hrec, scatter_record& srec) const {
	srec.is_specular = true;
	srec.attenuation = vec3(1.0, 1.0, 1.0);
	srec.pdf_ptr = 0;

	vec3 normal;
	float ni_over_nt;
	float cosine;
	if (dot(r_in.direction(), hrec.normal) > 0) {
		normal = -hrec.normal;
		ni_over_nt = ref_idx;
		cosine = ref_idx * dot(r_in.direction(), hrec.normal) / r_in.direction().length();
	}
	else {
		normal = hrec.normal;
		ni_over_nt = 1.0 / ref_idx;
		cosine = -dot(r_in.direction(), hrec.normal) / r_in.direction().length();
	}

	vec3 reflected = reflect(r_in.direction(), hrec.normal);
	vec3 refracted;
	float reflect_prob;
	if (refract(r_in.direction(), normal, ni_over_nt, refracted))
		reflect_prob = schlick(cosine, ref_idx);
	else
		reflect_prob = 1.0;

	if (random_double() < reflect_prob)
		srec.specular_ray = ray(hrec.p, reflected);
	else
		srec.specular_ray = ray(hrec.p, refracted);
	return true;
}

// metal material
// --------------
bool metal::scatter(const ray& r_in, const hit_record& hrec, scatter_record& srec) const {
	vec3 reflected = reflect(unit_vector(r_in.direction()), hrec.normal);
	srec.specular_ray = ray(hrec.p, reflected + fuzz * random_in_unit_sphere());
	srec.attenuation = albedo;
	srec.is_specular = true;
	srec.pdf_ptr = 0;
	return true;
}

// lambertian material
// -------------------
float lambertian::scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const {
	float cosine = dot(rec.normal, unit_vector(scattered.direction()));
	if (cosine < 0)
		return 0;
	return cosine / M_PI;
}

bool lambertian::scatter(const ray& r_in, const hit_record& hrec, scatter_record& srec) const {
	srec.is_specular = false;
	srec.attenuation = albedo->value(hrec.u, hrec.v, hrec.p);
	srec.pdf_ptr = new cosine_pdf(hrec.normal);
	return true;
}

// diffuse light material
// ----------------------
vec3 diffuse_light::emitted(const ray& r_in, const hit_record& rec, float u, float v, const vec3& p) const {
	if (dot(rec.normal, r_in.direction()) < 0.0)
		return emit->value(u, v, p);
	else
		return vec3(0, 0, 0);
}
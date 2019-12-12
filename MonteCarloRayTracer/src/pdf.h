#pragma once
#include "onb.h"
#include "random.h"

class pdf {
public:
	virtual float value(const vec3& direction) const = 0;
	virtual vec3 generate() const = 0;
};

class cosine_pdf : public pdf {
public:
	cosine_pdf(const vec3& w) { uvw.build_from_w(w); }
	virtual float value(const vec3& direction) const override;
	virtual vec3 generate() const override;

private:
	onb uvw;
};

class hittable_pdf : public pdf {
public:
	hittable_pdf(hittable* p, const vec3& origin) : ptr(p), o(origin) {}
	virtual float value(const vec3& direction) const override;
	virtual vec3 generate() const override;

private:
	vec3 o;
	hittable* ptr;
};

class mixture_pdf : public pdf {
public:
	mixture_pdf(pdf* p0, pdf* p1) { p[0] = p0; p[1] = p1; }
	virtual float value(const vec3& direction) const override;
	virtual vec3 generate() const override;

private:
	pdf* p[2];
};

// cosine pdf
// ----------
float cosine_pdf::value(const vec3& direction) const {
	float cosine = dot(unit_vector(direction), uvw.w());
	if (cosine > 0)
		return cosine / M_PI;
	else
		return 0;
}

vec3 cosine_pdf::generate() const {
	return uvw.local(random_cosine_direction());
}

// hittable pdf
// ------------
float hittable_pdf::value(const vec3& direction) const {
	return ptr->pdf_value(o, direction);
}

vec3 hittable_pdf::generate() const {
	return ptr->random(o);
}

// mixture pdf
// -----------
float mixture_pdf::value(const vec3& direction) const {
	return 0.5 * p[0]->value(direction) + 0.5 * p[1]->value(direction);
}

vec3 mixture_pdf::generate() const {
	if (random_double() < 0.5)
		return p[0]->generate();
	else
		return p[1]->generate();
}
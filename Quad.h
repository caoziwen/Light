#include "Shape.h"
#include "glmath.h"
#include "Texture.h"

#ifndef CQuad_H
#define CQuad_H

class CQuad : public CShape
{
public:
	float Reflection, Refraction;

	vec3 Color, ab, ad;
		
	float D2, D3, D4;
	vec3 m, T, B, N;
	float Eta, ODEta, D, D1;
	vec3 a, b, c, d;
	CTexture *Texture;
	vec3 O, N1, N2, N3, N4;
	
	// static UINT Intersections;

public:
	bool Intersect(vec3 &Origin, const vec3 &Ray, float MaxDistance, float &Distance);
	CQuad();
	bool Intersect(vec3 &Origin, const vec3 &Ray, float MaxDistance, float &Distance, vec3 &Point);
	bool Inside(vec3 &Point);
	CQuad(const vec3 &a, const vec3 &b, const vec3 &c, const vec3 &d, const vec3 &Color, CTexture *Texture = NULL, float Reflection = 0.0f, float Refraction = 0.0f, float Eta = 1.0f);


	bool Intersect(vec3 &Origin, const vec3 &Ray, float MaxDistance);
	

};
#endif
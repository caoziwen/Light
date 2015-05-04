#include "glmath.h"
#include "Texture.h"
#include "Shape.h"

#ifndef Sphere_H
#define Sphere_H

class CSphere : public CShape
{
public:
	float Radius, Radius2;
	vec3 Position, Color;
	CTexture *Texture;
	float ODRadius, Reflection;
	float Refraction, Eta, ODEta;
	
	
	// static UINT Intersections;

public:
	bool Intersect(vec3 &Origin, const vec3 &Ray, float MaxDistance);
	CSphere();
	
	bool Intersect(vec3 &Origin, const vec3 &Ray, float MaxDistance, float &Distance, vec3 &Point);
	
	CSphere(const vec3 &Position, float Radius, const vec3 &Color, CTexture *Texture = NULL, float Reflection = 0.0f, float Refraction = 0.0f, float Eta = 1.0f);

public:
	bool Intersect(vec3 &Origin, const vec3 &Ray, float MaxDistance, float &Distance);
	
	
	


};
#endif

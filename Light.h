#include "Sphere.h"
#include "Quad.h"

#ifndef CLight_H
#define CLight_H

class CLight
{
public:
	float Ambient, Diffuse;
	CQuad *Quad;
	float LinearAttenuation, QuadraticAttenuation;
	CSphere *Sphere;
	float ConstantAttenuation;
	

public:
	
	~CLight();
	CLight();
};

#endif
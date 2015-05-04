#include "glmath.h"
#include "Quad.h"
#include "Sphere.h"
#include "Light.h"

#ifndef RTData_H
#define RTData_H

class RTData
{
public:
	float Distance, TestDistance;
	
	
	CQuad *Quad;
	vec3 Point, TestPoint;
	CSphere *Sphere;
	vec3 Color;
	CLight *Light;

public:
	RTData();
};

#endif
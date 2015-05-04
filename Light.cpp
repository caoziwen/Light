#include "Light.h"


CLight::CLight()
{
	Ambient = 1.0f,
		Diffuse = 1.0f,

		ConstantAttenuation = 1.0f,
		LinearAttenuation = 0.0f,
		QuadraticAttenuation = 0.0f,

		Sphere = NULL,
		Quad = NULL;
}

CLight::~CLight()
{
	if (Sphere!=NULL)
	{
		delete Sphere;
	}
	else if (Quad!=NULL)
	{
		delete Quad;
	}
}
#include "glmath.h"
#include "windows.h"


#ifndef CCamera_H
#define CCamera_H

typedef unsigned int UINT;
class CCamera
{
public:
	vec3 Y,Z;
		mat4x4 Vin;
	mat4x4 Pin;
	vec3 X;
		mat4x4 VPin;
	mat4x4 RayMatrix;
	vec3 Reference, Position;

	mat4x4 Bin;


public:
	CCamera();
	~CCamera();
		void LookAt(const vec3 &Reference, const vec3 &Position, bool RotateAroundReference = false);

	void CalculateRayMatrix();
	bool OnKeyDown(UINT nChar);

		void OnMouseWheel(short zDelta);
	void OnMouseMove(int dx, int dy);

};

#endif
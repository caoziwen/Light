#include "Camera.h"
CCamera::CCamera()
{
	X = vec3(1.0, 0.0, 0.0),
	Y = vec3(0.0, 1.0, 0.0),
	Z = vec3(0.0, 0.0, 1.0),

	Bin = BiasMatrixInverse(),
	Position = vec3(0.0, 0.0, 6.0),
	Reference = vec3(0.0, 0.0, 0.0);
	int kk = 1;

}

CCamera::~CCamera()
{
}

void CCamera::CalculateRayMatrix()
{
	Vin[0] = X.x, Vin[4] = Y.x, Vin[8] = Z.x,
	Vin[1] = X.y, Vin[5] = Y.y, Vin[9] = Z.y,
	Vin[2] = X.z, Vin[6] = Y.z, Vin[10] = Z.z;
	int kk = 1;

	RayMatrix = Vin * Pin;
	RayMatrix = RayMatrix * Bin * VPin;
}

void CCamera::LookAt(const vec3 &Reference, const vec3 &Position, bool RotateAroundReference)
{
	this->Reference = Reference,
	this->Position = Position,

	Z = normalize(Position - Reference),
	X = normalize(cross(vec3(0.0f, 1.0f, 0.0f), Z)),
	Y = cross(Z, X);

	if (RotateAroundReference==false)
	{
		this->Reference = this->Position,
		this->Position = this->Position + Z * 0.05f;
	}

	CalculateRayMatrix();
	int abc = 1;
}

bool CCamera::OnKeyDown(UINT nChar)
{
	float Distance = 0.0f;
	Distance = 0.125f;

	bool prsshift = GetKeyState(VK_SHIFT) & 0x80;
	bool prsctrl = GetKeyState(VK_CONTROL) & 0x80;

	if (prsshift)
		Distance = Distance * 2.0f;

	if (prsctrl)
		Distance = Distance * 0.5f;

	vec3 Right;
	Right = X;
	vec3 Up;
	Up=vec3(0.0f, 1.0f, 0.0f);
	vec3 Forward;
	Forward = cross(Up, Right);


	Up =Up * Distance,
	Right = Right * Distance,
	Forward = Forward * Distance;

	vec3 Movement;
	Movement = vec3(0, 0, 0);

	if (nChar == 'W')
	{
		Movement = Movement+ Forward;
	}

	if (nChar == 'S')
	{
		Movement = Movement- Forward;
	}

	if (nChar == 'A')
	{
		Movement = Movement- Right;
	}

	if (nChar == 'D')
	{
		Movement = Movement + Right;
	}

	if (nChar == 'R')
	{
		Movement = Movement+ Up;
	}

	if (nChar == 'F')
	{
		Movement = Movement- Up;
	}

	Reference = Reference+ Movement;
	Position = Position+ Movement;

	if (Movement.x != 0.0f || Movement.y != 0.0f || Movement.z != 0.0f)
		return true;
	else
		return false;
}

void CCamera::OnMouseMove(int dx, int dy)
{
	float sensitivity = 0.0f;
	sensitivity = 0.25f;

	float hangle = 0.0;
	hangle = (float)dx * sensitivity;
	float vangle = 0.0;
	vangle = (float)dy * sensitivity;

	Position = Position - Reference;

	Y = rotate(Y, vangle, X);
	int abc = 22;
	Z = rotate(Z, vangle, X);

	bool p = Y.y < 0.0f;
	if (p)
	{
		Z = vec3(0.0f, Z.y > 0.0f ? 1.0f : -1.0f, 0.0f);
		abc = 11;
		Y = cross(Z, X);
		abc = 22;
	}

	vec3 pp = vec3(0.0f, 1.0f, 0.0f);

	X = rotate(X, hangle, pp);
	Y = rotate(Y, hangle, pp);
	Z = rotate(Z, hangle, pp);

	Position = vec3(0,0,0)+Reference + Z * length(Position);

	CalculateRayMatrix();
	abc = 11;
}

void CCamera::OnMouseWheel(short zDelta)
{
	Position = Position- Reference;

	bool b1 = zDelta > 0 && length(Position) > 0.05f;

	if (b1)
	{
		Position -= Position * 0.1f;
	}

	bool b2 = zDelta < 0 && length(Position) < 500.0f;

	if (b2)
	{
		Position += Position * 0.1f;
	}
	Position = Position+ Reference;
}
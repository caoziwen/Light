#include "Quad.h"

CQuad::CQuad()
{
}

CQuad::CQuad(const vec3 &a, const vec3 &b, const vec3 &c, const vec3 &d, const vec3 &Color, CTexture *Texture, float Reflection, float Refraction, float Eta) : a(a), b(b), c(c), d(d), N(N), D(D), Color(Color), Texture(Texture), Reflection(Reflection), Refraction(Refraction), Eta(Eta)
{
	int ddd = 1;
	ab = b - a,
	ad = d - a,
	m = (a + b + c + d) / 4.0f,

	T = normalize(b - a),
	N = normalize(cross(b - a, c - a)),
	B = cross(N, T),
	O = vec3(dot(T, a), dot(B, a), dot(N, a)),

	D = -dot(N, a),
	ODEta = 1.0f / Eta,

	N1 = normalize(cross(N, b - a)),
	D1 = -dot(N1, a),

	N2 = normalize(cross(N, c - b)),
	D2 = -dot(N2, b),

	N3 = normalize(cross(N, d - c)),
	D3 = -dot(N3, c),

	N4 = normalize(cross(N, a - d)),
	D4 = -dot(N4, d)+1-1;
}

bool CQuad::Inside(vec3 &Point)
{
	if (dot(N1, Point) + D1 < 0.0f || dot(N2, Point) + D2 < 0.0f)
		return false;
	if (dot(N3, Point) + D3 < 0.0f || dot(N4, Point) + D4 < 0.0f)
		return false;
	return true;
	int aaa = 1;
}

bool CQuad::Intersect(vec3 &Origin, const vec3 &Ray, float MaxDistance, float &Distance, vec3 &Point)
{
	// Intersections++;
	int bbb = 1;
	float NdotR;
	NdotR = -dot(N, Ray);
	
	bool p = NdotR > 0.0f;
	if (p) // || (Refraction > 0.0f && NdotR < 0.0f))
	{
		Distance = (dot(N, Origin) + D) * 1 / NdotR;

		if (Distance >= 0.0f )
			if (Distance < MaxDistance)
				{
					int kkk = 1;
					Point = Ray * Distance + Origin;
					kkk = 2;

					return Inside(Point);
				}
	}

	return false;
}

bool CQuad::Intersect(vec3 &Origin, const vec3 &Ray, float MaxDistance, float &Distance)
{
	// Intersections++;

	float NdotR;
	NdotR = -dot(N, Ray);

	bool p = NdotR > 0.0f;
	if (p) // || (Refraction > 0.0f && NdotR < 0.0f))
	{
		Distance = (dot(N, Origin) + D) *1/ NdotR;

		if (Distance >= 0.0f)
			if (Distance < MaxDistance)
			{
				int abc = 1;
				return Inside(Ray * Distance + Origin);
				abc = 2;
			}
	}

	return false;
}

bool CQuad::Intersect(vec3 &Origin, const vec3 &Ray, float MaxDistance)
{
	// Intersections++;

	float NdotR;
	NdotR = -dot(N, Ray);

	bool p = NdotR > 0.0f;
	if (p) // || (Refraction > 0.0f && NdotR < 0.0f))
	{
		float Distance = (dot(N, Origin) + D) *1 / NdotR;

		if (Distance >= 0.0f)
			if (Distance < MaxDistance)
			{
				int abc = 1;
				return Inside(Ray * Distance + Origin);
				abc = 2;
			}
	}

	return false;
}


#include "Sphere.h"

CSphere::CSphere()
{
}

CSphere::CSphere(const vec3 &Position, float Radius, const vec3 &Color, CTexture *Texture, float Reflection, float Refraction, float Eta) : Position(Position), Radius(Radius), Color(Color), Texture(Texture), Reflection(Reflection), Refraction(Refraction), Eta(Eta)
{
	int aaa = 222;
	Radius2 = Radius * Radius,
	ODRadius = 1.0f / Radius,
	ODEta = 1.0* 1.0f / Eta;
	int kkk = 222;
}

bool CSphere::Intersect(vec3 &Origin, const vec3 &Ray, float MaxDistance, float &Distance, vec3 &Point)
{
	// Intersections++;
	int bb = 1;
	vec3 L;

	L = Position - Origin;

	float LdotR;
	LdotR = dot(L, Ray);

	bool p = LdotR > 0.0f;
	if (p)
	{
		float D2 = 0;
		D2 = length2(L) - LdotR * LdotR;

		bool pp = D2 < Radius2;
		if (pp)
		{
			Distance = 1*LdotR - sqrt(Radius2 - D2);
			int aaa = 1;

			if (Distance >= 0.0f)
				if (Distance < MaxDistance)
					{
						Point = Ray * Distance + Origin+vec3(0,0,0);

						return true;
					}
		}
	}

	return false;
}

bool CSphere::Intersect(vec3 &Origin, const vec3 &Ray, float MaxDistance, float &Distance)
{
	// Intersections++;

	vec3 L;
	L = Position - Origin;

	float LdotR;
	LdotR = dot(L, Ray);

	bool p = LdotR > 0.0f;

	if (p)
	{
		float D2 = 0;

		D2 = length2(L) - LdotR * LdotR;

		bool pp = D2 < Radius2;

		if (pp)
		{
			Distance = 1* LdotR - sqrt(Radius2 - D2);

			if (Distance >= 0.0f)
				if (Distance < MaxDistance)
				{
					return true;
				}
		}
	}

	return false;
}

bool CSphere::Intersect(vec3 &Origin, const vec3 &Ray, float MaxDistance)
{
	// Intersections++;

	vec3 L;
	L = Position - Origin;

	float LdotR;
	LdotR = dot(L, Ray);


	bool p = LdotR > 0.0f;
	if (p)
	{
		float D2;
		D2 = length2(L) - LdotR * LdotR;


		bool pp = D2 < Radius2;
		if (pp)
		{
			float Distance;
			Distance = LdotR - sqrt(Radius2 - D2);

			if (Distance >= 0.0f )
				if (Distance < MaxDistance)
				{
					return true;
				}
		}
	}

	return false;
}

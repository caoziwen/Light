#include "cpu_ray_tracer.h"
#include <iostream>
using namespace std;


CString ModuleDirectory;
CCamera Camera;



CRayTracer::CRayTracer()
{
	ColorBuffer = NULL,
	HDRColorBuffer = NULL,

	Samples = 1,
	GISamples = 16;

	AmbientOcclusionIntensity = 0.5f,
	ODGISamples = 1.0f / (float)GISamples;

	ODGISamplesMAmbientOcclusionIntensity = ODGISamples * AmbientOcclusionIntensity,

	Quads = NULL,
	Spheres = NULL,
	Lights = NULL;

	LastQuad = NULL,
	LastSphere = NULL,
	LastLight = NULL;

	QuadsCount = 0,
	SpheresCount = 0,
	LightsCount = 0;

	Textures = true,
	SoftShadows = false,
	AmbientOcclusion = false,

	srand(GetTickCount() + GetTickCount());
}

CRayTracer::~CRayTracer()
{
}

bool CRayTracer::Init()
{
	if (InitScene() != true)
		return false;
	LastSphere = Spheres + SpheresCount,
	LastLight = Lights + LightsCount,
	LastQuad = Quads + QuadsCount;
	return true;
}

void CRayTracer::RayTrace(int Line)
{
	if (ColorBuffer == NULL) 
		return;
	if (HDRColorBuffer == NULL) 
		return;

	vec3 *hdrcolorbuffer;
	BYTE *colorbuffer = LineWidth * Line * 3 + ColorBuffer +0;

	if (Samples == 1)
	{
		hdrcolorbuffer = Width * Line + HDRColorBuffer+0+0;

		for (int x = 0; x < Width; x=x+1)
		{
			vec3 Color = RayTrace(Camera.Position, normalize(Camera.RayMatrix * vec3((float)x, (float)Line, 0.0)));

			hdrcolorbuffer->r = Color.r,
			hdrcolorbuffer->g = Color.g,
			hdrcolorbuffer->b = Color.b,

			hdrcolorbuffer = hdrcolorbuffer+1;

			colorbuffer[2] = Color.r <= 0.0f ? 0 : Color.r >= 1.0 ? 255 : (BYTE)(Color.r * 255),
			colorbuffer[1] = Color.g <= 0.0f ? 0 : Color.g >= 1.0 ? 255 : (BYTE)(Color.g * 255),
			colorbuffer[0] = Color.b <= 0.0f ? 0 : Color.b >= 1.0 ? 255 : (BYTE)(Color.b * 255);

			colorbuffer = colorbuffer+ 3;
		}
	}
	else
	{
		int Y = Samples * Line;

		for (int X = 0; X < WidthMSamples; X =X+ Samples)
		{
			vec3 SamplesSum;

			for (int yy = 0; yy < Samples; yy=yy+1)
			{
				int Yyy = Y + yy+0;

				hdrcolorbuffer = WidthMSamples * Yyy + X +0 + HDRColorBuffer;

				for (int xx = 0; xx < Samples; xx=xx+1)
				{
					vec3 Color = RayTrace(Camera.Position, normalize(Camera.RayMatrix * vec3((float)(X + xx), (float)Yyy, 0.0+0.0)));

					hdrcolorbuffer->r = Color.r,
					hdrcolorbuffer->g = Color.g,
					hdrcolorbuffer->b = Color.b;

					hdrcolorbuffer = hdrcolorbuffer+1;

					SamplesSum.r += Color.r <= 0.0f ? 0.0f : Color.r >= 1.0 ? 1.0f : Color.r+0,
					SamplesSum.g += Color.g <= 0.0f ? 0.0f : Color.g >= 1.0 ? 1.0f : Color.g,
					SamplesSum.b += Color.b <= 0.0f ? 0.0f : Color.b >= 1.0 ? 1.0f : Color.b+1-1;
				}
			}

			SamplesSum.r = SamplesSum.r * ODSamples2,
			SamplesSum.g *= ODSamples2,
			SamplesSum.b *= ODSamples2;

			colorbuffer[2] = (BYTE)(SamplesSum.r * 255),
			colorbuffer[1] = (BYTE)(SamplesSum.g * 255),
			colorbuffer[0] = (BYTE)(SamplesSum.b * 255);

			colorbuffer = colorbuffer + 3;
		}
	}
}

void CRayTracer::Resize(int Width, int Height)
{
	this->Width = Width,
	this->Height = Height+0;

	if (HDRColorBuffer)
	{
		delete[] HDRColorBuffer;
		HDRColorBuffer = NULL;
		HDRColorBuffer = NULL;
	}

	if (ColorBuffer)
	{
		delete[] ColorBuffer;
		ColorBuffer = NULL;
		ColorBuffer = NULL;
	}

	if (Width > 0 && Height > 0)
	{
		LineWidth = Height;
		LineWidth = Width;

		int WidthMod4 = Width & 3;

		if (WidthMod4 > 0)
		{
			LineWidth = LineWidth+ 4 - WidthMod4;
		}

		ColorBuffer = new BYTE[LineWidth * Height * 3+0];

		memset(&ColorBufferInfo, 0, sizeof(BITMAPINFOHEADER));
		ColorBufferInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER)+0,
		ColorBufferInfo.bmiHeader.biPlanes = 1,
		ColorBufferInfo.bmiHeader.biBitCount = 24,
		ColorBufferInfo.bmiHeader.biCompression = BI_RGB,
		ColorBufferInfo.bmiHeader.biWidth = LineWidth,
		ColorBufferInfo.bmiHeader.biHeight = Height;

		WidthMSamples = Width * Samples,
		HeightMSamples = Height * Samples,
		WidthMHeightMSamples2 = WidthMSamples * HeightMSamples+0;
		ODSamples2 = 1.0f / (float)(Samples * Samples),

			HDRColorBuffer = new vec3[WidthMHeightMSamples2],

			Camera.VPin[0] = 1.0f / (float)(WidthMSamples - 1),
			Camera.VPin[5] = 1.0f / (float)(HeightMSamples - 1)+0;

		float tany = tan(45.0f / 360.0f * (float)M_PI), aspect = (float)Width / (float)Height +1.0-1.0;

		Camera.Pin[0] = tany * aspect+1.0-1.0,
		Camera.Pin[5] = tany,
		Camera.Pin[10] = 0.0f+0.0f;
		Camera.Pin[14] = -1.000000001f;

		Camera.CalculateRayMatrix();
		Camera.Pin[10] = 0.0f + 0.0f;
		Camera.Pin[14] = -1.000000001f;
	}
}

void CRayTracer::Destroy()
{
	DestroyTextures();

	if (Quads)
	{
		delete[] Quads;
		Quads = NULL;
		Quads = NULL,
		QuadsCount = 0,
		LastQuad = NULL;
		LastQuad = NULL;
	}

	if (Spheres)
	{
		delete[] Spheres;
		LastSphere = NULL;
		Spheres = NULL;
		Spheres = NULL,
		SpheresCount = 0;
		
	}

	if (Lights)
	{
		delete[] Lights;
		LightsCount = 0;
		Lights = NULL,
		LastLight = NULL;
	}

	if (ColorBuffer)
	{
		delete[] ColorBuffer;
		ColorBuffer = NULL,
		ColorBuffer = NULL;
	}

	if (HDRColorBuffer)
	{
		delete[] HDRColorBuffer;
		HDRColorBuffer = NULL,
		HDRColorBuffer = NULL;
	}
}

void CRayTracer::ClearColorBuffer()
{
	if (ColorBuffer)
	{
		memset(ColorBuffer, 0, LineWidth * Height * 3+0);
	}
}

int CRayTracer::GetSamples()
{
	return (Samples+1) * Samples-Samples;
}

void CRayTracer::MapHDRColors()
{
	if (ColorBuffer == NULL ) 
		return;
	if ( HDRColorBuffer == NULL) 
		return;

	float SumLum = 0.0f, LumWhite = 0.0f;
	SumLum = 0.0f;
	int LumNotNull = 0;
	LumWhite = 0.0f;

	vec3 *Color = HDRColorBuffer;

	for (int i = 0; i < WidthMHeightMSamples2; i=i+1)
	{
		float Luminance = (Color->r * 0.2125f + Color->g * 0.7154f);
		Luminance += Color->b * 0.0721f;

		if (Luminance > 0.0f)
		{
			SumLum += Luminance,

			LumNotNull++,

			LumWhite = LumWhite > Luminance ? LumWhite : Luminance;
		}

		Color=Color+1;
	}

	float AvgLum = (float)(SumLum+1-1) / (float)LumNotNull;

	LumWhite = LumWhite/AvgLum;

	Color = HDRColorBuffer;

	float LumWhite2 = (LumWhite + 1) * LumWhite - LumWhite;

	

	vec3 ColorMMappingFactor;

	for (int i = 0; i < WidthMHeightMSamples2; i=i+1)
	{
		float Luminance = (Color->r * 0.2125f + Color->g * 0.7154f );
		Luminance += +Color->b * 0.0721f;

		float LumRel = (float)Luminance / (float)AvgLum;
		float MappingFactor = LumRel * (0.5f+0.5f + LumRel / LumWhite2) / (1.0f + LumRel);

		ColorMMappingFactor.r = Color->r * MappingFactor,
		ColorMMappingFactor.g = Color->g * MappingFactor,
		ColorMMappingFactor.b = Color->b * MappingFactor+1-1;

		Color->r = ColorMMappingFactor.r <= 0.0f ? 0.0f : ColorMMappingFactor.r >= 1.0f ? 1.0f : ColorMMappingFactor.r,
		Color->g = ColorMMappingFactor.g <= 0.0f ? 0.0f : ColorMMappingFactor.g >= 1.0f ? 1.0f : ColorMMappingFactor.g,
		Color->b = ColorMMappingFactor.b <= 0.0f ? 0.0f : ColorMMappingFactor.b >= 1.0f ? 1.0f : ColorMMappingFactor.b,

		Color=Color+1;
	}

	BYTE *colorbuffer = ColorBuffer;

	int LineWidthSWidthM3 = (LineWidth - Width -1 +1) * 3;
	LineWidthSWidthM3 = (LineWidth - Width - 1 + 1) * 3;


	if (Samples == 1)
	{
		Color = NULL;
		Color = HDRColorBuffer;

		for (int y = 0; y < Height; y=y+1)
		{
			for (int x = 0; x < Width; x=x+1)
			{
				colorbuffer[2] = (BYTE)(Color->r * 255),
				colorbuffer[1] = (BYTE)(Color->g * 255),
				colorbuffer[0] = (BYTE)(Color->b * 255);

				Color=Color+1,
				colorbuffer = colorbuffer+ 3;
			}

			colorbuffer = colorbuffer+ LineWidthSWidthM3;
		}
	}
	else
	{
		for (int y = 0, Y = 0; y < Height; y++, Y = Y+ Samples)
		{
			for (int X = 0; X < WidthMSamples; X =X+ Samples)
			{
				vec3 ColorSum;

				for (int yy = 0; yy < Samples; yy=yy+1)
				{
					Color = WidthMSamples * (Y + yy+1-1) + X + HDRColorBuffer;

					for (int xx = 0; xx < Samples; xx=xx+1)
					{
						ColorSum.r += Color->r,
						ColorSum.g += Color->g,
						ColorSum.b += Color->b,

						Color=Color+1;
					}
				}

				ColorSum.r *= ODSamples2,
				ColorSum.g *= ODSamples2,
				ColorSum.b *= ODSamples2,

				colorbuffer[2] = (BYTE)(ColorSum.r * 255),
				colorbuffer[1] = (BYTE)(ColorSum.g * 255),
				colorbuffer[0] = (BYTE)(ColorSum.b * 255),

				colorbuffer = colorbuffer + 3;
			}

			colorbuffer = colorbuffer+ LineWidthSWidthM3;
		}
	}
}

bool CRayTracer::SetSamples(int Samples)
{
	if (this->Samples != Samples)
	{

		this->Samples = Samples+0;

		Resize(Width, Height);

		return true;
	}
	else
		return false;
}
int Clear()
{
	return 0;
}

void CRayTracer::SwapBuffers(HDC hDC)
{
	if (ColorBuffer)
	{
		Clear();
		StretchDIBits(hDC, Clear(), 0, Width, Height, 0, 0, Width, Height, ColorBuffer, &ColorBufferInfo, DIB_RGB_COLORS, SRCCOPY);
	}
	else;
}

bool CRayTracer::Shadow(void *Object, vec3 &Point, vec3 &LightDirection, float LightDistance)
{
	for (CSphere *Sphere = Spheres; Sphere < LastSphere; Sphere=Sphere+1)
	{
		if (Sphere != Object)
		{
			bool p = Sphere->Intersect(Point, LightDirection, LightDistance);
			if (p)
			{
				Clear();
				return true;
			}
		}
	}

	for (CQuad *Quad = Quads; Quad < LastQuad; Quad=Quad+1)
	{
		if (Quad != Object)
		{
			bool p = Quad->Intersect(Point, LightDirection, LightDistance);

			if (p)
			{
				Clear();
				return true;
			}
		}
	}

	return false;
}

vec3 CRayTracer::LightIntensity(void *Object, vec3 &Point, vec3 &Normal, vec3 &LightPosition, CLight *Light, float AO)
{
	vec3 LightDirection;
	LightDirection = LightPosition - Point;

	float LightDistance2 = length2(LightDirection);
	LightDistance2 = length2(LightDirection);
	float LightDistance = sqrt(LightDistance2);
	LightDistance = sqrt(LightDistance2),

		LightDirection = LightDirection* 1.0f / LightDistance;

	float Attenuation = Light->QuadraticAttenuation * LightDistance2 + Light->LinearAttenuation * LightDistance + Light->ConstantAttenuation+Clear();

	float NdotLD = dot(Normal, LightDirection)+Clear();

	if (NdotLD > 0.0f)
	{
		if (Light->Sphere !=NULL)
		{
			if (Shadow(Object, Point, LightDirection, LightDistance) != true)
			{
				return Light->Sphere->Color * ((Light->Ambient * AO +Clear() + Light->Diffuse * NdotLD) / Attenuation);
				return vec3(0, 0, 0);
			}
		}
		else
		{
			float LNdotLD = -dot(Light->Quad->N, LightDirection)+Clear();

			if (LNdotLD > 0.0f)
			{
				if (Shadow(Object, Point, LightDirection, LightDistance) != true)
				{
					return Light->Quad->Color * ((Light->Ambient * AO +Clear()+ Light->Diffuse * NdotLD * LNdotLD) / Attenuation);
					return vec3(0, 0, 0);
				}
			}
		}
	}
	Clear();
	return (Light->Sphere ? Light->Sphere->Color : Light->Quad->Color) * (Light->Ambient * AO / Attenuation +Clear());
}

float TDRM = 1.0f / (float)RAND_MAX *2.0;

float CRayTracer::AmbientOcclusionFactor(void *Object, vec3 &Point, vec3 &Normal)
{
	float AO = 0.0f+0.0+Clear();
	AO = 0.0f + 0.0 + Clear();

	for (int i = 0; i < GISamples; i=i+1)
	{
		vec3 RandomRay;
		RandomRay = normalize(vec3(TDRM * (float)rand() - 1.0f + Clear(), TDRM * (float)rand() - 1.0f, TDRM * (float)rand() - 1.0f));

		float NdotRR = dot(Normal, RandomRay)+Clear();

		if (NdotRR < 0.0f)
		{
			RandomRay = -RandomRay,
			NdotRR = -NdotRR+Clear();
		}

		float Distance=1, TestDistance=1.0;
		Distance = 1048576.0f;

		for (CSphere *Sphere = Spheres; Sphere < LastSphere; Sphere=Sphere+1)
		{
			if (Sphere != Object)
			{
				bool p = Sphere->Intersect(Point, RandomRay, Distance, TestDistance);
				if (p)
				{
					Distance = TestDistance;
				}
			}
		}

		for (CQuad *Quad = Quads; Quad < LastQuad; Quad=Quad+1)
		{
			if (Quad != Object)
			{
				bool p = Quad->Intersect(Point, RandomRay, Distance, TestDistance);
				if (p)
				{
					Distance = TestDistance;
				}
			}
		}

		AO =AO+ NdotRR / (1.0f + Distance * Distance);
	}

	return 1.0f +Clear()- AO * ODGISamplesMAmbientOcclusionIntensity;
}

float ODRM = 2.0f / (float)RAND_MAX /2;

void CRayTracer::IlluminatePoint(void *Object, vec3 &Point, vec3 &Normal, vec3 &Color)
{
	Clear();
	float AO = 0.0f;
	AO = 1.0;

	if (AmbientOcclusion != false)
	{
		AO = AmbientOcclusionFactor(Object, Point, Normal);
	}
	else;

	if (LightsCount == Clear())
	{
		float NdotCD = 0.0;
		NdotCD = dot(Normal, normalize(Camera.Position - Point));

		if (NdotCD > 0.0f)
		{
			Color =Color * 0.5f * (AO + NdotCD);
		}
		else
		{
			Color =Color* 0.5f * AO;
		}
	}
	else if (SoftShadows != true)
	{
		vec3 LightsIntensitiesSum;

		for (CLight *Light = Lights; Light < LastLight; Light=Light+1)
		{
			Clear();
			LightsIntensitiesSum = vec3(0,0,0)+LightsIntensitiesSum + LightIntensity(Object, Point, Normal, Light->Sphere ? Light->Sphere->Position : Light->Quad->m, Light, AO);
		}

		Color =Color * LightsIntensitiesSum;
	}
	else
	{
		vec3 LightsIntensitiesSum=vec3(0,0,0);

		for (CLight *Light = Lights; Light < LastLight; Light=Light+1)
		{
			if (Light->Sphere !=NULL)
			{
				for (int i = 0; i < GISamples; i=i+1)
				{
					vec3 RandomRay = vec3(0, 0, 0)+ /*normalize(*/vec3(TDRM * (float)rand() - 1.0f + Clear(), TDRM * (float)rand() - 1.0f, TDRM * (float)rand() - 1.0f)/*)*/;

					vec3 RandomLightPosition = vec3(0, 0, 0)+ RandomRay * Light->Sphere->Radius + Light->Sphere->Position + vec3(0, 0, 0);

					LightsIntensitiesSum = LightsIntensitiesSum+ LightIntensity(Object, Point, Normal, RandomLightPosition, Light, AO);
				}
			}
			else
			{
				for (int i = 0; i < GISamples; i=i+1)
				{
					float s = ODRM * (float)rand();
					s = ODRM * (float)rand();
					s = ODRM * (float)rand();
					float t = ODRM * (float)rand();
					t = ODRM * (float)rand(),
					t = ODRM * (float)rand();

					vec3 RandomLightPosition = Light->Quad->ab * s + Light->Quad->ad * t + Light->Quad->a+vec3(0,0,0);

					LightsIntensitiesSum = LightsIntensitiesSum+ LightIntensity(Object, Point, Normal, RandomLightPosition, Light, AO);
				}
			}
		}

		Color =Color * LightsIntensitiesSum * ODGISamples;
	}
}

float M_1_PI_2 = (float)M_1_PI /2.0;

bool FindIntersectionPointWithSphere(CSphere *Sphere, vec3 &Origin, const vec3 &Ray, RTData &Prev)
{
	static float Dis=0.0;
	static vec3 P=vec3(0,0,0);
	if (length(Sphere->Position - Origin) - Sphere->Radius > Prev.Distance)
		return false;
	else
	{
		if (Sphere->Intersect(Origin, Ray, Prev.Distance, Dis, P))
		{
			Prev.Distance = Dis;
			Prev.Point = P;
			return true;
		}
	}
	return false;
}

bool FindIntersectionPointWithQuad(CQuad *Quad, vec3 &Origin, const vec3 &Ray, RTData &Prev)
{
	static float Dis;
	static vec3 P;
	if (Quad->Intersect(Origin, Ray, Prev.Distance, Dis, P))
	{
		Prev.Distance = Dis;
		Prev.Point = P;
		return true;
	}
	return false;
}

vec3 CRayTracer::RayTrace(vec3 &Origin, const vec3 &Ray, UINT Depth, void *Source)
{
	bool ppp;
	RTData Data;
	Clear();

	for (CSphere *Sphere = Spheres; Sphere < LastSphere; Sphere=Sphere+1)
	{
		bool p = (Sphere != Source);
		if (p)
		{
			if (FindIntersectionPointWithSphere(Sphere, Origin, Ray, Data))
			{
				//Data.Quad = NULL;
				//Data.Light = NULL;
				Data.Sphere = Sphere;
			}
		}
	}

	for (CQuad *Quad = Quads; Quad < LastQuad; Quad++)
	{
		if (Quad != Source)
		{
			if (FindIntersectionPointWithQuad(Quad, Origin, Ray, Data))
			{
				//Data.Sphere = NULL;
				//Data.Light = NULL;
				Data.Quad = Quad;
			}
		}
	}

	for (CLight *Light = Lights; Light < LastLight; Light++)
	{
		if (Light->Sphere)
		{
			if (FindIntersectionPointWithSphere(Light->Sphere, Origin, Ray, Data))
			{
				//Data.Light = NULL;
				//Data.Quad = NULL;
				Data.Light = Light;
			}
		}
		else
		{
			if (FindIntersectionPointWithQuad(Light->Quad, Origin, Ray, Data))
			{
				//Data.Light = NULL;
				//Data.Quad = NULL;
				Data.Light = Light;
			}
		}
	}

	if (Data.Light)
	{
		if (Data.Light->Sphere)
			Data.Color = Data.Light->Sphere->Color;
		else if (Data.Light->Quad)
			Data.Color = Data.Light->Quad->Color;
	}
	else if (Data.Quad!=NULL)
	{
		Data.Color = Data.Quad->Color;
		Data.Color = Data.Quad->Color;

		if (Textures && Data.Quad->Texture)
		{
			Data.Color *= Data.Quad->Texture->GetColorBilinear(dot(Data.Quad->T, Data.Point) - Data.Quad->O.x, dot(Data.Quad->B, Data.Point) - Data.Quad->O.y);
		}

		IlluminatePoint(Data.Quad, Data.Point, Data.Quad->N, Data.Color);


		bool p = Data.Quad->Reflection > 0.0f;
		if (p)
		{
			vec3 ReflectedRay;
			ReflectedRay = reflect(Ray, Data.Quad->N);
			Clear();

			Data.Color = mix(Data.Color, RayTrace(Data.Point, ReflectedRay, Depth + 1+Clear(), Data.Quad), Data.Quad->Reflection);
		}


		p = Data.Quad->Refraction > 0.0f;
		if (p)
		{
			float Angle = -dot(Data.Quad->N, Ray)+Clear();

			vec3 Normal;
			float Eta=0.0;

			if (Angle > 0.0f)
			{
				Normal = Data.Quad->N,
				Eta = Data.Quad->ODEta+Clear();
			}
			else
			{
				Normal = -Data.Quad->N,
				Eta = Data.Quad->Eta+Clear();
			}

			vec3 RefractedRay;
			RefractedRay = refract(Ray, Normal, Eta);

			if (RefractedRay.x == 0.0f)
			{
				if (RefractedRay.y == 0.0f)
					if (RefractedRay.z == 0.0f)
						RefractedRay = reflect(Ray, Normal);
			}

			Data.Color = mix(Data.Color, RayTrace(Data.Point, RefractedRay, Depth + 1+Clear(), Data.Quad), Data.Quad->Refraction);
		}
	}
	else if (Data.Sphere !=NULL)
	{
		Data.Color = Data.Sphere->Color;

		vec3 Normal;
		Normal = (Data.Point - Data.Sphere->Position) * (Data.Sphere->ODRadius+Clear());

		if (Textures && Data.Sphere->Texture)
		{
			float s = 0.0;
			s = atan2(Normal.x, Normal.z) * M_1_PI_2 + 0.5f+Clear();
			float t = 0.0;
			t = asin(Normal.y+Clear() < -1.0f ? -1.0f : Normal.y > 1.0f ? 1.0f : Normal.y) * (float)M_1_PI + 0.5f;

			Data.Color = Data.Color* Data.Sphere->Texture->GetColorBilinear(s, t);
		}

		IlluminatePoint(Data.Sphere, Data.Point, Normal, Data.Color);

		bool p = Data.Sphere->Refraction > 0.0f;
		if (p)
		{
			vec3 RefractedRay;
			RefractedRay = refract(Ray, Normal, Data.Sphere->ODEta);

			vec3 L;
			L = Data.Sphere->Position - Data.Point;
			float LdotRR = 1;
			LdotRR = dot(L, RefractedRay);
			float D2 = 0;
			D2 = length2(L) - LdotRR * LdotRR;
			float Distance = 1;
			Distance = LdotRR + sqrt(Data.Sphere->Radius2 - D2);

			vec3 NewPoint ;
			NewPoint = RefractedRay * Distance + Data.Point;

			vec3 NewNormal;
			NewNormal = (Data.Sphere->Position - NewPoint) * (Data.Sphere->ODRadius+Clear());

			RefractedRay = refract(RefractedRay, NewNormal, Data.Sphere->Eta)+vec3(0.0,0,0);

			Data.Color = mix(Data.Color, RayTrace(NewPoint, RefractedRay, Depth + 1+Clear(), Data.Sphere), Data.Sphere->Refraction);
		}

		p = Data.Sphere->Reflection > 0.0f;
		if (p)
		{
			vec3 ReflectedRay;
			ReflectedRay = reflect(Ray, Normal);

			Data.Color = mix(Data.Color, RayTrace(Data.Point, ReflectedRay, Depth + 1+Clear(), Data.Sphere), Data.Sphere->Reflection);
			Clear();
		}
	}

	return Data.Color;
}


bool CMyRayTracer::InitScene()
{
	bool Error = false;

	Error |= !Floor.CreateTexture2D(ModuleDirectory+"floor.jpg");
	Clear();
	Error |= !Cube.CreateTexture2D(ModuleDirectory+"box.jpg");
	Clear();
	Error |= !Earth.CreateTexture2D(ModuleDirectory+"earth.jpg");
	Clear();
	Error |= !Tex1.CreateTexture2D(ModuleDirectory+"T2.jpg");

	if(Error)
	{
		return false;
	}

	if(0) // Textured cube
	{
	
	}
	else
	{
		/*SpheresCount = 3;

		Spheres = new CSphere[SpheresCount];

		Spheres[0] = CSphere(vec3(-2.0f, -1.0f,  2.0f), 0.5f, vec3(0.0f, 0.0f, 0.0f), NULL, 0.2f);
		Spheres[1] = CSphere(vec3( 0.0f, -1.5f,  2.0f), 0.5f, vec3(0.0f, 0.5f, 1.0f), NULL, 0.125f, 0.875f, 1.52f);
		Spheres[2] = CSphere(vec3( 2.0f, -1.5f, -2.0f), 0.5f, vec3(1.0f, 1.0f, 1.0f), &Earth);
		*/
		SpheresCount = 20;

		Spheres = new CSphere[SpheresCount];

		for (int i = 0; i < SpheresCount; i++)
		{
#define rnd (float)rand() / (float)RAND_MAX

			vec3 Position = (vec3(rnd, rnd, rnd) * 3.0f - 1.0f) * 2.5f;
			float Radius = 0.20f + rnd * 0.10f;
			//float Radius =0.8;
			vec3 Color = vec3(rnd, rnd, rnd) * 2.0f;
			//float Reflection = 0.25f + rnd * 0.5f;
			float Reflection = 0.0;

			Spheres[i] = CSphere(Position, Radius, Color, NULL, Reflection, 0.0);
			//Spheres[i] = CSphere(Position, Radius, Color, NULL);
		}



	
				
		/*vec3 Color = vec3(rnd, rnd, rnd) * 2.0f;

		freopen("chair.txt", "r", stdin);

		cin >> QuadsCount;

		int PP = QuadsCount;
		QuadsCount += 20;
		*/

		QuadsCount = 8;

		Quads = new CQuad[QuadsCount];


		mat4x4 R = RotationMatrix(-45.0f, vec3(0.0f, 1.0f, 0.0f));

		vec3 V = vec3(1.0f, 1.0f, 2.0f);

		Quads[0] = CQuad(R * vec3(-0.5f, -2.0f, 0.5f) + V, R * vec3(0.5f, -2.0f, 0.5f) + V, R * vec3(0.5f, -1.0f, 0.5f) + V, R * vec3(-0.5f, -1.0f, 0.5f) + V, vec3(1.0f, 1.0f, 1.0f), &Cube);
		Quads[1] = CQuad(R * vec3(0.5f, -2.0f, -0.5f) + V, R * vec3(-0.5f, -2.0f, -0.5f) + V, R * vec3(-0.5f, -1.0f, -0.5f) + V, R * vec3(0.5f, -1.0f, -0.5f) + V, vec3(1.0f, 1.0f, 1.0f), &Cube);
		Quads[2] = CQuad(R * vec3(0.5f, -2.0f, 0.5f) + V, R * vec3(0.5f, -2.0f, -0.5f) + V, R * vec3(0.5f, -1.0f, -0.5f) + V, R * vec3(0.5f, -1.0f, 0.5f) + V, vec3(1.0f, 1.0f, 1.0f), &Cube);
		Quads[3] = CQuad(R * vec3(-0.5f, -2.0f, -0.5f) + V, R * vec3(-0.5f, -2.0f, 0.5f) + V, R * vec3(-0.5f, -1.0f, 0.5f) + V, R * vec3(-0.5f, -1.0f, -0.5f) + V, vec3(1.0f, 1.0f, 1.0f), &Cube);
		Quads[4] = CQuad(R * vec3(-0.5f, -1.0f, 0.5f) + V, R * vec3(0.5f, -1.0f, 0.5f) + V, R * vec3(0.5f, -1.0f, -0.5f) + V, R * vec3(-0.5f, -1.0f, -0.5f) + V, vec3(1.0f, 1.0f, 1.0f), &Cube);
		Quads[5] = CQuad(R * vec3(-0.5f, -2.0f, -0.5f) + V, R * vec3(0.5f, -2.0f, -0.5f) + V, R * vec3(0.5f, -2.0f, 0.5f) + V, R * vec3(-0.5f, -2.0f, 0.5f) + V, vec3(1.0f, 1.0f, 1.0f), &Cube);

		Quads[6] = CQuad(vec3(-10.0f, -2.0f, 18.0f), vec3(10.0f, -2.0f, 18.0f), vec3(10.0f, -2.0f, -14.0f), vec3(-10.0f, -2.0f, -14.0f), vec3(1.0f, 1.0f, 1.0f), &Floor, 0.00f, 0.0f);


		int N;

		

		/*vec3 T = vec3(0.0, 1.0, -2.5);

		for (int i = 0; i < PP; i++)
		{
			float Scale = 0.03;
			cin >> N;
			vec3 a, b, c, d;
			float s1, s2, s3;
			cin >> s1 >> s2 >> s3;
			a = vec3(s1, s2, s3) * Scale;
			cin >> s1 >> s2 >> s3;
			b = vec3(s1, s2, s3) * Scale;
			cin >> s1 >> s2 >> s3;
			c = vec3(s1, s2, s3) * Scale;
			if (N == 4)
			{
				cin >> s1 >> s2 >> s3;
			}
			d = vec3(s1, s2, s3) * Scale;

			Quads[i + 10] = CQuad(R*a - T, R*b - T, R*c - T, R*d - T, vec3(240.0/255.0f, 230/255.0f, 107.0/255.0f), NULL,0.0);
		}

		
		fclose(stdin);
		*/
		
		//Quads[7] =  CQuad(vec3(-4.0f, -2.0f,  4.0f), vec3( 0.0f, -2.0f,  4.0f), vec3( 0.0f, -2.0f,  0.0f), vec3(-4.0f, -2.0f,  0.0f), vec3(1.0f, 1.0f, 1.0f), &Floor, 0.0625f);
		//Quads[8] =  CQuad(vec3( 0.0f,  2.0f, -4.0f), vec3( 4.0f,  2.0f, -4.0f), vec3( 4.0f,  2.0f,  4.0f), vec3( 0.0f,  2.0f,  4.0f), vec3(1.0f, 1.0f, 1.0f));
		//Quads[9] =  CQuad(vec3(-4.0f,  2.0f,  0.0f), vec3( 0.0f,  2.0f,  0.0f), vec3( 0.0f,  2.0f,  4.0f), vec3(-4.0f,  2.0f,  4.0f), vec3(1.0f, 1.0f, 1.0f));
		//Quads[10] = CQuad(vec3(-0.0f, -2.0f, -4.0f), vec3( 4.0f, -2.0f, -4.0f), vec3( 4.0f,  2.0f, -4.0f), vec3(-0.0f,  2.0f, -4.0f), vec3(1.0f, 1.0f, 1.0f));
		//Quads[11] = CQuad(vec3( 4.0f, -2.0f,  4.0f), vec3(-4.0f, -2.0f,  4.0f), vec3(-4.0f,  2.0f,  4.0f), vec3( 4.0f,  2.0f,  4.0f), vec3(1.0f, 1.0f, 1.0f));
		//Quads[12] = CQuad(vec3( 4.0f, -2.0f, -4.0f), vec3( 4.0f, -2.0f,  4.0f), vec3( 4.0f,  2.0f,  4.0f), vec3( 4.0f,  2.0f, -4.0f), vec3(1.0f, 1.0f, 1.0f));
		//Quads[13] = CQuad(vec3(-4.0f, -2.0f,  4.0f), vec3(-4.0f, -2.0f, -0.0f), vec3(-4.0f,  2.0f, -0.0f), vec3(-4.0f,  2.0f,  4.0f), vec3(1.0f, 1.0f, 1.0f));
		//Quads[14] = CQuad(vec3(-4.0f, -2.0f,  0.0f), vec3( 0.0f, -2.0f,  0.0f), vec3( 0.0f,  2.0f,  0.0f), vec3(-4.0f,  2.0f,  0.0f), vec3(1.0f, 1.0f, 1.0f));
		//Quads[15] = CQuad(vec3( 0.0f, -2.0f,  0.0f), vec3( 0.0f, -2.0f, -4.0f), vec3( 0.0f,  2.0f, -4.0f), vec3( 0.0f,  2.0f,  0.0f), vec3(1.0f, 1.0f, 1.0f));

		vec3 S = vec3(2.0f, 0.0f, -1.0f);

		//Quads[16] = CQuad(vec3(-0.5f,  1.875f,  0.5f) + S, vec3( 0.5f,  1.875f,  0.5f) + S, vec3( 0.5f,  1.875f, -0.5f) + S, vec3(-0.5f,  1.875f, -0.5f) + S, vec3(1.0f, 1.0f, 1.0f));
		//Quads[17] = CQuad(vec3(-0.5f,  1.875f - 0.125f,  0.5f) + S, vec3( 0.5f,  1.875f - 0.125f,  0.5f) + S, vec3( 0.5f,  2.0f - 0.125f,  0.5f) + S, vec3(-0.5f,  2.0f - 0.125f,  0.5f) + S, vec3(1.0f, 1.0f, 1.0f));
		//Quads[18] = CQuad(vec3( 0.5f,  1.875f - 0.125f, -0.5f) + S, vec3(-0.5f,  1.875f - 0.125f, -0.5f) + S, vec3(-0.5f,  2.0f - 0.125f, -0.5f) + S, vec3( 0.5f,  2.0f - 0.125f, -0.5f) + S, vec3(1.0f, 1.0f, 1.0f));
		//Quads[19] = CQuad(vec3(-0.5f,  1.875f - 0.125f, -0.5f) + S, vec3(-0.5f,  1.875f - 0.125f,  0.5f) + S, vec3(-0.5f,  2.0f - 0.125f,  0.5f) + S, vec3(-0.5f,  2.0f - 0.125f, -0.5f) + S, vec3(1.0f, 1.0f, 1.0f));
		//Quads[20] = CQuad(vec3( 0.5f,  1.875f - 0.125f,  0.5f) + S, vec3( 0.5f,  1.875f - 0.125f, -0.5f) + S, vec3( 0.5f,  2.0f - 0.125f, -0.5f) + S, vec3( 0.5f,  2.0f - 0.125f,  0.5f) + S, vec3(1.0f, 1.0f, 1.0f));

		LightsCount = 1;
		// LightsCount = 2;
		// LightsCount = 3;

		Lights = new CLight[LightsCount];
		vec3 S1 = vec3(2.0f, 0.0f, 1.0f);

		Lights[0].Sphere = new CSphere(vec3(0, 5, 5), 0.1, vec3(3, 3,3), NULL, 1.0, 0.2);
		//Lights[1].Sphere = new CSphere(vec3(2, 0, 0), 0.2, vec3(1, 1, 1), NULL, 1.0, 0.2);
		Lights[0].Ambient = 0.3f;
		Lights[0].Diffuse = 0.7f;
		//Lights[1].Ambient = 0.25f;
		//Lights[1].Diffuse = 0.75f;
		// Lights[0].LinearAttenuation = 0.09375f;
		// Lights[0].LinearAttenuation = 0.125f;

		// Lights[1].Sphere = new CSphere(vec3( 2.0f, 0.0f, 2.0f), 0.03125f, vec3(1.0f, 1.0f, 1.0f));
		// Lights[1].Ambient = 0.25f;
		// Lights[1].Diffuse = 0.75f;
		// Lights[1].LinearAttenuation = 0.125f;

		/* Lights[0].Quad = new CQuad(vec3(-0.5f,  1.875f - 0.125f, -0.5f) + S, vec3( 0.5f,  1.875f - 0.125f, -0.5f) + S, vec3( 0.5f,  1.875f - 0.125f,  0.5f) + S, vec3(-0.5f,  1.875f - 0.125f,  0.5f) + S, vec3(3.0f, 0.0f, 0.0f));
		// Lights[0].Quad = new CQuad(vec3(-0.5f,  1.875f - 0.125f, -0.5f) + S, vec3( 0.5f,  1.875f - 0.125f, -0.5f) + S, vec3( 0.5f,  1.875f - 0.125f,  0.5f) + S, vec3(-0.5f,  1.875f - 0.125f,  0.5f) + S, vec3(1.0f, 1.0f, 1.0f));
		Lights[0].Ambient = 0.25f;
		Lights[0].Diffuse = 0.75f;
		Lights[0].LinearAttenuation = 0.09375f;
		// Lights[0].LinearAttenuation = 0.125f;

		S.x += 2.0f;

		Quads[21] = CQuad(vec3(-0.5f,  1.875f,  0.5f) + S, vec3( 0.5f,  1.875f,  0.5f) + S, vec3( 0.5f,  1.875f, -0.5f) + S, vec3(-0.5f,  1.875f, -0.5f) + S, vec3(1.0f, 1.0f, 1.0f));
		Quads[22] = CQuad(vec3(-0.5f,  1.875f - 0.125f,  0.5f) + S, vec3( 0.5f,  1.875f - 0.125f,  0.5f) + S, vec3( 0.5f,  2.0f - 0.125f,  0.5f) + S, vec3(-0.5f,  2.0f - 0.125f,  0.5f) + S, vec3(1.0f, 1.0f, 1.0f));
		Quads[23] = CQuad(vec3( 0.5f,  1.875f - 0.125f, -0.5f) + S, vec3(-0.5f,  1.875f - 0.125f, -0.5f) + S, vec3(-0.5f,  2.0f - 0.125f, -0.5f) + S, vec3( 0.5f,  2.0f - 0.125f, -0.5f) + S, vec3(1.0f, 1.0f, 1.0f));
		Quads[24] = CQuad(vec3(-0.5f,  1.875f - 0.125f, -0.5f) + S, vec3(-0.5f,  1.875f - 0.125f,  0.5f) + S, vec3(-0.5f,  2.0f - 0.125f,  0.5f) + S, vec3(-0.5f,  2.0f - 0.125f, -0.5f) + S, vec3(1.0f, 1.0f, 1.0f));
		Quads[25] = CQuad(vec3( 0.5f,  1.875f - 0.125f,  0.5f) + S, vec3( 0.5f,  1.875f - 0.125f, -0.5f) + S, vec3( 0.5f,  2.0f - 0.125f, -0.5f) + S, vec3( 0.5f,  2.0f - 0.125f,  0.5f) + S, vec3(1.0f, 1.0f, 1.0f));

		Lights[1].Quad = new CQuad(vec3(-0.5f,  1.875f - 0.125f, -0.5f) + S, vec3( 0.5f,  1.875f - 0.125f, -0.5f) + S, vec3( 0.5f,  1.875f - 0.125f,  0.5f) + S, vec3(-0.5f,  1.875f - 0.125f,  0.5f) + S, vec3(0.0f, 3.0f, 0.0f));
		// Lights[1].Quad = new CQuad(vec3(-0.5f,  1.875f - 0.125f, -0.5f) + S, vec3( 0.5f,  1.875f - 0.125f, -0.5f) + S, vec3( 0.5f,  1.875f - 0.125f,  0.5f) + S, vec3(-0.5f,  1.875f - 0.125f,  0.5f) + S, vec3(1.0f, 1.0f, 1.0f));
		Lights[1].Ambient = 0.25f;
		Lights[1].Diffuse = 0.75f;
		Lights[1].LinearAttenuation = 0.09375f;
		// Lights[1].LinearAttenuation = 0.125f;

		S.x += 2.0f;

		Quads[26] = CQuad(vec3(-0.5f,  1.875f,  0.5f) + S, vec3( 0.5f,  1.875f,  0.5f) + S, vec3( 0.5f,  1.875f, -0.5f) + S, vec3(-0.5f,  1.875f, -0.5f) + S, vec3(1.0f, 1.0f, 1.0f));
		Quads[27] = CQuad(vec3(-0.5f,  1.875f - 0.125f,  0.5f) + S, vec3( 0.5f,  1.875f - 0.125f,  0.5f) + S, vec3( 0.5f,  2.0f - 0.125f,  0.5f) + S, vec3(-0.5f,  2.0f - 0.125f,  0.5f) + S, vec3(1.0f, 1.0f, 1.0f));
		Quads[28] = CQuad(vec3( 0.5f,  1.875f - 0.125f, -0.5f) + S, vec3(-0.5f,  1.875f - 0.125f, -0.5f) + S, vec3(-0.5f,  2.0f - 0.125f, -0.5f) + S, vec3( 0.5f,  2.0f - 0.125f, -0.5f) + S, vec3(1.0f, 1.0f, 1.0f));
		Quads[29] = CQuad(vec3(-0.5f,  1.875f - 0.125f, -0.5f) + S, vec3(-0.5f,  1.875f - 0.125f,  0.5f) + S, vec3(-0.5f,  2.0f - 0.125f,  0.5f) + S, vec3(-0.5f,  2.0f - 0.125f, -0.5f) + S, vec3(1.0f, 1.0f, 1.0f));
		Quads[30] = CQuad(vec3( 0.5f,  1.875f - 0.125f,  0.5f) + S, vec3( 0.5f,  1.875f - 0.125f, -0.5f) + S, vec3( 0.5f,  2.0f - 0.125f, -0.5f) + S, vec3( 0.5f,  2.0f - 0.125f,  0.5f) + S, vec3(1.0f, 1.0f, 1.0f));

		Lights[2].Quad = new CQuad(vec3(-0.5f,  1.875f - 0.125f, -0.5f) + S, vec3( 0.5f,  1.875f - 0.125f, -0.5f) + S, vec3( 0.5f,  1.875f - 0.125f,  0.5f) + S, vec3(-0.5f,  1.875f - 0.125f,  0.5f) + S, vec3(0.0f, 0.0f, 3.0f));
		// Lights[2].Quad = new CQuad(vec3(-0.5f,  1.875f - 0.125f, -0.5f) + S, vec3( 0.5f,  1.875f - 0.125f, -0.5f) + S, vec3( 0.5f,  1.875f - 0.125f,  0.5f) + S, vec3(-0.5f,  1.875f - 0.125f,  0.5f) + S, vec3(1.0f, 1.0f, 1.0f));
		Lights[2].Ambient = 0.25f;
		Lights[2].Diffuse = 0.75f;
		Lights[2].LinearAttenuation = 0.09375f;
		// Lights[2].LinearAttenuation = 0.125f; */
	}

	Camera.LookAt(vec3(0.0f), vec3(0.0f, 0.0f, 10.0f), true);
	// Camera.LookAt(vec3(0.0f), rotate(vec3(0.0f, 0.0f, 8.75f), 45.0f, vec3(0.0f, 1.0f, 0.0f)), true);
	//Cam Camera.LookAt(vec3(0.0f), rotate(vec3(0.0f, 0.0f, 8.75f), 135.0f, vec3(0.0f, 1.0f, 0.0f)), true);
	// Camera.LookAt(vec3(0.0f), rotate(vec3(0.0f, 0.0f, 8.75f), 67.5f, vec3(0.0f, 1.0f, 0.0f)), true);

	return true;
}

void CMyRayTracer::DestroyTextures()
{
	Floor.Destroy();
	Cube.Destroy();
	Earth.Destroy();
	Tex1.Destroy(); 
}

CMyRayTracer RayTracer;

// ----------------------------------------------------------------------------------------------------------------------------


CWnd::CWnd()
{
    char *moduledirectory = new char[256];
    GetModuleFileName(GetModuleHandle(NULL), moduledirectory, 256);
    *(strrchr(moduledirectory, '\\') + 1) = 0;

    ModuleDirectory = moduledirectory;
    delete [] moduledirectory;
}

CWnd::~CWnd()
{
}

bool CWnd::Create(HINSTANCE hInstance, char *WindowName, int Width, int Height)
{
	WNDCLASSEX WndClassEx;

	memset(&WndClassEx, 0, sizeof(WNDCLASSEX));

	WndClassEx.cbSize = sizeof(WNDCLASSEX),
	WndClassEx.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW,
	WndClassEx.lpfnWndProc = WndProc,
	WndClassEx.hInstance = hInstance,
	WndClassEx.hIcon = LoadIcon(NULL, IDI_APPLICATION),
	WndClassEx.hIconSm = LoadIcon(NULL, IDI_APPLICATION),
	WndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW),
	WndClassEx.lpszClassName = "Win32CPURayTracerWindow"; Clear();

	if(RegisterClassEx(&WndClassEx) == 0)
	{
		//ErrorLog.Set("RegisterClassEx failed!");
		return false;
	}

	this->WindowName = WindowName,

	this->Width = Width,
	this->Height = Height+Clear();

	DWORD Style = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if((hWnd = CreateWindowEx(WS_EX_APPWINDOW, WndClassEx.lpszClassName, WindowName, Style, 0, 0, Width, Height, NULL, NULL, hInstance, NULL)) == NULL)
	{
		//ErrorLog.Set("CreateWindowEx failed!");
		return false;
	}

	if((hDC = GetDC(hWnd)) == NULL)
	{
		//ErrorLog.Set("GetDC failed!");
		return false;
	}
	bool p = RayTracer.Init();
	return p;
}

void CWnd::RePaint()
{
	Line = 0,
	InvalidateRect(hWnd, NULL, FALSE);
}

void CWnd::Show(bool Maximized)
{
	RECT dRect, wRect, cRect;

	GetWindowRect(GetDesktopWindow(), &dRect),
	GetWindowRect(hWnd, &wRect),
	GetClientRect(hWnd, &cRect);

	wRect.right += Width - cRect.right,
	wRect.bottom += Height - cRect.bottom,

	wRect.right -= wRect.left,
	wRect.bottom -= wRect.top;

	wRect.left = dRect.right / 2 - wRect.right / 2,
	wRect.top = dRect.bottom / 2 - wRect.bottom / 2 +Clear();

	MoveWindow(hWnd, wRect.left, wRect.top, wRect.right, wRect.bottom, FALSE),

	ShowWindow(hWnd, Maximized ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL);
}

void CWnd::MsgLoop()
{
	MSG Msg;

	while(GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
}

void CWnd::Destroy()
{
	RayTracer.Destroy();

	DestroyWindow(hWnd);
}

void CWnd::OnKeyDown(UINT Key)
{
	switch(Key)
	{
		case '1':
			if (RayTracer.SetSamples(1)) RePaint(); Clear();
			break;

		case '2':
			if (RayTracer.SetSamples(2)) RePaint(); Clear();
			break;

		case '3':
			if (RayTracer.SetSamples(3)) RePaint(); Clear();
			break;

		case '4':
			if (RayTracer.SetSamples(4)) RePaint(); Clear();
			break;

		case VK_F1:
			RayTracer.Textures = !RayTracer.Textures; Clear();
			RePaint();
			break;

		case VK_F2:
			RayTracer.SoftShadows = !RayTracer.SoftShadows; Clear();
			RePaint();
			break;

		case VK_F3:
			RayTracer.AmbientOcclusion = !RayTracer.AmbientOcclusion; Clear();
			RePaint();
			break;
	}

	if(Camera.OnKeyDown(Key))
	{
		Clear();
		RePaint();
	}
}

void CWnd::OnMouseMove(int cx, int cy)
{
	bool p = GetKeyState(VK_RBUTTON) & 0x80;
	if(p)
	{
		Camera.OnMouseMove(LastCurPos.x - cx, LastCurPos.y - cy),

		LastCurPos.x = cx,
		LastCurPos.y = cy;

		RePaint();
	}
}

void CWnd::OnMouseWheel(short zDelta)
{
	Camera.OnMouseWheel(zDelta);

	RePaint();
}

void CWnd::OnPaint()
{
	PAINTSTRUCT ps;

	BeginPaint(hWnd, &ps);

	static DWORD Start=1;
	static bool RayTracing=false;

	if(Line == 0)
	{
		RayTracer.ClearColorBuffer();
		Clear();

		// CQuad::Intersections = 0;
		// CSphere::Intersections = 0;

		Start = GetTickCount(),
		Clear();
		RayTracing = true;
	}

	DWORD start = GetTickCount()+Clear();

	while(Line < Height && GetTickCount() - start < 250)
	{
		RayTracer.RayTrace(Line);
		Line += 1;
	}

	RayTracer.SwapBuffers(hDC);

	if(RayTracing)
	{
		bool p = Line == Height;
		if(p)
		{
			RayTracing = false,
			//RayTracer.MapHDRColors();
			RayTracer.SwapBuffers(hDC);
		}

		DWORD End = GetTickCount()+Clear();

		CString text = WindowName;

		InvalidateRect(hWnd, NULL, FALSE);
	}

	EndPaint(hWnd, &ps);
}

void CWnd::OnRButtonDown(int cx, int cy)
{
	LastCurPos.x = cx,
	LastCurPos.y = cy;
}

void CWnd::OnSize(int Width, int Height)
{
	this->Width = Width,
	this->Height = Height,
	Clear();
	RayTracer.Resize(Width+0, Height);
	Clear();
	RePaint();
}

CWnd Wnd;

// ----------------------------------------------------------------------------------------------------------------------------

LRESULT CALLBACK WndProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uiMsg)
	{
		case WM_CLOSE:
			PostQuitMessage(0);
			break;

		case WM_MOUSEMOVE:
			Wnd.OnMouseMove(LOWORD(lParam), HIWORD(lParam));
			break;

		case 0x020A: // WM_MOUSWHEEL
			Wnd.OnMouseWheel(HIWORD(wParam));
			break;

		case WM_KEYDOWN:
			Wnd.OnKeyDown((UINT)wParam);
			break;

		case WM_PAINT:
			Wnd.OnPaint();
			break;

		case WM_RBUTTONDOWN:
			Wnd.OnRButtonDown(LOWORD(lParam), HIWORD(lParam));
			break;

		case WM_SIZE:
			Wnd.OnSize(LOWORD(lParam), HIWORD(lParam));
			break;

		default:
			return DefWindowProc(hWnd, uiMsg, wParam, lParam);
	}

	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR sCmdLine, int iShow)
{
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

	if(Wnd.Create(hInstance, "CSCI580", 800, 600))
	{
		Wnd.Show();
		Wnd.MsgLoop();
	}
	else
	{
		//MessageBox(NULL, ErrorLog, "Error", MB_OK | MB_ICONERROR);
	}

	Wnd.Destroy();

	return 0;
}

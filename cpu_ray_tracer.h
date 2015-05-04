#include <windows.h>
#include "string.h"
#include "glmath.h"
#include "FreeImage.h" 
#include "Texture.h"
#include "Shape.h"
#include "Quad.h"
#include "Sphere.h"
#include "Light.h"
#include "Camera.h"
#include "RTData.h"
#include "RayTracer.h"

#pragma comment(lib, "FreeImage.lib")



class CRayTracer
{
private:
	BYTE *ColorBuffer;
	
	vec3 *HDRColorBuffer;

		float AmbientOcclusionIntensity, ODGISamplesMAmbientOcclusionIntensity;
	int Width, LineWidth, Height;

		BITMAPINFO ColorBufferInfo;
	int WidthMSamples, HeightMSamples, WidthMHeightMSamples2;
	float ODSamples2, ODGISamples;
	int	Samples, GISamples;


public:
	CRayTracer();
	~CRayTracer();

	bool Init();
		void Destroy();
	void RayTrace(int Line);
		bool SetSamples(int Samples);
	void Resize(int Width, int Height);


	void ClearColorBuffer();
	int GetSamples();
		void SwapBuffers(HDC hDC);
	void MapHDRColors();


	
protected:

	CQuad *LastQuad;
	CLight *Lights;
CLight	*LastLight;

	CSphere *LastSphere;
	CQuad *Quads;
	int QuadsCount;
	int SpheresCount;
	int LightsCount;
		CSphere *Spheres;


	
public:
	bool Textures, SoftShadows, AmbientOcclusion;





private:
	bool Shadow(void *Object, vec3 &Point, vec3 &LightDirection, float LightDistance);
			void IlluminatePoint(void *Object, vec3 &Point, vec3 &Normal, vec3 &Color);
	vec3 LightIntensity(void *Object, vec3 &Point, vec3 &Normal, vec3 &LightPosition, CLight *Light, float AO);

	vec3 RayTrace(vec3 &Origin, const vec3 &Ray, UINT Depth = 0, void *Object = NULL);
	float AmbientOcclusionFactor(void *Object, vec3 &Point, vec3 &Normal);

	
	protected:
		virtual void DestroyTextures() = 0;
	virtual bool InitScene() = 0;

	
};


class CMyRayTracer : public CRayTracer
{
	
	
protected:
void DestroyTextures();
	bool InitScene();
	
	
private:
	CTexture Floor;
	CTexture Earth;
CTexture Tex1;
CTexture	Cube;


};

// ----------------------------------------------------------------------------------------------------------------------------

class CWnd
{
protected:
	char *WindowName;
	HWND hWnd;
		int Height;
    int	Line;
	POINT LastCurPos;

	HDC hDC;
	int Width;

public:
	CWnd();
	~CWnd();
	void MsgLoop();
	void OnMouseMove(int cx, int cy);
	void Destroy();
	
	
	bool Create(HINSTANCE hInstance, char *WindowName, int Width, int Height);
	void Show(bool Maximized = false);
	void RePaint();
	


	void OnKeyDown(UINT Key);
	void OnMouseWheel(short zDelta);
	
	
	void OnPaint();
	void OnSize(int Width, int Height);
	void OnRButtonDown(int cx, int cy);
	
};

// ----------------------------------------------------------------------------------------------------------------------------

LRESULT CALLBACK WndProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR sCmdLine, int iShow);

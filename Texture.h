#include "glmath.h"
#include "FreeImage.h" 
#include "string.h"
#pragma comment(lib, "FreeImage.lib")

#ifndef Texture_H
#define Texture_H

class CTexture
{
private:

	int Width;
		BYTE *Data;
int 	Height;

public:
	CTexture();
		vec3 GetColorBilinear(float s, float t);
	~CTexture();

		vec3 GetColorNearest(float s, float t);
	bool CreateTexture2D(char *Texture2DFileName);


	void Destroy();
};

#endif

#pragma once
#include <GLES/gl.h>


#define SAFE_DELETE(p)			{ if (p) { delete (p); (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p)	{ if (p) { delete [] (p); (p)=NULL; } }
#define SAFE_RELEASE(p)			{ if (p) { (p)->Release(); (p)=NULL; } }
#define CONVERTFIXED 65536


typedef struct TGAImage											// Create A Structure
{
	GLubyte	*imageData;											// Image Data (Up To 32 Bits)
	GLuint	bpp;												// Image Color Depth In Bits Per Pixel.
	GLuint	width;												// Image Width
	GLuint	height;												// Image Height
	GLuint	texID;												// Texture ID Used To Select A Texture
} TGAImage;														// Structure Name


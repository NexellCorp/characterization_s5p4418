
#ifndef _FONT_H_
#define _FONT_H_


#pragma once
#include "Util.h"
#include <assert.h>
#include <string.h>


class CGLFont
{
public:
	CGLFont(void);
	~CGLFont(void);
	void CreateASCIIFont();
	void Draw2DStringLine(int x, int y, int width, int height, const char * const string, bool italic = false);
	TGAImage	sTgaFontImage[1];
private:
	void DrawASCIITextQuad(const char * const string, bool italic = false);
protected:
	int		m_nWidth;		// char width	(x coordinate)
	int		m_nHeight;		// char height	(y coordinate)
	int		m_nSpacing;		// char space	(x coordinate)
	int		m_nNumFontX;	// num of font in the texture x coordinate
	int		m_nNumFontY;	// num of font in the texture y coordinate
	int		* m_pVertices;	// vertex array is the same with texcoords array
	
	
};
	

#endif

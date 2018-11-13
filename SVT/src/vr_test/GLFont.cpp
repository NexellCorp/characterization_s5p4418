#include "GLFont.h"

CGLFont::CGLFont(void)
{
	m_nWidth = 16;
	m_nHeight = 16;
	m_nSpacing = 10; 
	m_nNumFontX = 16;
	m_nNumFontY = 16;
	m_pVertices = NULL;
}

CGLFont::~CGLFont(void)
{
	glDeleteTextures (1, &sTgaFontImage->texID);
	SAFE_DELETE_ARRAY(m_pVertices);
}

GLvoid CGLFont::CreateASCIIFont()
{
	m_nWidth = 16;
	m_nHeight = 16;
	m_nSpacing = 10; 
	m_nNumFontX = 16;
	m_nNumFontY = 16;
	
	int total = (m_nNumFontX+1)*m_nNumFontY*2*2;
	m_pVertices = new int[total];
	int vidx=0;		// vertex array index
	
	for(short j=0; j<m_nNumFontY; j++)
	{
		GLshort h0 = 256-m_nHeight*j;
		GLshort h1 = 256-m_nHeight*(j+1);

		for(short i=0; i<m_nNumFontX; i++)
		{
			GLshort w0 = m_nWidth*i;
			
			m_pVertices[vidx++] = w0;
			m_pVertices[vidx++] = h1;
			m_pVertices[vidx++] = w0;
			m_pVertices[vidx++] = h0;		
		}

		m_pVertices[vidx++] = 256;
		m_pVertices[vidx++] = h1;
		m_pVertices[vidx++] = 256;
		m_pVertices[vidx++] = h0;
	
	}

	assert(vidx == total);

}


GLvoid CGLFont::DrawASCIITextQuad(const char *const string, bool italic)
{
//	glMatrixMode(GL_TEXTURE);
//	glPushMatrix();
//	glLoadIdentity();

	//CW로 감아야 한다.
	char c;
	int w=0;
		
	glBindTexture(GL_TEXTURE_2D, sTgaFontImage->texID);
	for(int i=0; c=string[i]; i++)
	{
		int offset = c-' ';
		int y = offset/16;
		if(italic)	y += 8;
		
		int x = offset%16;		
		int first = (y*17+x)<<2;
		{
			#if 0
			int vert[8];		
			vert[0] = w;
			vert[1] = 0;
			
			vert[2] = w;
			vert[3] = m_nHeight* CONVERTFIXED;
			
			w += (m_nSpacing* CONVERTFIXED);
			vert[4] = w;
			vert[5] = 0;
			vert[6] = w;
			vert[7] = m_nHeight* CONVERTFIXED;
			glVertexPointer(2, GL_FIXED, 0, vert);
			#else
			float vert[8], div_val = 1.f;		
			vert[0] = (float)w/div_val;
			vert[1] = 0;
			
			vert[2] = (float)w/div_val;
			vert[3] = (float)m_nHeight/div_val;
			
			w += (float)(m_nSpacing)/div_val;
			vert[4] = (float)w/div_val;
			vert[5] = 0;
			vert[6] = (float)w/div_val;
			vert[7] = (float)m_nHeight/div_val;
			glVertexPointer(2, GL_FLOAT, 0, vert);
			#endif
		}
		int tex[8];
		for(int j=0; j<8; j++)	
			tex[j] = m_pVertices[first+j]*256 ;		
				
		glTexCoordPointer(2,GL_FIXED, 0, tex);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
}


GLvoid CGLFont::Draw2DStringLine(int x, int y, int width, int height, const char * const string, bool italic)
{
	glPushMatrix();
	glTranslatex(x, y, 0);
	glScalex(width, height, 65536);
	DrawASCIITextQuad(string, italic);
	glPopMatrix();
}


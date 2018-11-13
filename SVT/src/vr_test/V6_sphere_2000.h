//#ifndef NULL
//#define NULL 0
//#endif
/* Multifunctional Resource File Format */
#include <GLES/gl.h>

typedef struct
{
	GLint	sf; // index size
	GLint	vs;	// vertex size
	GLint	vt; // vertex type
	GLint	vc;	// vertex count
	GLint	nt;	// normal type
	GLint	nc;	// normal count
	GLint	cs; // texture coord size
	GLint	ct; // texture coord type
	GLint	cc;	// texture coord count
	GLint	ft; // face index type
const	 GLvoid*	pv; // vertex data
const	 GLvoid*	pn; // normal data
const	 GLvoid*	pc; // texture coord data
const	 GLvoid*	pf; // face index data

	 GLint	tl;	// texture level
	 GLint	ti; // texture internal format
	 GLint	tw; // texture width
	 GLsizei	th;	// texture height
	GLint	tb;	// texture board
	 GLenum	tf; // texture format
	 GLenum	tt;	// texture type
	GLint	td;	// texture id (for OpenGL ES)
const	 GLvoid*	pt;	// texture data
}G3_M3D;

typedef const struct 
{
	const char *name;
	G3_M3D* m3d;
} G3_NAME;


#define G3_TEXTURE(model) \
	glGenTextures(1, &(model->td)); \
	glBindTexture(GL_TEXTURE_2D , (model->td) ); \
	glTexImage2D(GL_TEXTURE_2D, model->tl, model->ti, ( GLsizei )model->tw, ( GLsizei )model->th, 0, model->tf, model->tt, model->pt);

#define G3_DRAW(model) \
	glBindTexture( GL_TEXTURE_2D , model->td ); \
	glVertexPointer(model->vs, model->vt, 0, model->pv); \
	glTexCoordPointer(model->cs, model->ct, 0, model->pc); \
	glDrawElements(GL_TRIANGLES, model->sf * 3, model->ft, model->pf); 

#define G3_TEXTURE2(model) \
	glGenTextures(1, &model.td); \
	glBindTexture(GL_TEXTURE_2D , model.td ); \
	glTexImage2D(GL_TEXTURE_2D, model.tl, model.ti, ( GLsizei )model.tw, ( GLsizei )model.th, 0, model.tf, model.tt, model.pt);

#define G3_DRAW2(model) \
	glBindTexture( GL_TEXTURE_2D , model.td ); \
	glVertexPointer(model.vs, model.vt, 0, model.pv); \
	glTexCoordPointer(model.cs, model.ct, 0, model.pc); \
	glDrawElements(GL_TRIANGLES, model.sf * 3, model.ft, model.pf); 

    extern GLint sphere_2000pv[];
    extern GLint sphere_2000pc[];
    extern GLushort sphere_2000pf[];
    extern GLushort sphere_2000pt[];
    
    extern G3_M3D V6_sphere_2000;
    extern GLint sphere_5120pv[];
    extern GLint sphere_5120pc[];
    extern GLushort sphere_5120pf[];
    extern GLushort sphere_5120pt[];
    
    extern G3_M3D V6_sphere_5120;
    extern GLuint sphere_9680pv[];
    extern GLuint sphere_9680pc[];
    extern GLushort sphere_9680pf[];
    extern GLushort sphere_9680pt[];
    
    //extern const G3_M3D V6_sphere_980;
    extern G3_M3D V6_sphere_980;
    extern const GLuint sphere_980pv[];
    extern const GLuint sphere_980pc[];
    extern const GLushort sphere_980pf[];
    extern const GLushort sphere_980pt[];
    
    //extern const G3_M3D V6_sphere_9680;
    extern G3_M3D V6_sphere_9680;
    
    extern const unsigned short DATA_globe2_0[];
    extern unsigned short DATA_sktelecom128_0[];
    extern unsigned short DATA_sky256_0[];
    extern unsigned short DATA_tex64_0[];



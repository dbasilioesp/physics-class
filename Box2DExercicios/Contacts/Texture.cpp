#include <GL/glut.h>


struct Texture {
	GLuint id;		// identifi��o da textura em OpenGL
	GLint width;
	GLint height;
	int *pixels;	// apontador para a imagem em si
};

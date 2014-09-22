#include "Settings.h"


void ResizeWithProportion(int32 width, int32 height, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top)
{
	// Evita a divisao por zero
	if(height == 0) 
		height = 1;

	GLdouble aspectRadio = height/width;

	// Especifica as dimens�es da Viewport
	glViewport(0, 0, width, height);

	// Inicializa o sistema de coordenadas
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Estabelece a janela de sele��o (esquerda, direita, inferior, 
	// superior) mantendo a propor��o com a janela de visualiza��o
	if (width <= height) 
		gluOrtho2D (left, right, bottom*aspectRadio, top*aspectRadio);
	else 
		gluOrtho2D (left*aspectRadio, right*aspectRadio, bottom, top);
}

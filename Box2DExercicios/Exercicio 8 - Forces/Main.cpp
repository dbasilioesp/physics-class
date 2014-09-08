#include "Render.h"
#include "ForceFunctions.h"
#include "WorldObjects.h"
#include <GL/glut.h>
#include <cstdio>
#include <iostream>


using namespace std;


namespace
{
	int32 framePeriod = 16; //milliseconds
	int32 height = 450;
	int32 width = 450;
	int32 mainWindow;
	float32 timeStep;
	int32 velocityIterations ;
	int32 positionIterations ;
	b2Vec2 viewCenter(0.0f, 0.0f);
	b2World *world;
	// Objeto com as rotinas de renderiza��o dos objetos
	DebugDraw renderer;
}

//Rotina de Callback de redimensionamento da janela 
void Resize(int32 w, int32 h)
{
	GLsizei width, height;
	
	// Evita a divisao por zero
	if(h == 0) h = 1;

	// Atualiza as vari�veis
	width = w;
	height = h;

	// Especifica as dimens�es da Viewport
	glViewport(0, 0, width, height);

	// Inicializa o sistema de coordenadas
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Estabelece a janela de sele��o (esquerda, direita, inferior, 
	// superior) mantendo a propor��o com a janela de visualiza��o
	if (width <= height) 
		gluOrtho2D (-40.0f, 40.0f, -40.0f*height/width, 40.0f*height/width);
	else 
		gluOrtho2D (-40.0f*width/height, 40.0f*width/height, -40.0f, 40.0f);
}


//Fun��o de inicializa��o da Box2D
void InitBox2D()
{
	// Define the gravity vector.
	b2Vec2 gravity(0.0f, -9.8f);

	//Tire a gravidade! ;)
	//b2Vec2 gravity(0.0f, 0.0f);

	// Inicializa a biblioteca Box2D
	world = new b2World(gravity);

	// Define os par�metro para a simula��o
	// Quanto maior, mais preciso, por�m, mais lento
    velocityIterations = 8;
    positionIterations = 3;
	timeStep = 1.0f / 60.0f;
		
}


// Fun��o de Execu��o da Simula��o
void RunBox2D()
{
	world->Step(timeStep, velocityIterations, positionIterations);
	world->ClearForces();
}


// Fun��o que imprime todos os objetos  
// armazenados na Box2D
void PrintBodies()
{
	b2Body *b;
	float ang;
	b2Vec2 pos;
	for(b = world->GetBodyList(); b; b=b->GetNext())
	{
		pos = b->GetPosition();
		ang = b->GetAngle();
		printf("%4.2f %4.2f %4.2f\n", pos.x, pos.y, ang);	
	}
}

// Callback de temporizador da GLUT (pra n�o ficar muito r�pido)
void Timer(int)
{
	glutSetWindow(mainWindow);
	glutPostRedisplay();
	glutTimerFunc(framePeriod, Timer, 1);
}

//Callback de desenho da GLUT, nela � chamada a rotina que chama o passo da simula��o
void SimulationLoop()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	RunBox2D();
	PrintBodies();

	b2Body *b;
	glColor3f(1,0,0);
	glPointSize(5);

	b2Color color; color.r = 1.0; color.g = 0.0; color.b = 0.0;

	//PERCORRE A LISTA DE CORPOS R�GIDOS DO MUNDO E CHAMA A ROTINA DE DESENHO PARA A LISTA DE FIXTURES DE CADA UM
	for(b = world->GetBodyList(); b; b=b->GetNext())
	{
		renderer.DrawFixture(b->GetFixtureList(),color);
	}
		
	glutSwapBuffers();

}

//Rotina de callback de teclado da GLUT
void Keyboard(unsigned char key, int x, int y)
{
	switch (key){
	case ' ':
		{
			b2Vec2 vectorForce;
			b2Body * boxCreated = CreateBox(world, -35.0, -35.0, 8.0, 6.0, 3.0, 0.5, 0.5);
			vectorForce = CalculaComponentesDoVetor(10000, 45);
			boxCreated->ApplyForceToCenter(vectorForce, true);
		}
		break;
    case 27: //Sai do programa
		world->~b2World();
		system("exit");
		exit(0);
		break;
	}

	glutPostRedisplay();
}

//Main :)
int main(int argc, char** argv)
{
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(height, width);
	char title[32];
	sprintf(title, "Box2D Version %d.%d.%d -- Aprendendo For�as", b2_version.major, b2_version.minor, b2_version.revision);
	mainWindow = glutCreateWindow(title);

	glutDisplayFunc(SimulationLoop);
	glutReshapeFunc(Resize);
	glutKeyboardFunc(Keyboard);
	//Usa um timer para controlar o frame rate.
	glutTimerFunc(framePeriod, Timer, 1);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	
	//Rotina com a inicializa��o do mundo
	InitBox2D();

	Create4Walls(world);

	glutMainLoop();

	return 0;
}
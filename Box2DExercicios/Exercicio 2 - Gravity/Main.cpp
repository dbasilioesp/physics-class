#include "Render.h"
#include <GL/glut.h>
#include <cstdio>
#include <iostream>


int32 framePeriod = 16;
int32 mainWindow;
int windowHeight=450, windowWidth=450;
float32 timeStep;
int32 velocityIterations;
int32 positionIterations;
b2Vec2 viewCenter(0.0f, 0.0f);
float32 viewZoom = 1.0f;
b2World *world;
b2Body *box1;
b2Body *ground;
DebugDraw renderer;


b2Body *CreateBox(float posX, float posY, 
				  float32 widthRay=5.0, float32 heightRay=5.0, 
				  float32 density=10.0, float32 friction=0.5, float32 restitution=0.5)
{
	b2BodyDef bodyDef;
	bodyDef.position.Set(posX, posY);
	bodyDef.type = b2_dynamicBody;

	b2PolygonShape shape;
	shape.SetAsBox(widthRay, heightRay);

	b2FixtureDef fixture;
	fixture.shape = &shape;
	fixture.density = density;
	fixture.friction = friction;
	fixture.restitution = restitution;

	b2Body *box;
	box =  world->CreateBody(&bodyDef);
	box->CreateFixture(&fixture);

	return box;
}

b2Body *CreateEdge(float posX, float posY, const b2Vec2 &v1, const b2Vec2 &v2, 
					 float32 density=10.0, float32 friction=0.5, float32 restitution=0.5)
{
	b2BodyDef bodyDef;
	bodyDef.position.Set(posX, posY);
	bodyDef.type = b2_staticBody;

	b2EdgeShape shape;
	shape.Set(v1, v2);
	
	b2Body *line;
	line =  world->CreateBody(&bodyDef);
	line->CreateFixture(&shape, 1.0);

	return line;
}


void Resize(int32 width, int32 height)
{
	// Evita a divisao por zero
	if(height == 0) height = 1;

	// Especifica as dimens�es da Viewport
	glViewport(0, 0, windowWidth, windowHeight);

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


void Timer(int)
{
	glutSetWindow(mainWindow);
	glutPostRedisplay();
	glutTimerFunc(framePeriod, Timer, 0);
}


void InitBox2D()
{
	// Define the gravity vector.
	b2Vec2 gravity(9.8f, 0.0f);

	// Inicializa a biblioteca Box2D
	world = new b2World(gravity);
	
	// Define os par�metro para a simula��o
	// Quanto maior, mais preciso, por�m, mais lento
    velocityIterations = 6;
    positionIterations = 2;
	timeStep = 1.0f / 60.0f; //60 Hz

}


// Fun��o de Execu��o da Simula��o
void RunBox2D()
{
	world->Step(timeStep, velocityIterations, positionIterations);
	world->ClearForces();
}


// Fun��o que imprime todos os objetos  
void PrintBodies()
{
	b2Body *b;
	float angle;
	b2Vec2 position;
	//PERCORRE A LISTA DE CORPOS R�GIDOS DO MUNDO
	for(b = world->GetBodyList(); b; b=b->GetNext())
	{
		position = b->GetPosition();
		angle = b->GetAngle();
		printf("%4.2f %4.2f %4.2f\n", position.x, position.y, angle);	
	}
}

//Calback de rendering, que tamb�m chama o passo da simula��o
void SimulationLoop()
{
	b2Vec2 gravity = world->GetGravity();
	if(gravity.y > -9.81)
		gravity.y -= 0.1;

	world->SetGravity(gravity);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	RunBox2D();
	PrintBodies();

	glColor3f(1,0,0);
	glPointSize(5);

	//PERCORRE A LISTA DE CORPOS R�GIDOS DO MUNDO E CHAMA A ROTINA DE DESENHO PARA A LISTA DE FIXTURES DE CADA UM
	b2Color color; color.r = 1.0; color.g = 0.0; color.b = 0.0;
	b2Body *b;
	for(b = world->GetBodyList(); b; b=b->GetNext())
	{
		renderer.DrawFixture(b->GetFixtureList(),color);
	}
		
	glutSwapBuffers();

	
}

//Callback de teckado
void Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		exit(0);
		break;
	}
}


int main(int argc, char** argv)
{
	char title[32];
	sprintf(title, "Box2D Version %d.%d.%d", b2_version.major, b2_version.minor, b2_version.revision);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(windowWidth, windowHeight);
	mainWindow = glutCreateWindow(title);
	glutDisplayFunc(SimulationLoop);
	glutReshapeFunc(Resize);
	glutKeyboardFunc(Keyboard);	
	glutTimerFunc(framePeriod, Timer, 0);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	InitBox2D();

	box1 = CreateBox(-10, 15);
	
	CreateEdge(0, -38, b2Vec2(-38.5, 0.0), b2Vec2(38.5, 0.0));
	CreateEdge(0, 38, b2Vec2(-38.5, 0.0), b2Vec2(38.5, 0.0));
	CreateEdge(38, 0, b2Vec2(0, -38.5), b2Vec2(0, 38.5));
	CreateEdge(-38, 0, b2Vec2(0, -38.5), b2Vec2(0, 38.5));

	glutMainLoop();

	return 0;
}

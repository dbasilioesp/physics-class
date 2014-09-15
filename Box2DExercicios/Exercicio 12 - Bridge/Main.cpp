#include "Render.h"
#include "ForceFunctions.h"
#include "WorldObjects.h"
#include "Settings.h"
#include <GL/glut.h>
#include <cstdio>
#include <iostream>
#include <vector>


using namespace std;


int32 framePeriod = 16; //milliseconds
int32 height = 450;
int32 width = 450;
int32 mainWindow;
float32 timeStep;
int32 velocityIterations ;
int32 positionIterations ;
b2Vec2 viewCenter(0.0f, 0.0f);
b2World *world;  
DebugDraw renderer;  // Objeto com as rotinas de renderiza��o dos objetos
b2Body *bodySelected;
vector<b2Body*> bodies;
int32 selectedIndex = 0;
vector<b2Joint*> joints;


void CreateBridge()
{
	b2Body *leftBox = CreateBox(world, -18.0, -35.0, 10.0, 10.0, 1.0, 0.5, 0.5);
	leftBox->SetType(b2_staticBody);
	b2Body *rightBox = CreateBox(world, 18.0, -35.0, 10.0, 10.0, 1.0, 0.5, 0.5);
	rightBox->SetType(b2_staticBody);

	vector<b2Body*> blocks;
	for (int i = 0; i < 5; i++){
		blocks.push_back(
			CreateBox(world, -10.0 + i*5, -31.0, 5.0, 2.0, 1.0, 0.5, 0.5)
		);
	}
	
	{
		b2RevoluteJointDef rJointDef;
		rJointDef.Initialize(leftBox, blocks[0], leftBox->GetWorldPoint(b2Vec2(5.0, 5.0)));
		rJointDef.collideConnected = true;
		
		joints.push_back(
			world->CreateJoint(&rJointDef)
		);
	}

	{
		b2RevoluteJointDef rJointDef;
		int last = blocks.size() - 1;
		rJointDef.Initialize(rightBox, blocks[last], rightBox->GetWorldPoint(b2Vec2(-5.0, 5.0)));
		rJointDef.collideConnected = true;
		
		joints.push_back(
			world->CreateJoint(&rJointDef)
		);
	}

	for (int i = 0; i < blocks.size(); i++)
	{
		if(i+1 >= blocks.size())
			break;

		b2RevoluteJointDef rJointDef;
		rJointDef.Initialize(blocks[i], blocks[i+1], blocks[i]->GetWorldPoint(b2Vec2(2.5, 0.0)));
		rJointDef.collideConnected = true;

		joints.push_back(
			world->CreateJoint(&rJointDef)
		);
	}

}


void DrawJoint( b2Joint *joint, b2Color color)
{
	b2Vec2 anchor1, anchor2;
	anchor1 = joint->GetAnchorA();
	anchor2 = joint->GetAnchorB();

	glPointSize(5);
	glBegin(GL_POINTS);
	glVertex2f(anchor1.x,anchor1.y);
	glVertex2f(anchor2.x,anchor2.y);
	glEnd();
	glPointSize(1);

	renderer.DrawSegment(anchor1, anchor2, color);
}


//Rotina de Callback de redimensionamento da janela 
void Resize(int32 w, int32 h)
{
	ResizeWithProportion(w, h, -40.0, 40.0, -40.0, 40.0);
}


//Fun��o de inicializa��o da Box2D
void InitBox2D()
{
	b2Vec2 gravity(0.0f, -9.8f);

	world = new b2World(gravity);

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

// Callback de temporizador da GLUT (pra n�o ficar muito r�pido)
void Timer(int)
{
	glutSetWindow(mainWindow);
	glutPostRedisplay();
	glutTimerFunc(framePeriod, Timer, 1);
}

// Callback de desenho da GLUT, nela � chamada a rotina que chama o passo da simula��o
void SimulationLoop()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	RunBox2D();
	
	glColor3f(1,0,0);
	glPointSize(5);

	b2Color color; color.r = 1.0; color.g = 0.0; color.b = 0.0;

	b2Body *b;
	for(b = world->GetBodyList(); b; b=b->GetNext()){
		renderer.DrawFixture(b->GetFixtureList(),color);
	}

	for (int i = 0; i < joints.size(); i++)
		DrawJoint(joints[i], color);
	
	glutSwapBuffers();

}

// Rotina de callback de teclado da GLUT
void Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
    case 27:  //Sai do programa
		{
			world->~b2World();
			system("exit");
			exit(0);
			break;
		}
	}

	glutPostRedisplay();
}

//Main :)
int main(int argc, char** argv)
{
	char title[32];
	sprintf(title, "Box2D Version %d.%d.%d", b2_version.major, b2_version.minor, b2_version.revision);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(height, width);
	mainWindow = glutCreateWindow(title);
	glutDisplayFunc(SimulationLoop);
	glutReshapeFunc(Resize);
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(framePeriod, Timer, 1);  //Usa um timer para controlar o frame rate.

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	InitBox2D();

	Create4Walls(world);
	CreateBridge();

	glutMainLoop();

	return 0;
}
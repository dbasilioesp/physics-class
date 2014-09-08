#include "Render.h"
#include <GL/glut.h>

#include <cstdio>
#include <iostream>


int32 framePeriod = 16; //milliseconds
int32 mainWindow;

using namespace std;

int height=450, width=450;
float32 timeStep;
int32 velocityIterations ;
int32 positionIterations ;

int tx, ty, tw, th;
b2Vec2 viewCenter(0.0f, 0.0f);

// O objeto World serve para armazenar os dados da simula��o
b2World *world;

//Objetos ch�o, paredes e teto
b2Body* chao;
b2Body* paredeEsquerda;
b2Body* paredeDireita;
b2Body* teto;

//Objeto caixa
b2Body *caixa;

//Objeto com as rotinas de renderiza��o dos objetos
DebugDraw renderer;

//Estrutuda de dados que armazena o ponto do objeto onde a for�a foi aplicada e a for�a (vetor)
struct forcaAplicada
{
	b2Vec2 vetorForca;
	b2Vec2 pontoAplicado;
	forcaAplicada *proxima;
};

//Lista encadeada das for�as aplicadas em determinado instante de simula��o
forcaAplicada *ListaDeForcasAplicadas = NULL;
forcaAplicada *ultima = NULL;

//Rotina que calcula a norma (m�dulo) de um vetor
float norma(b2Vec2 v) {
      return sqrt(v.x*v.x + v.y*v.y);
}

//Rotina que normaliza um vetor
b2Vec2 normaliza(b2Vec2 v)
{
	float normav = norma(v);
	v.x = v.x / normav;
	v.y = v.y / normav;
	return v;
}

//Rotina que converte graus para radianos
float GrausParaRadianos(float angulo)
{
	return angulo*b2_pi/180;
}

//Rotina que converte radianos para graus
float RadianosParaGraus(float angle){
    return angle*180/b2_pi;
}

//Rotina que calcula as componentes vx e vy do vetor formado pelas posicoes x e y e um angul//Rotina que calcula as componentes vx e vy do vetor formado pelas posicoes x e y e um angulo
b2Vec2 CalculaComponentesDoVetor(float x, float y, float angulo)
{
	float v = sqrt(x*x+y*y);
	float angulorad = GrausParaRadianos(angulo);
	float vx = v*cos(angulorad);
	float vy = v*sin(angulorad);
	b2Vec2 vec(vx,vy);
	return vec;
}

//Rotina que calcula as componentes vx e vy do vetor formado pelo comprimento (magnitude) fornecido e o �ngulo
b2Vec2 CalculaComponentesDoVetor(float magnitude, float angulo)
{
	float v = magnitude;
	float angulorad = GrausParaRadianos(angulo);
	float vx = v*cos(angulorad);
	float vy = v*sin(angulorad);
	b2Vec2 vec(vx,vy);
	return vec;
}

//Rotina que adiciona uma for�a aplicada na lista
void AdicionaForcaAplicada(b2Vec2 p, b2Vec2 v)
{
	forcaAplicada *nova = new forcaAplicada();
	nova->pontoAplicado = p;
	nova->vetorForca = normaliza(v);
	nova->proxima = NULL;
	if (ListaDeForcasAplicadas == NULL) //primeira da lista
	{	
		ListaDeForcasAplicadas = nova;
		ultima = nova;
	}
	else
	{
		ultima->proxima = nova;
		ultima = nova;
	}
}

//Rotina para a cria��o de um objeto caixa parametrizado
b2Body *criaCaixa(float posX, float posY, float altura, float largura, float massa, float coefatrito, float coefrestituicao)
{
	//Cria o novo objeto (objeto novoObjeto)
	b2Body *objeto;

	//1� passo: cria��o da defini��o do corpo (b2BodyDef)
	b2BodyDef b;
	b.position.Set(posX,posY);
	b.type = b2_dynamicBody;
	//2� passo: cria��o do corpo pelo mundo (mundo cria corpo)
	objeto = world->CreateBody(&b);
	//3� passo: cria��o da defini��o da forma (b2PolygonShape, b2CircleShape ou b2EdgeShape)
	b2PolygonShape caixa;
	caixa.SetAsBox(largura/2, altura/2);
	//4� passo: cria��o da defini��o da fixture (b2FixtureDef)
		//N�o esquecer de associar a forma com a fixture!
	b2FixtureDef f;
	f.shape = &caixa;
	f.density = massa/(altura*largura);
	f.friction = coefatrito;
	f.restitution = coefrestituicao;
	//5� passo: cria��o da fixture pelo corpo (objeto cria fixture)
	objeto->CreateFixture(&f);
	
	return objeto;
}

//Rotina que desenha o vetor (normalizado * 10) das for�as aplicadas
void DesenhaForcasAplicadas()
{
	forcaAplicada *p;
	p = ListaDeForcasAplicadas;
	//Percorre a lista
	while (p != NULL)
	{
		glColor3f(0,0,1);
		b2Vec2 pFinal;

		glLineWidth(3);
		glBegin(GL_LINES);
		glVertex2d(p->pontoAplicado.x,p->pontoAplicado.y);
		pFinal.x =  p->pontoAplicado.x + p->vetorForca.x * 5;
		pFinal.y =  p->pontoAplicado.y + p->vetorForca.y * 5;
		glVertex2d( pFinal.x,pFinal.y);
		glEnd();
		glLineWidth(1);

		glPointSize(5);
		glBegin(GL_POINTS);
		glVertex2d( pFinal.x,pFinal.y);
		glEnd();
		//Atualiza o ponteiro com a proxima posicao
		p = p->proxima;		
	}

	//Reseta a lista
	ListaDeForcasAplicadas = NULL;
	ultima = NULL;
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

////Rotina que chama os m�todos de desenho da classe DebugDraw para desenhar os objetos da cena
void DrawFixture(b2Fixture* fixture, b2Color color)
	{
		
		const b2Transform& xf = fixture->GetBody()->GetTransform();

		switch (fixture->GetType())
		{
		case b2Shape::e_circle:
			{
				b2CircleShape* circle = (b2CircleShape*)fixture->GetShape();

				b2Vec2 center = b2Mul(xf, circle->m_p);
				float32 radius = circle->m_radius;

				renderer.DrawCircle(center, radius, color);
			}
			break;

		case b2Shape::e_polygon:
			{
				b2PolygonShape* poly = (b2PolygonShape*)fixture->GetShape();
				int32 vertexCount = poly->m_count;
				b2Assert(vertexCount <= b2_maxPolygonVertices);
				b2Vec2 vertices[b2_maxPolygonVertices];

				for (int32 i = 0; i < vertexCount; ++i)
				{
					vertices[i] = b2Mul(xf, poly->m_vertices[i]);
				}

				renderer.DrawPolygon(vertices, vertexCount, color);
			}
		
			break;
		case b2Shape::e_edge:
			{
				b2EdgeShape* edge = (b2EdgeShape*)fixture->GetShape();
				int32 vertexCount;
				
				b2Vec2 vertices[b2_maxPolygonVertices];
				int i=0;

				if (edge->m_hasVertex0) 
				{
						vertices[i] = b2Mul(xf, edge->m_vertex0);
						i++;
				}
				vertices[i] = b2Mul(xf, edge->m_vertex1); i++;
				vertices[i] = b2Mul(xf, edge->m_vertex2); i++;
				if (edge->m_hasVertex3) 
				{
						vertices[i] = b2Mul(xf, edge->m_vertex3);
						i++;
				}
				
				vertexCount = i;
				renderer.DrawPolygon(vertices, vertexCount, color);
			}
		
			break;
			
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

	//Chama a rotina que chama o passo da simula��o
	RunBox2D();

	//Define a cor dos objetos como vermelha
	b2Color color; color.r = 1.0; color.g = 0.0; color.b = 0.0;
	
	//Desenha os objetos
	if (chao != NULL)
		DrawFixture(chao->GetFixtureList(),color);

	if (paredeEsquerda != NULL)
		DrawFixture(paredeEsquerda->GetFixtureList(),color);

	if (paredeDireita != NULL)
		DrawFixture(paredeDireita->GetFixtureList(),color);

	if (teto != NULL)
		DrawFixture(teto->GetFixtureList(),color);

	if (caixa != NULL)
		DrawFixture(caixa->GetFixtureList(),color);

	//Desenha as for�as aplicadas
	DesenhaForcasAplicadas();
	
	glutSwapBuffers();

}

//Rotina de callback de teclado da GLUT
void Keyboard(unsigned char key, int x, int y)
{

	switch (key)
	{
    //Aplica for�a sobre o corpo
	case 'f':
		{
		b2Vec2 vetorForca;
		vetorForca = CalculaComponentesDoVetor(10000, 110);
		
		//Para aplicar a for�a no centro de massa do objeto
		caixa->ApplyForceToCenter(vetorForca,true);
		
		AdicionaForcaAplicada(caixa->GetWorldCenter(),vetorForca);

		//Para aplicar um impulso linear (quantidade de movimento gerada pela aplica��o de uma for�a)...
		//caixa->ApplyLinearImpulse(vetorForca,caixa->GetWorldCenter());
		
		//Para aplicar for�as em outro ponto do objeto...
		//b2Vec2 pontoLocal, pontoGlobal;
		//Pega a posicao do ponto no mundo, dado um ponto fornecido em coordenadas locais (0,0) � o centro
		/*pontoLocal.x = -3.0;
		pontoLocal.y = 4.0;
		pontoGlobal = caixa->GetWorldPoint(pontoLocal);		
		ponto.x = caixa->GetWorldPoint(b2Vec2(-3.0,4.0)).x;
		ponto.y = caixa->GetWorldPoint(b2Vec2(-3.0,4.0)).y;	
		caixa->ApplyForce(vetorForca, pontoGlobal);
		AdicionaForcaAplicada(pontoGlobal,vetorForca);*/

		//Para aplicar um impulso linear...
		//caixa->ApplyLinearImpulse(vetorForca,pontoGlobal);
		
		//b2Vec2 vetorVelocidade;
		//vetorVelocidade = CalculaComponentesDoVetor(30,45);
		//caixa->SetLinearVelocity(vetorVelocidade);

		break;
		}
    //Aplica torque sobre o corpo
	case 't':
		{
			//Para aplicar um torque no objeto (girar ao redor do seu centro)
			//caixa->ApplyTorque(1000);

			//Para aplicar um impulso angular no objeto (quantidade de movimento gerado pela aplica��o de um torque)
			//caixa->ApplyAngularImpulse(-1000);
		break;
		}
    //Sai do programa
	case 27:
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

	//Cria o ch�o
	b2BodyDef bd;
	chao = world->CreateBody(&bd);
	//Forma do ch�o: edge
	b2EdgeShape shape;
	shape.Set(b2Vec2(-39.5, -39.5), b2Vec2(39.5, -39.5));
	chao->CreateFixture(&shape,0.0);

	//Cria o teto
	teto = world->CreateBody(&bd);
	shape.Set(b2Vec2(-39.5, 39.5), b2Vec2(39.5, 39.5));
	teto->CreateFixture(&shape,0.0);

	//Cria a parede esquerda
	paredeEsquerda = world->CreateBody(&bd);
	shape.Set(b2Vec2(-39.5, 39.5), b2Vec2(-39.5, -39.5));
	paredeEsquerda->CreateFixture(&shape,0.0);

	//Cria a parede direita
	paredeDireita = world->CreateBody(&bd);
	shape.Set(b2Vec2(39.5, 39.5), b2Vec2(39.5, -39.5));
	paredeDireita->CreateFixture(&shape,0.0);


	//Cria uma caixa na posicao (0,0), com 8m de altura, 6m de largura, 3kg, coeficiente de atrito e restitui��o 0.5
	caixa = criaCaixa(0.0, 0.0, 8.0,6.0,3.0,0.5,0.5);

	glutMainLoop();

	return 0;
}
/*
 * GLUT Shapes Demo
 *
 * Written by Nigel Stewart November 2003
 *
 * This program is test harness for the sphere, cone
 * and torus shapes in GLUT.
 *
 * Spinning wireframe and smooth shaded shapes are
 * displayed until the ESC or q key is pressed.  The
 * number of geometry stacks and slices can be adjusted
 * using the + and - keys.
 */
#include<windows.h>
#include <GL/glut.h>
#include <GL/freeglut_ext.h>

#include <time.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <string>

#include "mundo.h"
#include "nave.h"
#include "proyectil.h"
#include "aerolito.h"
#include "partida.h"
#include <irrklang/irrKlang.h>

#define BUTTON_UP   0
#define BUTTON_DOWN 1

#define MOUSE_LEFT   0
#define MOUSE_MIDDLE 1
#define MOUSE_RIGHT  2

using namespace irrklang;
using std::string;

ISoundEngine *SoundEngine = createIrrKlangDevice();

unsigned char keyState[255];
unsigned char mouseState[3];

static int frames = 0;
static double lastTime = 0;
static double elapsedTime=0;
static double paso=0;
static double fps = 0;
static int slices = 3;
static int stacks = 3;

static double radioCritico1 = 4;
static double radioCritico2 = 3;
static double radioCritico3 = 2;


static double alpha = 1;


static int contador=0;
static double maxAngularSpeed = 2;
int destroyedAerolitos=0;
int aerolitosToRemove[20];
int createdAerolitos=0;
bool collisionsEnabled;
double rigidez=.9;

static GLdouble factorRelentizante = 2;

static GLdouble speedZ = 0;

enum Estado
{
    JUGANDO, MENU, OPCIONES
};

struct pareja
{
    int i, j;
};

pareja colisionPairs[200];
pareja lastColisionPairs[200];
static int colisionsNumber;
static int lastColisionsNumber;
int asteroidesNaveColliding[10];
int asteroidesProyectilColliding[10];
Estado estado = MENU;

Partida partida;
Nave nave;
Mundo mundo;
Aerolito aerolitos[100];

static GLdouble posZ = 0;
static float ar = 1.28f;
static float frustum =.678f;
static float viewWidth = 20;
static float viewHeight = 10/ar;

float xMax, yMax;
float margen = 1.07f;
bool fullscreen=false;

void keyboard(unsigned char key, int x, int y)
{
    keyState[key] = BUTTON_DOWN;
}

void keyboard_up(unsigned char key, int x, int y)
{
    keyState[key] = BUTTON_UP;
}

void mouse(int button, int button_state, int x, int y)
{
#define state ( (button_state == GLUT_DOWN) ? BUTTON_DOWN : BUTTON_UP ) // shortens later code

    switch (button)
    {
    case 0:
        mouseState[MOUSE_LEFT] = state;
        break;
    case 1:
        mouseState[MOUSE_MIDDLE] = state;
        break;
    case 2:
        mouseState[MOUSE_RIGHT] = state;
        break;
    }

#undef state // make sure the defined "state" code above is only used in this function
}



void renderBitmapString(float x, float y, float z, void *font,	char *string)
{
    char *c;
    glRasterPos3f(x, y,z);
    for (c=string; *c != '\0'; c++)
    {
        glutBitmapCharacter(font, *c);
    }
}



/* GLUT callback Handlers */

static void resize(int width, int height)
{
    ar = (float) width / (float) height;
    xMax = (viewWidth/2 + posZ)*margen;
    yMax =  xMax/ar;
    float fov= atan2(1,4);

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glFrustum(-ar, ar, -1, 1, ar, 100.0);
    //glFrustum(-ar, ar, -1.0, 1.0, 0.1, 100.0);
    //gluPerspective(fov,ar,-1,-100);
    gluLookAt(0,0,viewWidth/2.0,0,0,posZ,0,1,0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity() ;
}

static void calculoEstadoNave()
{
    double resistencia = mundo.viscosidad*200;
    if(!nave.destroyed)
    {
        nave.velocidad.x+=-nave.inpulse*paso*sin(nave.inpulseAngle*M_PI/180)/30;
        nave.velocidad.y+=nave.inpulse*paso*cos(nave.inpulseAngle*M_PI/180)/30;
        resistencia = mundo.viscosidad*3500;

    }

    nave.posicion.angle=nave.posicion.angle+nave.angularSpeed*paso;
    nave.velocidad.speed=sqrt(nave.velocidad.x*nave.velocidad.x + nave.velocidad.y* nave.velocidad.y);

    nave.posicion.x+=nave.velocidad.x*paso/factorRelentizante;
    nave.posicion.y+=nave.velocidad.y*paso/factorRelentizante;
    posZ+=speedZ/factorRelentizante;

    if (nave.angularSpeed>-resistencia && nave.angularSpeed<resistencia  && !nave.girando)
        nave.angularSpeed=0;
    if (nave.angularSpeed>0 && !nave.girando)
        nave.angularSpeed-=resistencia*paso;
    if (nave.angularSpeed<0 && !nave.girando)
        nave.angularSpeed+=resistencia*paso;
    // if (speed>-mundo.viscosidad/2 && speed<mundo.viscosidad/2) speed=0;
    //if (speed>0) speed-=mundo.viscosidad/2;
    if (nave.inpulse>0 && (nave.velocidad.x>0 || nave.velocidad.x<0))
        nave.velocidad.x-=nave.velocidad.x*mundo.viscosidad*1000*paso;
    if (nave.inpulse>0 && (nave.velocidad.y>0 || nave.velocidad.y<0))
        nave.velocidad.y-=nave.velocidad.y*mundo.viscosidad*1000*paso;
    if (nave.inpulse>0 && (speedZ>0 || speedZ<0))
        speedZ-=speedZ*mundo.viscosidad*1000*paso;
    if (nave.velocidad.x>0)
        nave.velocidad.x-=mundo.viscosidad*paso;
    if (nave.velocidad.y>0)
        nave.velocidad.y-=mundo.viscosidad*paso;
    if (speedZ>0)
        speedZ-=mundo.viscosidad;
    if (nave.velocidad.x<0)
        nave.velocidad.x+=mundo.viscosidad*paso;
    if (nave.velocidad.y<0)
        nave.velocidad.y+=mundo.viscosidad*paso;
    if (speedZ<0)
        speedZ+=mundo.viscosidad;


    if (nave.posicion.x<-xMax )
        nave.posicion.x += 2*xMax;
    if(nave.posicion.x>xMax)
        nave.posicion.x -= 2*xMax;
    if (nave.posicion.y<-yMax )
        nave.posicion.y += 2*yMax;
    if(nave.posicion.y>yMax)
        nave.posicion.y -= 2*yMax;


    if (posZ>viewWidth)
        posZ = -100;
    if (posZ<-100)
        posZ = viewWidth;

    if(nave.angularSpeed>maxAngularSpeed || nave.angularSpeed<-maxAngularSpeed) nave.angularSpeed=maxAngularSpeed;


}

static void calculoEstadoAerolito()
{
    for(int i=0; i<partida.nAerolitos; i++)
    {

        if(aerolitos[i].angularSpeedX>maxAngularSpeed || aerolitos[i].angularSpeedX<-maxAngularSpeed) aerolitos[i].angularSpeedX=maxAngularSpeed;
        if(aerolitos[i].angularSpeedY>maxAngularSpeed || aerolitos[i].angularSpeedX<-maxAngularSpeed) aerolitos[i].angularSpeedY=maxAngularSpeed;
        if(aerolitos[i].angularSpeedZ>maxAngularSpeed || aerolitos[i].angularSpeedX<-maxAngularSpeed) aerolitos[i].angularSpeedZ=maxAngularSpeed;
        aerolitos[i].posicion.angle=aerolitos[i].posicion.angle+aerolitos[i].angularSpeedZ*paso;
        aerolitos[i].angleX=aerolitos[i].angleX+aerolitos[i].angularSpeedX*paso;
        aerolitos[i].angleY=aerolitos[i].angleY+aerolitos[i].angularSpeedY*paso;
        aerolitos[i].angleZ=aerolitos[i].angleZ+aerolitos[i].angularSpeedZ*paso;

        aerolitos[i].velocidad.speed=sqrt(aerolitos[i].velocidad.x*aerolitos[i].velocidad.x + aerolitos[i].velocidad.y* aerolitos[i].velocidad.y);

        aerolitos[i].posicion.x+=aerolitos[i].velocidad.x*paso/factorRelentizante;
        aerolitos[i].posicion.y+=aerolitos[i].velocidad.y*paso/factorRelentizante;
        posZ+=speedZ*paso/factorRelentizante;


        if (aerolitos[i].posicion.x<-xMax )
            aerolitos[i].posicion.x += 2*xMax;
        if(aerolitos[i].posicion.x>xMax)
            aerolitos[i].posicion.x -= 2*xMax;
        if (aerolitos[i].posicion.y<-yMax )
            aerolitos[i].posicion.y += 2*yMax;
        if(aerolitos[i].posicion.y>yMax)
            aerolitos[i].posicion.y -= 2*yMax;

    }
}

static void calculoEstadoProyectil()
{
    for(int i = 0; i<nave.numProyectiles; i++)
    {
        if(nave.armamento[i].flying)
        {
            nave.armamento[i].posicion.x -= nave.armamento[i].speed*paso*sin(nave.armamento[i].posicion.angle*M_PI/180)/3;
            nave.armamento[i].posicion.y += nave.armamento[i].speed*paso*cos(nave.armamento[i].posicion.angle*M_PI/180)/3;
            nave.armamento[i].alcance -=0.035*paso;
            if(nave.armamento[i].alcance<0.1)
            {
                nave.armamento[i].flying=false;
            }
        }
    }
}

static void rebote(int i)
{
    // Distance between balls
    float fDistance = sqrt(pow(nave.posicion.x-aerolitos[i].posicion.x,2)+pow(nave.posicion.y-aerolitos[i].posicion.y,2));
    // Normal
    float nx = (aerolitos[i].posicion.x - nave.posicion.x) / fDistance;
    float ny = (aerolitos[i].posicion.y - nave.posicion.y) / fDistance;

    // Tangent
    float tx = -ny;
    float ty = nx;

    // Dot Product Tangent
    float dpTan1 = nave.velocidad.x * tx + nave.velocidad.y * ty;
    float dpTan2 = aerolitos[i].velocidad.x * tx + aerolitos[i].velocidad.y * ty;

    // Dot Product Normal
    float dpNorm1 = nave.velocidad.x * nx + nave.velocidad.y * ny;
    float dpNorm2 = aerolitos[i].velocidad.x * nx + aerolitos[i].velocidad.y * ny;

    // Conservation of momentum in 1D
    float m1 = (dpNorm1 * (nave.radius - aerolitos[i].radius) + 2.0f * aerolitos[i].radius * dpNorm2) / (nave.radius + aerolitos[i].radius);
    float m2 = (dpNorm2 * (aerolitos[i].radius - nave.radius) + 2.0f * nave.radius * dpNorm1) / (nave.radius + aerolitos[i].radius);


    double r1=nave.radius;
    double r2=aerolitos[i].radius;

    // Update ball velocities
    nave.velocidad.x = (tx * dpTan1 + nx * m1);
    nave.velocidad.y = (ty * dpTan1 + ny * m1);
    nave.angularSpeed = ((dpTan2-dpTan1)*r2/r1 +(aerolitos[i].angularSpeedZ*r2 - nave.angularSpeed*r1)*r2/r1);
    aerolitos[i].velocidad.x = (tx * dpTan2 + nx * m2);
    aerolitos[i].velocidad.y = (ty * dpTan2 + ny * m2);
    aerolitos[i].angularSpeedZ = ((dpTan1 - dpTan2)*r1/r2  + (nave.angularSpeed*r1 - aerolitos[i].angularSpeedZ*r2 )*r1/r2);

}

static void reboteAerolitos(int i, int j)
{
    // Distance between balls
    float fDistance = sqrt(pow(aerolitos[i].posicion.x-aerolitos[j].posicion.x,2)+pow(aerolitos[i].posicion.y-aerolitos[j].posicion.y,2));
    // Normal
    float nx = (aerolitos[j].posicion.x - aerolitos[i].posicion.x) / fDistance;
    float ny = (aerolitos[j].posicion.y - aerolitos[i].posicion.y) / fDistance;

    // Tangent
    float tx = -ny;
    float ty = nx;

    // Dot Product Tangent
    float dpTan1 = aerolitos[i].velocidad.x * tx + aerolitos[i].velocidad.y * ty;
    float dpTan2 = aerolitos[j].velocidad.x * tx + aerolitos[j].velocidad.y * ty;

    // Dot Product Normal
    float dpNorm1 = aerolitos[i].velocidad.x * nx + aerolitos[i].velocidad.y * ny;
    float dpNorm2 = aerolitos[j].velocidad.x * nx + aerolitos[j].velocidad.y * ny;

    // Conservation of momentum in 1D
    float m1 = (dpNorm1 * (aerolitos[i].radius - aerolitos[j].radius) + 2.0f * aerolitos[j].radius * dpNorm2) / (aerolitos[i].radius + aerolitos[j].radius);
    float m2 = (dpNorm2 * (aerolitos[j].radius - aerolitos[i].radius) + 2.0f * aerolitos[i].radius * dpNorm1) / (aerolitos[i].radius + aerolitos[j].radius);


    double r1=aerolitos[i].radius;
    double r2=aerolitos[j].radius;

    // Update ball velocities
    aerolitos[i].velocidad.x = (tx * dpTan1 + nx * m1);
    aerolitos[i].velocidad.y = (ty * dpTan1 + ny * m1);
    aerolitos[i].angularSpeedX = ((aerolitos[j].angularSpeedX*r2 - aerolitos[i].angularSpeedX*r1)*r2/r1); //((dpTan1 - dpTan2)*r2/r1 +
    aerolitos[i].angularSpeedY = ((aerolitos[j].angularSpeedY*r2 - aerolitos[i].angularSpeedY*r1)*r2/r1); //((dpTan1 - dpTan2)*r2/r1 +
    aerolitos[i].angularSpeedZ = ((aerolitos[j].angularSpeedZ*r2 - aerolitos[i].angularSpeedZ*r1)*r2/r1); //((dpTan1 - dpTan2)*r2/r1 +
    aerolitos[j].velocidad.x = (tx * dpTan2 + nx * m2);
    aerolitos[j].velocidad.y = (ty * dpTan2 + ny * m2);
    aerolitos[j].angularSpeedX =  ((aerolitos[i].angularSpeedX*r1 - aerolitos[j].angularSpeedX*r2 )*r1/r2); //((dpTan2 - dpTan1)*r1/r2 );// +
    aerolitos[j].angularSpeedY =  ((aerolitos[i].angularSpeedY*r1 - aerolitos[j].angularSpeedY*r2 )*r1/r2); //((dpTan2 - dpTan1)*r1/r2 );// +
    aerolitos[j].angularSpeedZ =  ((aerolitos[i].angularSpeedZ*r1 - aerolitos[j].angularSpeedZ*r2 )*r1/r2); //((dpTan2 - dpTan1)*r1/r2 );// +
    r1=r1;
}

Aerolito newAerolito(double radio, double velocidad, double angulo)
{
    Aerolito aerolito = Aerolito();

    double red = (double) rand()/RAND_MAX;
    double green = (double) rand()/RAND_MAX;
    double blue = (double) rand()/RAND_MAX;
    Color color = Color(0.3 + red/3,0.3 + green /3,0.3 + blue/3);
    double auxi = rand()%10;

    aerolito.radius =radio;
    aerolito.color = color;
    aerolito.slices =slices + rand()%6;
    aerolito.stacks =stacks + rand()%6;
    aerolito.dureza =partida.dificultad*2;
    aerolito.velocidad.speed = velocidad;
    aerolito.posicion.angle = 2*M_PI*(double)rand()/RAND_MAX;
    auxi = rand()%5;
    aerolito.angularSpeedX = auxi/20;
    auxi = rand()%5;
    aerolito.angularSpeedY = auxi/20;
    auxi = rand()%5;
    aerolito.angularSpeedZ = auxi/10;
    int distancia = 4+rand()%4;
    aerolito.velocidad.angle = angulo;
    aerolito.posicion.x=-distancia*sin(aerolito.posicion.angle);
    aerolito.posicion.y=distancia*cos(aerolito.posicion.angle);
    aerolito.posicion.z=posZ;
    aerolito.velocidad.x=-aerolito.velocidad.speed*sin(aerolito.velocidad.angle);
    aerolito.velocidad.y=aerolito.velocidad.speed*cos(aerolito.velocidad.angle);
    return aerolito;
}
void addAerolitoSon(int i, int partes){
    double radio =aerolitos[i].radius;
    double radio_2 = radio/partes+0.001*(double)rand()/RAND_MAX;
    double angulo_2 =aerolitos[i].velocidad.angle - M_PI_2 + M_PI*(double)rand()/RAND_MAX;
    double velocidad_2 =(aerolitos[i].velocidad.speed/2)+aerolitos[i].velocidad.speed*2*(double)rand()/RAND_MAX;
    aerolitos[partida.nAerolitos]= newAerolito(radio_2, velocidad_2, angulo_2);
    aerolitos[partida.nAerolitos].velocidad.x = -velocidad_2*sin(angulo_2);
    aerolitos[partida.nAerolitos].velocidad.y = +velocidad_2*cos(angulo_2);
    aerolitos[partida.nAerolitos].posicion.x = aerolitos[i].posicion.x - radio/2 + radio*(double)rand()/RAND_MAX;
    aerolitos[partida.nAerolitos].posicion.y = aerolitos[i].posicion.y - radio/2 + radio*(double)rand()/RAND_MAX;
    aerolitos[partida.nAerolitos].color = aerolitos[i].color;
    aerolitos[partida.nAerolitos].angularSpeedX = maxAngularSpeed*(double)rand()/RAND_MAX;
    aerolitos[partida.nAerolitos].angularSpeedY = maxAngularSpeed*(double)rand()/RAND_MAX;
    aerolitos[partida.nAerolitos].angularSpeedZ = maxAngularSpeed*(double)rand()/RAND_MAX;
    aerolitos[partida.nAerolitos].intouch = true;
    partida.nAerolitos++;
}

void addAerolito(int i, int level){
}

void parteAerolito(int i)
{

    double radio =aerolitos[i].radius;
    if(radio<aerolitos[i].minRadius)
    {
        aerolitos[i].destroyed = true;
        aerolitosToRemove[destroyedAerolitos]=i;
        destroyedAerolitos++;
    }
    else
    {
        double radio_1 =radio/2;

        aerolitos[i].intouch = true;
        if(radio*4>radioCritico1)
        {
            for(int j = 0; j<radioCritico1-1; j++)
            {
                addAerolitoSon(i,radioCritico1*.6);
            }
            aerolitos[i].radius = radio/(radioCritico1*.6);
        }
        else if(radio*3>radioCritico2)
        {
            for(int j = 0; j<radioCritico2-1; j++)
            {
                addAerolitoSon(i,radioCritico2*.4);
            }
            aerolitos[i].radius = radio/(radioCritico2*.4);
        }
        else
        {
            for(int j = 0; j<radioCritico3-1; j++)
            {
                addAerolitoSon(i,radioCritico3*1.1);
            }
            aerolitos[i].radius = radio/(radioCritico3*1.1);
        }
        double velocidad_1 =aerolitos[i].velocidad.speed*2*(double)rand()/RAND_MAX;
        double angulo_1 =aerolitos[i].velocidad.angle - M_PI_2 + M_PI*(double)rand()/RAND_MAX;
        aerolitos[i].velocidad.speed = (aerolitos[i].velocidad.speed/2)+2*velocidad_1*(double)rand()/RAND_MAX;
        aerolitos[i].posicion.x = aerolitos[i].posicion.x - radio/2 + radio*(double)rand()/RAND_MAX;
        aerolitos[i].posicion.y = aerolitos[i].posicion.y - radio/2 + radio*(double)rand()/RAND_MAX;
        aerolitos[i].velocidad.x = -velocidad_1*sin(angulo_1);
        aerolitos[i].velocidad.y = +velocidad_1*cos(angulo_1);
        aerolitos[i].velocidad.angle = angulo_1;

    }

}



static void colisiones(void)
{
    colisionsNumber=0;
    destroyedAerolitos =0;
    int choques = 0;
    int impactosNumber = 0;

    // Impactos Proyectil Aerolito
    for (int i = 0; i<partida.nAerolitos; i++)
    {
        if(!aerolitos[i].destroyed)
        {
            for(int j = 0; j<nave.numProyectiles; j++)
            {
                if(nave.armamento[j].flying)
                {
                    double distancia = sqrt(pow(aerolitos[i].posicion.x-nave.armamento[j].posicion.x,2)+pow(aerolitos[i].posicion.y-nave.armamento[j].posicion.y,2));
                    double distMin= aerolitos[i].radius+nave.armamento[j].radius;
                    if((distancia<=distMin)*.8f) // && !(nave.destroyed || aerolitos[i].destroyed))
                    {
                        asteroidesProyectilColliding[impactosNumber]=i;

                        impactosNumber++;
                        //parteAerolito(i);
                        nave.armamento[j].flying = false;
                    }
                }
            }
        }
    }
    // Procesamos los impactos
    for(int k = 0; k<impactosNumber; k++)
    {
        parteAerolito(asteroidesProyectilColliding[k]);
    }
    // Elimina los asteroides destruidos
    for(int k = destroyedAerolitos-1; k>-1; k--)
    {
        for(int i = aerolitosToRemove[k]; i<partida.nAerolitos-1; i++)
        {
            aerolitos[i]=aerolitos[i+1];
        }
        partida.nAerolitos--;
    }

    // Colisiones
    for (int i = 0; i<partida.nAerolitos; i++)
    {
        // Colision Nave-Aerolito
        double distance = sqrt(pow(nave.posicion.x-aerolitos[i].posicion.x,2)+pow(nave.posicion.y-aerolitos[i].posicion.y,2));
        double dMin= nave.radius+aerolitos[i].radius;
        if(distance<=dMin*.85f) // && !(nave.destroyed || aerolitos[i].destroyed))
        {
            nave.destroyed = true;
            asteroidesNaveColliding[choques]=i;
            choques++;
            // Distance between ball centers
            float fDistance = sqrtf(pow(nave.posicion.x - aerolitos[i].posicion.x,2)
                                    + pow(nave.posicion.y - aerolitos[i].posicion.y,2));

            // Calculate displacement required
            float fOverlap = 0.5f * (distance - nave.radius - aerolitos[i].radius)*.85f;

            // Displace Current Ball away from collision
            nave.posicion.x -= fOverlap * (nave.posicion.x - aerolitos[i].posicion.x) / distance;
            nave.posicion.y -= fOverlap * (nave.posicion.y - aerolitos[i].posicion.y) / distance;

            // Displace Target Ball away from collision
            aerolitos[i].posicion.x += fOverlap * (nave.posicion.x - aerolitos[i].posicion.x) / distance;
            aerolitos[i].posicion.y += fOverlap * (nave.posicion.y - aerolitos[i].posicion.y) / distance;

        }
        // Colision Aerolito-Aerolito
        for (int j = i+1; j<partida.nAerolitos; j++)
        {
            if (i!=j)
            {
                distance = sqrt(pow(aerolitos[i].posicion.x-aerolitos[j].posicion.x,2)
                               +pow(aerolitos[i].posicion.y-aerolitos[j].posicion.y,2));
                double dMin= aerolitos[i].radius+aerolitos[j].radius;
                if(distance<=dMin*.7f) // && !(nave.destroyed || aerolitos[i].destroyed))
                {
                    // Collision has occured
                    pareja par;
                    par.i=i;
                    par.j=j;
                    colisionPairs[colisionsNumber]=par;
                    colisionsNumber++;

                    // Distance between ball centers
                    float fDistance = sqrtf(pow(aerolitos[i].posicion.x - aerolitos[j].posicion.x,2)
                                            + pow(aerolitos[i].posicion.y - aerolitos[j].posicion.y,2));

                    // Calculate displacement required
                    float fOverlap = 0.5f * (fDistance - aerolitos[i].radius - aerolitos[j].radius)*.4f;

                    // Displace Current Ball away from collision
                    aerolitos[i].posicion.x -= fOverlap * (aerolitos[i].posicion.x - aerolitos[j].posicion.x) / fDistance;
                    aerolitos[i].posicion.y -= fOverlap * (aerolitos[i].posicion.y - aerolitos[j].posicion.y) / fDistance;

                    // Displace Target Ball away from collision
                    aerolitos[j].posicion.x += fOverlap * (aerolitos[i].posicion.x - aerolitos[j].posicion.x) / fDistance;
                    aerolitos[j].posicion.y += fOverlap * (aerolitos[i].posicion.y - aerolitos[j].posicion.y) / fDistance;

                }
            }
        }
    }

    /*
    for(int i = 0; i<partida.nAerolitos; i++)
    {
        int choques = 0;
        if(!aerolitos[i].destroyed)
        {
            // Colision Nave-Aerolito
            double distance = sqrt(pow(nave.posicion.x-aerolitos[i].posicion.x,2)+pow(nave.posicion.y-aerolitos[i].posicion.y,2));
            double dMin= nave.radius+aerolitos[i].radius;
            if((distance/dMin)<0.85) // && !(nave.destroyed || aerolitos[i].destroyed))
            {
                nave.destroyed = true;
                rebote(i);
            }
            for(int k = destroyedAerolitos-1; k>-1; k--)
            {
                for(int i = aerolitosToRemove[k]; j<partida.nAerolitos-1; j++)
                {
                    aerolitos[i]=aerolitos[i+1];
                }
                partida.nAerolitos--;
            }


            // Colision Aerolito-Aerolito
            for(int j = i+1; j<partida.nAerolitos; j++)
            {

                distance = sqrt(pow(aerolitos[i].posicion.x-aerolitos[j].posicion.x,2)+pow(aerolitos[i].posicion.y-aerolitos[j].posicion.y,2));
                double dMin= aerolitos[i].radius+aerolitos[j].radius;
                if((distance/dMin)<0.80) // && !(nave.destroyed || aerolitos[i].destroyed))
                {
                    if(!aerolitos[i].intouch || !aerolitos[i].intouch)
                    {
                        pareja par;
                        par.i=i;
                        par.j=j;
                        colisionPairs[colisionsNumber + choques]=par;
                        choques++;
                        aerolitos[i].intouch = true;
                        aerolitos[j].intouch = true;
                    }
                }
            }

            if(choques==0)
                aerolitos[i].intouch = false;
            colisionsNumber += choques;

            // Colision Aerolito-Proyectil
            for(int j = 0; j<nave.numProyectiles; j++)
            {
                if(nave.armamento[j].flying)
                {
                    double distancia = sqrt(pow(aerolitos[i].posicion.x-nave.armamento[j].posicion.x,2)+pow(aerolitos[i].posicion.y-nave.armamento[j].posicion.y,2));
                    double distMin= aerolitos[i].radius+nave.armamento[j].radius;
                    if((distancia/distMin)<0.7) // && !(nave.destroyed || aerolitos[i].destroyed))
                    {
                        parteAerolito(i);
                        nave.armamento[j].flying = false;
                    }
                }
            }
        }
    }*/

    for(int k = 0; k<colisionsNumber; k++)
    {
        reboteAerolitos(colisionPairs[k].i, colisionPairs[k].j);
    }
    for(int k = 0; k<choques; k++)
    {
        rebote(asteroidesNaveColliding[k]);
    }

}

void displayCountDown()
{
    char number[2];
    int counter = 3;
    _sleep(100);
    while (counter >= 0)
    {
        printf("%d ", counter);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glColor4d(1,1,0,alpha);

        glPushMatrix();
        glTranslated(0,0,-1);
        glScaled(10.0,10.0,120.0);
        sprintf(number,"%d",counter);
        renderBitmapString(0.0f, 0.0f, 0.0f, GLUT_BITMAP_HELVETICA_18, number);

        glPopMatrix();
        glutSwapBuffers();
        _sleep(1000);
        counter--;
    }
}

static void reset()
{
    nave.velocidad.x=0;
    nave.velocidad.y=0;
    speedZ=0;
    nave.posicion.x=0;
    nave.posicion.y=0;
    posZ=0;
    nave.radius=0.07;
    nave.posicion.angle=0;
    nave.destroyed=false;
    for(int i = 0 ; i< nave.numProyectiles; i++)
    {
        Proyectil proyectil = Proyectil();
        proyectil.posicion=nave.posicion;
        nave.armamento[i]=proyectil;
    }

}


void setupJuego(int level)
{
    partida.gameOver = false;
    partida.nAerolitos = level;
    partida.dificultad= level;
    for(int i = 0; i<partida.nAerolitos; i++)
    {


        double red = (double) rand()/RAND_MAX;
        double green = (double) rand()/RAND_MAX;
        double blue = (double) rand()/RAND_MAX;
        Color color = Color(0.3 + red/3,0.3 + green /3,0.3 + blue/3);
        double auxi = rand()%10;

        aerolitos[i]= Aerolito();

        aerolitos[i].radius =0.6f+((double)rand()/RAND_MAX)*((double) level)/12;
        aerolitos[i].color = color;
        aerolitos[i].slices =slices + rand()%6;
        aerolitos[i].stacks =stacks + rand()%6;
        aerolitos[i].dureza =partida.dificultad*2;
        aerolitos[i].velocidad.speed = 0.05 + partida.dificultad*0.002f*((double)rand()/RAND_MAX);// + ((double)rand()/RAND_MAX)*(double)partida.dificultad/50;
        aerolitos[i].posicion.angle = 2*M_PI*(double)rand()/RAND_MAX;
        auxi = rand()%5;
        aerolitos[i].angularSpeedX = auxi/20;
        auxi = rand()%5;
        aerolitos[i].angularSpeedY = auxi/20;
        auxi = rand()%5;
        aerolitos[i].angularSpeedZ = auxi/10;
        int distancia = 10+rand()%2;
        aerolitos[i].velocidad.angle = 2*M_PI*(double)rand()/RAND_MAX;
        aerolitos[i].posicion.x=-distancia*sin(aerolitos[i].posicion.angle);
        aerolitos[i].posicion.y=distancia*cos(aerolitos[i].posicion.angle);
        aerolitos[i].posicion.z=posZ;
        aerolitos[i].velocidad.x=-aerolitos[i].velocidad.speed*sin(aerolitos[i].velocidad.angle);
        aerolitos[i].velocidad.y=aerolitos[i].velocidad.speed*cos(aerolitos[i].velocidad.angle);
        aerolitos[i].intouch=true;
    }
}



void newGame(int i)
{
    //displayCountDown();
    reset();
    setupJuego(i);
}

void disparaProyectil()
{
    for(int i = 0; i<nave.numProyectiles; i++)
    {
        if(!nave.armamento[i].flying)
        {
            nave.armamento[i].posicion= nave.posicion;
            nave.armamento[i].alcance = nave.armamento[i].strength;
            nave.armamento[i].flying=true;
            i=nave.numProyectiles;
        }
    }
}


static void calculaEstado(void)
{
    if(partida.nAerolitos==0)
    {
        alpha = alpha-paso/200;
        if(alpha <=0)
        {
            partida.dificultad++;
            setupJuego(partida.dificultad);
        }
    }else if(nave.destroyed)
    {
        alpha = alpha-paso/200;
        if(alpha <=0)
        {
            reset();
        }
    }else if(alpha<1)
        alpha+=paso/100;

    calculoEstadoNave();
    calculoEstadoAerolito();
    calculoEstadoProyectil();
    colisiones();
}


static void compruebaMouse(void)
{
    if (mouseState[MOUSE_LEFT] == BUTTON_DOWN) // Walk forward
    {
        disparaProyectil();

    }
    if (mouseState[MOUSE_RIGHT] == BUTTON_DOWN) // Walk forward
    {

    }

    if (mouseState[MOUSE_MIDDLE] == BUTTON_DOWN) // Walk forward
    {
    }
}
static void compruebaTeclado(void)
{

    // some fancy code

    if (keyState[(unsigned char)'I'] == BUTTON_DOWN || keyState[(unsigned char)'i'] == BUTTON_DOWN) // Walk forward
    {
        //glutFullScreenToggle();
        fullscreen = !fullscreen;
        if (fullscreen)
        {
            glutFullScreen();                /* Go to full screen */
        }
        else
        {
            glutReshapeWindow(800, 600);        /* Restore us */
            glutPositionWindow(400,200);
        }
        keyState['I'] = BUTTON_UP;
        keyState['i'] = BUTTON_UP;
    }
    if (keyState[(unsigned char)'n'] == BUTTON_DOWN || keyState[(unsigned char)'N'] == BUTTON_DOWN) // Walk forward
    {
        if (partida.dificultad<30)
        {
            setupJuego(partida.dificultad+1);
        }
        keyState['n'] = BUTTON_UP;
        keyState['N'] = BUTTON_UP;
    }
    if (keyState[(unsigned char)'b'] == BUTTON_DOWN || keyState[(unsigned char)'B'] == BUTTON_DOWN) // Walk forward
    {
        if (partida.dificultad>1)
        {
            setupJuego(partida.dificultad-1);
        }
        keyState['b'] = BUTTON_UP;
        keyState['B'] = BUTTON_UP;
    }
    if (keyState[(unsigned char)'j'] == BUTTON_DOWN || keyState[(unsigned char)'J'] == BUTTON_DOWN) // Walk forward
    {
        //glutFullScreenToggle();
        if(estado==MENU)
        {
            if(partida.gameOver)
            {
                newGame(1);
            }
            estado=JUGANDO;
        }
        else if(estado==JUGANDO)
        {
            partida.paused = true;
            estado=MENU;
        }
        keyState['j'] = BUTTON_UP;
        keyState['J'] = BUTTON_UP;
    }
    if (keyState[(unsigned char)'j'] == BUTTON_DOWN || keyState[(unsigned char)'J'] == BUTTON_DOWN) // Walk forward
    {
        //glutFullScreenToggle();
        if(estado==MENU)
        {
            estado=JUGANDO;
        }
        else if(estado==JUGANDO)
        {
            estado=MENU;
        }
        keyState['j'] = BUTTON_UP;
        keyState['J'] = BUTTON_UP;
    }

    if (keyState[(unsigned char)'o'] == BUTTON_DOWN || keyState[(unsigned char)'O'] == BUTTON_DOWN) // Walk forward
    {
        //glutFullScreenToggle();
        if(estado==MENU)
        {
            estado=OPCIONES;
        }
        else if(estado==OPCIONES)
        {
            estado=MENU;
        }
        keyState['o'] = BUTTON_UP;
        keyState['O'] = BUTTON_UP;
    }
    if (keyState[(unsigned char)'m'] == BUTTON_DOWN || keyState[(unsigned char)'M'] == BUTTON_DOWN) // Walk forward
    {
        //glutFullScreenToggle();
        if(estado==MENU)
        {
            estado=JUGANDO;
        }
        else if(estado==JUGANDO)
        {
            estado=MENU;
        }
        keyState['m'] = BUTTON_UP;
        keyState['M'] = BUTTON_UP;
    }
    if (keyState['s'] == BUTTON_DOWN || keyState[(unsigned char)'S'] == BUTTON_DOWN) // Walk forward
    {
        //glutFullScreenToggle();
        if(estado==MENU)
        {
            exit(0);
        }
    }

    if (keyState[13] == BUTTON_DOWN) // Walk forward
    {
        disparaProyectil();

        keyState[13] = BUTTON_UP;
    }
    if (keyState[27] == BUTTON_DOWN) // Walk forward
    {
        //glutFullScreenToggle();
        if(estado==MENU)
        {
            exit(0);
        }
        else if(estado==JUGANDO || estado==OPCIONES)
        {
            estado=MENU;
        }
        keyState[27] = BUTTON_UP;
    }

    if (keyState[(unsigned char)'+'] == BUTTON_DOWN) // Walk forward
    {
        nave.radius+=.01;

        keyState['+'] = BUTTON_UP;
    }

    if (keyState[(unsigned char)'-'] == BUTTON_DOWN) // Walk forward
    {
        if (nave.radius>0.01)
        {
            nave.radius-=.01;
        }

        keyState['-'] = BUTTON_UP;
    }


    if (keyState[(unsigned char)'a'] == BUTTON_DOWN || keyState[(unsigned char)'A'] == BUTTON_DOWN) // Spin left
    {

        if(!nave.destroyed)
        {
            if (nave.angularSpeed<nave.maxAngularSpeed)
            {
                nave.girando = true;
                nave.angularSpeed+=nave.repris*50*paso;
            }
        }
    }
    if (keyState[(unsigned char)'d'] == BUTTON_DOWN || keyState[(unsigned char)'D'] == BUTTON_DOWN) // Spin right
    {

        if(!nave.destroyed)
        {
            if (nave.angularSpeed>-nave.maxAngularSpeed)
            {
                nave.girando = true;
                nave.angularSpeed-=nave.repris*50*paso;
            }
        }
    }
    if (keyState[(unsigned char)'d'] == BUTTON_UP || keyState[(unsigned char)'D'] == BUTTON_UP) // Spin right
    {
        nave.girando = false;
    }

    if (keyState[(unsigned char)'a'] == BUTTON_UP || keyState[(unsigned char)'A'] == BUTTON_UP) // Spin left
    {
        nave.girando = false;
    }

    if (keyState[(unsigned char)'t'] == BUTTON_DOWN || keyState[(unsigned char)'T'] == BUTTON_DOWN) // Spin right
    {

    }

    if (keyState[(unsigned char)'g'] == BUTTON_DOWN || keyState[(unsigned char)'G'] == BUTTON_DOWN) // X movement
    {

    }

    if (keyState[(unsigned char)'a'] == BUTTON_DOWN || keyState[(unsigned char)'A'] == BUTTON_DOWN) // X movement
    {
        if (nave.velocidad.x>-nave.maxSpeed)
        {
            //       nave.velocidad.x-=nave.repris;
        }
    }

    if (keyState[(unsigned char)'d'] == BUTTON_DOWN || keyState[(unsigned char)'D'] == BUTTON_DOWN) // X movement
    {
        if (nave.velocidad.x<nave.maxSpeed)
        {
            //     nave.velocidad.x+=nave.repris;
        }
    }

    if (keyState[(unsigned char)'w'] == BUTTON_DOWN || keyState[(unsigned char)'W'] == BUTTON_DOWN) // Y movement
    {
        if (nave.velocidad.y<nave.maxSpeed)
        {
            //    nave.velocidad.y+=nave.repris;
        }
    }

    if (keyState[(unsigned char)'s'] == BUTTON_DOWN || keyState[(unsigned char)'S'] == BUTTON_DOWN) // Y movement
    {
        if (nave.velocidad.y>-nave.maxSpeed)
        {
            //     nave.velocidad.y-=nave.repris;
        }
    }
    if (keyState[(unsigned char)'z'] == BUTTON_DOWN || keyState[(unsigned char)'Z'] == BUTTON_DOWN) //  Z movement
    {
        if (speedZ>-nave.maxSpeed*10)
        {
            speedZ-=nave.repris*paso/10;
        }
    }

    if (keyState[(unsigned char)'c'] == BUTTON_DOWN || keyState[(unsigned char)'C'] == BUTTON_DOWN) // Z movement
    {
        if (speedZ<nave.maxSpeed*10)
        {
            speedZ+=nave.repris*paso/10;
        }
    }
    if(!nave.destroyed )
    {
        if (keyState[(unsigned char)' '] == BUTTON_DOWN) // Boost
        {
            if(nave.inpulse<nave.inpulseMax)
            {
                nave.inpulse +=nave.repris;
            }
            nave.inpulseAngle=nave.posicion.angle;
        }

        if (keyState[(unsigned char)' '] == BUTTON_UP) // Boost
        {
            if(nave.inpulse>0)
            {
                nave.inpulse -=nave.repris*3;
            }
            else
            {
                nave.inpulse=0;
            }
        }
    }
    else
    {
        nave.inpulse=0;
    }
    if (keyState[(unsigned char)'r'] == BUTTON_DOWN || keyState[(unsigned char)'R'] == BUTTON_DOWN) // Walk forward
    {
        reset();
        keyState['r'] = BUTTON_UP;

    }

    if (keyState[(unsigned char)'1'] == BUTTON_DOWN) // render in Wire-Frame
    {
        // toggle wire-frame rendering

        // this key toggles wire-frame mode. We dont want it changing every frame,
        // Or it would keep changing to fast for the user to keep track of.
        // so we'll just say the key is up. Then it wont switch back before the user
        // is ready.
        keyState['1']
            = BUTTON_UP;
    }

    glutPostRedisplay();
}
static void calculos(void)
{

    compruebaMouse();
    compruebaTeclado();
    calculaEstado();

}

static void displayHud(void)
{
    char teclas[255];
    char mouseB[3];
    char number[51];
    int j =0;

    glColor4d(1,1,1,alpha);
    glPushMatrix();
    glTranslated(0,0,0);

    sprintf(number,"Radio: (%.2f)",nave.radius);
    renderBitmapString((viewWidth)*(-.49), viewWidth*(.48f)/ar, 0.0f, (void *)3,number);
    //number = "sfshjdflsdlf";
    memset(number, 0, sizeof number);
    sprintf(number,"Aerolitos: %d", partida.nAerolitos);
    renderBitmapString((viewWidth)*(-.49), viewWidth*(.46f)/ar, 0.0f, (void *)3,number);
    memset(number, 0, sizeof number);
    for(int i=0; i<nave.numProyectiles; i++)
    {
            number[nave.numProyectiles-i-1]=(nave.armamento[i].flying)?' ':'I';

    }
    renderBitmapString((viewWidth)*(-.49), viewWidth*(.44f)/ar, 0.0f, (void *)3,"Municion: ");
    renderBitmapString((viewWidth)*(-.40), viewWidth*(.44f)/ar, 0.0f, (void *)3,number);

    memset(number, 0, sizeof number);
    sprintf(number,"xMax:%.2f, yMax: %.2f",xMax, yMax);
    renderBitmapString((viewWidth)*(-0.49), viewWidth*(.42f)/ar, 0.0f, (void *)3,number);
        /*
    memset(number, 0, sizeof number);
    float y=2.5;
    float paso = .2;
    for(int i=0; i<partida.nAerolitos; i++){
        int touching =(aerolitos[i].intouch)?1:0;
        memset(number, 0, sizeof number);
        sprintf(number,"%d(%.2f, %.2f, %.2f) r: %.2f a: %.2f T: %d",i,aerolitos[i].posicion.x,aerolitos[i].posicion.y,aerolitos[i].posicion.z,aerolitos[i].radius,aerolitos[i].velocidad.angle, touching);
        y-=paso;
        renderBitmapString(-4.0f, y, 0.0f, (void *)7,number);
    }

*/

   // sprintf(number,"Angulo: (%.2f)",nave.posicion.angle);
    //renderBitmapString(-3.0f, 3.2f, 0.0f, (void *)3,number);
    memset(number, 0, sizeof number);
    sprintf(number,"Coordenadas: (%.2f, %.2f, %.2f )",nave.posicion.x,nave.posicion.y,posZ);
    renderBitmapString((viewWidth)*(-.49), viewWidth*(.40f)/ar, 0,(void *)3,number);
    memset(number, 0, sizeof number);
    sprintf(number,"Velocidad: (%.4f, %.4f, %.4f )",nave.velocidad.x,nave.velocidad.y,speedZ);
    renderBitmapString((viewWidth)*(-.49), viewWidth*(.38f)/ar, 0.0f, (void *)3,number);
    memset(number, 0, sizeof number);
    sprintf(number,"Inpulso: %.4f, Speed: %.4f, Max Speed: %.4f",nave.inpulse,nave.velocidad.speed,nave.maxSpeed);
    renderBitmapString((viewWidth)*(-.49), viewWidth*(.36f)/ar, 0.0f, (void *)3,number);
    memset(number, 0, sizeof number);
    sprintf(number,"Relacion de aspecto: %.4f",ar);
    renderBitmapString((viewWidth)*(-.49), viewWidth*(.34f)/ar, 0.0f, (void *)3,number);
    sprintf(number,"AlphaBlend: %.4f",alpha);
    renderBitmapString((viewWidth)*(-.49), viewWidth*(.32f)/ar, 0.0f, (void *)3,number);
    /*
    sprintf(number,"Radio: (%.2f)",nave.radius);
    renderBitmapString(-4.0f, 3.2f, 0.0f, (void *)3,number);
    sprintf(number,"Angulo: (%.2f)",nave.posicion.angle);
    renderBitmapString(-3.0f, 3.2f, 0.0f, (void *)3,number);
    sprintf(number,"Coordenadas: (%.2f, %.2f, %.2f )",nave.posicion.x,nave.posicion.y,posZ);
    renderBitmapString(-4.0f, 3.0f, 0.0f, (void *)3,number);
    sprintf(number,"Velocidad: (%.4f, %.4f, %.4f )",nave.velocidad.x,nave.velocidad.y,speedZ);
    renderBitmapString(-0.0f, 3.0f, 0.0f, (void *)3,number);
    sprintf(number,"Inpulso: %.4f, Speed: %.4f, Max Speed: %.4f",nave.inpulse,nave.velocidad.speed,nave.maxSpeed);
    renderBitmapString(-4.0f, 2.8f, 0.0f, (void *)3,number);
    sprintf(number,"Relacion de aspecto: %.4f",ar);
    renderBitmapString(-0.0f, 2.8f, 0.0f, (void *)3,number);
    for(int i=0; i<sizeof(keyState); i++)
    {
        if(keyState[i]==BUTTON_DOWN)
        {
            teclas[j]=i;
            j++;
        }
    }
    renderBitmapString(0.0f, 3.2f, 0.0f, (void *)3,"Teclas: ");
    renderBitmapString(0.5f, 3.2f, 0.0f, (void *)3,teclas);
    j=0;
    for(int i=0; i<sizeof(mouseState); i++)
    {
        if(mouseState[i]==BUTTON_DOWN)
        {
            mouseB[j]=i;
            j++;
        }
    }
    renderBitmapString(1.0f, 3.2f, 0.0f, (void *)3,"Mouse: ");
    renderBitmapString(1.5f, 3.2f, 0.0f, (void *)3,mouseB);
    */
    glPopMatrix();
}


static void displayProyectiles()
{
    for(int i=0; i<nave.numProyectiles; i++)
    {
        if(nave.armamento[i].flying==true)
        {
            glColor4d(.50, .97, 1,alpha);
            glPushMatrix();
            glTranslated(nave.armamento[i].posicion.x,nave.armamento[i].posicion.y,posZ);
            glutSolidSphere(nave.armamento[i].radius,4,3);
            glPopMatrix();
        }
    }
}


static void displayAerolitos()
{
    for(int i=0; i<partida.nAerolitos; i++)
    {

        if(!aerolitos[i].destroyed)
        {
            glColor4d(aerolitos[i].color.red,aerolitos[i].color.green,aerolitos[i].color.blue,alpha);
            glPushMatrix();
            glTranslated(aerolitos[i].posicion.x,aerolitos[i].posicion.y,posZ);
            glRotated(aerolitos[i].angleX,1,0,0);
            glRotated(aerolitos[i].angleY,0,1,0);
            glRotated(aerolitos[i].angleZ,0,0,1);
            glutSolidSphere(aerolitos[i].radius,aerolitos[i].stacks,aerolitos[i].slices);
            glPopMatrix();
        }
    }
}


static void displayNave(void)
{
    if(!nave.destroyed)
        glColor4d(0.1,.1,0.8,alpha);
    else
        glColor4d(0.2,.2,0.3,alpha);
    glPushMatrix();
    glTranslated(nave.posicion.x-sin(nave.posicion.angle*M_PI/180)/25,nave.posicion.y+cos(nave.posicion.angle*M_PI/180)/25,posZ);
    glRotated(nave.posicion.angle,0,0,1);
    glutSolidCone(nave.radius*5,nave.radius,slices,stacks);
    glPopMatrix();


    if(!nave.destroyed)
        glColor4d(0.8+nave.inpulse*40,0.8-nave.inpulse*20,0.8-nave.inpulse*40,alpha);
    else
        glColor4d(0.6,.6,0.7,alpha);
    glPushMatrix();
    glTranslated(nave.posicion.x,nave.posicion.y,posZ);
    glRotated(nave.posicion.angle,0,0,1);
    glutSolidCone(nave.radius*4,nave.radius,slices,stacks);
    glPopMatrix();

    if(nave.inpulse>0)
    {
        glColor4d(0.5+nave.inpulse*40,0.5+nave.inpulse*40,1-nave.inpulse*40,alpha);
        glPushMatrix();
        glTranslated(nave.posicion.x+sin(nave.posicion.angle*M_PI/180)/5.5,nave.posicion.y-cos(nave.posicion.angle*M_PI/180)/5.5,posZ);
        glRotated(nave.posicion.angle,0,0,1);
        glutSolidCone(nave.radius*nave.inpulse*110,nave.radius/2,slices+1,stacks);
        glPopMatrix();
    }

}
static void displayColisions(void)
{


    if(nave.destroyed)
    {
        double auxi = rand()%21;
        double r=auxi/20;
        printf(" %f ",auxi);
        glColor4d(r,r,0,alpha);
        glPushMatrix();
        glTranslated(nave.posicion.x+r/5-0.2,nave.posicion.y+r/5-0.2,posZ);
        glRotated(auxi/3,1,1,1);
        glutSolidSphere(nave.velocidad.speed*r*2,1+auxi/6,1+auxi/6);
        glPopMatrix();
    }
}
static void displayJuego(void)
{

    displayHud();
    displayAerolitos();
    displayNave();
    displayProyectiles();
    displayColisions();


}


static void displayMenu(void)
{
    char number[30];
    char teclas[255];

    compruebaTeclado();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor4d(0,.7,0,alpha);
    glPushMatrix();
    glTranslated(0,0,0);
    sprintf(number,"MENU");
    renderBitmapString((viewWidth)*(-.035), viewWidth*(.17f)/ar, 0.0f, GLUT_BITMAP_HELVETICA_18,number);
    sprintf(number,"JUGAR ................... J");
    renderBitmapString((viewWidth)*(-.13), viewWidth*(.07f)/ar, 0.0f, (void *)2,number);
    sprintf(number,"OPCIONES ................ O");
    renderBitmapString((viewWidth)*(-.13), viewWidth*(-.03f)/ar, 0.0f, (void *)2,number);
    sprintf(number,"SALIR ................... S");
    renderBitmapString((viewWidth)*(-.13), viewWidth*(-.13f)/ar, 0.0f, (void *)2,number);
    glPopMatrix();

    glLineWidth(1);
    glColor3f(.5,.97, 100.0);
    glBegin(GL_LINES);
    glVertex3f((viewWidth)*(-.49), viewWidth*(-.3f)/ar, 0);
    glVertex3f((viewWidth)*(.49), viewWidth*(-.3f)/ar, 0);
    glEnd();
    glLineWidth(1);
    glColor3f(.5,.97, 100.0);
    glBegin(GL_LINES);
    glVertex3f((viewWidth)*(-.49), viewWidth*(.3f)/ar, 0);
    glVertex3f((viewWidth)*(.49), viewWidth*(.3f)/ar, 0);
    glEnd();
}

static void displayOpciones(void)
{
    char number[30];
    char teclas[255];

    compruebaTeclado();

    glColor4d(1,1,0,alpha);
    glPushMatrix();
    glTranslated(0,0,posZ);
    sprintf(number,"OPCIONES");
    renderBitmapString((viewWidth)*(-.055), viewWidth*(.2f)/ar, 0.0f, GLUT_BITMAP_HELVETICA_18,number);
    sprintf(number,"DIFICULTAD .............. D");
    renderBitmapString((viewWidth)*(-.13), viewWidth*(.1f)/ar, 0.0f, (void *)2,number);
    sprintf(number,"CONTROLES ............... C");
    renderBitmapString((viewWidth)*(-.13), viewWidth*(.02f)/ar, 0.0f, (void *)2,number);
    sprintf(number,"GRAFICOS ................ G");
    renderBitmapString((viewWidth)*(-.13), viewWidth*(-.06f)/ar, 0.0f, (void *)2,number);
    sprintf(number,"SONIDO .................. S");
    renderBitmapString((viewWidth)*(-.13), viewWidth*(-.14f)/ar, 0.0f, (void *)2,number);
    sprintf(number,"VOLVER .................. O");
    renderBitmapString((viewWidth)*(-.13), viewWidth*(-.22f)/ar, 0.0f, (void *)2,number);
    glPopMatrix();

    glLineWidth(1);
    glColor3f(.5,.97, 100.0);
    glBegin(GL_LINES);
    glVertex3f((viewWidth)*(-.49), viewWidth*(-.3f)/ar, 0);
    glVertex3f((viewWidth)*(.49), viewWidth*(-.3f)/ar, 0);
    glEnd();
    glLineWidth(1);
    glColor3f(.5,.97, 100.0);
    glBegin(GL_LINES);
    glVertex3f((viewWidth)*(-.49), viewWidth*(.3f)/ar, 0);
    glVertex3f((viewWidth)*(.49), viewWidth*(.3f)/ar, 0);
    glEnd();
}

static void displayFPS()
{
    char teclas[255];
    char mouseB[3];
    char number[51];
    int j =0;
    xMax = (viewWidth/2 - posZ)*margen;
    yMax =  xMax/ar;

    glColor4d(.50,.97,1,alpha);
    glPushMatrix();
    glTranslated(0,0,0);
    sprintf(number,"FPS: %.0f, Z: %.2f Width: %.2f",fps,posZ, viewWidth);
    renderBitmapString((viewWidth)*(-.49), viewWidth*(-.49f)/ar, 0.0f, (void *)7,number);
    memset(number, 0, sizeof number);
    sprintf(number,"xMax: %.1f, yMax: %.1f",xMax, yMax);//(viewWidth-posZ)/2);
    renderBitmapString(viewWidth*(-.30), viewWidth*(-.49f)/ar, 0.0f, (void *)7,number);
    memset(number, 0, sizeof number);
    sprintf(number,"AT: %.3f ms",1000.0f/fps);
    renderBitmapString(viewWidth*(-.1), viewWidth*(-.49f)/ar, 0.0f, (void *)7,number);
    memset(number, 0, sizeof number);
    sprintf(number,"AR: %.3f (width/height)",ar);
    renderBitmapString(viewWidth*(.15), viewWidth*(-.49f)/ar, 0.0f, (void *)7,number);
    memset(number, 0, sizeof number);
    sprintf(number,"T: %.3f",elapsedTime);
    renderBitmapString(viewWidth*(.35), viewWidth*(-.49f)/ar, 0.0f, (void *)7,number);
    memset(number, 0, sizeof number);
    glPopMatrix();

}
static void display(void)
{
    const double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    const double a = t*90.0;
    elapsedTime = t - elapsedTime;
    paso = elapsedTime*100.0;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(estado==JUGANDO)
    {
        calculos();
        displayJuego();
        displayFPS();
    }
    else if(estado==MENU)
    {
        displayMenu();
    }
    else if(estado==OPCIONES)
    {
        displayOpciones();
    }

    frames ++;
    if(t - lastTime>=1){
        fps = frames;
        frames=0;
        lastTime = t;
    }
    elapsedTime=t;
    glutSwapBuffers();

}

static void idle(void)
{
    glutPostRedisplay();
}

const GLfloat light_ambient[]  = { 0.0f, 0.0f, 0.0f, 1.0f };
const GLfloat light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_position[] = { 2.0f, 5.0f, 5.0f, 0.0f };

const GLfloat mat_ambient[]    = { 0.7f, 0.7f, 0.7f, 1.0f };
const GLfloat mat_diffuse[]    = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_specular[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat high_shininess[] = { 100.0f };

/* Program entry point */

int main(int argc, char *argv[])
{
    srand(time(0));
    /**
     *          OpeGL gfg.
     */
    glutInit(&argc, argv);
    glutInitWindowSize(1024,800);
    glutInitWindowPosition(400,10);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutCreateWindow("Aerolitos");

    glClearColor(0,0,0,0);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);

    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);

    /**< addin functions */
    glutReshapeFunc(resize);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboard_up);
    glutMouseFunc(mouse);
    glutIdleFunc(idle);

    SoundEngine->play2D("audio/breakout.mp3", GL_TRUE);

    glutMainLoop(); /**< After this point the only know control of loop is inside display function */

    return EXIT_SUCCESS;
}

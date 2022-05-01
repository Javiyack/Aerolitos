#ifndef NAVE_H_INCLUDED
#define NAVE_H_INCLUDED
#include"proyectil.h"
#include"posicion.h"
class Nave {
  public:
    Nave();
    double radius = 0.1;
    Posicion posicion;
    Velocidad velocidad;
    bool destroyed = false;
    bool girando = false;
    double inpulseAngle = 0;
    double angularSpeed = 0;
    double repris = 0.001;
    double maxSpeed = 0.5;
    double maxAngularSpeed = 1.0;
    double inpulse = 0;
    double inpulseMax = 0.02;
    int numProyectiles=10;
    Proyectil armamento[10];

};


#endif // NAVE_H_INCLUDED

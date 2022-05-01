#ifndef PROYECTIL_H_INCLUDED
#define PROYECTIL_H_INCLUDED

#include"posicion.h"
class Proyectil {
  public:
    double strength = 10;
    double radius = 0.1;
    Posicion posicion;
    double speed = 0.1;
    double angle = 0;
    double alcance = 10;
    double rozamiento = 0.001;
    bool flying=false;

    Proyectil();

};

#endif // PROYECTIL_H_INCLUDED


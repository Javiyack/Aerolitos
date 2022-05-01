#ifndef AEROLITO_H
#define AEROLITO_H


#include"posicion.h"
class Aerolito
{
    public:
        Aerolito();
        bool destroyed = false;
        bool intouch = false;
        int dureza;
        int slices;
        int stacks;
        double radius;
        double minRadius = .3;
        double angularSpeedX;
        double angularSpeedY;
        double angularSpeedZ;
        double angleX=0;
        double angleZ=0;
        double angleY=0;
        Posicion posicion;
        Velocidad velocidad;
        Color color;
    protected:

    private:
};

#endif // AEROLITO_H

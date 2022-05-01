#ifndef POSICION_H
#define POSICION_H


#include"posicion.h"
class Posicion
{
    public:
        Posicion();
        double angle;
        double x;
        double y;
        double z;
        double Getx() { return x; }
        void Setx(double val) { x = val; }
        double Gety() { return y; }
        void Sety(double val) { y = val; }
        double Getz() { return z; }
        void Setz(double val) { z = val; }

    protected:

    private:
};

class Velocidad
{
public:
    double speed;
    double angle;
    double x;
    double y;
    double z;
};

class Color
{
public:
    double red;
    double green;
    double blue;
    double transparency;
    Color(){

    };
    Color(double r, double g, double b){
        red = r;
        green = g;
        blue = b;
    };
    Color(double r, double g, double b, double t){
        red = r;
        green = g;
        blue = b;
        transparency = t;
    };
};

#endif // POSICION_H

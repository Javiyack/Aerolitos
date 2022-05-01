#ifndef MUNDO_H_INCLUDED
#define MUNDO_H_INCLUDED

class Mundo {
  public:
    Mundo();
    double viscosidad = .000005;
    double getViscosidad(){
    return viscosidad;
    };
};

#endif // MUNDO_H_INCLUDED

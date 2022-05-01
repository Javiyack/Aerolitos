#ifndef PARTIDA_H_INCLUDED
#define PARTIDA_H_INCLUDED

#include"jugador.h"
class Partida
{
    public:
        Partida();
        Jugador player;
        int dificultad;
        int nAerolitos;
        int puntuacion;
        bool paused = true;
        bool gameOver = true;
    protected:

    private:
};

#endif // PARTIDA_H_INCLUDED

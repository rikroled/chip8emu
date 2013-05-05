#ifndef DEF_CPU
#define DEF_CPU

#define read_mem(adr)        (Chip8_MEM[(adr)&4095])
#define write_mem(adr,val)   (Chip8_MEM[(adr)&4095]=(val))

#define DEBUGNO

#include <stdio.h>
#include <stdlib.h>

#include <stdint.h> //pour uint(8,16)_t
#include <iostream> // pour memset et stderr
#include <cstdlib> // rand()
#include <fstream> //ouvrir le rom

#include "SDLWidget.h"

struct Chip8Context {
	unsigned char  reg_V[16];    // Les 16 registres
    unsigned short reg_PC;       // le registre PC
    unsigned short reg_SP;       // le registre SP
    unsigned short reg_I;        // le registre I
    unsigned char delay,sound;  // pour la gestion des timers delais et son
};

class Cpu
{
    public:
        Cpu(QSDLScreenWidget *sdlw);
        void Reset();
        bool getChip8_Exec() const;
        void setChip8_Exec(bool val);
        bool LoadRom(const char* fileName);
        void Chip8Execute();
        int getIperiod();
        void setIperiod(int period);

    private:
        void Chip8InitFont(); //charge les fonts en mémoire
        void Chip8GetOpcode(); //lit un opcode
        void ExecuteOpcode(); //exécute l'opcode lu précédemment

    private:
        QSDLScreenWidget *sdlw;
        //CPU chip8
        Chip8Context Chip8CPU;

        bool Chip8_Exec;  // Pour gérer si le cpu fonctionne ou pas

        unsigned char Chip8_Iperiod;        // Pour la gestion des cycles d'horloge
        unsigned char Chip8_Tches[16];      // Les 16 touches disponiblesdu Chip 8

        unsigned char Chip8_Display[64 * 32]; // Les pixels de l'ecran

        unsigned char Chip8_MEM[4096];      // Les 4Ko disponibles et les macros d'acces à la mémoire
        unsigned short Chip8_PILE[16];      // Les 16 emplacements de la pile

        unsigned char OpCode_1, OpCode_2, OpCode_3, OpCode_4;// Pour gérer la lecture des opcodes


};

#endif

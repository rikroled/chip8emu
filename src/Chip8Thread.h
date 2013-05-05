#ifndef CHIP8THREAD_H
#define CHIP8THREAD_H

#include <QtGui>
#include "Cpu.h"

class Chip8Thread : public QThread
{
    Q_OBJECT

public:
    Chip8Thread(QObject *parent, Cpu *chip8); //constructeur
    void run(); // this is virtual method, we must implement it in our subclass of QThread
private:
    Cpu *chip8;
};

#endif

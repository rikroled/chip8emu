#include "Chip8Thread.h"

Chip8Thread::Chip8Thread(QObject *parent, Cpu *m_chip8)
: QThread(parent)
{
    chip8=m_chip8;
}
 void Chip8Thread::run()
{
#ifdef DEBUG
    qDebug() << "Execution du thread chip8 ...";
#endif
    chip8->Reset();
    while(chip8->getChip8_Exec()){
        chip8->Chip8Execute();
    }
#ifdef DEBUG
    qDebug() << "Thread Chip8 fini";
#endif
    exit();
}

#ifndef HEADER_FENPRINCIPALE
#define HEADER_FENPRINCIPALE

#include <QtGui>
#include <QtDebug>
#include "SDLWidget.h"
#include "Cpu.h"
#include "Chip8Thread.h"
#include "ThreadRefresh.h"

class FenPrincipale : public QMainWindow
{
    Q_OBJECT

    public:
        FenPrincipale();

    private slots:
        void ouvrir_rom();
        void executer(); // lance le cpu et sdl
        void redemarrer(); //reset le cpu
        void arreter();
        void resolutionx1();
        void resolutionx2();
        void resolutionx3();
        void a_propos();
        void clavier();
        void dialog_fin();
        void cpu();

    protected:
        void closeEvent(QCloseEvent *event);

    private:
        QAction *actionResolutionx1; //320*224
        QAction *actionResolutionx2; //640*480
        QAction *actionResolutionx3; //768*576
        QDialog *aboutFenetre;
        QDialog *clavierFenetre;

        QSDLScreenWidget *sdlw;
        Cpu *chip8;
        bool romOuverte;
        Chip8Thread *chip8t;
        ThreadRefresh *refreshT;
};
#endif

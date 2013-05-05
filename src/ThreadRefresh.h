#ifndef THREADREFRESH_H
#define THREADREFRESH_H

#include <QtGui>
#include "SDLWidget.h"

class ThreadRefresh : public QThread
{
    Q_OBJECT

public:
    ThreadRefresh(QObject *parent, QSDLScreenWidget *sdlw); //constructeur
    void run(); // this is virtual method, we must implement it in our subclass of QThread
private:
    QSDLScreenWidget *sdlw;
};

#endif

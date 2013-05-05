#include "ThreadRefresh.h"

ThreadRefresh::ThreadRefresh(QObject *parent, QSDLScreenWidget *m_sdlw)
: QThread(parent)
{
    sdlw=m_sdlw;
}
 void ThreadRefresh::run()
{
    msleep(20);
    sdlw->refresh();
    exit();
}

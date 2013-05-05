#include <iostream>

#include <QX11Info>

#include <SDL.h>
#include "SDLWidget.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

QSDLScreenWidget::QSDLScreenWidget(int largeur, int hauteur, QWidget *parent, Qt::WFlags flags) :
    QWidget(parent, flags)
{
    // Turn off double buffering for this widget. Double buffering
    // interferes with the ability for SDL to be properly displayed
    // on the QWidget.
    //
    setAttribute(Qt::WA_PaintOnScreen);
#ifdef Q_WS_X11
    // Make sure we're not conflicting with drawing from the Qt library
    //
    XSync(QX11Info::display(), FALSE);
#endif
    setFixedSize(largeur,hauteur);
    home=NULL;
    carre_blanc=NULL;

    // We could get a resize event at any time, so clean previous mode.
    // You do this because if you don't you wind up with two windows
    // on the desktop: the Qt application and the SDL window. This keeps
    // the SDL region synchronized inside the Qt widget and the subsequent
    // application.
    //
    screen = 0;
    SDL_QuitSubSystem(SDL_INIT_VIDEO);

    // Set the new video mode with the new window size
    //
    char
        variable[64];
    sprintf(variable, "SDL_WINDOWID=0x%lx", winId());
    putenv(variable);

    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "Unable to init SDL: " << SDL_GetError() << std::endl;
        return;
    }

    screen = SDL_SetVideoMode(width(), height(), 32, SDL_HWSURFACE);

    if (!screen)
    {
        std::cerr << "Unable to set video mode: " << SDL_GetError() <<
            std::endl;
        return;
    }

    carre_blanc = SDL_CreateRGBSurface(SDL_HWSURFACE, largeur/64, hauteur/32, 32, 0, 0, 0, 0); //alloue chaque surface
    SDL_FillRect(carre_blanc, NULL, SDL_MapRGB(screen->format, 255, 255, 255)); // Remplissage de la surface avec une couleur

    home = SDL_LoadBMP("images/home.bmp"); //on charge l'image d'acceuil

    SDL_EnableKeyRepeat(10, 10); //permet de répéter une touche qui est enfoncée

    tpsParCycle=40;
}

QSDLScreenWidget::~QSDLScreenWidget(){
    SDL_FreeSurface(carre_blanc); // Libération de la surface
    SDL_FreeSurface(home);
    SDL_Quit();
}

void QSDLScreenWidget::refresh()
{
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));

    SDL_Rect dst;
    dst.x = (screen->w - home->w)/2;
    dst.y = (screen->h - home->h)/2;
    dst.w = home->w;
    dst.h = home->h;
    SDL_BlitSurface(home, NULL, screen, &dst);

    SDL_Flip(screen); // Mise à jour de l'écran
}

void QSDLScreenWidget::majEcran(unsigned char* chip8display)
{
    SDL_Rect position;

    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0)); // Effacement de l'écran

    for(int i=0; i<64 ;i++){
        for(int j=0; j<64 ;j++){
            if(chip8display[j*64 + i]==1){ //si c'est un pixel blanc
                position.x = i*(width()/64); //on détermine les coodonnées de la surface à poser
                position.y = j*(height()/32);
                SDL_BlitSurface(carre_blanc, NULL, screen, &position); //on pose la surface
            }
        }
    }
    SDL_Flip(screen); // Mise à jour de l'écran
}

void QSDLScreenWidget::CheckClavier(unsigned char *Chip8_Tches)
{
    SDL_Event event;
    for(int i=0; i<16; i++){
        Chip8_Tches[i]=0;
    }

    int tempsPrecedent = SDL_GetTicks(), tempsActuel = SDL_GetTicks();

    while (tempsActuel - tempsPrecedent <= tpsParCycle) //Si 40 ms se sont écoulées depuis le dernier tour de boucle
    {
        tempsActuel = SDL_GetTicks();
        SDL_PollEvent(&event);
        switch(event.type)
        {
            case SDL_KEYDOWN:
                switch(event.key.keysym.sym)
                {
                    case SDLK_UP:       Chip8_Tches[8]=1; break;
                    case SDLK_DOWN:     Chip8_Tches[2]=1; break;
                    case SDLK_RIGHT:    Chip8_Tches[6]=1; break;
                    case SDLK_LEFT:     Chip8_Tches[4]=1; break;
                    case SDLK_KP1:      Chip8_Tches[5]=1; break; //A
                    case SDLK_KP2:      Chip8_Tches[0]=1; break; //B
                    case SDLK_KP4:      Chip8_Tches[7]=1; break; //X
                    case SDLK_KP5:      Chip8_Tches[9]=1; break; //Y
                    case SDLK_KP_ENTER: Chip8_Tches[1]=1; break; //start
                    case SDLK_KP_MINUS: Chip8_Tches[3]=1; break; //select
                }
                break;
        }
    }
}

int QSDLScreenWidget::getTpsParCycle() const{
    return tpsParCycle;
}

void QSDLScreenWidget::setTpsParCycle(int tps){
    tpsParCycle=tps;
}

/*void QSDLScreenWidget::resizeEvent(QResizeEvent *)
{

}*/

/*void QSDLScreenWidget::paintEvent(QPaintEvent *)
{
    if (screen){
        if(home)
            refresh();
    }
}*/

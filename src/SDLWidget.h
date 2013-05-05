#ifndef SDLWIDGET_H
#define SDLWIDGET_H

#include <Qt>
#include <QWidget>

struct SDL_Surface;

class QSDLScreenWidget : public QWidget
{
    Q_OBJECT
public:
    QSDLScreenWidget(int largeur, int hauteur, QWidget *parent=0, Qt::WFlags flags=0);
    ~QSDLScreenWidget();
	void majEcran(unsigned char* chip8display);
	void CheckClavier(unsigned char *Chip8_Tches);
	void refresh();
	int getTpsParCycle() const;
	void setTpsParCycle(int tps);

protected:
    //void resizeEvent(QResizeEvent *);
    //void paintEvent(QPaintEvent *);

private:
    SDL_Surface *screen;
	SDL_Surface *carre_blanc;
	SDL_Surface *home;
	int tpsParCycle;
};
#endif

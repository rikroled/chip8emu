#include <QApplication>
#include "FenPrincipale.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // Ouverture de la fenêtre principale
    FenPrincipale fenetre;
    fenetre.show();

    return app.exec();
}

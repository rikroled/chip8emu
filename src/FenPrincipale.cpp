#include "FenPrincipale.h"

#define TAILLE_MENUBAR 21 //21=taille de le menu barre car menu()->height() ne renvoie pas la valeur attendu

FenPrincipale::FenPrincipale() : QMainWindow()
{
    setWindowTitle("Emulateur Chip8");
    setWindowIcon(QIcon("images/icone.ico"));
//************** Menu Fichier **************
    QAction *actionOuvrir_rom = new QAction("&Ouvrir ROM", this);
    actionOuvrir_rom->setShortcut(QKeySequence("Ctrl+O"));
    connect(actionOuvrir_rom, SIGNAL(triggered()), this, SLOT(ouvrir_rom()));

    QMenu *menuFichier = menuBar()->addMenu("&Fichier");
    menuFichier->addAction(actionOuvrir_rom);

//************** Menu Démarrer **************
    QAction *actionExecuter = new QAction("&Exécuter", this);
    actionExecuter->setShortcut(QKeySequence("F1"));
    connect(actionExecuter, SIGNAL(triggered()), this, SLOT(executer()));

    QAction *actionRedemarrer = new QAction("&Redémarrer", this);
    actionRedemarrer->setShortcut(QKeySequence("F2"));
    connect(actionRedemarrer, SIGNAL(triggered()), this, SLOT(redemarrer()));

    QAction *actionArreter = new QAction("&Arrêter", this);
    actionArreter->setShortcut(QKeySequence("F3"));
    connect(actionArreter, SIGNAL(triggered()), this, SLOT(arreter()));

    QMenu *Demarrer = menuBar()->addMenu("&Démarrer");
    Demarrer->addAction(actionExecuter);
    Demarrer->addAction(actionRedemarrer);
    Demarrer->addAction(actionArreter);

//************** Menu Configurer **************
    actionResolutionx1 = new QAction("x&1", this);
    actionResolutionx1->setCheckable(true);
    connect(actionResolutionx1, SIGNAL(triggered()), this, SLOT(resolutionx1()));

    actionResolutionx2 = new QAction("x&2", this);
    actionResolutionx2->setCheckable(true);
    connect(actionResolutionx2, SIGNAL(triggered()), this, SLOT(resolutionx2()));

    actionResolutionx3 = new QAction("x&3", this);
    actionResolutionx3->setCheckable(true);
    connect(actionResolutionx3, SIGNAL(triggered()), this, SLOT(resolutionx3()));


    QAction *actionCpu = new QAction("&Cpu", this);
    connect(actionCpu, SIGNAL(triggered()), this, SLOT(cpu()));

    QMenu *Configurer = menuBar()->addMenu("&Configurer");
    QMenu *menuResolution = Configurer->addMenu("&Résolution");
    menuResolution->addAction(actionResolutionx1);
    menuResolution->addAction(actionResolutionx2);
    menuResolution->addAction(actionResolutionx3);
    Configurer->addAction(actionCpu);

//******* Création d'une boite de dialogue pour le menu à propos *******
    aboutFenetre = new QDialog(this);
    aboutFenetre->setWindowTitle("A propos ...");
    QVBoxLayout *layout = new QVBoxLayout;
    QLabel *image = new QLabel(aboutFenetre);
    image->setPixmap(QPixmap("images/a_propos.png"));
    layout->addWidget(image);

    QLabel *texte = new QLabel("<center> mail: rikroled@gmail.com <br><br> Version non buguée (normalement) <br> Envoyez moi un mail si vous avez une erreur(et le temps). <br><br>Emulateur écrit en C++ avec les librairies Qt et SDL. <br><br> Fonctionne seulement sous windows à cause de SDL.</center>",aboutFenetre);
    layout->addWidget(texte);

    aboutFenetre->setLayout(layout);
    aboutFenetre->setFixedSize(360,300);

    connect(aboutFenetre, SIGNAL(finished(int)), this, SLOT(dialog_fin()));

//******* Création d'une boite de dialogue pour le menu Clavier *******
    clavierFenetre = new QDialog(this);
    clavierFenetre->setWindowTitle("Configuration Clavier");
    QVBoxLayout *layout2 = new QVBoxLayout;
    QLabel *image2 = new QLabel(clavierFenetre);
    image2->setPixmap(QPixmap("images/clavier.png"));
    layout2->addWidget(image2);

    clavierFenetre->setLayout(layout2);
    clavierFenetre->setFixedSize(650,260);

    connect(clavierFenetre, SIGNAL(finished(int)), this, SLOT(dialog_fin()));

//************** Menu Aide **************
    QAction *actionA_propos = new QAction("&A propos...", this);
    connect(actionA_propos, SIGNAL(triggered()), this, SLOT(a_propos()));

    QAction *actionclavier = new QAction("&Configuration Clavier", this);
    connect(actionclavier, SIGNAL(triggered()), this, SLOT(clavier()));

    QMenu *aide = menuBar()->addMenu("&?");
    aide->addAction(actionA_propos);
    aide->addAction(actionclavier);

//************** Instanciations et initialisations **************
    sdlw = new QSDLScreenWidget(640,480,this); //on instancie le widget sdl
    chip8 = new Cpu(sdlw); //on créer le cpu chip8
    romOuverte = false;
    setCentralWidget(sdlw); //on place le widget SDL dans la fenêtre Qt
    chip8t = new Chip8Thread(this,chip8); //on crée un thread pour pouvoir éxécuter le cpu
    refreshT = new ThreadRefresh(this,sdlw); //permet de rafraichir SDL après l'ouverture d'un fenêtre modale.

    setFixedSize(640,480+TAILLE_MENUBAR);
    actionResolutionx2->setChecked(true);
    sdlw->refresh();
}

//************** SLOTS **************
void FenPrincipale::ouvrir_rom()
{
    qDebug() << " width: " << width();
    qDebug() << " height: " << height();
    QString fichier = QFileDialog::getOpenFileName(this, "Ouvrir une ROM", QString());
    const char* fileName= fichier.toStdString().c_str();

    if(!chip8->LoadRom(fileName)){
        QMessageBox::information(this, "Fichier", "Le fichier que vous avez sélectionné n'a pas pu être chargé");
        romOuverte = false;
    }else{
        // Permet de mettre le nom du fichier ouvert dans le titre
        QString y = "/";
        QString nom_jeu;
        for(int i=(fichier.lastIndexOf(y)+1); i<fichier.size() ; i++){
            nom_jeu += fichier[i];
        }
        setWindowTitle("Emulateur Chip8 - " + nom_jeu);

        romOuverte = true;
    }
    refreshT->start();
}

void FenPrincipale::executer()
{
    if(!romOuverte){
        QMessageBox::information(this, "Fichier", "Veuillez ouvrir un jeu.");
        refreshT->start();
    }else{
        chip8t->start(QThread::HighestPriority); //on lance le thread chip8
    }
}

void FenPrincipale::redemarrer()
{
    if(chip8->getChip8_Exec()){ //si le cpu est en train de tourner
        chip8->setChip8_Exec(false);
        while(!chip8t->isFinished()){ //on attend que le thread se termine
        }
        chip8t->start(QThread::HighestPriority); //on relance le thread chip8
    }
}

void FenPrincipale::arreter()
{
    chip8->setChip8_Exec(false); //on arrête le thread chip8
    sdlw->refresh(); //pour effacer l'écran et remettre l'image d'accueil
}

void FenPrincipale::resolutionx1()
{
    if(actionResolutionx1->isChecked()){ //décoché vers coché
        bool estcequeleCPUtournait = false;
        if(chip8->getChip8_Exec()){ //si le cpu est en train de tourner
            chip8->setChip8_Exec(false);
            while(!chip8t->isFinished()){ //on attend que le thread se termine
            }
            estcequeleCPUtournait = true;
        }
        delete sdlw;
        sdlw = new QSDLScreenWidget(320,224,this);
        setCentralWidget(sdlw); //on place le widget SDL dans la fenêtre Qt
        setFixedSize(320,224+TAILLE_MENUBAR);
        if(estcequeleCPUtournait){
            chip8t->start(QThread::HighestPriority); //on relance le thread chip8
        }else{
            sdlw->refresh();
        }
        actionResolutionx2->setChecked(false);
        actionResolutionx3->setChecked(false);
    }else{ //coché vers décoché
        actionResolutionx1->setChecked(true);
    }
}

void FenPrincipale::resolutionx2()
{
    if(actionResolutionx2->isChecked()){ //décoché vers coché
        bool estcequeleCPUtournait = false;
        if(chip8->getChip8_Exec()){ //si le cpu est en train de tourner
            chip8->setChip8_Exec(false);
            while(!chip8t->isFinished()){ //on attend que le thread se termine
            }
            estcequeleCPUtournait = true;
        }
        delete sdlw;
        sdlw = new QSDLScreenWidget(640,480,this);
        setCentralWidget(sdlw); //on place le widget SDL dans la fenêtre Qt
        setFixedSize(640,480+TAILLE_MENUBAR);

        if(estcequeleCPUtournait){
            chip8t->start(QThread::HighestPriority); //on relance le thread chip8
        }else{
            sdlw->refresh();
        }
        actionResolutionx1->setChecked(false);
        actionResolutionx3->setChecked(false);
    }else{ //coché vers décoché
        actionResolutionx2->setChecked(true);
    }
}

void FenPrincipale::resolutionx3()
{
    if(actionResolutionx3->isChecked()){ //décoché vers coché
        bool estcequeleCPUtournait = false;
        if(chip8->getChip8_Exec()){ //si le cpu est en train de tourner
            chip8->setChip8_Exec(false);
            while(!chip8t->isFinished()){ //on attend que le thread se termine
            }
            estcequeleCPUtournait = true;
        }
        delete sdlw;
        sdlw = new QSDLScreenWidget(768,576,this);
        setCentralWidget(sdlw); //on place le widget SDL dans la fenêtre Qt
        setFixedSize(768,576+TAILLE_MENUBAR);

        if(estcequeleCPUtournait){
            chip8t->start(QThread::HighestPriority); //on relance le thread chip8
        }else{
            sdlw->refresh();
        }
        actionResolutionx1->setChecked(false);
        actionResolutionx2->setChecked(false);
    }else{ //coché vers décoché
        actionResolutionx3->setChecked(true);
    }
}

void FenPrincipale::a_propos(){
    aboutFenetre->exec(); //on affiche la fenêtre
}

void FenPrincipale::clavier(){
    clavierFenetre->exec(); //on affiche la fenêtre
}

void FenPrincipale::dialog_fin(){
    refreshT->start();
}

void FenPrincipale::cpu(){
    int tpsParCycle=QInputDialog::getInt(this,"Cpu","Entrer le temps d'affichage entre 2 images (en ms):", sdlw->getTpsParCycle(),0);
    sdlw->setTpsParCycle(tpsParCycle);
    int nbCycle=QInputDialog::getInt(this,"Cpu","Entrer le nombre de cycles effectués pour une image", chip8->getIperiod(),0);
    chip8->setIperiod(nbCycle);
    refreshT->start();
}

void FenPrincipale::closeEvent(QCloseEvent *event)
{
#ifdef DEBUG
    qDebug() << "Fermeture du programme";
#endif
    if(chip8t->isRunning()){
        chip8->setChip8_Exec(false);
        while(!chip8t->isFinished()){ //on attend que le thread se termine
        }
    }
    delete chip8;
    delete sdlw;
    delete chip8t;
    delete refreshT;
    event->accept();
}

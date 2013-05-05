#include "Cpu.h"

Cpu::Cpu(QSDLScreenWidget *m_sdlw){
    sdlw = m_sdlw;
    Chip8_Exec=false;
    Chip8_Iperiod = 15; // nbre de cycles exec par écran
}

//*****************************************************************************
void Cpu::Reset(){

  // Init des registres, de la mémoire graphique et des touches
  memset (&Chip8CPU,0,sizeof(Chip8CPU));
  memset (Chip8_Tches,0,sizeof(Chip8_Tches));
  memset (Chip8_Display,0,sizeof(Chip8_Display));

  Chip8CPU.reg_SP = 0xF; // Init du PC et du SP
  Chip8CPU.reg_PC = 0x200;

  Chip8InitFont();

  Chip8_Exec=true; // démarrage du CPU
}

void Cpu::Chip8Execute(){
    for (int i = 0; i < Chip8_Iperiod; i++) {
        Chip8GetOpcode();
        ExecuteOpcode();
    }

    if (Chip8CPU.delay) Chip8CPU.delay--;
    if (Chip8CPU.sound) Chip8CPU.sound--;

    sdlw->majEcran(Chip8_Display); //met à jour l'écran avec SDL
    sdlw->CheckClavier(Chip8_Tches); //vérifie quelles touches sont appuyées
}

//*****************************************************************************
void Cpu::Chip8GetOpcode() {
   OpCode_1 =  (read_mem(Chip8CPU.reg_PC    )  & 0xF0) >> 4;
   OpCode_2 =  (read_mem(Chip8CPU.reg_PC    )  & 0x0F);
   OpCode_3 =  (read_mem(Chip8CPU.reg_PC + 1)  & 0xF0) >> 4;
   OpCode_4 =  (read_mem(Chip8CPU.reg_PC + 1)  & 0x0F);

   Chip8CPU.reg_PC += 2;
}

//*****************************************************************************
void Cpu::ExecuteOpcode()
{   // Opcode // Instruction assembleur // Description

    unsigned int i; //pour les boucles
    unsigned char uNbre; //variable tmp
    unsigned int Lig, Col;
    unsigned char gfxVal, xVal, yVal;

    switch (OpCode_1) {
      case 0x0:
        switch(OpCode_3) {
          case 0xE:
            switch (OpCode_4) {
              case 0x0: // 00E0 // Cls //Efface l’écran

                for (Lig = 0; Lig < 32; Lig++)   {            // Pour toutes les lignes
                  for (Col = 0; Col < 64; Col++) {          // Pour toutes les colonnes
                     Chip8_Display[Lig * 64 + Col] = 0;       // Enlève les pixels
                  }
                }
               break;

              case 0xE: // 00EE // Rts //Fin de sous programme (donc retour au programme appelant)
                Chip8CPU.reg_SP++;                             // Prend la précédente adresse de pile
                Chip8CPU.reg_PC = Chip8_PILE[Chip8CPU.reg_SP]; // Recupere l'adresse du programme appelant
              break;
            }
            break;
        }
        break;
      case 0x1: // 1xxx //Jump xxx // Saute à l’adresse xxx
        Chip8CPU.reg_PC = (OpCode_2 * 256) + (OpCode_3 *16) + OpCode_4; // PC = contenu des 3 opcodes (les xxx)
      break;
      case 0x2: // 2xxx //Jsr xxx // Appel le sous-programme à l’adresse xxx // 16 appels maximum (cf la pile plus haut)
        Chip8_PILE[Chip8CPU.reg_SP] = Chip8CPU.reg_PC; // Sauvegarde l'adresse actuelle sur la pile
        Chip8CPU.reg_SP--;                             // Une adresse de moins de disponible
        Chip8CPU.reg_PC = (OpCode_2 * 256) + (OpCode_3 *16) + OpCode_4; // PC = contenu des 3 opcodes (les xxx)
      break;
      case 0x3: // 3rxx //Skeq vr,xx // Ne fait pas l’instruction suivante si registre vr = xx
        if (Chip8CPU.reg_V[OpCode_2] == ((OpCode_3 *16) + OpCode_4))    // Si condition remplie VX = xx
          Chip8CPU.reg_PC += 2;                                         // On saute l'instruction suivante
      break;
      case 0x4: // 4rxx //Skne vr,xx // Ne fait pas l’instruction suivante si registre vr <>xx
        if (Chip8CPU.reg_V[OpCode_2] != ((OpCode_3 *16) + OpCode_4))    // Si condition remplie VX <> xx
          Chip8CPU.reg_PC += 2;                                         // On saute l'instruction suivante
      break;
      case 0x5: // 5ry0 //Skeq vr,vy // Ne fait pas l’instruction suivante si registre vr = vy
        if (Chip8CPU.reg_V[OpCode_2] == Chip8CPU.reg_V[OpCode_3])       // Si condition remplie VX = VY
          Chip8CPU.reg_PC += 2;                                         // On saute l'instruction suivante
      break;
      case 0x6: // 6rxx // Mov vr,xx // Met xx dans vr
        Chip8CPU.reg_V[OpCode_2] = (OpCode_3 * 16) + OpCode_4;
      break;
      case 0x7: // 7rxx // Add vr,xx // Ajoute xx à vr
        Chip8CPU.reg_V[OpCode_2] += (OpCode_3 * 16) + OpCode_4;
      break;
      case 0x8:
        switch(OpCode_4) {
          case 0x0: // 8ry0 // Mov vr,vy // Met le registre vy dans vr
            Chip8CPU.reg_V[OpCode_2] = Chip8CPU.reg_V[OpCode_3];
          break;
          case 0x1: //8ry1  	Or vr,vy  	Fait l’opération OU avec vr et vy dans vr
            Chip8CPU.reg_V[OpCode_2] |= Chip8CPU.reg_V[OpCode_3];
          break;
          case 0x2: // 8ry2  	And vr,vy  	Fait l’opération ET avec vr et vy dans vr
            Chip8CPU.reg_V[OpCode_2] &= Chip8CPU.reg_V[OpCode_3];
          break;
          case 0x3: //8ry3  	Xor vr,vy  	Fait l’opération OU EXCLUSIF avec vr et vy dans vr
            Chip8CPU.reg_V[OpCode_2] ^= Chip8CPU.reg_V[OpCode_3];
          break;
          case 0x4: //8ry4    Add vr,vy  	Ajoute vy dans vr   Carry dans vf
            if(Chip8CPU.reg_V[OpCode_2] && Chip8CPU.reg_V[OpCode_3])   // Si condition (VX et VY <>0)
                Chip8CPU.reg_V[0xF] = 1;                                // Stocke la carry
            else
                Chip8CPU.reg_V[0xF] = 0;
            Chip8CPU.reg_V[OpCode_2] += Chip8CPU.reg_V[OpCode_3];        // VX += VY
          break;
          case 0x5: //8ry5  	Sub vr,vy  	Fait l’opération vr = vr –vy  	Carry dans vf si résultat <0
            if(Chip8CPU.reg_V[OpCode_2] > Chip8CPU.reg_V[OpCode_3])         // Si condition (VX > VY)
              Chip8CPU.reg_V[0xF] = 1;                                      // Stocke la carry
           else
              Chip8CPU.reg_V[0xF] = 0;

           Chip8CPU.reg_V[OpCode_2] -= Chip8CPU.reg_V[OpCode_3];           // VX -= VY
          break;
          case 0x6: //8r06  	Shr vr  	Décalage arithmétique à droite de vr
                                          //Equivaut donc à vr = vr /2. Le bit 0 va dans vf
            if (Chip8CPU.reg_V[OpCode_2] & 1)                               // Si condition (VX & 1)
              Chip8CPU.reg_V[0xF] = 1;                                      // Stocke la carry
           else
              Chip8CPU.reg_V[0xF] = 0;

           Chip8CPU.reg_V[OpCode_2] >>= 1;
          break;
          case 0x7: //8ry7  	Rsb vr,vy  	Fait l’opération vr = vy – vr  	Carry dans vf si résultat < 0
            if(Chip8CPU.reg_V[OpCode_3] > Chip8CPU.reg_V[OpCode_2])         // Si condition (VY > VX
              Chip8CPU.reg_V[0xF] = 1;                                      // Stocke la carry
           else
              Chip8CPU.reg_V[0xF] = 0;

           Chip8CPU.reg_V[OpCode_2] = Chip8CPU.reg_V[OpCode_3] - Chip8CPU.reg_V[OpCode_2];           // VX = VY - VX
          break;
          case 0xE: //8r0E  	Shl vr  	Décalage arithmétique à gauche de vr
                                          //Equivaut donc à vr = vr * 2. Le bit 7 va dans vf
           if (Chip8CPU.reg_V[OpCode_2] & 128)  // Si condition (VX & 128)
              Chip8CPU.reg_V[0xF] = 1;          // Stocke la carry
           else
              Chip8CPU.reg_V[0xF] = 0;

           Chip8CPU.reg_V[OpCode_2] <<= 1;
          break;
        }
        break;
      case 0x9: // 9ry0  	Skne vr,vy  	Ne fait pas l’instruction suivante si registre vr <>vy
        if (Chip8CPU.reg_V[OpCode_2] != Chip8CPU.reg_V[OpCode_3])  // Si condition (VX <> VY)
            Chip8CPU.reg_PC += 2;                                  // On saute l'instruction suivante
      break;
      case 0xA: //Axxx  	Mvi xxx  	Met xxx dans le registre I
        Chip8CPU.reg_I = (OpCode_2 * 256) + (OpCode_3 *16) + OpCode_4; // I = xxx
      break;
      case 0xB: //Bxxx  	Jmi xxx  	Va à l’adresse v0 + xxx
        Chip8CPU.reg_PC = Chip8CPU.reg_V[0] + (OpCode_2 * 256) + (OpCode_3 *16) + OpCode_4; // PC = V0 + xxx
      break;
      case 0xC: //Crxx  	Rand vr,xx  	Met un nombre aléatoire entre 0 et xx dans vr
        Chip8CPU.reg_V[OpCode_2] = (rand()%256) & ((OpCode_3 *16) + OpCode_4); // VX = random (0..xx)
      break;
      case 0xD: //Drys  	Sprite vr,vy,s  	Affiche le sprite en position vr,vy , de hauteur s

       // Pas de collision pour l'instant
       Chip8CPU.reg_V[0xF] = 0;

       for(Lig = 0; Lig < OpCode_4; Lig++) {         // Pour toutes les lignes possibles (1..15)
        gfxVal = read_mem(Chip8CPU.reg_I + Lig);     // Recup des 8 pixels de la ligne
        yVal = (Chip8CPU.reg_V[OpCode_3] + Lig);     // Recup la coordonnee Y d'affichage (opcode3 + 1..15

          for (Col = 0; Col < 8; Col++) {             // Pour les 8 pixels de la ligne
             xVal = (Chip8CPU.reg_V[OpCode_2] + Col); // Recup la coordonnee X d'affichage (opcode2 + 1..8)
             if ((gfxVal & (0x80 >> Col)) != 0) {     // Si le pixel est de couleur (donc <>0)
                if (Chip8_Display[((yVal % 32) * 64) + (xVal % 64)] == 1)   //  Si il y a dejà un pixel a cet endroit
                   Chip8CPU.reg_V[0xF] = 1;           //   Stocke la collision de pixel

                Chip8_Display[((yVal % 32) * 64) + (xVal % 64)] ^= 1;    //  Allume ou eteind le pixel en conséquence
             }
          }
       }

      break;
      case 0xE:
        switch(OpCode_3) {
          case 0x9: //Ek9E    Skpr k  	Ne fait pas l’instruction suivant sur la touche k (registre rk) est enfoncée K est un nombre
            if (Chip8_Tches[Chip8CPU.reg_V[OpCode_2]] == 1)  // Si touche enfoncee
              Chip8CPU.reg_PC += 2;                          // On saute l'instruction suivante
          break;
          case 0xA: //EkA1    Skup k  	Ne fait pas l’instruction suivant sur la touche k (registre rk) n’est pas enfoncée
            if (Chip8_Tches[Chip8CPU.reg_V[OpCode_2]] == 0)  // Si touche relachee
              Chip8CPU.reg_PC += 2;                          // On saute l'instruction suivante
          break;
        }
        break;
      case 0xF:
        switch (OpCode_3) {
          case 0x0:
            switch(OpCode_4) {
              case 0x7: //Fr07  	Gdelay vr  	Met le contenu du timer de délais dans vr
                Chip8CPU.reg_V[OpCode_2] = Chip8CPU.delay; // VX = timers delais
              break;
              case 0xA:{ //Fr0A  	Key vr  	Attend qu’une touche soit appuyée et met cette touche dans vr
                  bool KeyPressed = false;
                  for (i = 0; i < 16; i++) {   // Regarde si une des 16 touches est pressée
                    if (Chip8_Tches[i] == 1) {
                      Chip8CPU.reg_V[OpCode_2] = i;     // Si Oui, Vx = Touche
                      KeyPressed = true;
                    }
                  }

                  if (!KeyPressed)  {
                       Chip8CPU.reg_PC -= 2;  // Si Non, on reste sur la même instruction
                  }
              }
              break;
            }
            break;
          case 0x1:
            switch(OpCode_4) {
              case 0x5: //Fr15  	Sdelay vr  	Met vr dans le timer de delais
                Chip8CPU.delay = Chip8CPU.reg_V[OpCode_2]; // timers delais = VX
              break;
              case 0x8: //Fr18  	Ssound vr  	Met le contenu du timer de son dans vr
                Chip8CPU.sound = Chip8CPU.reg_V[OpCode_2]; // timers sound = VX
              break;
              case 0xE: //Fr1E  	Adi vr  	Ajoute le registre vr au registre I
                Chip8CPU.reg_I += Chip8CPU.reg_V[OpCode_2]; // I += VX
              break;
            }
            break;
          case 0x2: //Fr29  	Font vr  	Pointe I vers le sprite de caractère hexadécimal contenu dans vr
                                            //Les sprites sont de 5 pixels de haut
            Chip8CPU.reg_I = 0x0000+(Chip8CPU.reg_V[OpCode_2] * 0x5);  // I = Carac num VX
          break;
          case 0x3: //Fr33  	Bcd vr  	Stock la représentation BCD (Binaire codé décimal) du registre vr à
                                            //l’adresse  I, I+1 et I+2  	Ne change pas I
              uNbre = Chip8CPU.reg_V[OpCode_2];
           for (i = 3; i > 0; i--)   { //décimal->I+2, puis dizaine->I+1, puis centaine->I+0
              write_mem(Chip8CPU.reg_I + (i - 1),uNbre % 10);        // [I..I+2] = BCD(VX)
              uNbre /= 10;
           }
          break;
          case 0x5: //Fr55  Str v0-vr  	Stock les registres v0-vr à l’adresse pointée par I
                                        //I est incrémenté pour arriver à l’adresse suivante, donc à r+1
           for (i = 0; i <= OpCode_2; i++) {
              write_mem(Chip8CPU.reg_I + i, Chip8CPU.reg_V[i]);
           }
          break;
          case 0x6: //Fr65  	Ldr v0-vr  	Stock dans les registres v0-vr le contenu pointé par I
                                            //I est incrémenté pour arriver à l’adresse suivante, donc à r+1
           for (i = 0; i <= OpCode_2; i++) {
              Chip8CPU.reg_V[i] = read_mem(Chip8CPU.reg_I + i);
           }
          break;
        }
        break;
      default:  // Code illegal
        fprintf(stderr, "ERREUR: Opcode non reconnu !");
        break;
    } // switch (OpCode_1)
}

//*********************************************************************
bool Cpu::getChip8_Exec() const{
    return Chip8_Exec;
}

void Cpu::setChip8_Exec(bool val){
    Chip8_Exec=val;
}

int Cpu::getIperiod(){
    int tmp = Chip8_Iperiod;
    return tmp;
}

void Cpu::setIperiod(int period){
    unsigned char tmp = period;
    Chip8_Iperiod = tmp;
}

bool Cpu::LoadRom(const char* fileName)
{

    std::ifstream rom(fileName, std::ios::binary);

    if(!rom.is_open()) return false;

    rom.seekg(0, std::ios::end);
    long file_size = rom.tellg();
    rom.seekg(0, std::ios::beg);

    rom.read((char*)&Chip8_MEM[0x200], file_size);

    rom.close();
    //afficheMem();
    return true;

}

void Cpu::Chip8InitFont() {

   Chip8_MEM[0x0000] = 96;     //.11.....
   Chip8_MEM[0x0001] = 144;    //1..1....
   Chip8_MEM[0x0002] = 144;    //1..1....
   Chip8_MEM[0x0003] = 144;    //1..1....
   Chip8_MEM[0x0004] = 96;     //.11.....

   Chip8_MEM[0x0005] = 96;     //.11.....
   Chip8_MEM[0x0006] = 224;    //111.....
   Chip8_MEM[0x0007] = 96;     //.11.....
   Chip8_MEM[0x0008] = 96;     //.11.....
   Chip8_MEM[0x0009] = 240;    //1111....

   Chip8_MEM[0x000A] = 96;     //.11.....
   Chip8_MEM[0x000B] = 144;    //1..1....
   Chip8_MEM[0x000C] = 32;     //..1.....
   Chip8_MEM[0x000D] = 64;     //.1......
   Chip8_MEM[0x000E] = 240;    //1111....

   Chip8_MEM[0x000F] = 224;    //111.....
   Chip8_MEM[0x0010] = 16;     //...1....
   Chip8_MEM[0x0011] = 96;     //.11.....
   Chip8_MEM[0x0012] = 16;     //...1....
   Chip8_MEM[0x0013] = 224;    //111.....

   Chip8_MEM[0x0014] = 160;    //1.1.....
   Chip8_MEM[0x0015] = 160;    //1.1.....
   Chip8_MEM[0x0016] = 240;    //1111....
   Chip8_MEM[0x0017] = 32;     //..1.....
   Chip8_MEM[0x0018] = 32;     //..1.....

   Chip8_MEM[0x0019] = 240;    //1111....
   Chip8_MEM[0x001A] = 128;    //1.......
   Chip8_MEM[0x001B] = 240;    //1111....
   Chip8_MEM[0x001C] = 16;     //...1....
   Chip8_MEM[0x001D] = 240;    //1111....

   Chip8_MEM[0x001E] = 240;    //1111....
   Chip8_MEM[0x001F] = 128;    //1.......
   Chip8_MEM[0x0020] = 240;    //1111....
   Chip8_MEM[0x0021] = 144;    //1..1....
   Chip8_MEM[0x0022] = 240;    //1111....

   Chip8_MEM[0x0023] = 240;    //1111....
   Chip8_MEM[0x0024] = 16;     //...1....
   Chip8_MEM[0x0025] = 32;     //..1.....
   Chip8_MEM[0x0026] = 64;     //.1......
   Chip8_MEM[0x0027] = 128;    //1.......

   Chip8_MEM[0x0028] = 96;     //.11.....
   Chip8_MEM[0x0029] = 144;    //1..1....
   Chip8_MEM[0x002A] = 96;     //.11.....
   Chip8_MEM[0x002B] = 144;    //1..1....
   Chip8_MEM[0x002C] = 96;     //.11.....

   Chip8_MEM[0x002D] = 240;    //1111....
   Chip8_MEM[0x002E] = 144;    //1..1....
   Chip8_MEM[0x002F] = 240;    //1111....
   Chip8_MEM[0x0030] = 16;     //...1....
   Chip8_MEM[0x0031] = 240;    //1111....

   Chip8_MEM[0x0032] = 96;     //.11.....
   Chip8_MEM[0x0033] = 144;    //1..1....
   Chip8_MEM[0x0034] = 240;    //1111....
   Chip8_MEM[0x0035] = 144;    //1..1....
   Chip8_MEM[0x0036] = 144;    //1..1....

   Chip8_MEM[0x0037] = 224;    //111.....
   Chip8_MEM[0x0038] = 144;    //1..1....
   Chip8_MEM[0x0039] = 224;    //111.....
   Chip8_MEM[0x003A] = 144;    //1..1....
   Chip8_MEM[0x003B] = 224;    //111.....

   Chip8_MEM[0x003C] = 112;    //.111....
   Chip8_MEM[0x003D] = 128;    //1.......
   Chip8_MEM[0x003E] = 128;    //1.......
   Chip8_MEM[0x003F] = 128;    //1.......
   Chip8_MEM[0x0040] = 112;    //.111....

   Chip8_MEM[0x0041] = 224;    //111.....
   Chip8_MEM[0x0042] = 144;    //1..1....
   Chip8_MEM[0x0043] = 144;    //1..1....
   Chip8_MEM[0x0044] = 144;    //1..1....
   Chip8_MEM[0x0045] = 224;    //111.....

   Chip8_MEM[0x0046] = 240;    //1111....
   Chip8_MEM[0x0047] = 128;    //1.......
   Chip8_MEM[0x0048] = 224;    //111.....
   Chip8_MEM[0x0049] = 128;    //1.......
   Chip8_MEM[0x004A] = 240;    //1111....

   Chip8_MEM[0x004B] = 240;    //1111....
   Chip8_MEM[0x004C] = 128;    //1.......
   Chip8_MEM[0x004D] = 224;    //111.....
   Chip8_MEM[0x004E] = 128;    //1.......
   Chip8_MEM[0x004F] = 128;    //1.......
}

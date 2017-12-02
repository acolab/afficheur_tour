/*
FG le 1/12/2017 dans le cadre de la modification du tour de l'association ACOLAB-Clermont -Fd
Ce code Aduino permet d'afficher la vitesse de la broche d'un tour sur un afficheur 4 digits-7 segments
Pour cela, on utilise un Arduino nano câblé sur un afficheur 5461AS
Le moteur du tour est piloté par un variateur de fréquence Altivar ...
Une sortie analogique paramétrable permet de transmettre la consigne de fréquence de rotation envoyée au moteur
Cette sortie transmet un signal 0-20mA
L'entrée analogique du nano "A0" reçoit ce signal.
Echelle :  0mA = 0 Hz ... 20mA =  100 Hz soit vitesse moteur = 3000 t/mn
Les entrées ANA du nano ne pouvant interpréter qu'une variation de tension à ses bornes, ce signal est remis en forme 0-10V grâce à une résistance de 500 ohms +/- 1%.
On vient donc lire la chute de tension aux bornes de la résistance U=RI ; pour I = 20mA => U= 500 x 0,020 = 10 V
Merci à Oussama Amri ( http://www.ithepro.com/) pour son travail de décodage bien sympa... Cela évite de réinventer la roue ! J'en ai repris la base...
Quelques modifications cependant afin d'éviter le phénomène de scintillement : 2 paramètres ajustables permettent de limiter celui-ci : le temps de rafraichissement
et le pitch (pas ou écart) selon la stabilité du signal reçu.
*/

const int adressePin = A0;    // Pin entrée signal 0-20mA / 0-10V (sur résistance 500 ohm)

// Affichage 7 segments câblage nano => 5461AS
int segA = 5;   // >> 11
int segB = 13;  // >> 7
int segC = 10;  // >> 4
int segD = 8;   // >> 2
int segE = 7;   // >> 1
int segF = 4;   // >> 10
int segG = 11;  // >> 5
int segPt = 9;  // >> 3

//------------//

// Affichage du digit
int d1 = 6;     // >> 12 avec résistance 220 ohms
int d2 = 3;     // >> 9           "       "
int d3 = 2;     // >> 8           "       "
int d4 = 12;    // >> 6           "       "

//------------//

// variables S/P vitesse
int entreeAna = 0;          // valeur entrée vitesse
int vitesse = 0;            // valeur vitesse vers S/P affichage
int valeurMin = 0;          // minimum vitesse
int valeurMax = 1023;       // maximum vitesse
int mil = 0;                // digit milliers
int cent = 0;               // digit centaines
int diz = 0;                // digit dizaines
int unit = 0;               // digit unités
int reste = 0;              // calcul du code BCD
int nb = 0;                 // calcul du code BCD
int delayTime = 5000;       // Fréquence d'affichage
int i=0;                    // index d'incrémentation boucle affichage digit
int sauveValeur = 0;        // sauve valeur mesure pour anti scintillement
int pitch = 3;              // valeur du pas pour anti scintillement

//-----------//
// vriables S/P tempos
int duree1 = 300;                   // durée tempo 1 : 0,3 sec
int duree2 = 1000;                  // durée tempo 2 : 1 sec
int memTempo1 = 0;                  // RAZ tempo 
int memTempo2 = 0;                  // RAZ tempo
unsigned long prev1;                // sauvegarde valeur millisecondes pour tempo1
unsigned long prev2;                // sauvegarde valeur millisecondes pour tempo2
int finTempo1 = 0;                  // bit fin tempo 1
int finTempo2 = 0;                  // bit fin tempo 2
int valTempo1 = 0;                  // bit lancement tempo 1
int valTempo2 = 0;                  // bit lancement tempo 2

//=============================================//

// définition des sorties
void setup() {
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);  

  prev1 = millis();          // initialise variables sauve millisecondes 
  prev2 = millis();
}
//=============================================//
void loop() {
  temp();                      // tempos 1 et 2
  vit();                       // acquisition signal vitesse
  bcd();                       // conversion décimal => BCD
  writeN(mil,cent,diz,unit);   // Ecriture du nombre à 4 digits
}

//=============================================//
void temp(){
  // sous programme temporisations 1 & 2

  // Tempo 1

  if ((valTempo1==1) &  (memTempo1==0))                 // lance tempo ?
  { 
    memTempo1 = 1;                                  // mise à 1 mémoire tempo en cours
    prev1 = millis();                               // charge nbre millisec en mémoire
  }
  if ((valTempo1== 1) &  (memTempo1==1))           // tempo en cours
  {
    if ((millis() - prev1) > duree1)        // valeur courrante == valeur préselection
    { 
      finTempo1 = 1;                    // bit fin tempo à 1
    }
    else 
    {
      finTempo1 = 0;                    // RAZ bit fin tempo
    }
  }

  if (valTempo1==0) {                                    // tempo non validée
    memTempo1=0;                                        // RAZ tempo en cours
    finTempo1 = 0;                                      // RAZ fin tempo
  }    

  Serial.print(" valeurTempo 1: ");
  Serial.println(prev1);                           



  // Tempo 2

  if ((valTempo2==1) &  (memTempo2==0))                 // lance tempo ?
  { 
    memTempo2 = 1;                                  // mise à 1 mémoire tempo en cours
    prev2 = millis();                               // charge nbre millisec en mémoire
  }
  if ((valTempo2== 1) &  (memTempo2==1))           // tempo en cours
  {
    if ((millis() - prev2) > duree2)        // valeur courrante == valeur préselection
    { 
      finTempo2 = 1;                    // bit fin tempo à 1
    }
    else 
    {
      finTempo2 = 0;                    // RAZ bit fin tempo
    }
  }

  if (valTempo2==0) {                                    // tempo non validée
    memTempo2=0;                                        // RAZ tempo en cours
    finTempo2 = 0;                                    // RAZ fin tempo
  }     

}
//==============================================//
void vit(){
  // acquisition signal vitesse
  entreeAna = analogRead(adressePin);                                                                    // lecture entrée ANA

  if(valTempo1 == 0)   valTempo1 = 1;                                                                    // lance tempo 1 "anti scintillement"

  if(( finTempo1 == 1 && ((entreeAna > (sauveValeur + pitch))||(entreeAna  < (sauveValeur - pitch)))))   // variation de la valeur dans l'intervalle fixé ?
  {
    sauveValeur = entreeAna;                                                                               // mémorise nouvelle valeur lue
    vitesse = map(sauveValeur, valeurMin, valeurMax, 0, 1700);                                             // mise à l'échelle 0-10V -> 0-100Hz -> 0-1700 t/mn
    valTempo1 = 0;                                                                                         // RAZ tempo 1
  }
}

//=============================================//
// conversion décimal => BCD
void bcd() {                                              // Commentaires                     ex: nb = 1234                   ex: nb = 903
  nb=vitesse;                                             //                                  
  if (nb < 1000){                                         // nombre < 1000 ?                  non                             oui
    mil =0;                                         // mil = 0                                                          mil = 0
    reste = nb;}                                    // charge reste avec nb             reste = 234                     reste = 904
  else {                                          // donc nombre > 1000     
    mil = nb/1000;                           // calcul milliers                  mil = 1               
    reste = nb - (mil*1000);                 // calcul reste                     reste = 234               
  }
  if (reste < 100){                                       // valeur < 100 ?                   non                             non
    cent = 0;}                                  // cent = 0
  else {                                      // donc reste > 100
    cent = reste/100;                     // calcul centaines                 cent = 2                        cent = 9
    reste = reste - (cent*100);           // calcul reste                     reste = 34                      reste = 03
  }
  if (reste < 10){                            // valeur < 10 ?                    non                             oui
    diz = 0;}                               // diz = 0                                                          diz = 0
  else {                                  // donc reste > 10
    diz = reste/10;                  // calcul dizaines                  diz = 3
    reste = reste-(diz*10);          // calcul reste                     reste = 4                       reste = 3                      
  }       
  unit = reste;                                            // reste l'unité                    unit = 4                        unit = 3
}

//=============================================//
//Ecriture du nombre à 4 digits - writeN(1,2,3,4)
void writeN(int a,int b,int c,int d){
  selectDwriteL(1,a);
  selectDwriteL(2,b);
  selectDwriteL(3,c);
  selectDwriteL(4,d);
}

//=============================================//
//Selection Digit (selectD) à afficher (writeL)
void selectDwriteL(int d,int l){
  switch (d) {                              // sélection du digit
    case 0: digitalWrite(d1, LOW);          // case 0 - 4 digits
            digitalWrite(d2, LOW);
            digitalWrite(d3, LOW);
            digitalWrite(d4, LOW);
            break;
    case 1: digitalWrite(d1, LOW);          // case 1 - Digit 1
            digitalWrite(d2, HIGH);
            digitalWrite(d3, HIGH);
            digitalWrite(d4, HIGH);
            break;
    case 2: digitalWrite(d1, HIGH);         // case 1 - Digit 2
            digitalWrite(d2, LOW);
            digitalWrite(d3, HIGH);
            digitalWrite(d4, HIGH);
            break;
    case 3: digitalWrite(d1, HIGH);         // case 1 - Digit 3
            digitalWrite(d2, HIGH);
            digitalWrite(d3, LOW);
            digitalWrite(d4, HIGH);
            break;
    case 4: digitalWrite(d1, HIGH);         // case 1 - Digit 4
            digitalWrite(d2, HIGH);
            digitalWrite(d3, HIGH);
            digitalWrite(d4, LOW);
            break;
  }

  switch (l) {                              // choix chiffre à afficher
    case 0: zero();
            break;
    case 1: one();
            break;
    case 2: two();
            break;
    case 3: three();
            break;
    case 4: four();
            break;
    case 5: five();
            break;
    case 6: six();
            break;
    case 7: seven();
            break;
    case 8: eight();
            break;
    case 9: nine();
            break;
    case 10: point();                       // Point
             break;
    case 11: none();                        // Aucun 
             break;
  }

  delayMicroseconds(delayTime);               // Fréquence d'affichage

}

// codage 7 segments
//=============================================//
void zero(){
  digitalWrite(segA, HIGH);
  digitalWrite(segB, HIGH);
  digitalWrite(segC, HIGH);
  digitalWrite(segD, HIGH);
  digitalWrite(segE, HIGH);
  digitalWrite(segF, HIGH);
  digitalWrite(segG, LOW);
  digitalWrite(segPt, LOW);
}
//=============================================//
void one(){
  digitalWrite(segA, LOW);
  digitalWrite(segB, HIGH);
  digitalWrite(segC, HIGH);
  digitalWrite(segD, LOW);
  digitalWrite(segE, LOW);
  digitalWrite(segF, LOW);
  digitalWrite(segG, LOW);
  digitalWrite(segPt, LOW);
}
//=============================================//
void two(){
  digitalWrite(segA, HIGH);
  digitalWrite(segB, HIGH);
  digitalWrite(segC, LOW);
  digitalWrite(segD, HIGH);
  digitalWrite(segE, HIGH);
  digitalWrite(segF, LOW);
  digitalWrite(segG, HIGH);
  digitalWrite(segPt, LOW);
}
//=============================================//
void three(){
  digitalWrite(segA, HIGH);
  digitalWrite(segB, HIGH);
  digitalWrite(segC, HIGH);
  digitalWrite(segD, HIGH);
  digitalWrite(segE, LOW);
  digitalWrite(segF, LOW);
  digitalWrite(segG, HIGH);
  digitalWrite(segPt, LOW);
}
//=============================================//
void four(){
  digitalWrite(segA, LOW);
  digitalWrite(segB, HIGH);
  digitalWrite(segC, HIGH);
  digitalWrite(segD, LOW);
  digitalWrite(segE, LOW);
  digitalWrite(segF, HIGH);
  digitalWrite(segG, HIGH);
  digitalWrite(segPt, LOW);
}
//=============================================//
void five(){
  digitalWrite(segA, HIGH);
  digitalWrite(segB, LOW);
  digitalWrite(segC, HIGH);
  digitalWrite(segD, HIGH);
  digitalWrite(segE, LOW);
  digitalWrite(segF, HIGH);
  digitalWrite(segG, HIGH);
  digitalWrite(segPt, LOW);
}
//=============================================//
void six(){
  digitalWrite(segA, HIGH);
  digitalWrite(segB, LOW);
  digitalWrite(segC, HIGH);
  digitalWrite(segD, HIGH);
  digitalWrite(segE, HIGH);
  digitalWrite(segF, HIGH);
  digitalWrite(segG, HIGH);
  digitalWrite(segPt, LOW);
}
//=============================================//
void seven(){
  digitalWrite(segA, HIGH);
  digitalWrite(segB, HIGH);
  digitalWrite(segC, HIGH);
  digitalWrite(segD, LOW);
  digitalWrite(segE, LOW);
  digitalWrite(segF, LOW);
  digitalWrite(segG, LOW);
  digitalWrite(segPt, LOW);
}
//=============================================//
void eight(){
  digitalWrite(segA, HIGH);
  digitalWrite(segB, HIGH);
  digitalWrite(segC, HIGH);
  digitalWrite(segD, HIGH);
  digitalWrite(segE, HIGH);
  digitalWrite(segF, HIGH);
  digitalWrite(segG, HIGH);
  digitalWrite(segPt, LOW);
}
//=============================================//
void nine(){
  digitalWrite(segA, HIGH);
  digitalWrite(segB, HIGH);
  digitalWrite(segC, HIGH);
  digitalWrite(segD, HIGH);
  digitalWrite(segE, LOW);
  digitalWrite(segF, HIGH);
  digitalWrite(segG, HIGH);
  digitalWrite(segPt, LOW);
}
//=============================================//
void point(){
  digitalWrite(segA, LOW);
  digitalWrite(segB, LOW);
  digitalWrite(segC, LOW);
  digitalWrite(segD, LOW);
  digitalWrite(segE, LOW);
  digitalWrite(segF, LOW);
  digitalWrite(segG, LOW);
  digitalWrite(segPt, HIGH);
}
//=============================================//
void none(){
  digitalWrite(segA, LOW);
  digitalWrite(segB, LOW);
  digitalWrite(segC, LOW);
  digitalWrite(segD, LOW);
  digitalWrite(segE, LOW);
  digitalWrite(segF, LOW);
  digitalWrite(segG, LOW);
  digitalWrite(segPt, LOW);
}


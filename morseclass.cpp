#include "morseclass.h"
#include <cstring>
#include <cstdlib> // exit()


namespace {  // Zeichenvorrat CW-Zeichen: 0-9, A-Z, und . , / = - Whitespace
    const char zeichen[][10] {
      {"----- "}, // 0
      {".---- "},
      {"..--- "},
      {"...-- "},
      {"....- "},
      {"..... "},
      {"-.... "},
      {"--... "},
      {"---.. "},
      {"----. "},
      {".- "}, // a  Pos.10
      {"-... "},
      {"-.-. "},
      {"-.. "},
      {". "},
      {"..-. "},
      {"--. "},
      {".... "},
      {".. "},
      {".--- "},
      {"-.- "},  // k   Pos.20
      {".-.. "},
      {"-- "},
      {"-. "},
      {"--- "},
      {".--. "},
      {"--.- "},
      {".-. "},
      {"... "},
      {"- "},
      {"..- "},  //  u   Pos.30
      {"...- "},
      {".-- "},
      {"-..- "},
      {"-.-- "},
      {"--.. "}, // z
      {".-.-.- "}, //   "."
      {"--..-- "}, //   ","
      {"-..-. "}, //    "/"
      {"-...- "}, //    "="
      {"-....- "}, //   "-"    Pos.40
      {"..--.. "}, //   "?"
      {"  "} // Whitespace
    };
} // ende anonymer namespace


/* Konstruktor:
Ausgabepin wird festgelegt, wahlweise nach 
wiringPi 'w' oder GPIO 'g' -Pinbelegung
Gebegeschwindigkeit wird berechnet (WPM 'W' oder BPM 'B')
und gespeichert/ Sendetext wird in ASCII-Grossbuchstaben 
uebersetzt und gespeichert */
Morseclass::Morseclass(char pmode, int pin, char smode, int speed, const char* text) {
  cwPin = pin;

  cwSpeed = speed;
  

  // Auswahl Pinbelegung nach WiringPi- oder GPIO-Bibliothek
  if(pmode == 'w') {
    wiringPiSetup();
    pinMode(cwPin, OUTPUT);
  }
  else if (pmode == 'g') {
    wiringPiSetupGpio();
    pinMode(cwPin, OUTPUT);
  }
  else {
    exit(-1);
  }

  // Auswahl Gebegeschwindigkeit nach WPM oder BPM
  if(smode == 'W') {
    if(cwSpeed < 5 || cwSpeed > 50) exit(-1);
    dotLen = 1200 / cwSpeed;
  }
  else if (smode == 'B') {
    if(speed < 9 || speed > 275) exit(-1);
    dotLen = 6000 / speed;
  }
  else {
    exit(-1);
  }

  // String in Grossbuchstaben wandeln und kopieren
  int laenge = strlen(text);
  if(laenge >= 100) exit(-1); // maximal moegliche Laenge des Textes
  int index {0};
  while(index < laenge) {
    char temp = text[index];
    if(temp >= 'a' && temp <= 'z') temp -= DIFF_KLEIN_GROSS;
    sendeStr[index] = temp;
    ++index;
  }
  sendeStr[index] = '\0';
}  // Konstruktor Ende


// nimmt Zeichen aus Sendetext und gibt Zeilenposition in der 
// Morsetabelle zurueck
// bricht ab wenn ein unerlaubtes Zeichen gesendet werden soll
int Morseclass::findeZeichenInTabelle(const char einZeichen) {
  int temp;
  if(einZeichen >= 'A' && einZeichen <= 'Z') temp = einZeichen - DIFF_GROSS_TABELLE;
  else if(einZeichen >= '0' && einZeichen <= '9') temp = einZeichen - DIFF_ZAHL;
  else if(einZeichen == '.') temp = 36;
  else if(einZeichen == ',') temp = 37;
  else if(einZeichen == '/') temp = 38;
  else if(einZeichen == '=') temp = 39;
  else if(einZeichen == '-') temp = 40;
  else if(einZeichen == '?') temp = 41;
  else if(einZeichen == ' ') temp = 42;
  else exit(-1); // Abbruch, da kein anderes Zeichen als oben angegeben vorkommen darf
  return temp; 
}

// schreibt den Punkt auf den Ausgang
void Morseclass::dot() {
  digitalWrite(cwPin, HIGH);
  delay(dotLen);
  digitalWrite(cwPin, LOW);
  delay(dotLen);
}

// schreibt den Strich auf den Ausgang
void Morseclass::dash() {
  digitalWrite(cwPin, HIGH);
  delay(3*dotLen);
  digitalWrite(cwPin, LOW);
  delay(dotLen);
}

// wartet Pause zwischen zwei Zeichen ab
void Morseclass::pause() {
  delay(3*dotLen);
}


// Zeichen des Sendetextes werden durchlaufen, anhand jedes Zeichens wird
// das dazugehoerige Morsezeichen aus der Tabelle gesucht und abgespielt
void Morseclass::cwRun() {

  int strIndex {0};
  while(sendeStr[strIndex] != '\0') { // durchlaeuft Sendetext
    char aktuellesZeichen = sendeStr[strIndex++];
    int tabZeile = findeZeichenInTabelle(aktuellesZeichen);
    int zeichenIndex {0};
    while(zeichen[tabZeile][zeichenIndex] != '\0') { // durchlaeuft Morsezeichen
      char aktuellesCwElement = zeichen[tabZeile][zeichenIndex++];
      switch(aktuellesCwElement) {
        case '.': dot();
                  break;
        case '-': dash();
                  break;
        case ' ': pause();
                  break;
        default:  break;
      }
    }
  }
}

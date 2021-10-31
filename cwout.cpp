/* CWOUT 1.0 - Harry Seiler DJ5MY - diese Software ist freie Software 
   Kopie, Weitergabe und Weiterentwicklung ausdruecklich erlaubt und 
   erwuenscht - viel Spass beim basteln */


/* Unterstützte Flags:
   -w Deklaration des benutzten Pins nach wiringPi-Schema
   -g Deklaration des benutzten Pins nach GPIO-Schema
   -W Einstellen des CW-Speeds Aufgrund WPM
   -B Einstellen des CW-Speeds Aufgrund BPM
   -v "verbose"-Modus, ausführliche Ausgabe aller Werte
   -q "quiet"-Modus, Unterdrückung aller Ausgaben, auch Fehler, zum Einsatz
      in Skripten
   -s zu sendender String
*/

#include <iostream>
#include "wiringPi.h"
#include <cstring>
#include <cstdlib> // fuer exit()
#include <cassert> // fuer assert()
#include "morseclass.h"


using namespace std;



bool nurZiffernImString(const char* str); // prueft auf Ziffern ab Index 0

bool nurZiffernImTeilString(const char* str); // prueft auf ziffern ab Index 2

bool validCwStr(const char* str);

void showHelpPage();



// Main
int main(int argc, char* argv[]) {
  
  enum class Ausgabemodus {NORMAL, VERBOSE, QUIET} modus = Ausgabemodus::NORMAL; // Normal: assert() wird ausgegeben, Vorgabeeinstellung
                                                                                 // Verbose: ausführliche Fehlerausgabe und anderes
                                                                                 // Quiet: nichts auf der Standardausgabe, fuer Skripteinsatz

  enum class Pindecl {WIRINGPI, GPIO} pinDeclarationMode = Pindecl::WIRINGPI; // Auswahl des benutzten Pin-Schemas mit Vorbelegung wiringPi
  bool pinStatusSet = false; // noch keine Option zur Pinbelegung festgelegt, wird genutzt zur Verriegelung falls -w + -g zusammen
                             // angegeben werden
  
  int cwPin {0}; //  nimmt den Pin auf, gilt für WiringPi Schema und für das GPIO Schema

  constexpr int MIN_PIN {0};
  constexpr int MAX_PIN {31}; // ein Pin aus dieser Menge muss gewaehlt werden

  enum class Speeddecl {WPM, BPM} speedDeclarationMode = Speeddecl::WPM; // Auswahl des benutzten Speed-Schemas mit Vorbelegung WordsPerMinute
  bool speedStatusSet = false; // noch keine Option zur Speeddeklaration festgelegt, wird genutzt zur Verriegelung falls -W + -B zusammen
                               // angegeben werden

  int cwSpeed {0}; // speichert die CW-Ausgabegeschwindigkeit, gilt für WPM und BPM 


  char sendeStr[100]; // speichert den CW-Text

  int ii {1};  
  while(argv[ii]) {    // zuerst alle Opionen nach -v -q durchsuchen
    if(strstr(argv[ii], "-v")) {
      if(modus != Ausgabemodus::QUIET) modus = Ausgabemodus::VERBOSE;
    }
    if(strstr(argv[ii], "-q")) { 
      if(modus != Ausgabemodus::VERBOSE) modus = Ausgabemodus::QUIET;
    } // gegenseitige Verriegelung zum Schutz gegen Mehrfachauswahl -q -v
    if(strstr(argv[ii], "--help")) {
      showHelpPage();
    } 
    ++ii;
  }

  int i {1};
  while(argv[i]) {

    int laenge = strlen(argv[i]);
    if(argv[i][0] == '-') {

      switch(argv[i][1]) {
        case 'w':
          if(!pinStatusSet) {
            pinStatusSet = true; // Vorbelegung Pin-Schema wiringPi wird beibehalten und verriegelt (zusätzliche Auswahl GPIO nicht mehr möglich)
            pinDeclarationMode = Pindecl::WIRINGPI;

            if(laenge > 2) {  // gilt, falls der Wert für die Pinbelegung ohne Whitespace nach der Option angegeben wurde
              if(!nurZiffernImTeilString(argv[i])) { // Ist die Option länger als 2 Zeichen und sind die Zeichen keine Ziffern -> Abbruch
                if(modus == Ausgabemodus::NORMAL || modus == Ausgabemodus::VERBOSE) {
                  cerr << "Unbekannte Option fuer \"-w\" \n"; // Abbruch mit detailierter Fehlermeldung
                  assert(nurZiffernImString(argv[i]));
                }
                if(modus == Ausgabemodus::QUIET) { // Abbruch ohne Fehlermeldung
                  exit(-1);
                }
              }
              int index {2};
              int temp;
              while(index < laenge) {     // extrahieren der Zahl aus dem String
                temp = argv[i][index] - '0';
                cwPin = cwPin * 10 + temp;
                ++index;
              }
            }
            else { // gilt falls nur Option angegeben wurde. Im naechsten String muss das Argument dazu sein.
              ++i;
              if(!nurZiffernImString(argv[i])) { // Prüfung auf validen String, sind die Zeichen keine Ziffern -> Abbruch
                if(modus == Ausgabemodus::NORMAL || modus == Ausgabemodus::VERBOSE) {
                  cerr << "Unbekannte Option fuer \"-w\" \n"; // Abbruch mit detailierter Fehlermeldung
                  assert(nurZiffernImString(argv[i]));
                }
                if(modus == Ausgabemodus::QUIET) { // Abbruch ohne Fehlermeldung
                  exit(-1);
                }
              }
              else { // valider String, nur Ziffern enthalten
                int index {0};
                int temp;
                while(index < strlen(argv[i])) {
                  temp = argv[i][index] - '0';
                  cwPin = cwPin * 10 + temp;
                  ++index;
                }
              }
            }
          }
          break;
        case 'g':
          if(!pinStatusSet) { 
            pinStatusSet = true; // Verriegelung der Option, Auswahl von Schema WiringPi nicht mehr möglich
            pinDeclarationMode = Pindecl::GPIO;
           
            if(laenge > 2) {  // gilt, falls der Wert für die Pinbelegung ohne Whitespace nach der Option angegeben wurde
              if(!nurZiffernImTeilString(argv[i])) { // Ist die Option länger als 2 Zeichen und sind die Zeichen keine Ziffern -> Abbruch
                if(modus == Ausgabemodus::NORMAL || modus == Ausgabemodus::VERBOSE) {
                  cerr << "Unbekannte Option fuer \"-g\" \n"; // Abbruch mit detailierter Fehlermeldung
                  assert(nurZiffernImString(argv[i]));
                }
                if(modus == Ausgabemodus::QUIET) { // Abbruch ohne Fehlermeldung
                  exit(-1);
                }
              }
              int index {2};
              int temp;
              while(index < laenge) {     // extrahieren der Zahl aus dem String
                temp = argv[i][index] - '0';
                cwPin = cwPin * 10 + temp;
                ++index;
              }
            }
            else { // gilt falls nur Option angegeben wurde. Im naechsten String muss das Argument dazu sein.
              ++i;
              if(!nurZiffernImString(argv[i])) { // Prüfung auf validen String, sind die Zeichen keine Ziffern -> Abbruch
                if(modus == Ausgabemodus::NORMAL || modus == Ausgabemodus::VERBOSE) {
                  cerr << "Unbekannte Option fuer \"-g\" \n"; // Abbruch mit detailierter Fehlermeldung
                  assert(nurZiffernImString(argv[i]));
                }
                if(modus == Ausgabemodus::QUIET) { // Abbruch ohne Fehlermeldung
                  exit(-1);
                }
              }
              else { // valider String, nur Ziffern enthalten
                int index {0};
                int temp;
                while(index < strlen(argv[i])) {
                  temp = argv[i][index] - '0';
                  cwPin = cwPin * 10 + temp;
                  ++index;
                }
              }
            }
          }
          break;
        case 'W':
          if(!speedStatusSet) { 
            speedStatusSet = true; // Verriegelung der Option, Auswahl von Schema WiringPi nicht mehr möglich
            speedDeclarationMode = Speeddecl::WPM;
           
            if(laenge > 2) {  // gilt, falls der Wert für die Pinbelegung ohne Whitespace nach der Option angegeben wurde
              if(!nurZiffernImTeilString(argv[i])) { // Ist die Option länger als 2 Zeichen und sind die Zeichen keine Ziffern -> Abbruch
                if(modus == Ausgabemodus::NORMAL || modus == Ausgabemodus::VERBOSE) {
                  cerr << "Unbekannte Option fuer \"-W\" \n"; // Abbruch mit detailierter Fehlermeldung
                  assert(nurZiffernImString(argv[i]));
                }
                if(modus == Ausgabemodus::QUIET) { // Abbruch ohne Fehlermeldung
                  exit(-1);
                }
              }
              int index {2};
              int temp;
              while(index < laenge) {     // extrahieren der Zahl aus dem String
                temp = argv[i][index] - '0';
                cwSpeed = cwSpeed * 10 + temp;
                ++index;
              }
            }
            else { // gilt falls nur Option angegeben wurde. Im naechsten String muss das Argument dazu sein.
              ++i;
              if(!nurZiffernImString(argv[i])) { // Prüfung auf validen String, sind die Zeichen keine Ziffern -> Abbruch
                if(modus == Ausgabemodus::NORMAL || modus == Ausgabemodus::VERBOSE) {
                  cerr << "Unbekannte Option fuer \"-W\" \n"; // Abbruch mit detailierter Fehlermeldung
                  assert(nurZiffernImString(argv[i]));
                }
                if(modus == Ausgabemodus::QUIET) { // Abbruch ohne Fehlermeldung
                  exit(-1);
                }
              }
              else { // valider String, nur Ziffern enthalten
                int index {0};
                int temp;
                while(index < strlen(argv[i])) {
                  temp = argv[i][index] - '0';
                  cwSpeed = cwSpeed * 10 + temp;
                  ++index;
                }
              }
            }
          }
          break;
        case 'B':
          if(!speedStatusSet) { 
            speedStatusSet = true; // Verriegelung der Option, Auswahl von Schema WiringPi nicht mehr möglich
            speedDeclarationMode = Speeddecl::BPM;
           
            if(laenge > 2) {  // gilt, falls der Wert für die Pinbelegung ohne Whitespace nach der Option angegeben wurde
              if(!nurZiffernImTeilString(argv[i])) { // Ist die Option länger als 2 Zeichen und sind die Zeichen keine Ziffern -> Abbruch
                if(modus == Ausgabemodus::NORMAL || modus == Ausgabemodus::VERBOSE) {
                  cerr << "Unbekannte Option fuer \"-B\" \n"; // Abbruch mit detailierter Fehlermeldung
                  assert(nurZiffernImString(argv[i]));
                }
                if(modus == Ausgabemodus::QUIET) { // Abbruch ohne Fehlermeldung
                  exit(-1);
                }
              }
              int index {2};
              int temp;
              while(index < laenge) {     // extrahieren der Zahl aus dem String
                temp = argv[i][index] - '0';
                cwSpeed = cwSpeed * 10 + temp;
                ++index;
              }
            }
            else { // gilt falls nur Option angegeben wurde. Im naechsten String muss das Argument dazu sein.
              ++i;
              if(!nurZiffernImString(argv[i])) { // Prüfung auf validen String, sind die Zeichen keine Ziffern -> Abbruch
                if(modus == Ausgabemodus::NORMAL || modus == Ausgabemodus::VERBOSE) {
                  cerr << "Unbekannte Option fuer \"-B\" \n"; // Abbruch mit detailierter Fehlermeldung
                  assert(nurZiffernImString(argv[i]));
                }
                if(modus == Ausgabemodus::QUIET) { // Abbruch ohne Fehlermeldung
                  exit(-1);
                }
              }
              else { // valider String, nur Ziffern enthalten
                int index {0};
                int temp;
                while(index < strlen(argv[i])) {
                  temp = argv[i][index] - '0';
                  cwSpeed = cwSpeed * 10 + temp;
                  ++index;
                }
              }
            }
          }
          break;
        case 's':
          if(laenge > 2) {
            if(laenge > 100) { // Pruefung auf maximale Laenge des Strings
              if(modus == Ausgabemodus::NORMAL || modus == Ausgabemodus::VERBOSE) {
                 cerr << "Text ist laenger als 100 Zeichen!\n"; // Abbruch mit detailierter Fehlermeldung
                 assert(laenge <= 100);
              }
              if(modus == Ausgabemodus::QUIET) { // Abbruch ohne Fehlermeldung
                exit(-1);
              }
            }
            if(!validCwStr(argv[i])) { // Pruefung auf unerlaubte Zeichen
              if(modus == Ausgabemodus::NORMAL || modus == Ausgabemodus::VERBOSE) {
                 cerr << "Text enthält undefinierte Zeichen \n"; // Abbruch mit detailierter Fehlermeldung
                 assert(validCwStr(argv[i]));
              }
              if(modus == Ausgabemodus::QUIET) { // Abbruch ohne Fehlermeldung
                exit(-1);
              }
            }
            else { // nur valide Zeichen im String, kopiere Sendetext
              int index_argv {2}; // Sourcestring beginnt bei 2, die ersten beiden Zeichen sind "-s"
              int index_sendeStr {index_argv - 2}; // Destinationstring beginnt bei 0
              while(index_argv < strlen(argv[i])) {
                sendeStr[index_sendeStr] = argv[i][index_argv];
                ++index_argv;
                ++index_sendeStr;
              }
              sendeStr[index_sendeStr] = '\0';
            }
          }
          else { // der Sendestring befindet sich im nächsten String von argv[i] 
            ++i;
            if(strlen(argv[i]) > 100) { // Pruefung auf maximale Laenge des Strings
              if(modus == Ausgabemodus::NORMAL || modus == Ausgabemodus::VERBOSE) {
                 cerr << "Text ist laenger als 100 Zeichen!\n"; // Abbruch mit detailierter Fehlermeldung
                 assert(strlen(argv[i]) <= 100);
              }
              if(modus == Ausgabemodus::QUIET) { // Abbruch ohne Fehlermeldung
                exit(-1);
              }
            }

            if(!validCwStr(argv[i])) { // Pruefung auf unerlaubte Zeichen im String
              if(modus == Ausgabemodus::NORMAL || modus == Ausgabemodus::VERBOSE) {
                cerr << "Text enthält undefinierte Zeichen \n"; // Abbruch mit detailierter Fehlermeldung
                assert(validCwStr(argv[i]));
              }
              if(modus == Ausgabemodus::QUIET) { // Abbruch ohne Fehlermeldung
                exit(-1);
              }
            }
            else { // kopiere Sendetext
              int index_argv {0};
              int index_sendeStr {0}; // Quell- und Zielstring beginnen bei 0
              while(index_argv < strlen(argv[i])) {
                sendeStr[index_sendeStr] = argv[i][index_argv];
                ++index_argv;
                ++index_sendeStr;
              }
              sendeStr[index_sendeStr] = '\0';
            }
          }
          break;
        default:
          break;
      } 
    }
    ++i;
  }
  
  // Falls keine Optionen für den Speed angegeben wurden wird die Standartbelegung definiert:
  if(cwSpeed == -1) cwSpeed = 20;

  // Abbruch falls Pin ausserhalb Menge gewaehlt
  if(modus == Ausgabemodus::NORMAL || modus == Ausgabemodus::VERBOSE) {
    if(cwPin < MIN_PIN || cwPin > MAX_PIN) {
      cerr << "nicht zulaessiger Pin gewaehlt\n";
      assert(cwPin >= MIN_PIN && cwPin <= MAX_PIN); // Abbruch mit detailierter Fehlermeldung
    }
  }
  if(modus == Ausgabemodus::QUIET) {
    if(cwPin < MIN_PIN || cwPin > MAX_PIN) {
      exit(-1); // Abbruch ohne Fehlermeldung
    }
  }


  // Protokollausgaben des Programms im -v Mode
  if(modus == Ausgabemodus::VERBOSE) {
    cout << "Pin für CW-Ausgabe: " << cwPin << " nach" << '\n';
      if(pinDeclarationMode == Pindecl::WIRINGPI) cout << "Schema fuer die WiringPi-Bibliothek\n";
      else cout << "GPIO - Schema des Raspberry\n";
    cout << "Gewählte Ausgabegeschwindigkeit: " << cwSpeed << " ";
      if(speedDeclarationMode == Speeddecl::WPM) cout << "WPM\n";
      else cout << "BPM\n";
    cout << "Text: \"" << sendeStr << "\" wird gegeben\n";
  }
  
  // Aufruf der Morse-Bibliothek
  char pinDecl;
  if(pinDeclarationMode == Pindecl::WIRINGPI) pinDecl = 'w';
  else pinDecl = 'g';

  char speedDecl;
  if(speedDeclarationMode == Speeddecl::WPM) speedDecl = 'W';
  else speedDecl = 'B';

  // Call der Morseklasse
  Morseclass cw(pinDecl, cwPin, speedDecl, cwSpeed, sendeStr);
  cw.cwRun();

  // restliche Ausgaben auf die Standardausgabe
  if(modus == Ausgabemodus::VERBOSE) {
    cout << "Text mit " << cwSpeed  << " ";
    if(speedDeclarationMode == Speeddecl::WPM) cout << "WPM ";
    else cout << "BPM ";
    cout << " ausgegeben\n";
  } 
  return 0;
}


// Funktionsdefinitionen


bool nurZiffernImString(const char* str) { // Prueft ab Index 0 - Rückgabewert false wenn noch andere Zeichen ausser Ziffern im String sind
  bool zifferEnthalten {true};
  int laenge = strlen(str);
  int index {0};
  while(index < laenge) {
    if(str[index] < '0' || str[index] > '9') zifferEnthalten = false;
    ++index;
  }
  return zifferEnthalten;
}


bool nurZiffernImTeilString(const char* str) { // Prueft ab Index 2 - Rückgabewert false wenn noch andere Zeichen ausser Ziffern im String sind
  bool zifferEnthalten {true};
  int laenge = strlen(str);
  int index {2};
  while(index < laenge) {
    if(str[index] < '0' || str[index] > '9') zifferEnthalten = false;
    ++index;
  }
  return zifferEnthalten;
}


bool validCwStr(const char* str) { // Prueft auf erlaubte Zeichen die im CW-Vorrat enthalten sind
  bool validSign {true};
  int laenge = strlen(str);
  int index {0};
  while(index < laenge) {
    if((str[index] < '0' || str[index] > '9') && (str[index] < 'a' || str[index] > 'z') && (str[index] < 'A' || str[index] > 'Z') && (str[index] != '?') && (str[index] != '/') && (str[index] != '=') && (str[index] != '-') && (str[index] != '.') && (str[index] != ',') && (str[index] != '+') && (str[index] != ' ')) validSign = false;
    ++index;
  }
  return validSign;
}

void showHelpPage() {
  cout << "Helppage cwout --- Version 1.0 --- Harry DJ5MY, May 2020" << '\n';
  cout << "*\n" << "*\n";
  cout << "Dieses Programm gibt einen beliebigen Text bis zu 100 Zeichen \n";
  cout << "in Morsetelegrafie auf einem GPIO-Pin aus\n";
  cout << '\n' << '\n';
  cout << "cwout [Options] [string] " << '\n' << '\n';
  cout << "Options: \n\n";
  cout << "   -w setzt Pin gemaess der WiringPi-Bibliothek als Output\n\n";
  cout << "   -g setzt Pin gemaess der GPIO-Bibliothek als Output\n\n";
  cout << "   -W setzt die Gebegeschwindigkeit in WPM \n\n";
  cout << "   -B setzt die Gebegeschwindigkeit in BPM \n\n";
  cout << "   -s der zu sendende CW-Text als String\n\n";
  cout << "   -v Verbose Mode: ausfuehliche Ausgabe des Programms auf der\n";
  cout << "      Standardausgabe\n\n";
  cout << "   -q Quiet Mode: keinerlei Ausgabe auf die Standardausgabe\n\n";
  cout << "      zum Einsatz in Skripten\n\n";
  cout << "   --help gibt diese Hilfeseite aus und beendet das Programm\n\n";
  cout << "    Werte werden direkt oder mit Whitespace nach den Optionen\n";
  cout << "    -w -g -W -B -s aufgefuehrt.\n\n";
  cout << "*****************************************************************\n\n";
  cout << "Beispiel: ./cwout -w28 -W15 -q -s \"Beispieltext\" setzt den \n";
  cout << "Pin 28 nach WiringPi Bibliothek als Ausgang. Der Text wird mit \n";
  cout << "15 WPM ausgegeben. Das Programm gibt keine Ausgaben auf der Stand-\n";
  cout << "ardausgabe aus und beendet sich wenn der Text gegeben wurde.\n";
  exit(0);
}

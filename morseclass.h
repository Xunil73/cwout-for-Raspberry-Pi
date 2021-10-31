#ifndef MORSECLASS_H
#define MORSECLASS_H


#include "wiringPi.h"

class Morseclass  {
  public:
    Morseclass(char pmode, int pin, char smode, int speed, const char* text); 
    // pmode: 'w'-Pinbelegung nach WiringPi-Schema, 'g'-nach GPIO-Schema
    // pin: Nummer des Ausgabepins
    // smode: 'W'-Geschwindigkeit in WPM, 'B'-Geschwindigkeit in BPM
    // speed: Gebegeschwindigkeit
    // text: String der in CW ausgegeben wird

    void cwRun(); // gibt Text auf dem Ausgang aus

  private:
    int cwPin; // speichert den Ausgabepin

    int cwSpeed; // speichert die Gebegeschwindigkeit

    int dotLen; // Länge eines Dot in Millisekunden

    char sendeStr[101]; // speichert den zu sendenden Text

    void dot(); // schreibt einen Punkt und die Pause auf den Ausgang

    void dash(); // schreibt Strich und einen Punkt auf den Ausgang

    void pause(); // wartet Pause zwischen zwei Zeichen ab

    int findeZeichenInTabelle(const char einZeichen); // sucht den CW-
    // Code passend zum Buchstaben aus der Tabelle
    
    const int DIFF_ZAHL = '0'; // Differenz Zeichen '0' zur Zahl 0

    const int DIFF_KLEIN_GROSS = 'a' - 'A'; // Diff Klein-/Grossbuchstaben

    const int DIFF_GROSS_TABELLE = 55; // Diff ASCII-Grossbuchstabe auf meine Zeichentabelle
};


#endif // Morseclass.h

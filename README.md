# cwout for Raspberry-Pi
a small tool to send morse code to an GPIO-Pin of any Raspberry Pi 
via the command line.
The code is written in C++ using the WiringPi-Lib.

This is the first raw version of the code, but it seems to work fine.

Feel free to try it out, just save the three files 
cwout.cpp - morseclass.h - morseclass.cpp in a directory of your choice
and compile it with

    g++ -o cwout -l wiringPi cwout.cpp morseclass.h morseclass.cpp

After the compilation finished you can send a morse text to the GPIO-Pin
of your choice by typing f.e.:

./cwout -w28 -W15 -q -s "this text is written to an GPIO-Pin as morse code"

This sends the text behind the -s flag to the pin number 28 (-w = wiringPi library),
with a speed of 15 words per minute. If you prefer using the other counting method of
the pins according the GPIO library you can choose the pin also with -g as flag.
The -q (quiet) flag means that no text output at standard output happens while the program
is running. The other option is the -v (verbose) switch who gives you a detailed information 
what happens at the standard output.

You can modify, improve, change the program as you want. If you make improvements 
it would be nice if you make a branch and share the changes via commits so that 
the program can be improved for all interested people.

i do not warrant any damage to software or hardware including your data of your raspberry pi 
by using the software. I make the software available here for everyone in the hope that it is useful.

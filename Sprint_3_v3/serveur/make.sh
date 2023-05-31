#!/bin/sh

gcc -c global.c -o global.o -pthread
gcc -c funcServ.c -o funcServ.o -pthread
gcc -c serveur.c -o serveur.o -pthread
gcc global.o funcServ.o serveur.o -o server -pthread

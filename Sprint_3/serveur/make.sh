#!/bin/sh

gcc -c global.c -o global.o
gcc -c funcServ.c -o funcServ.o
gcc -c serveur.c -o serveur.o
gcc global.o funcServ.o serveur.o -o serveur
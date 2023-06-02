#!/bin/sh

# Créer le répertoire "bin" s'il n'existe pas déjà
mkdir -p bin

gcc -c src/global.c -o bin/global.o -pthread
gcc -c src/funcServ.c -o bin/funcServ.o -pthread
gcc -c src/serveur.c -o bin/serveur.o -pthread
gcc bin/global.o bin/funcServ.o bin/serveur.o -o server -pthread

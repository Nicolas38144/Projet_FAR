#!/bin/sh

# Créer le répertoire "bin" s'il n'existe pas déjà
mkdir -p bin

gcc -c src/global.c -o bin/global.o -pthread
gcc -c src/sendFileFunc.c -o bin/sendFileFunc.o -pthread
gcc -c src/receivFileFunc.c -o bin/receivFileFunc.o -pthread
gcc -c src/verifFunc.c -o bin/verifFunc.o -pthread
gcc -c src/messageFunc.c -o bin/messageFunc.o -pthread
gcc -c src/client.c -o bin/client.o -pthread
gcc bin/global.o bin/sendFileFunc.o bin/receivFileFunc.o bin/verifFunc.o bin/messageFunc.o bin/client.o -o client -pthread
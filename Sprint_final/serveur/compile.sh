#!/bin/sh
mkdir -p bin1
gcc -c src/global.c -o bin1/global.o -pthread
gcc -c src/messageFunc.c -o bin1/messageFunc.o -pthread
gcc -c src/receivFileFunc.c -o bin1/receivFileFunc.o -pthread
gcc -c src/sendFileFunc.c -o bin1/sendFileFunc.o -pthread
gcc -c src/channelFunc.c -o bin1/channelFunc.o -pthread
gcc -c src/funcServ.c -o bin1/funcServ.o -pthread
gcc -c src/serveur.c -o bin1/serveur.o -pthread
gcc bin1/global.o bin1/messageFunc.o bin1/receivFileFunc.o bin1/sendFileFunc.o bin1/channelFunc.o bin1/funcServ.o bin1/serveur.o -o server -pthread
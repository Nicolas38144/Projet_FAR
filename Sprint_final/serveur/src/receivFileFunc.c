#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <dirent.h>

#include "../include/receivFileFunc.h"
#include "../include/global.h"
#include "../include/funcServ.h"
#include "../include/messageFunc.h"


/*------------------------------------------------FONCTION DE RÉCEPTION D'UN FICHIER------------------------------------------------*/
void * receiveFile_th(void * fileNameParam){

    /*Accepter une connexion*/
    long dSCFile = acceptConnection(dSFileReceiv);
    char * fileName = (char *)fileNameParam;


    /*Création du chemin pour enregister le fichier*/ 
    char * pathToFile = (char *) malloc(sizeof(char)*130);
    strcpy(pathToFile,"./FileServeur/");
    strcat(pathToFile,fileName);
    /*Création du fichier et du buffer pour recevoir les données*/
    char buffer[1024];
    FILE * f = fopen(pathToFile, "wb");
    if((long int)f ==(long int) -1){
        printf("erreur au open");
        exit(1);
    }

    int nbBytesRead = receivingInt(dSCFile);
    /*Reception*/
    while(nbBytesRead > 0) {
        int res = recv(dSCFile, buffer, nbBytesRead, 0);
        if (res <= 0) {
            printf("Erreur de réception de la data");
            break;
        }
        fwrite(buffer, sizeof(char), nbBytesRead, f);
        nbBytesRead = receivingInt(dSCFile);
        if (nbBytesRead < 0){ 
            printf("Erreur de reception de la taille");
            break;
        }
    }
    printf("\n**Fichier envoyé**\n");
    fclose(f);
    close(dSCFile);
    return NULL;
}



void receiveFile(int dSC) {
    /*Reception du nom du fichier à recevoir*/
    char * fileName = (char *) malloc(sizeof(char)*30);
    receiveMsg(dSC, fileName, sizeof(char)*30);

    printf("\nNom du fichier à recevoir: %s \n", fileName);
    if (strcmp(fileName,"error") == 0) {
        printf("Nom de fichier incorrect\n");
    }
    else {
        printf("%s", fileName);
        fileName = strtok(fileName, "\n");
        /*Création du thread pour gérer la reception du fichier*/
        pthread_t threadFile;
        int thread = pthread_create(&threadFile, NULL, receiveFile_th, (void *)fileName);
        if(thread==-1){
            perror("error thread");
        }
    }  
}
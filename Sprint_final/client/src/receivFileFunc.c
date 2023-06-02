#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>

#include "../include/receivFileFunc.h"
#include "../include/verifFunc.h"
#include "../include/messageFunc.h"
#include "../include/global.h"


/*------------------------------------------------FONCTION DE RÉCEPTION D'UN FICHIER------------------------------------------------*/
void * receivingFile_th(void * dSFile) {

    int dS = *(int *)dSFile;
    /*char * buffer = (char *) malloc(sizeof(char)*16);
    printf("buffer : %s\n",buffer);*/

    /*Récupérer le nom du fichier*/ 
    char * fileServerName = (char *) malloc(sizeof(char)*32);
    /*receiveMsg(dS, fileServerName, sizeof(char)*32);*/
    strcpy(fileServerName, ultimate);
    printf("fileServerName : %s\n", fileServerName);
    if (strlen(fileServerName) == 0) {
        printf("Erreur lors de l'ouverture du fichier dans le serveur\n");
    }
    else {
        /*Création du chemin pour enregister le fichier*/ 
        char * pathToFile = (char *) malloc(sizeof(char)*130);
        strcpy(pathToFile,"./FileClientToSend/");
        strcat(pathToFile,fileServerName);
        char *token;
        token = strtok(pathToFile, "\n");
        while (token != NULL) {
            token = strtok(NULL, "\n");
        }

        /*Création du fichier et du buffer pour recevoir les données*/
        char buffer[1024];
        FILE * f = fopen(pathToFile, "wb");
        if((long int)f ==(long int) -1){
            printf("erreur au open");
            exit(1);
        }
        int nbBytesRead = receivingInt(dS);
        printf("%d\n", nbBytesRead);
        /*Reception*/
        while(nbBytesRead > 0) {
            int res = recv(dS, buffer, nbBytesRead, 0);
            if (res <= 0) {
                printf("Erreur de réception de la data");
                break;
            }
            fwrite(buffer, sizeof(char), nbBytesRead, f);
            nbBytesRead = receivingInt(dS);
            if (nbBytesRead < 0){ 
                printf("Erreur de reception de la taille");
                break;
            }
        }
        printf("\n**Fichier reçu**\n\n");
        free(pathToFile);
        fclose(f);
    }
    free(fileServerName);
    close(dS);
    return NULL;
}


void receivingFile(int dSC) {

    printf("Liste des fichiers dans le server : \n");
    usleep(400000);
      
    /* Préparer la socket */
    int dSFile = socket(PF_INET, SOCK_STREAM, 0);
    if (dSFile == -1) {
        perror("Erreur lors de la création de la socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in aS;
    aS.sin_family = AF_INET;
    inet_pton(AF_INET, arg1, &(aS.sin_addr));
    aS.sin_port = htons(atoi(arg2)+2);
    /* Demander une connexion */
    socklen_t lgA = sizeof(struct sockaddr_in);
    int connectR = connect(dSFile, (struct sockaddr *) &aS, lgA);
    if (connectR == -1){
        perror("Error when connect\n");
        exit(-1);
    }

    printf("\nNom du fichier à recevoir : \n");
    char * fileName = (char *) malloc(sizeof(char)*20);
    scanf("%s", fileName);
    getchar();
    if (strcmp("X", fileName) == 0) {
        printf("Retour sur la discussion\n\n");
    }
    else {
        sendMsg(dSC, fileName);
        /* Réponse du serveur sur l'existance du fichier */
        int ok = receivingInt(dSC);
        /*printf("ok :  %d\n",ok);*/
        if (ok == 1) {
            /* Création du thread pour gérer la reception du fichier */
            strcpy(ultimate, fileName);
            pthread_t threadFile;
            int *dSFilePtr = malloc(sizeof(int));
            *dSFilePtr = dSFile;
            int thread = pthread_create(&threadFile, NULL, receivingFile_th, (void *)dSFilePtr);
            if (thread == -1) {
                perror("Error thread \n");
            }
        }
        else {
            printf("\nFichier inexistant dans le serveur.\nRetour sur la discussion\n\n");
        }
    }  
    free(fileName);
}
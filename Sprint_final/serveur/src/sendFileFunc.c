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

#include "../include/sendFileFunc.h"
#include "../include/global.h"
#include "../include/funcServ.h"
#include "../include/messageFunc.h"


/*------------------------------------------------FONCTION D'ENVOI D'UN FICHIER------------------------------------------------*/
void * sendFile_th(void * file_param) {

    /*Accepter une connexion*/
    long dSCFile = acceptConnection(dSFileSend);
    char * file = (char *)file_param;
    printf("Nom du fichier que l'utilisateur veut -> :\n%s\n", file);
    /*sendPersonnalMsg(dSCFile, file, 0);*/
  
    /*Création du chemin pour trouver le fichier*/
    char * pathToFile = (char *) malloc(sizeof(char)*130);
    strcpy(pathToFile,"./FileServeur/");
    strcat(pathToFile,file);
    char *token;
    token = strtok(pathToFile, "\n");
    while (token != NULL) {
        token = strtok(NULL, "\n");
    }

    printf("pathToFile : %s\n", pathToFile);

    /*Ouverture et envoi du fichier*/
    FILE * f = NULL;
    f = fopen(pathToFile,"rb");
    if (f == NULL) { 
        sendPersonnalMsg(dSCFile, "Erreur lors de l'ouverture du fichier dans le serveur\n", 0);
        printf("Erreur! Fichier inconnu\n\n"); 
    }
    else {
        /*Envoi du nom du fichier au client*/
        /*sendPersonnalMsg(dSCFile, file, 0);*/
        /*send(dSCFile, &blocLenth, sizeof(int), 0);*/
        char data[1024] = "";
        int blocLenth = 1;
        while(blocLenth != 0) {
            blocLenth = fread(data, sizeof(char), 1024, f);
            if (blocLenth == -1) {
                perror("Error in reading file\n");
                exit(EXIT_FAILURE);
            }
            sendingInt(dSCFile, blocLenth);
            if(blocLenth != 0){
                if (send(dSCFile, data, blocLenth, 0) == -1) {
                    perror("[-]Error in sending file.\n");
                    exit(EXIT_FAILURE);
                }
            }
            printf("blocLenth : %d\n",blocLenth);
        }
        printf("\n** Fichier envoyé **\n\n");
        fclose(f);        
    }
    free(pathToFile);
    free(file);
    close(dSCFile);
    return NULL;
}


void sendFile(int dSC) {
    /*Envoyer les noms des fichiers du dossier*/
    DIR *dir;
    struct dirent *ent;
    char * buffer = (char *) malloc(sizeof(char)*1);
    int buffer_pos = 0;
    int buffer_size = 0;

    // Ouvrir le répertoire
    dir = opendir("./FileServeur/");
    if (dir == NULL) {
        perror("Erreur lors de l'ouverture du repertoire");
        exit(0);
    }
    /* Lire les fichiers du répertoire */
    while ((ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            continue;
        }
        int filename_len = strlen(ent->d_name);
        if (buffer_pos + filename_len + 1 >= buffer_size) {
            buffer_size = buffer_pos + filename_len + 1;
            buffer = (char *)realloc(buffer, buffer_size);
            if (buffer == NULL) {
                perror("Erreur lors de l'allocation de mémoire");
                exit(0);
            }
        }
        strncpy(buffer + buffer_pos, ent->d_name, filename_len);
        buffer_pos += filename_len;
        buffer[buffer_pos++] = '\n'; // Ajouter un espace entre les noms de fichiers
    }
    closedir(dir);
    buffer[buffer_pos] = '\0';

    if (strcmp(buffer,"") == 0) {
        printf("Aucun fichier n'est présent dans le serveur\n");
        sendPersonnalMsg(dSC, "X", 0);
        free(buffer);
    }
    else {
        printf("Taille du buffer : %ld\n", strlen(buffer));
        /* Envoie la liste das fichiers du serveur */
        sendPersonnalMsg(dSC, buffer, 0);
        
        char * f1 = (char *) malloc(sizeof(char)*32);
        receiveMsg(dSC, f1, sizeof(char)*32);
        strcat(f1, "\n");
        int okk = 1;
        if (strstr(buffer, f1) != NULL) {
            /*sendPersonnalMsg(dSC, "ok\n", 0);*/
            sendingInt(dSC, okk);
            pthread_t threadFile;
            int thread=pthread_create(&threadFile,NULL,sendFile_th,(void*)f1);
            if(thread == -1) {
                perror("Error thread\n");
            }
        }
        else {
            /*sendPersonnalMsg(dSC, "no", 0);*/
            okk = 0;
            sendingInt(dSC, okk);
        }
        /*free(f1);*/
    }
    free(buffer);
}
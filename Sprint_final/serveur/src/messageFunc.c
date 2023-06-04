#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#include "../include/global.h"
#include "../include/messageFunc.h"


/*
*   receiveMsg(int dS, char * rep, ssize_t size) :
*       Receptionne un message d'une socket 
*       int dS : la socket
*       char * msg : message à recevoir
*       ssize_t size : taille maximum du message à recevoir
*/
void receiveMsg(int dS, char * msg, ssize_t size) {
    int receiveResult = recv(dS, msg, size, 0);
    if (receiveResult == -1) { 
        perror("Erreur lors de la reception du message");
        exit(EXIT_FAILURE);
    }
}



/*
*   sendMsg(int dS, const char * message) :
*       Envoi un message à toutes les sockets présentent dans le tableau des clients
*       int dS : expéditeur du message
*       const char * message : message à envoyer
*/
void sendMsg(int dS, const char * message) {
    int i;
    for (i=0; i<MAX_CLIENT; i++) {
        /*On envoie à tous les clients excepté celui qui a envoyé le message*/
        if (tabClient[i].connected==1 && dS != tabClient[i].dSC) {
            int sendResult = send(tabClient[i].dSC, message, strlen(message)+1, 0);
            /*Gestion s'il y a une erreur*/
            if (sendResult == -1) {
                perror("Erreur lors de l'envoi du message");
                exit(EXIT_FAILURE);
            }
        }
    }
}



/*
*   sendPersonnalMsg(int dS, const char * message) :
*       Envoi un message uniquement à l'expéditeur du message
*       int dS : expéditeur du message
*       const char * message : message à envoyer
*/
void sendPersonnalMsg(int dS, char * message, int condition) {
    /*Envoi un message uniquement à l'expéditeur du message*/
    int sendResult = send(dS, message, strlen(message)+1, 0);
    /*Gestion s'il y a une erreur*/
    if (sendResult == -1) {
        perror("Erreur lors de l'envoi du message");
        exit(EXIT_FAILURE);
    }
    if (condition == 1) {
        free(message);
    }
}



/*
*   void sendingPrivate(int numClient, char *msgReceived) :
*       envoie un message privé au client
*/
void sendingPrivate(int numClient, char *msgReceived) {
    /*Récupération du nom du destinataire*/
    char *nameDest = strtok(msgReceived, " ");
    nameDest++;
    /* Suppression du caractère @*/
    /*Récupération du message*/
    char *token = strtok(NULL, "");
    char * msgToSend = (char *) malloc(sizeof(char)*200);
    char * nameSender = tabClient[numClient].name; 
    strcat(msgToSend, "[Message Privé] ");
    strcat(msgToSend, nameSender);
    strcat(msgToSend, " : ");
    strcat(msgToSend, token);
    /*Recherche du destinataire dans le tableau des clients connectés*/
    int destFound = 0;
    int i;
    for (i = 0; i < MAX_CLIENT; i++) {
        if (tabClient[i].connected == 1 && strcmp(nameDest, tabClient[i].name) == 0) {
            /*Envoi du message au destinataire*/
            int sendResult = send(tabClient[i].dSC, msgToSend, strlen(msgToSend) + 1, 0);
            /*Gestion s'il y a une erreur*/
            if (sendResult == -1) {
                perror("Erreur lors de l'envoi du message");
                exit(EXIT_FAILURE);
            } 
            destFound = 1;
            break;
        }
    }
    /*Destinataire non trouvé*/
    if (destFound == 0) {
        printf("Le destinataire n'a pas été trouvé.\n");
    }
}



int receivingInt(long dS) {
    int number;
    if(recv(dS, &number, sizeof(int), 0) == -1){ /*vérification de la valeur de retour*/
        perror("erreur au recv d'un int");
        exit(-1);
    }
    return number;
}



void sendingInt(int dS, int number) {
    /*Envoi de l'entier 'number' à la socket 'dS'*/
    int sendR = send(dS, &number, sizeof(int), 0);
    if (sendR == -1){ /*vérification de la valeur de retour*/
        perror("erreur au send");
        exit(-1);
    }
}
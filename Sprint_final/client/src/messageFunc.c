#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>

#include "../include/messageFunc.h"
#include "../include/verifFunc.h"
#include "../include/global.h"
#include "../include/sendFileFunc.h"
#include "../include/receivFileFunc.h"



/*----------------------------------------------FONCTION D'ENVOI D'UN MESSAGE---------------------------------------------*/

/*
 *   sendMsg(int dS, const char * message) :
 *       Message envoyé à une socket et teste la conformité
 *       int dS : expéditeur du message
 *       const char * message : message à envoyer
 */
void sendMsg(int dS, const char *msg)
{
    /*Envoi les données à la socket*/
    ssize_t num_bytes = send(dS, msg, strlen(msg) + 1, 0);
    if (num_bytes < 0) {
        perror("Erreur dans l'envoi des données");
        exit(EXIT_FAILURE);
    }
}



/*
 *   sending_th(void * dSparam) :
 *       Envoie des messages à travers une connexion de socket, jusqu'à ce que la variable "isFinished" soit vraie.
 *       dSparam : descripteur de socket. Il est ensuite convertit en entier et est utilisé pour envoyer des messages.
 */
void *sending_th(void *dSparam)
{
    int dS = (long)dSparam;
    save_dS = dS;
    while (!isFinished)
    {

        /*Saisie du message au clavier*/
        char *m = (char *)malloc(sizeof(char) * 100);
        fgets(m, 100, stdin);

        /*On vérifie si le client veut quitter la communication*/
        isFinished = checkLogOut(m);
        /*Envoi*/
        sendMsg(dS, m);

        /*Si on envoie un fichier*/
        if (isSendingFile(m)) {
            sendingFile(dS);
        }

        /*Si on veut récupérer un fichier*/
        if (checkDownload(m)) {
            receivingFile(dS);
        }
        free(m);
    }
    close(dS);
    return NULL;
}





/*------------------------------------------------FONCTION DE RÉCEPTION D'UN MESSAGE------------------------------------------------*/

/*
 *   receiveMsg(int dS, char * buffer, ssize_t size) :
 *       Réceptionne un message envoyé à une socket et teste sa conformité
 *       int dS : la socket
 *       char * msg : message à recevoir
 *       ssize_t size : taille maximum du message à recevoir
 */
void receiveMsg(int dS, char *buffer, ssize_t size)
{
    ssize_t num_bytes = recv(dS, buffer, size, 0); /*Reçoit les données de la part du socket*/
    if (num_bytes < 0)
    {
        perror("Erreur dans la réception de la donnée");
        exit(EXIT_FAILURE);
    }
    buffer[num_bytes] = '\0';
}



/*
 *   receiveMsg(int dS, char * buffer, ssize_t size) :
 *       Fonction pour le thread de reception
 *       dSparam : Descripteur de socket. Il est ensuite convertit en entier et est utilisé pour recevoir des messages.
 */
void *receiving_th(void *dSparam) {
    int dS = (int)(intptr_t)dSparam;
    char r[1024]; /*Tampon de réception*/
    while (!isFinished)
    {
        /*Réception du message dans le tampon de réception*/
        receiveMsg(dS, r, sizeof(r));
        printf("%s", r);
    }
    close(dS);
    return NULL;
}
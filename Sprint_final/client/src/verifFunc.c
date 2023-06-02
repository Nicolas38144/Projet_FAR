#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>

#include "../include/verifFunc.h"
#include "../include/global.h"
#include "../include/messageFunc.h"



/*----------------------------------------------FONCTION DE VERIFICATION---------------------------------------------*/

/*
 *   checkLogOut(char * msg) :
 *       Check si le client veut se déconnecter, il enverra au serveur le mot "fin"
 *       char * msg : Le message du client
 */
int checkLogOut(char *msg) {
    if (strcmp(msg, "/logout\n") == 0) {
        strcpy(msg, "a quitté la conversation\n");
        return 1;
    }
    return 0;
}



void handle_sigint(int sig) {
    sendMsg(save_dS, "a quitté la conversation\n");
}



/*
 *   isSendingFile(char * msg)
 *       Check si le client souhaite envoyer un fichier
 *        char * msg : message du client à vérifier
 *        Retour : 1 si le client veut envoyer un fichier, 0 sinon
 */
int isSendingFile(char *msg) {
    if (strcmp(msg, "/upload\n") == 0) {
        return 1;
    }
    return 0;
}



/*
 *   checkUpload(char * msg)
 *       Check si le client souhaite télécharger un fichier
 *       char * msg : message du client à vérifier
 *       Retour : 1 si le client veut télécharger un fichier, 0 sinon
 */
int checkDownload(char *msg) {
    if (strcmp(msg, "/download\n") == 0) {
        return 1;
    }
    return 0;
}



void sendingInt(int dS, int number) {
    int sendR = send(dS, &number, sizeof(int), 0);
    if (sendR == -1){
        perror("erreur au send");
        exit(EXIT_FAILURE);
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
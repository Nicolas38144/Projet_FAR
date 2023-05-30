#ifndef GLOBALS_H
#define GLOBALS_H
#include <pthread.h>


#define MAX_CLIENT 3


/*
*   Structure Client qui regroupe les informations du client
*   connected   : 0 si le client s'est déconnecté, 1 si connecté au serveur
*   name        : Nom du client
*   dSC         : Socket du client
*/
typedef struct Client Client;
struct Client{
    int connected;
    char * name;
    long dSC;
};


/*
*   tabClient           : Tableau contenant les clients connectés
*   tabThread           : Tableau contenant les threads utilisés pour le traitement de chaque client
*   nbConnectedClient   : Nombre de clients actuellement connectés
*/
extern Client tabClient[MAX_CLIENT];
extern pthread_t tabThread[MAX_CLIENT];
extern int nbConnectedClient;
extern int tabThreadToKill[MAX_CLIENT];
extern pthread_mutex_t lock;
extern long dSFileReceiv;
extern long dSFileSend;
extern char * arg1;
#endif /* GLOBALS_H */


/* 
Pour compiler le tout :
taper bash make.sh
*/
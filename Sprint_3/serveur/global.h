#ifndef GLOBALS_H
#define GLOBALS_H
#include <pthread.h>


#define MAX_CLIENT 3
#define NB_ROOMS 5


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
    int idRoom;
};


/*Structure d'un canal de discussion */

typedef struct Room Room;
struct Room{
    int id;
    char * name;
    char * descr;
    int members[MAX_CLIENT];
    int created; /*Permet de savoir s'il faut afficher le salon au client, s'il est créé ou non*/
};

/*
*   tabClient           : Tableau contenant les clients connectés
*   tabThread           : Tableau contenant les threads utilisés pour le traitement de chaque client
*   nbConnectedClient   : Nombre de clients actuellement connectés
*/
extern Client tabClient[MAX_CLIENT];
extern Room rooms[NB_ROOMS];
extern pthread_t tabThread[MAX_CLIENT];
extern int nbConnectedClient;
extern int tabThreadToKill[MAX_CLIENT];
extern int nbThreadToKill;
extern pthread_mutex_t lock;
extern long dSFile;
extern char * arg1;
#endif /* GLOBALS_H */


/* 
Pour compiler le tout :
taper bash make.sh
*/



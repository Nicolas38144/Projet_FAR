#ifndef GLOBALS_H
#define GLOBALS_H
#include <pthread.h>

#define MAX_CHANNELS 10
#define MAX_CLIENT 3
#define MAX_MEMBERS 50

/*
*   Structure Client qui regroupe les informations du client
*   connected   : 0 si le client s'est déconnecté, 1 si connecté au serveur
*   name        : Nom du client
*   dSC         : Socket du client
*/
typedef struct Client Client;
struct Client{
    int idRoom;
    int connected;
    char * name;
    long dSC;
};


/*
*   tabClient           : Tableau contenant les clients connectés
*   tabThread           : Tableau contenant les threads utilisés pour le traitement de chaque client
*   nbConnectedClient   : Nombre de clients actuellement connectés
*/
typedef struct Channel Channel;
struct Channel {
  int Idchannel;
  char* name;
  int socket_fd;
  char descr[100];
  int memberCount;
  int members[MAX_MEMBERS];
  int created; //vérifie si c'est créé
};

/*
*   Idchannel           : numéro d'identifiant du channel
*   name                : nom du channel
*   socket_fd           : possible amélioration futures
*   descr[100]          : description du commentaire
*   memberCount         : nombre de personnes sur un channel
*   members[Max_Members]: futur utilisation pour stoquer les membres par id
*   created             : permet de dire que le channel est créé
*/

extern int channelCount;
extern Channel channels[MAX_CHANNELS];
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
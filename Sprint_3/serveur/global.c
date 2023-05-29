#include "global.h"

Client tabClient[MAX_CLIENT];
Room rooms[NB_ROOMS];
pthread_t tabThread[MAX_CLIENT];
int nbConnectedClient = 0;
int tabThreadToKill[MAX_CLIENT];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int nbThreadToKill;

extern pthread_mutex_t lock;

char * arg1 = "";
long dSFile = 0;
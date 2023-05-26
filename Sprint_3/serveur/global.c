#include "global.h"

Client tabClient[MAX_CLIENT];
Channel channelAvailable[NB_CHANNEL];
pthread_t tabThread[MAX_CLIENT];
int nbConnectedClient = 0;
int tabThreadToKill[MAX_CLIENT];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

char * arg1 = "";
long dSFile = 0;
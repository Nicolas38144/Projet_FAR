#include "global.h"

Client tabClient[MAX_CLIENT];
pthread_t tabThread[MAX_CLIENT];
int nbConnectedClient = 0;
int tabThreadToKill[MAX_CLIENT];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

char * arg1 = "";
long dSFileReceiv = 0;
long dSFileSend = 0;
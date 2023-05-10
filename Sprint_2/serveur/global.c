#include "global.h"

Client tabClient[MAX_CLIENT];
pthread_t tabThread[MAX_CLIENT];
int nbConnectedClient = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;;
#include "global.h"

Client tabClient[MAX_CLIENT];
pthread_t tabThread[MAX_CLIENT];
long nbConnectedClient = 0;
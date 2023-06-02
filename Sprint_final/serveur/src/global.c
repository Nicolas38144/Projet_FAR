#include "../include/global.h"

Channel channels[MAX_CHANNELS];
Client tabClient[MAX_CLIENT];
pthread_t tabThread[MAX_CLIENT];
int nbConnectedClient = 0;
int tabThreadToKill[MAX_CLIENT];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int channelCount = 0;

char * arg1 = "";
long dSFileReceiv = 0;
long dSFileSend = 0;
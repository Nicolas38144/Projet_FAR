#ifndef RECEIVFILEFUNC_H
#define RECEIVFILEFUNC_H

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

extern void * receiveFile_th(void * fileNameParam);
extern void receiveFile(int dSC);


#endif
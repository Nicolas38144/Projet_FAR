#ifndef SENDFILEFUNC_H
#define SENDFILEFUNC_H

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

extern void * sendFile_th(void * file_param);
extern void sendFile(int dSC);


#endif
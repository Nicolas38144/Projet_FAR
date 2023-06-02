#ifndef MESSAGEFUNC_H
#define MESSAGEFUNC_H

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

extern void sendMsg(int dS, const char *msg);
extern void *sending_th(void *dSparam);
extern void receiveMsg(int dS, char *buffer, ssize_t size);
extern void *receiving_th(void *dSparam);

#endif
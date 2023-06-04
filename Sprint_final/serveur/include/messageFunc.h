#ifndef MESSAGEFUNC_H
#define MESSAGEFUNC_H

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

extern void receiveMsg(int dS, char *msg, ssize_t size);
extern void sendMsg(int dS, const char *message);
extern void sendPersonnalMsg(int dS, char * message, int condition);
extern void sendingPrivate(int numClient, char *msgReceived);
extern int receivingInt(long dS);
extern void sendingInt(int dS, int number);

#endif
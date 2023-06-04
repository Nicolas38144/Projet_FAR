#ifndef FUNCSERV_H
#define FUNCSERV_H

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

extern int getNumClient();
extern int checkLogOut(char *msg);
extern void helpCommand(int dS);
extern int checkIsCommand(char *msg, int dS);
extern void killThread();
extern int isNameAvailable(char * name);
extern void addName(char *message, char *name);
extern void All(int numClient, char* message);
extern int createSocket(int port);
extern int acceptConnection(int dS);

#endif
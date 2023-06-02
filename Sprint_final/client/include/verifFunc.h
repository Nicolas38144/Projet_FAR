#ifndef VERIFFUNC_H
#define VERIFFUNC_H

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

extern int checkLogOut(char *msg);
extern void handle_sigint(int sig);
extern int isSendingFile(char *msg);
extern int checkDownload(char *msg);
extern void sendingInt(int dS, int number);
extern int receivingInt(long dS);


#endif
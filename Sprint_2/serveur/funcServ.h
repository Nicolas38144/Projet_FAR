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
extern void receiveMsg(int dS, char *msg, ssize_t size);
extern void sendMsg(int dS, const char *message);
extern int checkLogOut(char *msg);
extern void helpCommand();
extern int checkIsCommand(char *msg);

#endif /* FUNCSERV_H */
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
extern void sendPersonnalMsg(int dS, char * message);
extern int checkLogOut(char *msg);
extern void helpCommand(int dS);
extern int checkIsCommand(char *msg, int dS);
extern void killThread();
extern void sendingPrivate(int numClient, char *msgReceived);
extern int isSendingFille(char * msg);
extern void receiveFile(int dSC);
extern int isNameAvailable(char * name);
extern void send_integer(int dS, int number);
extern void All(int numClient, char* message);
/*extern void * sendingFile_th(void * fpParam);
extern void sendFile(int dS, FILE* fp);*/
extern int createSocket(int port);
extern int acceptConnection(int dS);
extern void * receiveFile_th( void * fileNameParam);
extern int receivingInt(long dS);

#endif /* FUNCSERV_H */
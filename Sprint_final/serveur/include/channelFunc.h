#ifndef CHANNELFUNC_H
#define CHANNELFUNC_H

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

extern void link_command(char* msg, long numcli);
extern void createChannel(const char* name);
extern void joinChannelByName(const char* name, int memberID);
extern int findChannelByName(const char* name);
extern void joinChannel(int channelIndex, int memberID);
extern void sendMessagetoRoom(int numCli, const char* message);


extern void changedesch(const char* element, int numcli);
extern void printnomch(int numcli);


#endif
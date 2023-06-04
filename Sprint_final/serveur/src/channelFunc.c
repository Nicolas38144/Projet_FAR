#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>

#include "../include/channelFunc.h"
#include "../include/global.h"


/* -----------Channel ----------*/

void link_command(char* msg, long numcli) {
    char *command = strtok(msg, " ");
    char *element = strtok(NULL, " ");

    if (command != NULL && element != NULL) {
        if (strcmp(command, "#create") == 0) {
            
            printf("Commande de création : %s\n", element);
            
          createChannel(element);
        } else if (strcmp(command, "#remove") == 0) {
          
            printf("Commande de suppression : %s\n", element);
        } else if (strcmp(command, "#join") == 0) {
            // Le command est "#join" et l'élément suivant est dans la variable "element"
            printf("Commande de jointure : %s\n", element);
          joinChannelByName(element, numcli);
        } else if (strcmp(command, "#changedesc") == 0){
            printf("Commande change nom channel\n");
          changedesch(element, numcli);
        
        } else if (strcmp(command, "#printnomch") == 0) {
          printf("Print nom channel\n");
          printnomch(numcli);
        }
        else{
                  printf("Commande non reconnue\n");
        }
        
    } else {
             printf("Format de commande incorrect\n");
          
    }
}




void changedesch(const char* element, int numcli) {
  int indexchannel = tabClient[numcli].idRoom;
  if (indexchannel != -1) {
    size_t newDescrLength = strlen(element);
    size_t maxDescrLength = sizeof(channels[indexchannel].descr) - 1;  // Taille maximale de la description moins 1 pour le caractère nul
    if (newDescrLength > maxDescrLength) {
      printf("La nouvelle description dépasse la taille maximale autorisée.\n");
      return;
    }
    strcpy(channels[indexchannel].descr, element);
    printf("La description du canal '%s' a été modifiée avec succès.\n", element);
  } else {
    printf("Le canal '%s' n'a pas été trouvé.\n", element);
  }
}




void printnomch(int numcli) {
  char *msgtosend = (char *)malloc(sizeof(char) * 200);
  strcpy(msgtosend, "------Channel------\n");

  for (int i = 0; i < channelCount; i++) {
    strcat(msgtosend, channels[i].name);
    strcat(msgtosend, " / ");
  }

  send(tabClient[numcli].dSC, msgtosend, strlen(msgtosend), 0);

  free(msgtosend);
}




void createChannel(const char* name) {
    if (channelCount >= MAX_CHANNELS) {
        printf("Le nombre maximum de canaux a été atteint.\n");
        return;
    }

    // Vérifier si le nom du canal est unique
    for (int i = 0; i < channelCount; i++) {
        if (strcmp(channels[i].name, name) == 0) {
            printf("Un canal avec le même nom existe déjà.\n");
            return;
        }
    }

    channels[channelCount].name = (char *)malloc(sizeof(char)*100);
    strncpy(channels[channelCount].name, name,sizeof( channels[channelCount].name));
    channels[channelCount].Idchannel = channelCount;
    channels[channelCount].created = 1;
    printf("channelCount : %d \n",channelCount);
   printf("channels[channelCount].Idchannel : %d \n",channels[channelCount].Idchannel);
    channelCount+= 1;
  /*for (int i =0 ; i< MAX_CLIENT;i++ ){
    printf("%d\n",channels[i].Idchannel);
  }*/

    printf("Le canal '%s' a été créé avec succès.\n", name);
}




void joinChannelByName(const char* name, int memberID) {
  printf("(joinChannel member ID :%d \n",memberID);
    int channelIndex = findChannelByName(name);
    printf("channel Index : %d \n",channelIndex);
    if (channelIndex == -1) {
        printf("Le canal '%s' n'existe pas.\n", name);
        return;
    }
    joinChannel(channelIndex, memberID);
}




int findChannelByName(const char* name) {
    for (int i = 0; i < channelCount; i++) {
        if (strcmp(channels[i].name, name) == 0) {
            return i;  // Retourne l'indice du canal si le nom correspond
        }
    }
    return -1;  // Retourne -1 si le canal n'a pas été trouvé
}




void joinChannel(int channelIndex, int memberID) {
    if (channelIndex < 0 || channelIndex >= channelCount) {
        printf("Index de canal invalide.\n");
        return;
    }

    Channel* channel = &channels[channelIndex];

    if (channel->memberCount >= MAX_MEMBERS) {
        printf("Le canal est plein, impossible de rejoindre.\n");
        return;
    }

    for (int i = 0; i < channel->memberCount; i++) {
        if (channel->members[i] == memberID) {
            printf("Vous êtes déjà membre de ce canal.\n");
            return;
        }
    }
    printf("%d \n",memberID);
    tabClient[memberID].idRoom = channelIndex;
    channels[channelIndex].memberCount += 1;
    printf("id de la room : %d \n",tabClient[memberID].idRoom);
    printf("Vous avez rejoint le canal '%s'.\n", channel->name);
}




void sendMessagetoRoom(int numCli, const char* message) {
    int ds = tabClient[numCli].dSC;
    int channelIndex = tabClient[numCli].idRoom;
    if (channelIndex < 0 || channelIndex >= channelCount) {
        printf("Index de canal invalide.\n");
        return;
    }
      printf("id channel : send msg room %d \n",channelIndex);
    int i;
    for (i = 0; i < MAX_CLIENT; i++) {
        if(tabClient[i].idRoom == channelIndex && ds != tabClient[i].dSC && tabClient[i].connected==1 ){
          printf("id de la room : %d\n",tabClient[i].idRoom);
          send(tabClient[i].dSC, message, strlen(message)+1, 0);
        }
    }
}
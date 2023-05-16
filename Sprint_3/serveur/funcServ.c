#include "global.h"
#include "funcServ.h"
#define MAX_CLIENT 5



/*
*   getNumClient() :
*       Permet de l'indice du premier emplacement disponible           
*       Return -1 si tout les emplecements sont occupés. 
*/
int getNumClient(){
    int indexTabClient = 0;
    while (indexTabClient < MAX_CLIENT && tabClient[indexTabClient].connected == 1){
        indexTabClient += 1;
    }
    if (indexTabClient < MAX_CLIENT) {
        return indexTabClient;
    }
    return -1;
}


/*
*   receiveMsg(int dS, char * rep, ssize_t size) :
*       Receptionne un message d'une socket 
*       int dS : la socket
*       char * msg : message à recevoir
*       ssize_t size : taille maximum du message à recevoir
*/
void receiveMsg(int dS, char * msg, ssize_t size) {
    int receiveResult = recv(dS, msg, size, 0);
    if (receiveResult == -1) { 
        perror("Erreur lors de la reception du message");
        exit(EXIT_FAILURE);
    }
}


/*
*   sendMsg(int dS, const char * message) :
*       Envoi un message à toutes les sockets présentent dans le tableau des clients
*       int dS : expéditeur du message
*       const char * message : message à envoyer
*/
void sendMsg(int dS, const char * message) {
    int i;
    for (i=0; i<MAX_CLIENT; i++) {
        /*On envoie à tous les clients excepté celui qui a envoyé le message*/
        if (tabClient[i].connected==1 && dS != tabClient[i].dSC) {
            int sendResult = send(tabClient[i].dSC, message, strlen(message)+1, 0);
            /*Gestion s'il y a une erreur*/
            if (sendResult == -1) {
                perror("Erreur lors de l'envoi du message");
                exit(EXIT_FAILURE);
            }
        }
    }
}


/*
*   sendPersonnalMsg(int dS, const char * message) :
*       Envoi un message uniquement à l'expéditeur du message
*       int dS : expéditeur du message
*       const char * message : message à envoyer
*/
void sendPersonnalMsg(int dS, char * message) {
    /*Envoi un message uniquement à l'expéditeur du message*/
    int sendResult = send(dS, message, strlen(message)+1, 0);
    printf("%d\n",sendResult);
    /*Gestion s'il y a une erreur*/
    if (sendResult == -1) {
        perror("Erreur lors de l'envoi du message");
        exit(EXIT_FAILURE);
    }
    free(message);
}


/*
*   checkLogOut(char * msg) :
*       Check si le client veut se déconnecter
*       char * msg : Le message du client
*/
int checkLogOut(char * msg) {
    if (strcmp(msg, "a quitté la conversation\n") ==0 ) {
        return 1;
    }
    return 0;
}


/*
*   helpCommand(char * msg) :
*       Check si le client fait la commande /help
*       char * msg : Le message du client
*/
void helpCommand(int dS) {
    FILE *f;
    char c;
    char * content = (char *) malloc(sizeof(char)*3000);
    f = fopen("commande.txt", "rt");
    int i = 0;
    while((c = fgetc(f)) != EOF) { 
        content[i] = c; 
        i++; 
    }
    fclose(f);
    content = strtok(content, "");
    sendPersonnalMsg(dS, content);
}

/* 
*   isSendingFille(char * msg) :
*       Regarde si le client a envoyé la commande /File
*/

int isSendingFille(char * msg){
    if (strcasecmp(msg, "/File") == 0){
        return 1;
    }
    return 0;
}

void receiveFile(int dSC) {
    /* Réception du nom du fichier à recevoir */
    char fileName[30];
    receiveMsg(dSC, fileName, sizeof(char) * 30);
    printf("\nNom du fichier à recevoir: %s \n", fileName);

    fileName[strcspn(fileName, "\n")] = '\0';  // Remove newline character from fileName

    /* Création du chemin pour accéder au fichier */
    char pathToFile[130];
    strcpy(pathToFile, "FileServeur/");
    strcat(pathToFile, fileName);

    /* Ouverture du fichier en mode binaire pour éviter les problèmes d'encodage */
    FILE* fp;
    fp = fopen(pathToFile, "wb");

    /* Booléen pour contrôler la fin de la réception du fichier */
    int isEndRecvFile;
    recv(dSC, &isEndRecvFile, sizeof(int), 0);

    /* Réception et écriture dans le fichier */
    char buffer[1024];
    size_t bytesRead;
    while (!isEndRecvFile) {
        bytesRead = recv(dSC, buffer, 1024, 0);
        fwrite(buffer, sizeof(char), bytesRead, fp);
        recv(dSC, &isEndRecvFile, sizeof(int), 0);
        memset(buffer, 0, sizeof(buffer));
    }
    fclose(fp);
}



int checkIsCommand(char * msg, int dS) {
    if (msg[0] == '/') {
        if (strcmp(msg, "/help\n") == 0) {
            helpCommand(dS);
        }
        /*else if (strcmp(msg, "/NOM_COMMANDE") == 0) {
            NOM_COMMANDE();
        }*/
        return 1;
    }
    return 0;
}
/*
*   void sendingPrivate(int numClient, char *msgReceived) :
*       envoie un message privé au client
*/

void sendingPrivate(int numClient, char *msgReceived) {
  /*Récupération du nom du destinataire*/
  char *nameDest = strtok(msgReceived, " ");
  nameDest++;
  /* Suppression du caractère @*/
  /*Récupération du message*/
  char *token = strtok(NULL, "");
  char * msgToSend = (char *) malloc(sizeof(char)*200);
  char * nameSender = tabClient[numClient].name; 
  strcat(msgToSend, "[Message Privé] ");
  strcat(msgToSend, nameSender);
  strcat(msgToSend, " : ");
  strcat(msgToSend, token);
  /*Recherche du destinataire dans le tableau des clients connectés*/
  int destFound = 0;
  int i;
  for (i = 0; i < MAX_CLIENT; i++) {
    if (tabClient[i].connected == 1 && strcmp(nameDest, tabClient[i].name) == 0) {
      /*Envoi du message au destinataire*/
      int sendResult = send(tabClient[i].dSC, msgToSend, strlen(msgToSend) + 1, 0);
      /*Gestion s'il y a une erreur*/
      if (sendResult == -1) {
        perror("Erreur lors de l'envoi du message");
        exit(EXIT_FAILURE);
      }

      destFound = 1;
      break;
    }
  }
    /*Destinataire non trouvé*/
  if (destFound == 0) {
    printf("Le destinataire n'a pas été trouvé.\n");
  }
}

void killThread() {
    pthread_mutex_lock(&lock);
    int i ;
    for(i =0; i< MAX_CLIENT; i++){
        if(tabThreadToKill[i]){
            pthread_cancel(tabThread[i]);
            tabThreadToKill[i] = 0;
        }
    }
}
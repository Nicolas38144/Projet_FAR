#include "global.h"
#include "funcServ.h"
#include <fcntl.h>

#define MAX_CLIENT 3
#define MAX_MSG 150



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
    /*Reception du nom du fichier à recevoir*/
    char * fileName = (char *) malloc(sizeof(char)*30);
    receiveMsg(dSC, fileName, sizeof(char)*30);

    printf("\nNom du fichier à recevoir: %s \n", fileName);
    if (strcmp(fileName,"error") == 0) {
        printf("Nom de fichier incorrect\n");
    }
    else {
        fileName = strtok(fileName, "\n");
        /*Création du thread pour gérer la reception du fichier*/
        pthread_t threadFile;
        int thread = pthread_create(&threadFile, NULL, receiveFile_th, (void *)fileName);
        if(thread==-1){
            perror("error thread");
        }
    }     
}



int checkIsCommand(char * msg, int dS) {
    if (msg[0] == '/') {
        if (strcmp(msg, "/help\n") == 0) {
            helpCommand(dS);
        }
        else if (strcmp(msg, "/file\n") == 0) {
            receiveFile(dS);
        }
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
    pthread_mutex_unlock(&lock);
}


/* 
 * Fonction qui vÃ©rifie si le pseudo saisi n'est pas dÃ©jÃ  utilisÃ© 
 * Retour: 1 si le pseudo n'est pas encore utilisÃ©, 0 sinon 
 */
int isNameAvailable(char * name){
    int i= 0;
    int available = 1;

    pthread_mutex_lock(&lock); /*Début d'une section critique*/
    
    while (i<MAX_CLIENT && available){
        
        if(tabClient[i].connected){
            if(strcmp(tabClient[i].name,name)==0){
                available = 0;
            }
        }
        i+=1;
    }
    pthread_mutex_unlock(&lock); /*Fin d'une section critique*/
    
    return available;
}



void send_integer(int dS, int number){
    int send_result = send(dS, &number, sizeof(int), 0);
    if (send_result == -1){ /*vÃ©rification de la valeur de retour*/
        perror("erreur lors de l'envoi");
        exit(EXIT_FAILURE);
    }
}


void addName(char *message, char *name) {
    char newMessage[MAX_MSG];
    snprintf(newMessage, MAX_MSG, "[%s] %s", name, message);
    strcpy(message, newMessage);
}


void All(int numClient, char* message) {

    /*pthread_mutex_lock(&lock); /*Début d'une section critique*/

    int dSC = tabClient[numClient].dSC;

    addName(message, tabClient[numClient].name);

    for (int i = 0; i < MAX_CLIENT; i++) {
        if (tabClient[i].connected && dSC == tabClient[i].dSC) {
            sendMsg(tabClient[i].dSC, message);
        }
    }

    /*pthread_mutex_unlock(&lock); /*Fin d'une section critique*/
}

void * receiveFile_th(void * fileNameParam){

    long dSCFile;
    /*Accepter une connexion*/
    dSCFile = acceptConnection(dSFile);

    char * fileName = (char *)fileNameParam;

    /*Création d'un fichier au même nom et reception du contenu du fichier*/

    /*Création du chemin pour enregister le fichier*/ 
    char * pathToFile = (char *) malloc(sizeof(char)*130);
    strcpy(pathToFile,"FileServeur/");
    strcat(pathToFile,fileName);

    printf("Je reçois le fichier %s du client le socket %ld\n",fileName,dSFile);

    /*Création du fichier et du buffer pour recevoir les données*/
    char buffer[1024];
    int f = open(pathToFile, O_WRONLY |  O_CREAT, 0666);
    if(f == -1){
        printf("erreur au open");
        exit(1);
    }
    int nbBytes = receivingInt(dSCFile);

    /*Reception*/
    while(nbBytes > 0){
        int bytesRead = recv(dSCFile, buffer, 1024, 0);
        write(f, buffer,nbBytes);
        nbBytes = receivingInt(dSCFile);
        nbBytes -= bytesRead;
        memset(buffer, 0, 1024);
    }
    printf("\n**Fichier reçu**\n");
    close(f);
    close(dSCFile);
    return NULL;
}



int createSocket(int port) {
    /*Création de la socket*/
	int dS = socket(PF_INET, SOCK_STREAM, 0);
    if (dS == -1) {
        perror("La création de la socket a échoué\n");
        exit(EXIT_FAILURE);
    }

	struct sockaddr_in ad;
	ad.sin_family = AF_INET;
	ad.sin_addr.s_addr = INADDR_ANY;
	ad.sin_port = htons(port);

	/*Nommage de la socket*/
	int bindResult = bind(dS, (struct sockaddr*)&ad, sizeof(ad));
	if (bindResult == -1) {
		perror("Erreur au bind");
		exit(EXIT_FAILURE);
	}

	/*Passer la socket en mode écoute*/
	int listenResult = listen(dS, MAX_CLIENT);
	if (listenResult == -1) {
		perror("Erreur au listen");
		exit(EXIT_FAILURE);
	}
    
    return dS;
};


int acceptConnection(int dS){
    int dSC;
    struct sockaddr_in aC;
    socklen_t lg = sizeof(struct sockaddr_in);
    dSC = accept(dS, (struct sockaddr*) &aC,&lg);
    if (dSC == -1){
        perror("Erreur au accept");
        exit(-1);
    }
    return dSC;
}


int receivingInt(long dS) {
    int number;
    if(recv(dS, &number, sizeof(int), 0) == -1){ /*vérification de la valeur de retour*/
        perror("erreur au recv d'un int");
        exit(-1);
    }
    return number;
}
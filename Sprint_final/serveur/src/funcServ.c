#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/global.h"
#include "../include/funcServ.h"
#include "../include/messageFunc.h"
#include "../include/receivFileFunc.h"
#include "../include/sendFileFunc.h"

#define MAX_CLIENT 3
#define MAX_MSG 150



/*
*   getNumClient() :
*       Permet de l'indice du premier emplacement disponible           
*       Return -1 si tout les emplecements sont occupés. 
*/
int getNumClient(){
    int indexTabClient = 0;
    while (indexTabClient < MAX_CLIENT && tabClient[indexTabClient].connected == 1) {
        indexTabClient += 1;
    }
    if (indexTabClient < MAX_CLIENT) {
        return indexTabClient;
    }
    return -1;
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
    printf("2");
    sendPersonnalMsg(dS, content, 1);
}



int checkIsCommand(char * msg, int dS) {
    if (msg[0] == '/') {
        if (strcmp(msg, "/help\n") == 0) {
            helpCommand(dS);
            return 1;
        }
        else if (strcmp(msg, "/upload\n") == 0) {
            receiveFile(dS);
            return 1;
        }
        else if (strcmp(msg, "/download\n") == 0) {
            sendFile(dS);
            return 1;
        }
        else {
            return 0;
        }  
    }
    return 0;
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
*   int isNameAvailable(char * name)
*       Fonction qui vérifie si le pseudo saisi n'est pas déjà  utilisé
*       Retour: 1 si le pseudo n'est pas encore utilisé, 0 sinon 
*/
int isNameAvailable(char * name) {
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



void addName(char *message, char *name) {
    char newMessage[MAX_MSG];
    snprintf(newMessage, MAX_MSG, "[%s] %s", name, message);
    strcpy(message, newMessage);
}



void All(int numClient, char* message) {
    /*pthread_mutex_lock(&lock); /*Debut d'une section critique*/
    int dSC = tabClient[numClient].dSC;
    addName(message, tabClient[numClient].name);
    for (int i = 0; i < MAX_CLIENT; i++) {
        if (tabClient[i].connected && dSC == tabClient[i].dSC) {
            sendMsg(tabClient[i].dSC, message);
        }
    }
    /*pthread_mutex_unlock(&lock); /*Fin d'une section critique*/
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
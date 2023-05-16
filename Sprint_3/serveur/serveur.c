#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#include "global.h"
#include "funcServ.h"

#define MAX_CLIENT 5

sem_t semNbClient;


/*
Compiler gcc -pthread -Wall -ansi -o serveur serveur.c
Lancer avec ./serveur <votre_port>
*/



/*
*   broadcast(void * numeroClient) :
*       Crée un nouveau thread qui exécute la fonction broadcast en utilisant l'entier numClient comme argument.
*       void * numeroClient : Le numéro du client...
*/
void * broadcast(void * numeroClient){
    int isFinished = 0;
    int isCommand = 0;
    int numClient = (long) numeroClient;
    char * nameSender = tabClient[numClient].name;

    while(isFinished != 1) {
        /*Réception du message (il a une longueur maximale de 100 caractères)*/
        char * msgReceived = (char *) malloc(sizeof(char)*100);
        receiveMsg(tabClient[numClient].dSC, msgReceived, sizeof(char)*100);
        printf("\nMessage recu: %s\n", msgReceived);

        /* condition_a permet de gerer le cas ou le message ne commene pas par un @*/
        int condition_a = 1;
        /* s'il y a @ */
        char first = msgReceived[0];
        if (strcmp(&first, "@") == 0) {
            sendingPrivate(numClient, msgReceived);
        }else{
            condition_a = 0;
        }

        /*Le client veut-il se log out ?*/
        isFinished = checkLogOut(msgReceived);
        isCommand = checkIsCommand(msgReceived, tabClient[numClient].dSC);
        if (isCommand == 0 && condition_a == 0) {
            /*Ajout du nom (=name) de l'expéditeur devant le message à envoyer*/
            char * msgToSend = (char *) malloc(sizeof(char)*200);
            strcat(msgToSend, nameSender);
            strcat(msgToSend, " : ");
            strcat(msgToSend, msgReceived);

            /*Envoi du message à tous les auters clients connectés*/
            printf("Envoi du message au(x) %d client(s) connecté(s).\n", nbConnectedClient);
            sendMsg(tabClient[numClient].dSC, msgToSend);

            /*Libération de la mémoire du message envoyé*/
            free(msgToSend);
        }
        /* si la commande est /File */
        if (isSendingFille(msgReceived) == 1){
            receiveFile(tabClient[numClient].dSC);
        }
        
        /*Libération de la mémoire du message reçu*/
        free(msgReceived);
    }

    /*Fermeture du socket client*/
    /*nbConnectedClient -= 1;*/
    tabClient[numClient].connected = 0;
    tabThreadToKill[numClient] = 1;
    close(tabClient[numClient].dSC);
    sem_post(&semNbClient);

    return NULL;
}


/*
* _____________________ MAIN _____________________
*/
int main(int argc, char *argv[]) {

    sem_init(&semNbClient, 0, MAX_CLIENT);

    /*Verification des paramètres*/
    if (argc < 2) {
        perror("Erreur : Lancez avec ./serveur <votre_port>");
		exit(EXIT_FAILURE);
    }

    int i;
    for(i=0;i<MAX_CLIENT;i++){
        tabThreadToKill[i]=0;
    }

	/*Création de la socket*/
	int dS = socket(PF_INET, SOCK_STREAM, 0);
    if (dS == -1) {
        perror("La création de la socket a échoué\n");
        exit(EXIT_FAILURE);
    }

	struct sockaddr_in ad;
	ad.sin_family = AF_INET;
	ad.sin_addr.s_addr = INADDR_ANY;
	ad.sin_port = htons(atoi(argv[1]));

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


    /*Pour faire fonctionner le serveur en continue*/
    while(1) {

        /*Tant qu'on peut accepter des clients */
        sem_wait(&semNbClient); 

        /*On tue les threads pour lesquels les clients ont quitté la connexion*/
        killThread();


        /*Accepter une connexion*/
        struct sockaddr_in aC;
        socklen_t sk = sizeof(struct sockaddr_in);
        int dSC = accept(dS, (struct sockaddr*) &aC,&sk);
        if (dSC == -1){
            perror("Erreur lors de l'acceptation du client");
            exit(EXIT_FAILURE);
        }


        /*Récupération du nombre de client connectés*/
        int valueSem;
        sem_getvalue(&semNbClient, &valueSem);
        nbConnectedClient = MAX_CLIENT-valueSem;

        /*Attribution au client de son numéro*/
        long numClient = getNumClient();
        printf("Client %ld connecté\n", numClient);

        /*Envoi au client le nombre de clients qui sont déjà connectés*/
        int sendClient = send(dSC, &nbConnectedClient, sizeof(int), 0);
        if (sendClient == -1){
            perror("Erreur lors de l'envoie du nombre de client connctés au nouveau client");
            exit(EXIT_FAILURE);
        }


        /*On enregistre la socket du client*/
        tabClient[numClient].dSC = dSC;
        tabClient[numClient].connected = 1;
            

        /*Réception du nom (=name) du client. Son name <= 100 caractères*/
        char * name = (char *) malloc(sizeof(char)*100);
        receiveMsg(dSC, name, sizeof(char)*100);


        /*Enregistrement du nom (=name) du client*/
        name = strtok(name, "\n");/*permet de ne garder que le nom sans tout l'espace à côté*/
        tabClient[numClient].name = (char *) malloc(sizeof(char)*100);
        strcpy(tabClient[numClient].name,name);


        /*On avertie tout le monde de l'arriver du nouveau client*/
        strcat(name," à rejoint la communication\n");
        sendMsg(dSC, name);

        /*On libère la mémoire de "name"*/
        free(name);
        
        
        /*Crée un nouveau thread qui exécute la fonction broadcast en utilisant l'entier numClient comme argument.*/
        int threadClient = pthread_create(&tabThread[numClient],NULL,broadcast,(void *)numClient);
        if(threadClient == -1){
            perror("Erreur lors de la création du thread");
        }
    }
    sem_destroy(&semNbClient);
    close(dS);
    return 0;
}
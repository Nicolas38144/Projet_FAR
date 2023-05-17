
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

#define MAX_CLIENT 3

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
        }
        else {
            condition_a = 0;
        }

        /*Le client veut-il se log out ?*/
        isFinished = checkLogOut(msgReceived);
        isCommand = checkIsCommand(msgReceived, tabClient[numClient].dSC);
        /*Si ce n'est ni une commande ni un message privé*/
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
        
        
        
        /*****************************************/
        /* si la commande est /File */
        /*else if (isSendingFille(msgReceived) == 1){
             /*Reception du nom du fichier à recevoir*/
        /*    char * fileName = (char *) malloc(sizeof(char)*30);
            receiveMsg(tabClient[numClient].dSC, fileName, sizeof(char)*30);
            printf("\nNom du fichier à recevoir: %s \n", fileName);

            fileName = strtok(fileName, "\n");

            /*Création du thread pour gérer la reception du fichier*/
        /*    pthread_t threadFile;
            int thread = pthread_create(&threadFile, NULL, receivingFile_th, (void *)fileName);
            if(thread==-1){
                perror("error thread");
            }
        }*/
        /*****************************************/

        
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

    arg1 = argv[1];
    sem_init(&semNbClient, 0, MAX_CLIENT);

    /*Verification des paramètres*/
    if (argc < 2) {
        perror("Erreur : Lancez avec ./serveur <votre_port>");
		exit(EXIT_FAILURE);
    }

    /*Création des sockets*/
	int dS = createSocket(atoi(arg1));
    dSFile = createSocket(atoi(arg1)+1);


    int i;
    for(i=0;i<MAX_CLIENT;i++) {
        tabThreadToKill[i]=0;
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
        send_integer(dSC, numClient);

        /*Réception du nom (=name) du client. Son name <= 100 caractères*/
        char * name = (char *) malloc(sizeof(char)*100);
        /*receiveMsg(dSC, name, sizeof(char)*100);*/

        int availableName = 0; /*false*/

        while(!availableName){
            send_integer(dSC,availableName);
            receiveMsg(dSC, name, sizeof(char)*100);
            name = strtok(name, "\n");
            availableName = isNameAvailable(name);
            printf("Pseudo non disponible\n");
        }
        /*pseudo valide*/
        send_integer(dSC, availableName);



        pthread_mutex_lock(&lock); /*Début d'une section critique*/

        /*name = strtok(name, "\n");permet de ne garder que le nom sans tout l'espace à côté*/
        tabClient[numClient].name = (char *) malloc(sizeof(char)*100);
        strcpy(tabClient[numClient].name,name);
        
        /*On enregistre la socket du client*/
        tabClient[numClient].dSC = dSC;
        tabClient[numClient].connected = 1;

        pthread_mutex_unlock(&lock); /*Fin d'une section critique*/
        

        /*On avertie tout le monde de l'arriver du nouveau client*/
        strcpy(name," a rejoint la communication\n");
        All(numClient, name);

        printf("%s\n", name);


        /*On libère la mémoire de "name"*/
        free(name);/*On avertie tout le monde de l'arriver du nouveau client*/

        
        /*Crée un nouveau thread qui exécute la fonction broadcast en utilisant l'entier numClient comme argument.*/
        int threadClient = pthread_create(&tabThread[numClient],NULL,broadcast,(void *)numClient);
        if(threadClient == -1){
            perror("Erreur lors de la création du thread\n");
        }
    }
    sem_destroy(&semNbClient);
    close(dS);
    return 0;
}
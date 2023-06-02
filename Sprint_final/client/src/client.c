#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>

#include "../include/client.h"
#include "../include/messageFunc.h"
#include "../include/global.h"



/*Compiler gcc -pthread -Wall -ansi -o client client.c */
/*Lancer avec ./client <IP_serveur> <port_serveur>*/


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;



/*------------------------------------------------------- MAIN---------------------------------------*/
int main(int argc, char *argv[]) {

    /*Verification des paramètres*/
    if (argc != 3) {
        printf("Erreur : Lancez avec ./client <votre_ip> <votre_port> ");
    }

    arg1 = argv[1];
    arg2 = argv[2];

    isFinished = 0;

    /*On vérifie si le client fait Ctrl+C*/
    signal(SIGINT, handle_sigint);

    /*Création de la socket*/
    int dS = socket(PF_INET, SOCK_STREAM, 0);
    printf("Socket Créé\n");
    struct sockaddr_in aS;
    aS.sin_family = AF_INET;

    inet_pton(AF_INET, arg1, &(aS.sin_addr));
    aS.sin_port = htons(atoi(arg2));
  
    /*Demander une connexion*/
    socklen_t lgA = sizeof(struct sockaddr_in);
    connect(dS, (struct sockaddr *) &aS, lgA);
    printf("Socket Connecté\n");

       /*Compte le nombre de clients connecté*/
    int nbClient = 0;
    if (recv(dS, &nbClient, sizeof(int), 0) == -1){ 
        perror("erreur au recv du numClient\n");
        exit(EXIT_FAILURE);
    }
    
    printf("Vous êtes le client numéro %d. \n", nbClient);


    /*choix pseudo du client qui se connecte*/
    int availableName;
    char * name = (char *) malloc(sizeof(char)*100);
    recv(dS,&availableName,sizeof(int),0);
   

    printf("Entrez votre pseudo : ");
    fgets(name, 100, stdin);
    sendMsg(dS,name);
    recv(dS,&availableName,sizeof(int),0);
   
    while(!availableName){
        printf("Pseudo déjà  utilisé!\nVotre pseudo: ");
        fgets(name, 100, stdin);
        sendMsg(dS,name);
        recv(dS,&availableName,sizeof(int),0);
    }

    printf("Votre pseudo est : %s\n", name);
    printf("\n");


    /*En attente d'un autre client*/
    if (nbClient == 0){
        printf("En attente d'un autre client\n");

        /*Reception du premier message informant de l'arrivée d'un autre client*/
        char *msg = (char *)malloc(sizeof(char) * 100);
        receiveMsg(dS, msg, sizeof(char) * 100);
        printf("%s", msg);

        free(msg);
    }



    /*--------------------------------------------------COMMUNICATION-----------------------------------------------*/ 
    /*Création d'un thread d'envoi*/
    pthread_t thread_sending;
    pthread_t thread_receiving;

    int thread1 = pthread_create(&thread_sending, NULL, sending_th, (void *)(intptr_t)dS);
    if (thread1 == -1) {
        perror("Error thread 1\n");
    }

    /*Création d'un thread de reception*/
    int thread2 = pthread_create(&thread_receiving, NULL, receiving_th, (void *)(intptr_t)dS);
    if (thread2 == -1) {
        perror("Error thread 2\n");
    }

    /*Attente de la fin des threads*/
    pthread_join(thread_sending, NULL);
    pthread_join(thread_receiving, NULL);

    return 0;
}
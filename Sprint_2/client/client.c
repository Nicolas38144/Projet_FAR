#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include "client.h"

/*Compiler gcc -pthread -Wall -ansi -o client client.c*/
/*Lancer avec ./client <IP_serveur> <port_serveur>*/



int isFinished = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int save_dS;


/*
*   checkLogOut(char * msg) :
*       Check si le client veut se déconnecter, il enverra au serveur le mot "fin"
*       char * msg : Le message du client
*/
int checkLogOut(char *msg) {
    if (strcmp(msg, "/logout\n") == 0) {
        strcpy(msg, "a quitté la conversation\n");
        return 1;
    }
    return 0;
}

void handle_sigint(int sig) {
    sendMsg(save_dS, "a quitté la conversation\n");
}


/*----------------------------------------------FONCTION D'ENVOI---------------------------------------------*/

/*
*   sendMsg(int dS, const char * message) :
*       Message envoyé à une socket et teste la conformité
*       int dS : expéditeur du message
*       const char * message : message à envoyer
*/
void sendMsg(int dS, const char * msg) {
    /*Envoi les données à la socket*/
    ssize_t num_bytes = send(dS, msg, strlen(msg)+1, 0);
    if (num_bytes < 0) {
        perror("Erreur dans l'envoi des données");
        exit(EXIT_FAILURE);
    }
}


/*
*   sending_th(void * dSparam) :
*       Envoie des messages à travers une connexion de socket, jusqu'à ce que la variable "isFinished" soit vraie.
*       dSparam : descripteur de socket. Il est ensuite convertit en entier et est utilisé pour envoyer des messages.
*/
void * sending_th(void * dSparam){
    int dS = (long)dSparam;
    save_dS = dS;
    while (!isFinished){

        /*Saisie du message au clavier*/
        char * m = (char *) malloc(sizeof(char)*100);
        fgets(m, 100, stdin);
        
        /*On vérifie si le client veut quitter la communication*/
        isFinished = checkLogOut(m);        
        /*Envoi*/
        sendMsg(dS, m);
        free(m);
    }
    close(dS);
    return NULL;
}



/*------------------------------------------------FONCTION DE RÉCEPTION------------------------------------------------*/

/*
*   receiveMsg(int dS, char * buffer, ssize_t size) :
*       Réceptionne un message envoyé à une socket et teste sa conformité
*       int dS : la socket
*       char * msg : message à recevoir
*       ssize_t size : taille maximum du message à recevoir
*/
void receiveMsg(int dS, char * buffer, ssize_t size) {
    ssize_t num_bytes = recv(dS, buffer, size, 0);     /*Reçoit les données de la part du socket*/
    if (num_bytes < 0) {
        perror("Erreur dans la réception de la donnée");
        exit(EXIT_FAILURE);
    }
    buffer[num_bytes] = '\0';
}



/*
*   receiveMsg(int dS, char * buffer, ssize_t size) :
*       Fonction pour le thread de reception
*       dSparam : Descripteur de socket. Il est ensuite convertit en entier et est utilisé pour recevoir des messages.
*/
void *receiving_th(void *dSparam) {
    int dS = (int)(intptr_t)dSparam;
    char r[1024]; /*Tampon de réception*/
    while (!isFinished){
        /*Réception du message dans le tampon de réception*/
        receiveMsg(dS, r, sizeof(r));
        printf("%s", r);
    }
    close(dS);
    return NULL;
}


/*------------------------------------------------------- MAIN---------------------------------------*/
int main(int argc, char *argv[])
{

    /*Verification des paramètres*/
    if (argc < 3)
    {
        printf("Erreur : Lancez avec ./client <votre_ip> <votre_port> ");
    }

    /*On vérifie si le client fait Ctrl+C*/
    signal(SIGINT,handle_sigint);

    /*Création de la socket*/
    int dS = socket(PF_INET, SOCK_STREAM, 0);
    printf("Socket Créé\n");
    struct sockaddr_in aS;
    aS.sin_family = AF_INET;
    inet_pton(AF_INET,argv[1],&(aS.sin_addr)) ;
    aS.sin_port = htons(atoi(argv[2])) ;

    /*Demander une connexion*/
    socklen_t lgA = sizeof(struct sockaddr_in) ;
    connect(dS, (struct sockaddr *) &aS, lgA) ;
    printf("Socket Connecté\n");

    /*Compte le nombre de clients connecté*/
    int nbClient = 0;
    if (recv(dS, &nbClient, sizeof(int), 0) == -1){ 
        perror("erreur au recv du numClient");
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


    /*Enlever le caractere de saut de ligne (\n) a la fin de la chaine
    size_t len = strlen(name);
    if (name[len-1] == '\n') {
        name[len-1] = '\0';
    }*/

    printf("Votre pseudo choisi est : %s\n", name);
    printf("\n");
    
    /*Envoi du message*/
    sendMsg(dS, name);
    

    
   


    /*En attente d'un autre client*/
    if (nbClient == 0)
    {
        printf("En attente d'un autre client\n");

        /*Reception du premier message informant de l'arrivée d'un autre client*/
        char *msg = (char *)malloc(sizeof(char) * 100);
        receiveMsg(dS, msg, sizeof(char) * 100);
        printf("%s", msg);

        free(msg);
    }
   
    

    /*--------------------------------------------------COMMUNICATION-----------------------------------------------*/ 

    

    /*Création d'un thread d'envoi*/
    pthread_t thread_sendind;
    pthread_t thread_receiving;

    int thread1 = pthread_create(&thread_sendind, NULL, sending_th, (void *)(intptr_t)dS);
    if (thread1 == -1)
    {
        perror("Error thread 1");
    }

    /*Création d'un thread de reception*/
    int thread2 = pthread_create(&thread_receiving, NULL, receiving_th, (void *)(intptr_t)dS);
    if (thread2 == -1)
    {
        perror("Error thread 2");
    }

    /*Attente de la fin des threads*/
    pthread_join(thread_sendind, NULL);
    pthread_join(thread_receiving, NULL);

    return 0;
}
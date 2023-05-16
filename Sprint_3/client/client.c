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

char * arg1;
char * arg2;


/*----------------------------------------------FONCTION DE VERIFICATION---------------------------------------------*/
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

/*
*   isSendingFile(char * msg)
*       Check si le client souhaite envoyer un fichier
*           char * msg : message du client à vérifier
*           Retour : 1 si le client veut envoyer un fichier, 0 sinon
*/
int isSendingFile(char * msg){
    if (strcmp(msg, "/file\n") == 0) {
        return 1;
    }
    return 0;
}



/*----------------------------------------------FONCTION D'ENVOI D'UN MESSAGE---------------------------------------------*/
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

        if (isSendingFile(m)){
            int sys = system("ls ./FileClient");
            if(sys == -1){
                printf("commande echouée");
            }

            /*Saisie du nom du fichier au clavier*/
            char * fileName = (char *) malloc(sizeof(char)*100);

            printf("\nSaisissez le nom d'un fichier à envoyer : \n");
            fgets(fileName, 100, stdin);
            fileName = strtok(fileName, "\n");

            pthread_t threadFile;
            int thread = pthread_create(&threadFile, NULL, sendingFile_th, (void *)fileName);
            if(thread==-1){
                perror("error thread");
            }
        }
        free(m);
    }
    close(dS);
    return NULL;
}



/*------------------------------------------------FONCTION DE RÉCEPTION D'UN MESSAGE------------------------------------------------*/
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



/*------------------------------------------------FONCTION D'ENVOI D'UN FICHIER------------------------------------------------*/
void * sendingFile_th(void * fileNameParam){

    /*Création de la socket*/
	long dSFile = socket(PF_INET, SOCK_STREAM, 0);
    if (dSFile == -1) {
        perror("Erreur lors de la création de la socket");
        exit(EXIT_FAILURE);
    }
	struct sockaddr_in aS;
	aS.sin_family = AF_INET;
	inet_pton(AF_INET, arg1, &(aS.sin_addr));
	aS.sin_port = htons(atoi(arg2)+1);

    /*Demander une connexion*/
	socklen_t lgA = sizeof(struct sockaddr_in);
	int connectR = connect(dSFile, (struct sockaddr *) &aS, lgA);
	if (connectR == -1){
		perror("Error when connect");
		exit(-1);
	}

    char * fileName = (char *)fileNameParam;
    
    /*Création du chemin pour trouver le fichier*/
    char * pathToFile = (char *) malloc(sizeof(char)*130);
    strcpy(pathToFile,"FileClient/");
    strcat(pathToFile,fileName);

    /*Ouverture et envoi du fichier*/
    FILE * f = NULL;
    f = fopen(pathToFile,"r");
    if (f == NULL) {   
        printf("Error! Could not open file\n"); 
        exit(-1); 
    }
    char data[1024] = "";
    int isEndSendFile = 0;

    while(fgets(data, 1024, (FILE *)f) != NULL) {
        if (send(dSFile, &isEndSendFile, sizeof(int), 0) == -1) {
            perror("[-]Error in sending file.");
            exit(1);
        }
        if (send(dSFile, data, sizeof(data), 0) == -1) {
            perror("[-]Error in sending file.");
            exit(1);
        }
        sleep(2);
        bzero(data, 1024);
    }

    isEndSendFile = 1;
    if (send(dSFile, &isEndSendFile, sizeof(int), 0) == -1) {
        perror("[-]Error in sending file.");
        exit(1);
    }
        
    fclose(f);
    free(pathToFile);
    return NULL;
}



/*------------------------------------------------FONCTION DE RÉCEPTION D'UN FICHIER------------------------------------------------*/





/*------------------------------------------------------- MAIN---------------------------------------*/
int main(int argc, char *argv[])
{

    /*Verification des paramètres*/
    if (argc != 3) {
        printf("Erreur : Lancez avec ./client <votre_ip> <votre_port> ");
    }

    arg1 = argv[1];
    arg2 = arg2[2];

    /*On vérifie si le client fait Ctrl+C*/
    signal(SIGINT,handle_sigint);

    /*Création de la socket*/
    int dS = socket(PF_INET, SOCK_STREAM, 0);
    printf("Socket Créé\n");
    struct sockaddr_in aS;
    aS.sin_family = AF_INET;
    inet_pton(AF_INET,arg1,&(aS.sin_addr)) ;
    aS.sin_port = htons(atoi(arg2)) ;

    /*Demander une connexion*/
    socklen_t lgA = sizeof(struct sockaddr_in) ;
    connect(dS, (struct sockaddr *) &aS, lgA) ;
    printf("Socket Connecté\n");

    /*Compte le nombre de clients connecté*/
    int nbClient = 0;
    if (recv(dS, &nbClient, sizeof(int), 0) == -1){ 
        perror("erreur au recv du numClient");
        exit(-1);
    }
    
    printf("Vous êtes le client numéro %d. \n", nbClient);


    /*choix pseudo du client qui se connecte*/
    char myString[100];

    printf("Entrez votre pseudo : ");
    fgets(myString, 100, stdin);

    /*Enlever le caractere de saut de ligne (\n) a la fin de la chaine*/
    size_t len = strlen(myString);
    if (myString[len-1] == '\n') {
        myString[len-1] = '\0';
    }

    printf("Votre pseudo choisi est : %s\n", myString);
    printf("\n");
    
    /*Envoi du message*/
    sendMsg(dS, myString);


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
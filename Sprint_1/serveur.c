#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_CLIENT 1000000

/*
Compiler gcc -pthread -Wall -ansi -o serveur serveur.c
Lancer avec ./serveur <votre_port>
*/


/*
*   Structure Client qui regroupe les informations du client
*   connected   : 0 si le client s'est déconnecté, 1 si connecté au serveur
*   name        : Nom du client
*   dSC         : Socket du client
*/
typedef struct Client Client;
struct Client{
    int connected;
    char * name;
    long dSC;
};


/*
*   tabClient           : Tableau contenant les clients connectés
*   tabThread           : Tableau contenant les threads utilisés pour le traitement de chaque client
*   nbConnectedClient   : Nombre de clients actuellement connectés
*/
Client tabClient[MAX_CLIENT];
pthread_t tabThread[MAX_CLIENT];
long nbConnectedClient = 0;



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
*   checkLogOut(char * msg) :
*       Check si le client veut se déconnecter, il enverra au serveur le mot "fin"
*       char * msg : Le message du client
*/
int checkLogOut(char * msg){
    if (strcmp(msg, "a quitté la conversation\n") ==0 ) {
        return 1;
    }
    return 0;
}



/*
*   broadcast(void * numeroClient) :
*       Crée un nouveau thread qui exécute la fonction broadcast en utilisant l'entier numClient comme argument.
*       void * numeroClient : Le numéro du client...
*/
void * broadcast(void * numeroClient){
    int isFinished = 0;
    int numClient = (long) numeroClient;
    char * nameSender = tabClient[numClient].name;

    while(isFinished != 1) {
        /*Réception du message (il a une longueur maximale de 100 caractères)*/
        char * msgReceived = (char *) malloc(sizeof(char)*100);
        receiveMsg(tabClient[numClient].dSC, msgReceived, sizeof(char)*100);
        printf("\nMessage recu: %s\n", msgReceived);

        /*Le client veut-il se log out ?*/
        isFinished = checkLogOut(msgReceived);

        /*Ajout du nom (=name) de l'expéditeur devant le message à envoyer*/
        char * msgToSend = (char *) malloc(sizeof(char)*200);
        strcat(msgToSend, nameSender);
        strcat(msgToSend, " : ");
        strcat(msgToSend, msgReceived);

        /*Envoi du message à tous les auters clients connectés*/
        printf("Envoi du message au(x) %ld client(s) connecté(s).\n", nbConnectedClient);
        sendMsg(tabClient[numClient].dSC, msgToSend);

        /*Libération de la mémoire des 2 messages : celui reçu et celui envoyé*/
        free(msgToSend);
        free(msgReceived);
    }

    /*Fermeture du socket client*/
    nbConnectedClient -= 1;
    tabClient[numClient].connected = 0;
    close(tabClient[numClient].dSC);

    return NULL;
}


/*
* _____________________ MAIN _____________________
*/
int main(int argc, char *argv[]) {
    /*Verification des paramètres*/
    if (argc < 2) {
        perror("Erreur : Lancez avec ./serveur <votre_port>");
		exit(EXIT_FAILURE);
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

        if(nbConnectedClient < MAX_CLIENT){

            /*Accepter une connexion*/
            struct sockaddr_in aC;
            socklen_t sk = sizeof(struct sockaddr_in);
            int dSC = accept(dS, (struct sockaddr*) &aC,&sk);
            if (dSC == -1){
                perror("Erreur lors de l'acceptation du client");
                exit(EXIT_FAILURE);
            }


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
            nbConnectedClient += 1;
        }
    }
    close(dS);
    return 0;
}

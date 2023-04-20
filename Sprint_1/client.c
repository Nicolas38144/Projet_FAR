#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>

/*Compiler gcc -pthread -Wall -ansi -o client client.c*/
/*Lancer avec ./client votre_ip votre_port*/

int isEnd = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/*Vérifie si un client souhaite quitter la communication*/
int endOfCommunication(char *msg)
{
    if (strcmp(msg, "fin\n") == 0)
    {
        strcpy(msg, "a quitté la conversation\n");
        return 1;
    }
    return 0;
}

/*----------------------------------------------FONCTION D'ENVOI---------------------------------------------*/

/*Message envoyé à une socket et teste la conformité*/
void sending(int dS, const char * msg) {
    /*Envoi les données à la socket*/
    ssize_t num_bytes = send(dS, msg, strlen(msg)+1, 0);
    if (num_bytes < 0) {
        perror("Erreur dans l'envoi des données");
        exit(EXIT_FAILURE);
    }
}

/*Fonction pour le thread d'envoi*/
void * sending_th(void * dSparam){
    int dS = (long)dSparam;
    while (!isEnd){

        /*Saisie du message au clavier*/
        char * m = (char *) malloc(sizeof(char)*100);

        /*printf("~");*/
        fgets(m, 100, stdin);

        /*On vérifie si le client veut quitter la communication*/
        isEnd = endOfCommunication(m);
        
        /*Envoi*/
        sending(dS, m);

        free(m);
    }
    close(dS);
    return NULL;
}



/*------------------------------------------------FONCTION DE RÉCEPTION------------------------------------------------*/

/*Réceptionne un message envoyé à une socket et teste sa conformité*/
void receiving(int dS, char * buffer, ssize_t size) {
    /*Reçoit les données de la part du socket*/
    ssize_t num_bytes = recv(dS, buffer, size, 0);
    if (num_bytes < 0) {
        perror("Erreur dans la réception de la donnée");
        exit(EXIT_FAILURE);
    }

     
    buffer[num_bytes] = '\0';

    /*affiche les données reçues*/
    /*printf("Réception : %s\n", buffer);*/
}


/*Fonction pour le thread de reception*/
void *receiving_th(void *dSparam)
{
    int dS = (int)(intptr_t)dSparam;
    char r[1024]; /*Tampon de réception*/
    while (!isEnd)
    {
        /*Réception du message dans le tampon de réception*/
        receiving(dS, r, sizeof(r));
        printf("%s", r);
        /*printf(">%s", r);*/
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

    printf("Votre pseudo choisi est : %s\n Stylé !\n", myString);

    /*Envoi du message*/
    sending(dS, myString);


    /*En attente d'un autre client*/
    if (nbClient == 0)
    {
        printf("En attente d'un autre client\n");

        /*Reception du premier message informant de l'arrivée d'un autre client*/
        char *msg = (char *)malloc(sizeof(char) * 100);
        receiving(dS, msg, sizeof(char) * 100);
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
        perror("error thread 1");
    }

    /*Création d'un thread de reception*/
    int thread2 = pthread_create(&thread_receiving, NULL, receiving_th, (void *)(intptr_t)dS);
    if (thread2 == -1)
    {
        perror("error thread 2");
    }

    /*Attente de la fin des threads*/
    pthread_join(thread_sendind, NULL);
    pthread_join(thread_receiving, NULL);

    
   
    return 0;
}
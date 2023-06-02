#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>

#include "../include/sendFileFunc.h"
#include "../include/verifFunc.h"
#include "../include/messageFunc.h"
#include "../include/global.h"


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
	aS.sin_port = htons(atoi(arg2)+1); /*on passe au port d'après !!!!!!!!!*/

    /*Demander une connexion*/
	socklen_t lgA = sizeof(struct sockaddr_in);
	int connectR = connect(dSFile, (struct sockaddr *) &aS, lgA);
	if (connectR == -1){
		perror("Error when connect\n");
		exit(-1);
	}
    
    FILE * f = (FILE *)fileNameParam;

    char data[1024] = "";
    int blocLenth = 1;
    while(blocLenth != 0) {
        blocLenth = fread(data, sizeof(char), 1024, f);
        if (blocLenth == -1) {
            perror("Error in reading file\n");
            exit(EXIT_FAILURE);
        }

        sendingInt(dSFile, blocLenth);
        if(blocLenth != 0){
            if (send(dSFile, data, blocLenth, 0) == -1) {
                perror("[-]Error in sending file.\n");
                exit(EXIT_FAILURE);
            }
        }
    } 
    printf("\n** Fichier envoyé **\n\n");
    fclose(f);
    close(dSFile);
    return NULL;
}




void sendingFile(int dS) {
    int fileCount = 0;
    char fileNames[10][100];
    printf("\n ----- Listes de fichiers disponibles à l'envoi ----- \n");

    FILE* filePipe = popen("ls --format=single-column ./FileClientToSend", "r");
    if (filePipe == NULL) {
        printf("Erreur lors de l'exécution de la commande\n");
    } 
    else {
        char fileName0[100];
        while (fgets(fileName0, sizeof(fileName0), filePipe) != NULL) {
            fileCount++;
            strcpy(fileNames[fileCount - 1], fileName0); /* Stocke le nom du fichier dans le tableau */
            printf("%2d - %s", fileCount, fileName0);
        }
        pclose(filePipe);
    }

    if (fileCount == 0) {
        sendMsg(dS, "error");
        printf("Aucun fichier disponible.\n\n");
    }
    else {
        /* Saisie du numéro du fichier à envoyer */
        int fileNumber;
        printf("\n--- Entrez le numéro du fichier à envoyer : ");
        scanf("%d", &fileNumber);
        getchar();

        if (fileNumber >= 1 && fileNumber <= fileCount) {
            char* selectedFileName = fileNames[fileNumber - 1];
            printf("Vous avez sélectionné le fichier : %s\n", selectedFileName);
            selectedFileName[strcspn(selectedFileName, "\n")] = '\0';
            
            /*Création du chemin pour trouver le fichier*/
            char * pathToFile = (char *) malloc(sizeof(char)*130);
            strcpy(pathToFile,"./FileClientToSend/");
            strcat(pathToFile,selectedFileName);

            /*Ouverture et envoi du fichier*/
            FILE * f = NULL;
            f = fopen(pathToFile,"rb");
            if (f == NULL) { 
                sendMsg(dS, "error");  
                printf("Erreur! Fichier inconnu\n\n"); 
            }
            else {
                /*Envoi du nom du fichier au serveur*/
                sendMsg(dS,selectedFileName);

                /*Création du thread d'envoi de fichier*/
                pthread_t threadFile;
                int thread = pthread_create(&threadFile, NULL, sendingFile_th, (void *)f);
                if(thread==-1){
                    perror("Error thread\n");
                }
            }
            free(pathToFile);
        } 
        else {
            printf("Numéro de fichier invalide.\n\n");
            sendMsg(dS, "error");
        }
    }
}
#include "global.h"
#include "funcServ.h"
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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


void sendingInt(int dS, int number){
    /*Envoi de l'entier 'number' à la socket 'dS'*/
    int sendR = send(dS, &number, sizeof(int), 0);
    if (sendR == -1){ /*vérification de la valeur de retour*/
        perror("erreur au send");
        exit(-1);
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
void sendPersonnalMsg(int dS, char * message, int condition) {
    /*Envoi un message uniquement à l'expéditeur du message*/
    int sendResult = send(dS, message, strlen(message)+1, 0);
    /*Gestion s'il y a une erreur*/
    if (sendResult == -1) {
        perror("Erreur lors de l'envoi du message");
        exit(EXIT_FAILURE);
    }
    if (condition == 1) {
        free(message);
    }
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
    sendPersonnalMsg(dS, content, 1);
}





/*------------------------------------------------FONCTION DE RÉCEPTION D'UN FICHIER------------------------------------------------*/
void * receiveFile_th(void * fileNameParam){

    /*Accepter une connexion*/
    long dSCFile = acceptConnection(dSFileReceiv);
    char * fileName = (char *)fileNameParam;


    /*Création du chemin pour enregister le fichier*/ 
    char * pathToFile = (char *) malloc(sizeof(char)*130);
    strcpy(pathToFile,"FileServeur/");
    strcat(pathToFile,fileName);
    /*Création du fichier et du buffer pour recevoir les données*/
    char buffer[1024];
    FILE * f = fopen(pathToFile, "wb");
    if((long int)f ==(long int) -1){
        printf("erreur au open");
        exit(1);
    }

    int nbBytesRead = receivingInt(dSCFile);
    /*Reception*/
    while(nbBytesRead > 0) {
        int res = recv(dSCFile, buffer, nbBytesRead, 0);
        if (res <= 0) {
            printf("Erreur de réception de la data");
            break;
        }
        fwrite(buffer, sizeof(char), nbBytesRead, f);
        nbBytesRead = receivingInt(dSCFile);
        if (nbBytesRead < 0){ 
            printf("Erreur de reception de la taille");
            break;
        }
    }
    printf("\n**Fichier envoyé**\n");
    fclose(f);
    close(dSCFile);
    return NULL;
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
        printf("%s", fileName);
        fileName = strtok(fileName, "\n");
        /*Création du thread pour gérer la reception du fichier*/
        pthread_t threadFile;
        int thread = pthread_create(&threadFile, NULL, receiveFile_th, (void *)fileName);
        if(thread==-1){
            perror("error thread");
        }
    }  
}





/*------------------------------------------------FONCTION D'ENVOI D'UN FICHIER------------------------------------------------*/
void * sendFile_th(void * file_param) {

    /*Accepter une connexion*/
    long dSCFile = acceptConnection(dSFileSend);
    char * file = (char *)file_param;
    printf("Nom du fichier que l'utilisateur veut -> :\n%s\n", file);
    /*sendPersonnalMsg(dSCFile, file, 0);*/
  
    /*Création du chemin pour trouver le fichier*/
    char * pathToFile = (char *) malloc(sizeof(char)*130);
    strcpy(pathToFile,"./FileServeur/");
    strcat(pathToFile,file);
    char *token;
    token = strtok(pathToFile, "\n");
    while (token != NULL) {
        token = strtok(NULL, "\n");
    }

    printf("pathToFile : %s\n", pathToFile);

    /*Ouverture et envoi du fichier*/
    FILE * f = NULL;
    f = fopen(pathToFile,"rb");
    if (f == NULL) { 
        sendPersonnalMsg(dSCFile, "Erreur lors de l'ouverture du fichier dans le serveur\n", 0);
        printf("Erreur! Fichier inconnu\n\n"); 
    }
    else {
        /*Envoi du nom du fichier au client*/
        /*sendPersonnalMsg(dSCFile, file, 0);*/
        /*send(dSCFile, &blocLenth, sizeof(int), 0);*/
        char data[1024] = "";
        int blocLenth = 1;
        while(blocLenth != 0) {
            blocLenth = fread(data, sizeof(char), 1024, f);
            if (blocLenth == -1) {
                perror("Error in reading file\n");
                exit(EXIT_FAILURE);
            }
            send_integer(dSCFile, blocLenth);
            if(blocLenth != 0){
                if (send(dSCFile, data, blocLenth, 0) == -1) {
                    perror("[-]Error in sending file.\n");
                    exit(EXIT_FAILURE);
                }
            }
          printf("blocLenth : %d\n",blocLenth);
        }
        printf("\n** Fichier envoyé **\n\n");
        fclose(f);        
    }
    free(pathToFile);
    free(file);
    close(dSCFile);
    return NULL;
}


void sendFile(int dSC) {
    /*Envoyer les noms des fichiers du dossier*/
    DIR *dir;
    struct dirent *ent;
    char * buffer = (char *) malloc(sizeof(char)*1);
    int buffer_pos = 0;
    int buffer_size = 0;

    // Ouvrir le répertoire
    dir = opendir("./FileServeur");
    if (dir == NULL) {
        perror("Erreur lors de l'ouverture du repertoire");
        exit(0);
    }
    /* Lire les fichiers du répertoire */
    while ((ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            continue;
        }
        int filename_len = strlen(ent->d_name);
        if (buffer_pos + filename_len + 1 >= buffer_size) {
            buffer_size = buffer_pos + filename_len + 1;
            buffer = (char *)realloc(buffer, buffer_size);
            if (buffer == NULL) {
                perror("Erreur lors de l'allocation de mémoire");
                exit(0);
            }
        }
        strncpy(buffer + buffer_pos, ent->d_name, filename_len);
        buffer_pos += filename_len;
        buffer[buffer_pos++] = '\n'; // Ajouter un espace entre les noms de fichiers
    }
    closedir(dir);
    buffer[buffer_pos] = '\0';

    if (strcmp(buffer,"") == 0) {
        printf("Aucun fichier n'est présent dans le serveur\n");
        sendPersonnalMsg(dSC, "X", 0);
        free(buffer);
    }
    else {
        printf("Taille du buffer : %ld\n", strlen(buffer));
        /* Envoie la liste das fichiers du serveur */
        sendPersonnalMsg(dSC, buffer, 0);
        
        char * f1 = (char *) malloc(sizeof(char)*32);
        receiveMsg(dSC, f1, sizeof(char)*32);
        strcat(f1, "\n");
        int okk = 1;
        if (strstr(buffer, f1) != NULL) {
            /*sendPersonnalMsg(dSC, "ok\n", 0);*/
            send_integer(dSC, okk);
            pthread_t threadFile;
            int thread=pthread_create(&threadFile,NULL,sendFile_th,(void*)f1);
            if(thread == -1) {
                perror("Error thread\n");
            }
        }
        else {
            /*sendPersonnalMsg(dSC, "no", 0);*/
            okk = 0;
            send_integer(dSC, okk);
        }
        /*free(f1);*/
    }
    free(buffer);
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



void send_integer(int dS, int number){
    int send_result = send(dS, &number, sizeof(int), 0);
    if (send_result == -1){ /*vérification de la valeur de retour*/
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


int receivingInt(long dS) {
    int number;
    if(recv(dS, &number, sizeof(int), 0) == -1){ /*vérification de la valeur de retour*/
        perror("erreur au recv d'un int");
        exit(-1);
    }
    return number;
}


/* -----------Channel ----------*/

void link_command(char* msg, long numcli) {
    char *command = strtok(msg, " ");
    char *element = strtok(NULL, " ");

    if (command != NULL && element != NULL) {
        if (strcmp(command, "#create") == 0) {
            
            printf("Commande de création : %s\n", element);
            
          createChannel(element);
        } else if (strcmp(command, "#remove") == 0) {
          
            printf("Commande de suppression : %s\n", element);
        } else if (strcmp(command, "#join") == 0) {
            // Le command est "#join" et l'élément suivant est dans la variable "element"
            printf("Commande de jointure : %s\n", element);
          joinChannelByName(element, numcli);
        } else if (strcmp(command, "#changedesc") == 0){
            printf("Commande change nom channel\n");
          changedesch(element, numcli);
        
        } else if (strcmp(command, "#printnomch") == 0) {
          printf("Print nom channel\n");
          printnomch(numcli);
        }
        else{
                  printf("Commande non reconnue\n");
        }
        
    } else {
             printf("Format de commande incorrect\n");
          
    }
}
void changedesch(const char* element, int numcli) {
  int indexchannel = tabClient[numcli].idRoom;
  if (indexchannel != -1) {
    size_t newDescrLength = strlen(element);
    size_t maxDescrLength = sizeof(channels[indexchannel].descr) - 1;  // Taille maximale de la description moins 1 pour le caractère nul
    if (newDescrLength > maxDescrLength) {
      printf("La nouvelle description dépasse la taille maximale autorisée.\n");
      return;
    }
    strcpy(channels[indexchannel].descr, element);
    printf("La description du canal '%s' a été modifiée avec succès.\n", element);
  } else {
    printf("Le canal '%s' n'a pas été trouvé.\n", element);
  }
}
void printnomch(int numcli) {
  char *msgtosend = (char *)malloc(sizeof(char) * 200);
  strcpy(msgtosend, "------Channel------\n");

  for (int i = 0; i < channelCount; i++) {
    strcat(msgtosend, channels[i].name);
    strcat(msgtosend, " / ");
  }

  send(tabClient[numcli].dSC, msgtosend, strlen(msgtosend), 0);

  free(msgtosend);
}



void createChannel(const char* name) {
    if (channelCount >= MAX_CHANNELS) {
        printf("Le nombre maximum de canaux a été atteint.\n");
        return;
    }

    // Vérifier si le nom du canal est unique
    for (int i = 0; i < channelCount; i++) {
        if (strcmp(channels[i].name, name) == 0) {
            printf("Un canal avec le même nom existe déjà.\n");
            return;
        }
    }

    channels[channelCount].name = (char *)malloc(sizeof(char)*100);
    strncpy(channels[channelCount].name, name,sizeof( channels[channelCount].name));
    channels[channelCount].Idchannel = channelCount;
    channels[channelCount].created = 1;
    printf("channelCount : %d \n",channelCount);
   printf("channels[channelCount].Idchannel : %d \n",channels[channelCount].Idchannel);
    channelCount+= 1;
  /*for (int i =0 ; i< MAX_CLIENT;i++ ){
    printf("%d\n",channels[i].Idchannel);
  }*/

    printf("Le canal '%s' a été créé avec succès.\n", name);
}
void joinChannelByName(const char* name, int memberID) {
  printf("(joinChannel member ID :%d \n",memberID);
    int channelIndex = findChannelByName(name);
    printf("channel Index : %d \n",channelIndex);
    if (channelIndex == -1) {
        printf("Le canal '%s' n'existe pas.\n", name);
        return;
    }
    joinChannel(channelIndex, memberID);
}
int findChannelByName(const char* name) {
    for (int i = 0; i < channelCount; i++) {
        if (strcmp(channels[i].name, name) == 0) {
            return i;  // Retourne l'indice du canal si le nom correspond
        }
    }
    return -1;  // Retourne -1 si le canal n'a pas été trouvé
}
void joinChannel(int channelIndex, int memberID) {
    if (channelIndex < 0 || channelIndex >= channelCount) {
        printf("Index de canal invalide.\n");
        return;
    }

    Channel* channel = &channels[channelIndex];

    if (channel->memberCount >= MAX_MEMBERS) {
        printf("Le canal est plein, impossible de rejoindre.\n");
        return;
    }

    for (int i = 0; i < channel->memberCount; i++) {
        if (channel->members[i] == memberID) {
            printf("Vous êtes déjà membre de ce canal.\n");
            return;
        }
    }
    printf("%d \n",memberID);
    tabClient[memberID].idRoom = channelIndex;
    channels[channelIndex].memberCount += 1;
    printf("id de la room : %d \n",tabClient[memberID].idRoom);
    printf("Vous avez rejoint le canal '%s'.\n", channel->name);
}
void sendMessagetoRoom(int numCli, const char* message) {
    int ds = tabClient[numCli].dSC;
    int channelIndex = tabClient[numCli].idRoom;
    if (channelIndex < 0 || channelIndex >= channelCount) {
        printf("Index de canal invalide.\n");
        return;
    }
      printf("id channel : send msg room %d \n",channelIndex);
    int i;
    for (i = 0; i < MAX_CLIENT; i++) {
        if(tabClient[i].idRoom == channelIndex && ds != tabClient[i].dSC && tabClient[i].connected==1 ){
          printf("id de la room : %d\n",tabClient[i].idRoom);
          send(tabClient[i].dSC, message, strlen(message)+1, 0);
        }
    }
}

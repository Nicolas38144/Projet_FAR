#include "global.h"
#include "funcServ.h"
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_CLIENT 3
#define MAX_MSG 150

/*
 *   getNumClient() :
 *       Permet de l'indice du premier emplacement disponible
 *       Return -1 si tout les emplecements sont occupés.
 */
int getNumClient()
{
    int indexTabClient = 0;
    while (indexTabClient < MAX_CLIENT && tabClient[indexTabClient].connected == 1)
    {
        indexTabClient += 1;
    }
    if (indexTabClient < MAX_CLIENT)
    {
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
void receiveMsg(int dS, char *msg, ssize_t size)
{
    int receiveResult = recv(dS, msg, size, 0);
    if (receiveResult == -1)
    {
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
void sendMsg(int dS, const char *message)
{
    int i;
    for (i = 0; i < MAX_CLIENT; i++)
    {
        /*On envoie à tous les clients excepté celui qui a envoyé le message*/
        if (tabClient[i].connected == 1 && dS != tabClient[i].dSC)
        {
            int sendResult = send(tabClient[i].dSC, message, strlen(message) + 1, 0);
            /*Gestion s'il y a une erreur*/
            if (sendResult == -1)
            {
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
void sendPersonnalMsg(int dS, char *message)
{
    /*Envoi un message uniquement à l'expéditeur du message*/
    int sendResult = send(dS, message, strlen(message) + 1, 0);
    printf("%d\n", sendResult);
    /*Gestion s'il y a une erreur*/
    if (sendResult == -1)
    {
        perror("Erreur lors de l'envoi du message");
        exit(EXIT_FAILURE);
    }
    free(message);
}

/*
 *   checkLogOut(char * msg) :
 *       Check si le client veut se déconnecter
 *       char * msg : Le message du client
 */
int checkLogOut(char *msg)
{
    if (strcmp(msg, "a quitté la conversation\n") == 0)
    {
        return 1;
    }
    return 0;
}

/*
 *   helpCommand(char * msg) :
 *       Check si le client fait la commande /help
 *       char * msg : Le message du client
 */
void helpCommand(int dS)
{
    FILE *f;
    char c;
    char *content = (char *)malloc(sizeof(char) * 3000);
    f = fopen("commande.txt", "rt");
    int i = 0;
    while ((c = fgetc(f)) != EOF)
    {
        content[i] = c;
        i++;
    }
    fclose(f);
    content = strtok(content, "");
    sendPersonnalMsg(dS, content);
}

/*
 *   isSendingFile(char * msg) :
 *       Regarde si le client a envoyé la commande /File
 */
/*int isSendingFille(char * msg){
    if (strcasecmp(msg, "/File") == 0){
        return 1;
    }
    return 0;
}*/

void receiveFile(int dSC)
{
    /*Reception du nom du fichier à recevoir*/
    char *fileName = (char *)malloc(sizeof(char) * 30);
    receiveMsg(dSC, fileName, sizeof(char) * 30);

    printf("\nNom du fichier à recevoir: %s \n", fileName);
    if (strcmp(fileName, "error") == 0)
    {
        printf("Nom de fichier incorrect\n");
    }
    else
    {
        printf("%s", fileName);
        fileName = strtok(fileName, "\n");
        /*Création du thread pour gérer la reception du fichier*/
        pthread_t threadFile;
        int thread = pthread_create(&threadFile, NULL, receiveFile_th, (void *)fileName);
        if (thread == -1)
        {
            perror("error thread");
        }
    }
}

void sendFile(int dSC)
{

    /*Envoyer les noms des fichiers du dossier*/
    DIR *dir;
    struct dirent *ent;
    char buffer[4096];
    int buffer_pos = 0;

    // Ouvrir le répertoire
    dir = opendir("./FileServeur");
    if (dir == NULL)
    {
        perror("Erreur lors de l'ouverture du repertoire");
        exit(0);
    }

    // Lire les fichiers du répertoire
    while ((ent = readdir(dir)) != NULL)
    {
        // Ignorer les entrées spéciales . et ..
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
        {
            continue;
        }
        // Concaténer le nom du fichier dans le buffer
        strncpy(buffer + buffer_pos, ent->d_name, 256 - 1);
        buffer_pos += strlen(ent->d_name);
        buffer[buffer_pos++] = '\n'; // Ajouter un espace entre les noms de fichiers
    }
    // Fermer le répertoire
    closedir(dir);
    // Terminer la chaîne de caractères dans le buffer
    buffer[buffer_pos] = '\0';
    // Afficher tous les noms de fichiers concaténés
    printf("Noms des fichiers :\n%s\n", buffer);

    /*Reception du nom du fichier à recevoir*/
    /*char * fileName = (char *) malloc(sizeof(char)*30);
    receiveMsg(dSC, fileName, sizeof(char)*30);

    printf("\nNom du fichier à recevoir: %s \n", fileName);
    if (strcmp(fileName,"error") == 0) {
        printf("Nom de fichier incorrect\n");
    }
    else {
        printf("%s", fileName);
        fileName = strtok(fileName, "\n");*/
    /*Création du thread pour gérer la reception du fichier*/
    /*pthread_t threadFile;
    int thread = pthread_create(&threadFile, NULL, receivingFile_th, (void *)fileName);
    if(thread==-1){
        perror("error thread");
    }
}  */
}

int checkIsCommand(char *msg, int dS)
{
    if (msg[0] == '/')
    {
        if (strcmp(msg, "/help\n") == 0)
        {
            helpCommand(dS);
            return 1;
        }
        else if (strcmp(msg, "/upload\n") == 0)
        {
            receiveFile(dS);
            return 1;
        }
        else if (strcmp(msg, "/download\n") == 0)
        {
            sendFile(dS);
            return 1;
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

/*
 *   void sendingPrivate(int numClient, char *msgReceived) :
 *       envoie un message privé au client
 */
void sendingPrivate(int numClient, char *msgReceived)
{
    /*Récupération du nom du destinataire*/
    char *nameDest = strtok(msgReceived, " ");
    nameDest++;
    /* Suppression du caractère @*/
    /*Récupération du message*/
    char *token = strtok(NULL, "");
    char *msgToSend = (char *)malloc(sizeof(char) * 200);
    char *nameSender = tabClient[numClient].name;
    strcat(msgToSend, "[Message Privé] ");
    strcat(msgToSend, nameSender);
    strcat(msgToSend, " : ");
    strcat(msgToSend, token);
    /*Recherche du destinataire dans le tableau des clients connectés*/
    int destFound = 0;
    int i;
    for (i = 0; i < MAX_CLIENT; i++)
    {
        if (tabClient[i].connected == 1 && strcmp(nameDest, tabClient[i].name) == 0)
        {
            /*Envoi du message au destinataire*/
            int sendResult = send(tabClient[i].dSC, msgToSend, strlen(msgToSend) + 1, 0);
            /*Gestion s'il y a une erreur*/
            if (sendResult == -1)
            {
                perror("Erreur lors de l'envoi du message");
                exit(EXIT_FAILURE);
            }
            destFound = 1;
            break;
        }
    }
    /*Destinataire non trouvé*/
    if (destFound == 0)
    {
        printf("Le destinataire n'a pas été trouvé.\n");
    }
}

void killThread()
{
    pthread_mutex_lock(&lock);
    int i;
    for (i = 0; i < MAX_CLIENT; i++)
    {
        if (tabThreadToKill[i])
        {
            pthread_cancel(tabThread[i]);
            tabThreadToKill[i] = 0;
        }
    }
    pthread_mutex_unlock(&lock);
}

/*
 *   int isNameAvailable(char * name)
 *       Fonction qui vérifie si le name saisi n'est pas déjà  utilisé
 *       Retour: 1 si le name n'est pas encore utilisé, 0 sinon
 */
int isNameAvailable(char *name)
{
    int i = 0;
    int available = 1;

    pthread_mutex_lock(&lock); /*Début d'une section critique*/

    while (i < MAX_CLIENT && available)
    {

        if (tabClient[i].connected)
        {
            if (strcmp(tabClient[i].name, name) == 0)
            {
                available = 0;
            }
        }
        i += 1;
    }
    pthread_mutex_unlock(&lock); /*Fin d'une section critique*/

    return available;
}

void send_integer(int dS, int number)
{
    int send_result = send(dS, &number, sizeof(int), 0);
    if (send_result == -1)
    { /*vÃ©rification de la valeur de retour*/
        perror("erreur lors de l'envoi");
        exit(EXIT_FAILURE);
    }
}

void addName(char *message, char *name)
{
    char newMessage[MAX_MSG];
    snprintf(newMessage, MAX_MSG, "[%s] %s", name, message);
    strcpy(message, newMessage);
}

void All(int numClient, char *message)
{
    /*pthread_mutex_lock(&lock); /*Début d'une section critique*/
    int dSC = tabClient[numClient].dSC;
    addName(message, tabClient[numClient].name);
    for (int i = 0; i < MAX_CLIENT; i++)
    {
        if (tabClient[i].connected && dSC == tabClient[i].dSC)
        {
            sendMsg(tabClient[i].dSC, message);
        }
    }
    /*pthread_mutex_unlock(&lock); /*Fin d'une section critique*/
}

void *receiveFile_th(void *fileNameParam)
{
    long dSCFile;
    /*Accepter une connexion*/
    dSCFile = acceptConnection(dSFile);
    char *fileName = (char *)fileNameParam;
    /*Création d'un fichier au même nom et reception du contenu du fichier*/
    /*Création du chemin pour enregister le fichier*/
    char *pathToFile = (char *)malloc(sizeof(char) * 130);
    strcpy(pathToFile, "FileServeur/");
    strcat(pathToFile, fileName);
    /*Création du fichier et du buffer pour recevoir les données*/
    char buffer[1024];
    FILE *f = fopen(pathToFile, "wb");
    if ((long int)f == (long int)-1)
    {
        printf("erreur au open");
        exit(1);
    }
    int nbBytesRead = receivingInt(dSCFile);
    /*Reception*/
    while (nbBytesRead > 0)
    {
        int res = recv(dSCFile, buffer, nbBytesRead, 0);
        if (res <= 0)
        {
            printf("Erreur de réception de la data");
            break;
        }
        fwrite(buffer, sizeof(char), nbBytesRead, f);
        nbBytesRead = receivingInt(dSCFile);
        if (nbBytesRead < 0)
        {
            printf("Erreur de reception de la taille");
            break;
        }
    }
    printf("\n**Fichier reçu**\n");
    fclose(f);
    close(dSCFile);
    return NULL;
}

int createSocket(int port)
{
    /*Création de la socket*/
    int dS = socket(PF_INET, SOCK_STREAM, 0);
    if (dS == -1)
    {
        perror("La création de la socket a échoué\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in ad;
    ad.sin_family = AF_INET;
    ad.sin_addr.s_addr = INADDR_ANY;
    ad.sin_port = htons(port);

    /*Nommage de la socket*/
    int bindResult = bind(dS, (struct sockaddr *)&ad, sizeof(ad));
    if (bindResult == -1)
    {
        perror("Erreur au bind");
        exit(EXIT_FAILURE);
    }

    /*Passer la socket en mode écoute*/
    int listenResult = listen(dS, MAX_CLIENT);
    if (listenResult == -1)
    {
        perror("Erreur au listen");
        exit(EXIT_FAILURE);
    }

    return dS;
};

int acceptConnection(int dS)
{
    int dSC;
    struct sockaddr_in aC;
    socklen_t lg = sizeof(struct sockaddr_in);
    dSC = accept(dS, (struct sockaddr *)&aC, &lg);
    if (dSC == -1)
    {
        perror("Erreur au accept");
        exit(-1);
    }
    return dSC;
}

int receivingInt(long dS)
{
    int number;
    if (recv(dS, &number, sizeof(int), 0) == -1)
    { /*vérification de la valeur de retour*/
        perror("erreur au recv d'un int");
        exit(-1);
    }
    return number;
}

/*-----------------------FONCTION CHANNEL--------------------*/

void initRoom()
{
    int i = 0;
    char buffer[100] = "";

    /*Ouverture du fichier contenant la dernière sauvegarde des salons*/
    FILE *fp = fopen("channel.txt", "r");
    if (fp == NULL)
    {
        perror("error\n");
    }

    pthread_mutex_lock(&lock); /*Début d'une section critique*/

    /*Initialisation du tableau des salons à partir du contenu du fichier*/
    while (fgets(buffer, 100, (FILE *)fp) != NULL && i < NB_ROOMS)
    {

        /*ID*/
        rooms[i].id = atoi(strtok(buffer, ","));

        /*NAME*/
        rooms[i].name = (char *)malloc(sizeof(char) * 100);
        strcpy(rooms[i].name, strtok(NULL, ","));

        /*DESCRIPTION*/
        rooms[i].descr = (char *)malloc(sizeof(char) * 100);
        strcpy(rooms[i].descr, strtok(NULL, ","));

        /*CREATED*/
        rooms[i].created = atoi(strtok(NULL, ","));

        /*MEMBERS*/
        /*Initialisation du tableau des membres à 0 (false: pas présent)*/
        int j;
        for (j = 0; j < MAX_CLIENT; j++)
        {
            rooms[i].members[j] = 0;
        }
        i++;
    }

    pthread_mutex_unlock(&lock); /*Fin d'une section critique*/

    fclose(fp);

    printf("\n-- Salons initialisés --\n");
}

void welcomeMsg(int dS)
{
    int i;
    char *msg = (char *)malloc(sizeof(char) * 300);
    strcpy(msg, "\n___Bienvenue dans le salon général___\nVoici les membres présents : \n");

    pthread_mutex_lock(&lock); /*Début d'une section critique*/

    /*Ajout des membres présents dans la salon général*/
    for (i = 0; i < MAX_CLIENT; i++)
    {
        if (rooms[0].members[i])
        {
            strcat(msg, tabClient[i].name);
            strcat(msg, "\n");
        }
    }

    pthread_mutex_unlock(&lock); /*Fin d'une section critique*/

    strcat(msg, "________Bonne communication________\n");
    sendMsg(dS, msg);
    free(msg);
}

void presentationRoom(int dS)
{
    int i;
    int j;
    char *msg = (char *)malloc(sizeof(char) * 400);
    strcpy(msg, "\n______Liste des salons existants______\n");

    pthread_mutex_lock(&lock); /*Début d'une section critique*/

    for (i = 0; i < NB_ROOMS; i++)
    {
        if (rooms[i].created)
        {

            /*NAME*/
            strcat(msg, "\n** ");
            strcat(msg, rooms[i].name);
            strcat(msg, " **\n");

            /*DESCRIPTION*/
            strcat(msg, " -- ");
            strcat(msg, rooms[i].descr);
            strcat(msg, " --\n");

            /*MEMBERS*/
            for (j = 0; j < MAX_CLIENT; j++)
            {
                if (rooms[i].members[j])
                {
                    strcat(msg, tabClient[j].name);
                    strcat(msg, "\n");
                }
            }
        }
    }
    pthread_mutex_unlock(&lock); /*Fin d'une section critique*/

    strcat(msg, "______________________________________\n\n");
    printf("%s\n", msg);

    /*Envoi du message de présentation*/
    sendMsg(dS, msg);
    free(msg);
}

void createRoom(int numClient, char *msg)
{
    char *error = (char *)malloc(sizeof(char) * 60);

    char *roomName = (char *)malloc(sizeof(char) * 300);
    strtok(msg, " ");              /*suppression de la commande dans le message*/
    roomName = strtok(NULL, "\0"); /*récupération du nom du salon à créer*/

    if (roomName == NULL)
    {
        strcpy(error, "Saisissez un nom de salon.\n");
        sendMsg(tabClient[numClient].dSC, error);
    }
    else if (!isNameAvailable(roomName))
    {
        strcpy(error, "Un nom de salon ne peut pas contenir d'espace.\n");
        sendMsg(tabClient[numClient].dSC, error);
    }
    else
    {

        /*ID*/
        int idRoom = getNonCreatedRoom();

        if (idRoom != NB_ROOMS)
        {                              /*Un salon est disponible*/
            pthread_mutex_lock(&lock); /*Début d'une section critique*/

            /*CREATED*/
            rooms[idRoom].created = 1;
            /*NAME*/
            strcpy(rooms[idRoom].name, roomName);

            /*MAJ NOM dans le fichier*/
            updateRoom();

            pthread_mutex_unlock(&lock); /*Fin d'une section critique*/
        }
        else
        { /*Aucun salon n'est disponible*/
            strcpy(error, "Le nombre maximum de salons a été atteint.\n");
            sendMsg(tabClient[numClient].dSC, error);
        }
    }
    /*free(roomName);
    free(error);*/
}

int getRoomByName(char *roomName)
{
    int found = 0;
    int i = 0;
    int indice = -1;

    pthread_mutex_lock(&lock); /*Début d'une section critique*/

    while (i < NB_ROOMS && !found)
    {
        if (strcmp(rooms[i].name, roomName) == 0)
        {
            found = 1;
            indice = i;
        }
        i++;
    }

    pthread_mutex_unlock(&lock); /*Fin d'une section critique*/

    return indice;
}

void addMember(int numClient, int idRoom)
{

    pthread_mutex_lock(&lock); /*Début d'une section critique*/

    /*Ajout de l'id du salon au client*/
    tabClient[numClient].idRoom = idRoom;
    /*On indique dans les membres du salon que le client est présent*/
    rooms[idRoom].members[numClient] = 1;

    pthread_mutex_unlock(&lock); /*Fin d'une section critique*/

    /*Envoi d'un message pour informer les autres membres du salon*/
    char *joinNotification = (char *)malloc(sizeof(char) * 100);
    strcpy(joinNotification, "** a rejoint le salon **\n");
    sendingRoom(numClient, joinNotification);

    free(joinNotification);
}

void deleteMember(int numClient, int idRoom)
{
    /*Envoi d'un message pour informer les autres membres du salon*/
    char *leaveNotification = (char *)malloc(sizeof(char) * 100);
    strcpy(leaveNotification, "** a quitté le salon **\n");
    sendingRoom(numClient, leaveNotification);

    free(leaveNotification);

    pthread_mutex_lock(&lock); /*Début d'une section critique*/

    /*On indique dans les membres du salon que le client n'est plus présent*/
    rooms[idRoom].members[numClient] = 0;

    pthread_mutex_unlock(&lock); /*Fin d'une section critique*/
}

void joinRoom(int numClient, char *msg)
{
    char *error = (char *)malloc(sizeof(char) * 60);

    char *roomName = (char *)malloc(sizeof(char) * 300);
    strtok(msg, " ");              /*suppression de la commande dans le message*/
    roomName = strtok(NULL, "\0"); /*récupération du nom du salon à créer*/

    if (roomName == NULL)
    {
        strcpy(error, "Saisissez un nom de salon.\n");
        sendMsg(tabClient[numClient].dSC, error);
    }
    else
    {
        /*ID*/
        int idRoom = getRoomByName(roomName);

        if (idRoom == -1)
        { /*Aucun salon n'a été trouvé*/

            strcpy(error, "Aucun salon trouvé.\n");
            sendMsg(tabClient[numClient].dSC, error);
        }
        else
        { /*Un salon à été trouvé, on fait le changement de salon*/

            pthread_mutex_lock(&lock); /*Début d'une section critique*/
            int idRoomClient = tabClient[numClient].idRoom;
            pthread_mutex_unlock(&lock); /*Fin d'une section critique*/

            deleteMember(numClient, idRoomClient);
            addMember(numClient, idRoom);
        }
    }

    return;
}

/*Attention cette fonction est toujours appelée dans une section critique, ne pas mettre de mutex dedans*/
void updateRoom()
{
    char *line = (char *)malloc(sizeof(char) * 200);

    /*Suppression fichier*/
    int ret = remove("channel.txt");
    if (ret == -1)
    {
        perror("erreur suppression updateRoom: \n");
        exit(1);
    }

    /*Création du fichier*/
    int fd = open("channel.txt", O_CREAT | O_WRONLY, 0666);
    if (fd == -1)
    {
        perror("erreur création updateRoom: \n");
        exit(1);
    }

    int i;
    for (i = 0; i < NB_ROOMS; i++)
    {
        printf("for %d", i);
        char *id = malloc(sizeof(int));
        char *create = malloc(sizeof(int));

        /*ID*/
        sprintf(id, "%d", rooms[i].id);
        strcpy(line, id);
        strcat(line, ",");

        /*NOM*/
        strcat(line, rooms[i].name);
        strcat(line, ",");

        /*DESCRIPTION*/
        strcat(line, rooms[i].descr);
        strcat(line, ",");

        /*CREATED*/
        sprintf(create, "%d", rooms[i].created);
        strcat(line, create);
        strcat(line, "\n\0");

        int w = write(fd, line, strlen(line));
        if (w == -1)
        {
            perror("erreur au write\n");
            exit(1);
        }

        free(id);
        free(create);
    }
    free(line);
    return;
}

int isOccupiedRoom(idRoom)
{
    int isO = 0;
    int i = 0;
    while (i < MAX_CLIENT)
    {
        isO = rooms[idRoom].members[i];
        i++;
    }
    return isO;
}

void removeRoom(int numClient, char *msg)
{
    char *error = (char *)malloc(sizeof(char) * 60);

    char *roomName = (char *)malloc(sizeof(char) * 300);
    strtok(msg, " ");              /*suppression de la commande dans le message*/
    roomName = strtok(NULL, "\0"); /*récupération du nom du salon à créer*/

    if (roomName == NULL)
    {
        strcpy(error, "Saisissez un nom de salon.\n");
        sendMsg(tabClient[numClient].dSC, error);
    }
    else
    {

        /*ID*/
        int idRoom = getRoomByName(roomName);

        if (idRoom == -1)
        { /*Aucun salon n'a été trouvé*/

            strcpy(error, "Aucun salon trouvé.\n");
            sendMsg(tabClient[numClient].dSC, error);
        }
        else if (idRoom == 0)
        { /*On ne peut modifier le salon principal*/

            strcpy(error, "Vous ne pouvez pas supprimer le salon général.\n");
            sendMsg(tabClient[numClient].dSC, error);
        }
        else if (isOccupiedRoom(idRoom))
        { /*Des clients sont présents dans le salon à supprimer*/

            strcpy(error, "Vous ne pouvez pas supprimer un salon occupé.\n");
            sendMsg(tabClient[numClient].dSC, error);
        }
        else
        { /*Le salon peut être supprimé*/

            pthread_mutex_lock(&lock); /*Début d'une section critique*/

            rooms[idRoom].created = 0;
            strcpy(rooms[idRoom].descr, "Default");

            /*MAJ NOM dans le fichier*/
            updateRoom();

            pthread_mutex_unlock(&lock); /*Fin d'une section critique*/
        }
    }
}

int getNonCreatedRoom()
{
    int i = 0;

    pthread_mutex_lock(&lock); /*Début d'une section critique*/

    while (i < NB_ROOMS && rooms[i].created)
    {
        i++;
    }

    pthread_mutex_unlock(&lock); /*Fin d'une section critique*/

    return i;
}

void updateNameRoom(int numClient, char *msg)
{
    char *error = (char *)malloc(sizeof(char) * 60);

    /*On récupère le nom du salon à modifier*/
    char *roomName = (char *)malloc(sizeof(char) * 300);
    strtok(msg, " ");             /*suppression de la commande dans le message*/
    roomName = strtok(NULL, " "); /*récupération du nom du salon à créer*/

    if (roomName == NULL)
    {
        strcpy(error, "Saisissez un nom de salon existant et un nouveau nom de salon.\n");
        sendMsg(tabClient[numClient].dSC, error);
    }
    else
    {

        /*On récupère le nouveau nom*/
        char *newName = (char *)malloc(sizeof(char) * 300);
        newName = strtok(NULL, "\0");

        if (newName == NULL)
        {
            strcpy(error, "Saisissez un nouveau nom de salon.\n");
            sendMsg(tabClient[numClient].dSC, error);
        }
        else if (!isNameAvailable(newName))
        {
            strcpy(error, "Un nom de salon ne peut pas contenir d'espace.\n");
            sendMsg(tabClient[numClient].dSC, error);
        }
        else
        {

            /*ID*/
            int idRoom = getRoomByName(roomName);

            if (idRoom == -1)
            { /*Aucun salon n'a été trouvé*/

                strcpy(error, "Aucun salon trouvé.\n");
                sendMsg(tabClient[numClient].dSC, error);
            }
            else if (idRoom == 0)
            { /*On ne peut modifier le salon principal*/

                strcpy(error, "Vous ne pouvez pas modifier le salon général.\n");
                sendMsg(tabClient[numClient].dSC, error);
            }
            else
            { /*On peut modifier le salon*/

                pthread_mutex_lock(&lock); /*Début d'une section critique*/

                strcpy(rooms[idRoom].name, newName);

                /*MAJ NOM dans le fichier*/
                updateRoom();

                pthread_mutex_unlock(&lock); /*Fin d'une section critique*/
            }
        }
    }
}

void updateDescrRoom(int numClient, char *msg)
{
    char *error = (char *)malloc(sizeof(char) * 60);

    /*On récupère le nom du salon à modifier*/
    char *roomName = (char *)malloc(sizeof(char) * 300);
    strtok(msg, " ");             /*suppression de la commande dans le message*/
    roomName = strtok(NULL, " "); /*récupération du nom du salon à créer*/

    if (roomName == NULL)
    {
        strcpy(error, "Saisissez un nom de salon existant et une nouvelle description.\n");
        sendMsg(tabClient[numClient].dSC, error);
    }
    else
    {

        /*On récupère la nouveau description*/
        char *newDescr = (char *)malloc(sizeof(char) * 300);
        newDescr = strtok(NULL, "\0");

        if (newDescr == NULL)
        {
            strcpy(error, "Saisissez une nouvelle description.\n");
            sendMsg(tabClient[numClient].dSC, error);
        }
        else
        {

            /*ID*/
            int idRoom = getRoomByName(roomName);

            if (idRoom == -1)
            { /*Aucun salon n'a été trouvé*/

                strcpy(error, "Aucun salon trouvé.\n");
                sendMsg(tabClient[numClient].dSC, error);
            }
            else if (idRoom == 0)
            { /*On ne peut modifier le salon principal*/

                strcpy(error, "Vous ne pouvez pas modifier le salon général.\n");
                sendMsg(tabClient[numClient].dSC, error);
            }
            else
            { /*On peut modifier le salon*/

                pthread_mutex_lock(&lock); /*Début d'une section critique*/

                strcpy(rooms[idRoom].descr, newDescr);

                /*MAJ NOM dans le fichier*/
                updateRoom();

                pthread_mutex_unlock(&lock); /*Fin d'une section critique*/
            }
        }
    }
}

int numCommande(char *msg)
{
    /*Récupération de la commande*/
    char *msgCopy = (char *)malloc(sizeof(char) * 30);
    strcpy(msgCopy, msg);
    char *cmd = (char *)malloc(sizeof(char) * 30);
    cmd = strtok(msgCopy, " ");
    char first = cmd[0];

    /*Analyse de la commande pour trouver son numéro*/
    int num = 0;
    if (strcmp(cmd, "/man") == 0)
    {
        num = 1;
    }
    else if (strcmp(cmd, "#whoishere") == 0)
    {
        num = 2;
    }
    else if (strcmp(cmd, "#pseudo") == 0)
    {
        num = 3;
    }
    else if (strcmp(cmd, "#rooms") == 0)
    {
        num = 4;
    }
    else if (strcmp(cmd, "#join") == 0)
    {
        num = 5;
    }
    else if (strcmp(cmd, "#create") == 0)
    {
        num = 6;
    }
    else if (strcmp(cmd, "#remove") == 0)
    {
        num = 7;
    }
    else if (strcmp(cmd, "#name") == 0)
    {
        num = 8;
    }
    else if (strcmp(cmd, "#descr") == 0)
    {
        num = 9;
    }
    else if (strcmp(cmd, "#upload") == 0)
    {
        num = 10;
    }
    else if (strcmp(cmd, "#download") == 0)
    {
        num = 11;
    }
    else if (strcmp(cmd, "#logout") == 0)
    {
        num = 12;
    }
    else if (strcmp(cmd, "@all") == 0)
    {
        num = 13;
    }
    else if (strcmp(&first, "@") == 0)
    {
        num = 14;
    }
    return num;
}

void sendingRoom(int numClient, char *msg)
{

    pthread_mutex_lock(&lock); /*Début d'une section critique*/

    int dS = tabClient[numClient].dSC;

    printf("Je reçois le message du client avec le socket %d\n", dS);

    addPseudoToMsg(msg, tabClient[numClient].name);

    int i;
    int idRoom = tabClient[numClient].idRoom;
    for (i = 0; i < MAX_CLIENT; i++)
    {

        /*On envoie le message à tout les clients présent dans le salon
        mais on ne l'envoie pas au client qui a écrit le message*/
        if (rooms[idRoom].members[i] && dS != tabClient[i].dSC)
        {
            sendMsg(tabClient[i].dSC, msg);
        }
    }

    pthread_mutex_unlock(&lock); /*Fin d'une section critique*/
}


void displayClient(int numClient)
{

    char *msg = (char *)malloc(sizeof(char) * 12 * MAX_CLIENT);
    strcpy(msg, "_____ Liste des clients connectés _____ \n");
    int i;

    for (i = 0; i < MAX_CLIENT; i++)
    {

        /*Si le client i est connecté*/
        if (tabClient[i].connected)
        {
            strcat(msg, "-- ");
            strcat(msg, tabClient[i].name);
            strcat(msg, "\n");
        }
    }
    strcat(msg, "_______________________________________ \n");
    sendMsg(tabClient[numClient].dSC, msg);
    free(msg);

    return;
}


void addPseudoToMsg(char *msg, char *pseudoSender)
{
    char *msgToSend = (char *)malloc(sizeof(char) * 150);
    strcpy(msgToSend, pseudoSender);
    strcat(msgToSend, " : ");
    strcat(msgToSend, msg);
    strcpy(msg, msgToSend);
    free(msgToSend);
    return;
}

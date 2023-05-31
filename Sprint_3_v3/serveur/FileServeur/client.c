#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <dirent.h>
#include <unistd.h>

#define PORT_SENDF 2004
#define PORT_RECEIVE 3204
#define TAILLE_MAX 1024
#define TAILLE_MAX_FICHIER 1024
#define TAILLE_MAX_BUFFER 100

//TODO changer les deux ports send et recieve en fonction du port d'entree
// /dc problemes
// serveur crash des fois

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int dS;
int dSF;
int dSD;

int fileExists(char **fileList, char *fileName, int tailleTableau)
{
  int i = 0;
  while (i < tailleTableau && strcmp(fileList[i], fileName) != 0)
  {
    i++;
  }

  if (i == tailleTableau)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

char *chooseFile(char **fileList, int tailleTableau)
{

  char *fileName;
  fileName = malloc(TAILLE_MAX);
  printf("Entree dans choose\n");
  //pthread_mutex_lock(&mutex); // lock pour eviter deux thread sur un fgets
  int fileExist = -1;
  do
  {
    printf("Entrez le nom de votre fichier : \n");
    
    fgets(fileName, TAILLE_MAX, stdin);
    
    char *pos = strchr(fileName, '\n');
    *pos = '\0';

    fileExist = fileExists(fileList, fileName, tailleTableau);
    
    if (fileExist != 0)
    {
      printf("Ce fichier n'existe pas veuillez reessayer\n");
    }
  } while (fileExist != 0);
  //pthread_mutex_unlock(&mutex);

  return fileName;
}

/* Renvois la liste des fichiers dans /clientfiles  */
char **listFile(int *tailleListe)
{
  DIR *d;
  struct dirent *dir;
  d = opendir("./clientfiles");
  if (d)
  {
    int i = 0;
    char **fileList = NULL;
    while ((dir = readdir(d)) != NULL)
    {
      if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0)
      {
        fileList = realloc(fileList, (i + 1) * sizeof(char *));
        fileList[i] = malloc(strlen(dir->d_name) + 1);
        strcpy(fileList[i], dir->d_name);
        i++;
      }
    }
    closedir(d);
    fileList = realloc(fileList, (i + 1) * sizeof(char *)); // ajouter un pointeur NULL à la fin du tableau
    fileList[i] = NULL;
    *tailleListe = i;
    return fileList;
  }
  return NULL;
}

void *sendFile(void *fileName)
{
  char *fileN = (char*) fileName;
  FILE *file;
  size_t bytesRead;
  char buffer[TAILLE_MAX_BUFFER];

  dSF = socket(PF_INET, SOCK_STREAM, 0);
  if (dSF == -1)
  {
    perror("Probleme création socket client sendfile");
    exit(1);
  }
  printf("Socket Créé\n");

  struct sockaddr_in aSF;
  aSF.sin_family = AF_INET;

  if (inet_pton(AF_INET, "127.0.0.1", &(aSF.sin_addr)) == -1)
  {
    perror("Probleme conversion adresse IP client sendfile");
    exit(1);
  }
  aSF.sin_port = htons(PORT_SENDF);
  socklen_t lgAF = sizeof(struct sockaddr_in);

  printf("dSF (sendFile) : %d\n", dSF);

  if (connect(dSF, (struct sockaddr *)&aSF, lgAF) == -1)
  {
    perror("Probleme connection client sendfile");
    exit(1);
  }

  char *filePath = malloc(sizeof(char) * 150);
  strcpy(filePath, "./clientfiles/");
  strcat(filePath, fileN);
  printf("FilePath client : %s\n", filePath);
  
  file = fopen(filePath, "rb");
  //printf("file : %s", file);

  /* Envoi nom de fichier */
  printf("Nom du fichier envoyé : %s\n", fileN);
  if(send(dSF, fileN, TAILLE_MAX ,0) == -1){
    perror("Probleme envoi nom de fichier");
    exit(1);
  }

  // Lire et envoyer le contenu du fichier par tranches
  while ((bytesRead = fread(buffer, 1, TAILLE_MAX_BUFFER, file)) > 0)
  {
    if (send(dSF, buffer, bytesRead, 0) == -1)
    {
      perror("Problème d'envoi du fichier");
      exit(1);
    }
  }
  free(fileN);
  free(filePath);
  fclose(file);

  if(close(dSF) == -1) {
    perror("Probleme shutdown serveur");
    exit(1);
  }

  pthread_exit(0);
}
/*
 * Telecharge un fichier choisi dans la liste des fichiers du serveur
 */
 //TODO sortir avant le fgets du thread ->  thread est important pour transfert de fichier pas pour le choisir
void *downloadFile() {

  char * affichageListe = malloc(TAILLE_MAX);

  char *filePath = malloc(TAILLE_MAX);

  char *fileData = malloc(TAILLE_MAX);

  int nbFichier;

  dSD = socket(PF_INET, SOCK_STREAM, 0);
  if (dSD == -1)
  {
    perror("Probleme création socket client downloadFile");
    exit(1);
  }
  printf("Socket Créé\n");

  struct sockaddr_in aSD;
  aSD.sin_family = AF_INET;

  if (inet_pton(AF_INET, "127.0.0.1", &(aSD.sin_addr)) == -1)
  {
    perror("Probleme conversion adresse IP client downloadFile");
    exit(1);
  }
  aSD.sin_port = htons(PORT_RECEIVE);
  socklen_t lgAD = sizeof(struct sockaddr_in);

  printf("dSD (downloadFile) : %d\n", dSD);
  if (connect(dSD, (struct sockaddr *)&aSD, lgAD) == -1)
  {
    perror("Probleme connection client downloadFile");
    exit(1);
  }

  /* Recois le nombre de fichiers */
  if (recv(dSD, &nbFichier, TAILLE_MAX, 0) == -1) {
    perror("Erreur de la recepetion du nombre de fichier");
  }

  char** fileList = malloc((nbFichier + 1) * sizeof(char *));

  /* Liste des fichiers en char** recup pour que l'util choisisse dedans */
  for(int i = 0; i < nbFichier; i++){

    fileList[i] = malloc(TAILLE_MAX + 1);

    if (recv(dSD, fileList[i], TAILLE_MAX + 1 , 0) == -1) {
      perror("Erreur de la reception de la liste de fichier");
    }
    printf("fileList[%d] : %s\n", i, fileList[i]);
  }

  /* Reception de la liste des fichiers en mode affichage */
  if (recv(dSD, affichageListe, TAILLE_MAX + 1, 0) == -1) {
    perror("Erreur dans reception de la liste de fichier formattée");
  }
  
  printf("%s\n", affichageListe);

  /* L'utilisateur choisit son fichier */
  char * nomFichierSelect = chooseFile(fileList, nbFichier);

  printf("Fichier selectionné : %s\n", nomFichierSelect);

  /* Envois du fichier select */
  if (send(dSD, nomFichierSelect, TAILLE_MAX, 0) == -1){
      perror("Probleme envoi client msg1");
      exit(1);
  }

  strcpy(filePath, "./clientfiles/");
  strcat(filePath, nomFichierSelect);

  FILE *fichier;
  fichier = fopen(filePath, "wb");  // Ouverture du fichier en mode écriture binaire
  if (fichier == NULL) {
    printf("Impossible d'ouvrir le fichier.\n");
  }

  printf("Fichier créé avec succès.\n");

  int recF = 1;

  /* reception des paquets et du serveur et ecriture dans le fichier */
  while (recF != 0) {
    recF = recv(dSD, fileData, TAILLE_MAX_BUFFER, 0); 
      
    if (recF == -1) {
      perror("Erreur reception dans server");
      exit(1);
    } else if (recF > 0) { 
      fwrite(fileData,1,recF,fichier);
    }
  }

  printf("Fichier download avec succès\n");

  /* Fermeture et libération */
  fclose(fichier);
  free(nomFichierSelect);
  free(affichageListe);
  free(filePath);


  for (int i = 0; i < nbFichier; i++) {
      free(fileList[i]);
  }
  free(fileList);

  pthread_exit(0);
}

// Fonction appelé par le thread créé par le programme principale
// Cette fonction gère l'envoi d'un message du client au serveur
void *threadSaisieEnvoie()
{

  char m1[TAILLE_MAX];
  // Boucle à l'infini pour permettre d'envoyer un message à n'importe quand

  while (1)
  {
    char *m1=malloc(TAILLE_MAX);
    //pthread_mutex_lock(&mutex);
    fgets(m1, TAILLE_MAX, stdin);

    char *pos2 = strchr(m1, '\n');
    *pos2 = '\0';
        
    //pthread_mutex_unlock(&mutex);
    
    // Envoi du message saisi
    if (send(dS, m1, strlen(m1) + 1, 0) == -1)
    {
      perror("Probleme envoi client msg1");
      exit(1);
    }
    // Si le client saisi "/dc"
    if (strcmp(m1, "/dc") == 0)
    {
      // Alors fermeture de la socket
      if (shutdown(dS, 2) == -1)
      {
        perror("Probleme shutdown client");
        exit(1);
      }
      exit(0);
    }
    if (strcmp(m1, "/sendfile") == 0)
    {
      pthread_t threadSendFile;

      printf("Liste des fichiers pouvant être envoyé : \n");
      int nbFichier;
      char **fileList = listFile(&nbFichier);
      for (int i = 0; i<nbFichier; i++){
        printf("- %s\n", fileList[i]);
      }
      char *file = chooseFile(fileList, nbFichier);
      for (int i = 0; i<nbFichier; i++){
        free(fileList[i]);
      }
      pthread_create(&threadSendFile, NULL, sendFile, (void *)file); // Création du thread qui s'occupera de l'envoi du fichier
      
    }

    if (strcmp(m1, "/download") == 0) {
      
      pthread_t threadDownloadFile;
      pthread_create(&threadDownloadFile, NULL, downloadFile, NULL); // Création du thread qui s'occupera de la reception du fichier
      pthread_join(threadDownloadFile, NULL);
      printf("Thread download fini \n");
    }
    
    free(m1);
  }

  // Fermeture du thread
  pthread_exit(0);
}

void traitementSigint()
{
  printf("Traitement du ctrl+c\n");
  char *m1 = "/dc";
  if (send(dS, m1, strlen(m1) + 1, 0) == -1)
  {
    perror("Probleme envoi client controle+c");
    exit(1);
  }

  if (shutdown(dS, 2) == -1)
  {
    perror("Probleme shutdown client");
    exit(1);
  }
  exit(0);
  pthread_exit(0);
}

int main(int argc, char *argv[])
{

  printf("Début programme\n");
  dS = socket(PF_INET, SOCK_STREAM, 0);
  if (dS == -1)
  {
    perror("Probleme création socket client");
    exit(1);
  }
  printf("dS : %d\n", dS);
  printf("Socket Créé\n");

  struct sockaddr_in aS;
  aS.sin_family = AF_INET;
  if (inet_pton(AF_INET, argv[1], &(aS.sin_addr)) == -1)
  {
    perror("Probleme conversion adresse IP client");
    exit(1);
  }
  aS.sin_port = htons(atoi(argv[2]));
  socklen_t lgA = sizeof(struct sockaddr_in);

  if (connect(dS, (struct sockaddr *)&aS, lgA) == -1)
  {
    perror("Probleme connection client");
    exit(1);
  }
  printf("Dans la liste d'attente...\n");

  pthread_t thread;

  char m1[TAILLE_MAX];

  // La réception est géré dans la boucle infini ci-dessous dans le programme principale
  while (1)
  {
    signal(SIGINT, traitementSigint); // traitement du ctrl+c

    int rec = recv(dS, m1, TAILLE_MAX, 0);
    if (rec == -1)
    {
      perror("Erreur reception dans client");
      exit(1);
    }
    else if (rec != 0)
    {
      printf("%s\n", m1);
    }

    if (strcmp(m1, "\033[33m[Serveur]\033[0m Fin de l'attente, entrée dans le serveur...") == 0)
    {
      printf("Création du thread d'envoi\n");

      pthread_create(&thread, NULL, threadSaisieEnvoie, (void *)1); // Création du thread qui s'occupera de l'envoi
    }
  }

  pthread_join(thread, NULL);

  if (shutdown(dS, 2) == -1)
  {
    perror("Probleme shutdown client");
    exit(1);
  }
  printf("Fin du programme\n");
}

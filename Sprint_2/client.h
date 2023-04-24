#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/stat.h>

/**
 * @file client.h
 * @brief Fichier du client de messagerie
 * Details
 */


/**
 * @brief Vérifie qu'un client veuille quitter la conversation
 * 
 * @param msg message du client à vérifier
 * @return int
 */
int checkLogOut(char *msg);

/**
 * @brief Message envoyé à une socket et teste la conformité
 * 
 * @param dS la socket
 * @param dS message à envoyer
 * @return void
 */
void sendMsg(int dS, const char * msg);

/**
 * @brief Fonction pour le thread d'envoi
 * 
 * @param dSparam la socket
 * @return void
 */
void * sending_th(void * dSparam);

/**
 * @brief Réceptionne un message envoyé à une socket et teste sa conformité
 * 
 * @param dS la socket
 * @param buffer un pointeur vers un tampon (buffer) où les données reçues seront stockées
 * @return void
 */
void receiveMsg(int dS, char * buffer, ssize_t size);


/**
 * @brief Fonction pour le thread de reception
 * 
 * @param dSparam pointeur générique
 * @return void
 */
void *receiving_th(void *dSparam);
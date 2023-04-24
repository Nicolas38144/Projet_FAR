# Projet_FAR

## Voici les différentes étapes du scénario :

 * Client se connecte au serveur.
 * Création d’un thread de réception.
 * Client envoie son pseudo au thread.
 * Le pseudo est créé et création d’un thread d’envoi.
 * Un client envoie un message.
 * Transfert du message aux autres clients.
 * Étapes 5 et 6 se répètent jusqu'à ce qu'un des clients envoie "fin".
 * Déconnexion du client.


## Comment compiler et exécuter le code :

  * Placez-vous dans le dossier : cd Projet_FAR
  * Pour compiler le client : gcc -pthread -Wall -ansi -o client client.c
  * Pour compiler le serveur : gcc -pthread -Wall -ansi -o serveur serveur.c
  * Pour lancer le serveur : ./serveur <votre_port>
  * Pour lancer le(s) client(s) : ./client <IP_serveur> <port_serveur>
  * Vous pouvez maintenant connecter vos clients au serveur et les faire parler entre eux.
  * Ecrivez “fin” dans un client pour que celui-ci quitte la conversation 

# Projet_FAR

## Voici les différentes étapes du scénario :

 1. Client se connecte au serveur.
 2. Création d’un thread de réception.
 3. Client envoie son pseudo au thread.
 4. Le pseudo est créé et création d’un thread d’envoi.
 5. Un client envoie un message.
 6. Transfert du message aux autres clients.
 7. Étapes 5 et 6 se répètent jusqu'à ce qu'un des clients envoie "fin".
 8. Déconnexion du client.


## Comment compiler et exécuter le code :

  * Placez-vous dans le dossier : 
  ```sh
  cd Projet_FAR
  ```
  * Pour compiler le client : 
  ```sh
  gcc -pthread -Wall -ansi -o client client.c
  ```
  * Pour compiler le serveur : 
  ```sh
  gcc -pthread -Wall -ansi -o serveur serveur.c
  ```
  * Pour lancer le serveur : 
  ```sh
  ./serveur <votre_port>
  ```
  * Pour lancer le(s) client(s) : 
  ```sh
  ./client <IP_serveur> <port_serveur>
  ```
  * Vous pouvez maintenant connecter vos clients au serveur et les faire parler entre eux.
  * Ecrivez “fin” dans un client pour que celui-ci quitte la conversation 

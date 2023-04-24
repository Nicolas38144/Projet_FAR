# Projet_FAR

Comment compiler et exécuter le code :

  * Placez-vous dans le dossier : cd Projet_FAR
  * Pour compiler le client : gcc -pthread -Wall -ansi -o client client.c
  * Pour compiler le serveur : gcc -pthread -Wall -ansi -o serveur serveur.c
  * Pour lancer le serveur : ./serveur <votre_port>
  * Pour lancer le(s) client(s) : ./client <IP_serveur> <port_serveur>
  * Vous pouvez maintenant connecter vos clients au serveur et les faire parler entre eux.
  * Ecrivez “fin” dans un client pour que celui-ci quitte la conversation 

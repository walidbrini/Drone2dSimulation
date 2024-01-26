
## Build project 
sudo make 


## Build Commande 
gcc -o commande commande_udp.c

## Build Server
gcc serveur_udp.c utilitaires.o threads_utility.o -o serveur -pthread

#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include <string.h>

#include<stdlib.h>
#include<strings.h>
#include <unistd.h>
#include "jeu.h"
#include "threads_utlity.h"

#define EOF_SIGNAL "END_OF_FILE"
#define EOF_SIGNAL_LENGTH (sizeof(EOF_SIGNAL) - 1)

#define LEPORT 2020
#define MAX_MESSAGE_LENGTH 1024

void receiveFile(int socket, const struct sockaddr_in *p_exp, const char *filename) {
    FILE *file = fopen(filename, "w");
    printf("Attente Info Carte et Commandes drones \n");
    if (file == NULL) {
        perror("Erreur ouverture fichier");
        exit(-1);
    }

    char buffer[MAX_MESSAGE_LENGTH];
    int bytesRead;

    do {
        bytesRead = recvfrom(socket, buffer, MAX_MESSAGE_LENGTH, 0, NULL, NULL);
        if (bytesRead == -1) {
            perror("Erreur receive");
            fclose(file);
            exit(-1);
        }

        // Check for end-of-file signal
        if (bytesRead == EOF_SIGNAL_LENGTH && memcmp(buffer, EOF_SIGNAL, EOF_SIGNAL_LENGTH) == 0) {
            break;  // Exit the loop without writing the signal to the file
        }

        // Write the received data to the file
        fwrite(buffer, 1, bytesRead, file);

    } while (bytesRead > 0);

    printf("File received successfully.\n");
    fclose(file);
}

void version_concurrente_socket(char* filename) {
    parse_config(filename, &carte, &drones, &num_drones);
    generer_map(&carte, 0.1, drones, num_drones);

    pthread_t drone_threads[num_drones];

    for (int i = 0; i < num_drones; ++i) {
        pthread_create(&drone_threads[i], NULL, execute_drone, (void*)&drones[i]);
    }

    pthread_t display_thread;
    pthread_create(&display_thread, NULL, display_map, NULL);

    for (int i = 0; i < num_drones; ++i) {
        pthread_join(drone_threads[i], NULL);
    }

    // Wait for the display thread to finish
    pthread_join(display_thread, NULL);

}

// create a function that sends a message here 

int main()
{
int s = 0,taille,bd,errno,lg = 40;
char msg[40];
char msg1[40] = "message bien recu ";
char filename[50] = "commande_recu.txt";

struct sockaddr_in *padin; //pointeur adresse internet locale
struct sockaddr_in *p_exp; //pointeur adresse internet expediteur (recuperée de l'entete paquet UDP recu)

/* RAPPEL de cette structure
struct sockaddr_in
{
    short      sin_family;
    unsigned short   sin_port;
    struct   in_addr   sin_addr;
    char   sin_zero[8];
};
*/

s = socket(AF_INET, SOCK_DGRAM, AF_UNSPEC);
if(s == -1)
  {
      printf("erreur creation socket %d\n",errno);
      exit(-1);
  }
printf("le socket est identifie par : %d \n",s);

taille = sizeof(struct sockaddr_in);
padin  = (struct sockaddr_in *) (malloc(taille));
bzero((char*)padin,taille);

padin -> sin_family = AF_INET;
padin -> sin_port   = htons(LEPORT);

bd = bind(s, (struct sockaddr *)padin, taille);
if(bd == -1)	
  {
    printf("Erreur d'attachement : %d \n",errno);
    exit(-1);
  }

p_exp = (struct sockaddr_in *) (malloc(sizeof(struct sockaddr_in)));
socklen_t p_lgexp = sizeof(struct sockaddr_in);

        //Attente ficher 
        printf("Attente Information Carte et Commandes deones");
        receiveFile(s, padin, filename);

        // lanci 
        version_concurrente_socket(filename);

        //
    

    bd = recvfrom(s,msg,lg,0,(struct sockaddr *)p_exp, &p_lgexp);
    if(bd == -1)	
      {
	printf("Erreur receive %d\n",bd);
	exit(-1);
      }
    printf("%s\n",msg);
    
    char num_drones_str[MAX_MESSAGE_LENGTH];
    snprintf(num_drones_str, sizeof(num_drones_str), "%d", num_drones);

    bd = sendto(s, num_drones_str, lg, 0, (struct sockaddr *)p_exp, taille);
    
    for (int i = 0; i < num_drones; i++) {
        // Prepare the message for each drone
        snprintf(msg1, sizeof(msg1), "Drone %d - Position: x = %d, y = %d", i, drones[i].posc, drones[i].posl);

        // Send the message for each drone
        bd = sendto(s, msg1, lg, 0, (struct sockaddr *)p_exp, taille);
        if (bd == -1) {
            perror("Error sending drone position");
            exit(-1);
        }
    }


close(s);
return 0;
}

  


#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "jeu.h"
#include "threads_utility.h"
#include<stdlib.h>

#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include <string.h>



pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t drone_finished_cond = PTHREAD_COND_INITIALIZER;
int drones_finished_count = 0;
carte_t carte;  
drone_t *drones;  
int num_drones = 0;

#define EOF_SIGNAL "END_OF_FILE"
#define EOF_SIGNAL_LENGTH (sizeof(EOF_SIGNAL) - 1)

#define LEPORT 2020
#define MAX_MESSAGE_LENGTH 1024



typedef struct {
    int socket;
    struct sockaddr_in *address;
} ThreadArgs;

void sendDrones(int socket, const struct sockaddr_in *p_exp, drone_t *drones, int num_drones) {
    char pos_str[MAX_MESSAGE_LENGTH];

    for (int i = 0; i < num_drones; i++) {
        snprintf(pos_str, sizeof(pos_str), "%d %d", drones[i].posc, drones[i].posl);
        printf("Sending position: x = %d, y = %d\n", drones[i].posc, drones[i].posl);
        sendto(socket, pos_str, strlen(pos_str) + 1, 0, (struct sockaddr *)p_exp, sizeof(struct sockaddr_in));
    }
}

void sendCarte(int socket, const struct sockaddr_in *p_exp) {
    sendto(socket, &carte.row, sizeof(carte.row), 0, (struct sockaddr *)p_exp, sizeof(struct sockaddr_in));
    sendto(socket, &carte.col, sizeof(carte.col), 0, (struct sockaddr *)p_exp, sizeof(struct sockaddr_in));
}


void sendDronePositions(int socket, const struct sockaddr_in *p_exp, int lg, drone_t *drones, int num_drones) {
    char msg[MAX_MESSAGE_LENGTH];

    for (int i = 0; i < num_drones; i++) {
        snprintf(msg, sizeof(msg), "Drone %d - Position: x = %d, y = %d", i, drones[i].posc, drones[i].posl);

        // Send the message for each drone
        int bd = sendto(socket, msg, lg, 0, (struct sockaddr *)p_exp, sizeof(struct sockaddr_in));
        if (bd == -1) {
            perror("Error sending drone position");
            exit(-1);
        }
    }
}

void afficher_map_drones() {
    //clearScreen();

    for (int i = 0; i < carte.col; ++i) {
        printf("=");
    }
    printf("\n");

    for (int i = 0; i < carte.row; ++i) {
        for (int j = 0; j < carte.col; ++j) {
            int isDronePosition = 0;

            // Check if the current position is one of the drones' positions
            for (int k = 0; k < num_drones; ++k) {
                if (drones[k].posl == i && drones[k].posc == j) {
                    isDronePosition = 1;
                    switch (drones[k].direction) {
                        case EST:
                            printf(">");
                            break;
                        case OUEST:
                            printf("<");
                            break;
                        case NORD:
                            printf("^");
                            break;
                        case SUD:
                            printf("v");
                            break;
                    }
                    carte.grid[i][j] = 2;
                    break;
                }
            }

            if (!isDronePosition) {
                if (carte.grid[i][j] == 1) {
                    printf("#");
                } else if (carte.grid[i][j] == 0) {
                    printf(" ");
                }
            }
        }
        printf("\n");
    }

    for (int i = 0; i < carte.col; ++i) {
        printf("=");
    }
    printf("\n");
}

void* execute_drone(void* arg) {
    drone_t* drone = (drone_t*)arg;

    for (int i = 0; drone->command[i] != '\0'; ++i) {
        pthread_mutex_lock(&mutex);
        executer_commande(drone, &carte, drone->command[i]);
        pthread_mutex_unlock(&mutex);

        // Signal that a drone has executed a command
        pthread_mutex_lock(&mutex);
        pthread_cond_signal(&drone_finished_cond);
        pthread_mutex_unlock(&mutex);

        // Sleep after releasing the mutex to avoid contention
        sleep(1);
    }

    // Increment the count of finished drones
    pthread_mutex_lock(&mutex);
    drones_finished_count++;
    printf("Drone %d Finished with coordinates posl : %d and posc %d \n", drone->num_drone,drone->posc,drone->posl);

    // Signal after updating the count
    pthread_cond_signal(&drone_finished_cond);

    pthread_mutex_unlock(&mutex);
}

void* display_map_conc(void* arg) {
    ThreadArgs *args = (ThreadArgs*) arg;
    int s = args->socket;                 
    struct sockaddr_in *p_exp = args->address; 

    pthread_mutex_lock(&mutex);
    
    while (drones_finished_count < num_drones) {
        pthread_cond_wait(&drone_finished_cond, &mutex);
        
        // Show the map after each drone completes a command
        clearScreen();
        afficher_map_drones();
        
        // SEND DRONE POS TO CLINET
        //sendCarte(s, p_exp ,carte); 
        sendDrones(s,p_exp, drones, num_drones);
        //sendDronePositions(s, p_exp, 40, drones, num_drones);

    }

    pthread_mutex_unlock(&mutex);

    clearScreen();
    afficher_map_drones();

    // Print the final positions 
    for (int i = 0; i < num_drones; i++) {
        printf("La position finale du drone %d est pos x : %d  pos y : %d\n", i, drones[i].posc, drones[i].posl);
    }
    //termination signal
    char* terminationMsg = "END_OF_DRONE_COMMANDS";
    sendto(s, terminationMsg, strlen(terminationMsg) + 1, 0, (struct sockaddr *)p_exp, sizeof(struct sockaddr_in));
}

void* display_map(void* arg) {
  
    pthread_mutex_lock(&mutex);
    
    while (drones_finished_count < num_drones) {
        pthread_cond_wait(&drone_finished_cond, &mutex);
        
        // Show the map after each drone completes a command
        clearScreen();
        afficher_map_drones();
        
    }

    pthread_mutex_unlock(&mutex);

    // All drones have finished, now print the final positions
    clearScreen();
    afficher_map_drones();

    // Print the final positions after all drones have finished
    for (int i = 0; i < num_drones; i++) {
        printf("La position finale du drone %d est pos x : %d  pos y : %d\n", i, drones[i].posc, drones[i].posl);
    }
}

void version_concurrente(char* filename) {
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

void version_concurrente_sock(char* filename, int s, struct sockaddr_in *p_exp) {
    ThreadArgs *args = malloc(sizeof(ThreadArgs));
    args->socket = s;
    args->address = p_exp;
    generer_map(&carte, 0.1, drones, num_drones);

    pthread_t drone_threads[num_drones];

    for (int i = 0; i < num_drones; ++i) {
        pthread_create(&drone_threads[i], NULL, execute_drone, (void*)&drones[i]);
    }

    pthread_t display_thread;
    pthread_create(&display_thread, NULL, display_map_conc, args);

    for (int i = 0; i < num_drones; ++i) {
        pthread_join(drone_threads[i], NULL);
    }

    // Wait for the display thread to finish
    pthread_join(display_thread, NULL);

}

int run_server(){


    /* RAPPEL de cette structure
    struct sockaddr_in
    {
        short      sin_family;
        unsigned short   sin_port;
        struct   in_addr   sin_addr;
        char   sin_zero[8];
    };
    */

    int s = 0,taille,bd,errno,lg = 40;
    char msg[40];
    char msg1[40] = "message bien recu ";
    char filename[50] = "commande_recu.txt";

    struct sockaddr_in *padin; //pointeur adresse internet locale
    struct sockaddr_in *p_exp; //pointeur adresse internet expediteur (recuperée de l'entete paquet UDP recu)


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

    receiveFile(s, padin, filename);

    bd = recvfrom(s,msg,lg,0,(struct sockaddr *)p_exp, &p_lgexp);

    if(bd == -1)	
    {
    printf("Erreur receive %d\n",bd);
    exit(-1);
    }
    printf("%s\n",msg);
    parse_config(filename, &carte, &drones, &num_drones);

    char num_drones_str[MAX_MESSAGE_LENGTH];
    snprintf(num_drones_str, sizeof(num_drones_str), "%d", num_drones);

    bd = sendto(s, num_drones_str, lg, 0, (struct sockaddr *)p_exp, taille);
    
    //sendCarte(s, p_exp); 

    version_concurrente_sock(filename, s, p_exp);

   

close(s);
return 0;

}
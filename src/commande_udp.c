#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include "jeu.h"


#define EOF_SIGNAL "END_OF_FILE"
#define EOF_SIGNAL_LENGTH (sizeof(EOF_SIGNAL) - 1)
#define LEPORT 2020
#define MAX_MESSAGE_LENGTH 1024
#define LEPORT 2020
#define FILE_BUFFER_SIZE 1024

carte_t carte;  
drone_t *drones;  
int num_drones = 0;

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
                    printf("D");
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

// Function to send a file to the server
void sendFile(int socket, const struct sockaddr_in *padin, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Erreur ouverture fichier");
        exit(-1);
    }

    char buffer[FILE_BUFFER_SIZE];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, FILE_BUFFER_SIZE, file)) > 0) {
        if (sendto(socket, buffer, bytesRead, 0, (struct sockaddr *)padin, sizeof(*padin)) == -1) {
            perror("Erreur send");
            exit(-1);
        }
    }

    // Send the end-of-file signal
    if (sendto(socket, EOF_SIGNAL, EOF_SIGNAL_LENGTH, 0, (struct sockaddr *)padin, sizeof(*padin)) == -1) {
        perror("Erreur send");
        exit(-1);
    }

    printf("-------Fichier envoyé------\n");
    fclose(file);
}


void receiveCarte(int socket) {

    // First, receive the dimensions of the grid
    recv(socket, &carte.row, sizeof(carte.row), 0);
    recv(socket, &carte.col, sizeof(carte.col), 0);

    // Allocate memory for the grid
    carte.grid = malloc(carte.row * sizeof(int *));
    for (int i = 0; i < carte.row; i++) {
        carte.grid[i] = malloc(carte.col * sizeof(int));
    }

    // Now receive the grid row by row
    for (int i = 0; i < carte.row; i++) {
        recv(socket, carte.grid[i], carte.col * sizeof(int), 0);
    }
    
}



int main()
{ 
    char num_drones_str[MAX_MESSAGE_LENGTH];
    int s = 0,taille,bd,errno,lg = 40, un_entier;  
    struct sockaddr_in  padin; //adresse internet locale
    struct sockaddr_in *p_exp; //pointeur vers adresse internet expediteur (recuperée de l'entete paquet UDP recu)
    socklen_t p_lgexp = sizeof(struct sockaddr_in); 
    char *name = "localhost";
    char *adr_serv="127.0.0.1";
    char msg[40] = "coucou c'est moi, ton client ENSEM";
    char msg1[40]; // pour recevoir ACK
    int *p_lgexp1;
    char nom_fichier[40]; 

    struct hostent *host; 

    /* RAPPEL de struct hostent, definie dans netdb.h
    struct  hostent {
        char    *h_name;        // official name of host
        char    **h_aliases;    // alias list 
        int     h_addrtype;     // host address type 
        int     h_length;       // length of address 
        char    **h_addr_list;  // list of addresses from name server 
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
    bzero((char*)&padin,taille);

    // Avec le DNS, on obtient l'adresse ip par gethostbyname() 
    if ((host = gethostbyname(name))==NULL)
            {perror("Nom de machine"); 
            exit(2);};
    bcopy(host -> h_addr_list[0], &padin.sin_addr,host -> h_length);

    // si on connait l'adresse ip de destination en chiffre, on peut l'affecter directement:
    //un_entier=inet_aton(adr_serv, &padin.sin_addr);
    padin.sin_family = AF_INET;
    padin.sin_port   = htons(LEPORT);


        printf("Donner le nom du fichier a envoyer : ");
        scanf("%s",nom_fichier);
        sendFile(s, &padin, nom_fichier);
        printf("Attente position finales des drones par le serveur \n ");

        bd = sendto(s,"Sending the client Final Coordinates",lg,0,&padin,sizeof(padin));

        if(bd == -1)
        {
        printf("Erreur send \n");
        exit(-1);
        }
    
        p_exp = (struct sockaddr_in *)(malloc(taille));
        p_lgexp1 = (int*)malloc(sizeof(int));
        *p_lgexp1 = sizeof(p_exp);



        // Receive the number of drones as a string from the server
        bd = recvfrom(s, num_drones_str, sizeof(num_drones_str), 0, (struct sockaddr *)&p_exp, &taille);
        if (bd == -1) {
            perror("Error receiving number of drones");
            exit(-1);
        }

        // Convert the received string to an integer
        int num_drones = atoi(num_drones_str);
        printf("Number of drones: %d\n", num_drones);
        drone_t *drones = malloc(num_drones * sizeof(drone_t));
        
        receiveCarte(s); 
        afficher_map_drones();
    int continueReceiving = 1;

        while (continueReceiving) {
            for (int i = 0; i < num_drones; i++) {
                char pos_str[MAX_MESSAGE_LENGTH];
                int received_bytes = recvfrom(s, pos_str, MAX_MESSAGE_LENGTH, 0, (struct sockaddr *)&p_exp, &taille);

                if (received_bytes > 0) {
                    if (strcmp(pos_str, "END_OF_DRONE_COMMANDS") == 0) {
                        printf("All drone commands executed.\n");
                        continueReceiving = 0;
                        break;
                    }

                    sscanf(pos_str, "%d %d", &drones[i].posc, &drones[i].posl);
                    printf("Drone %d - Position: x = %d, y = %d\n", i, drones[i].posc, drones[i].posl);

                }
            }
            printf("-----------------------------------------\n");
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
                        printf("D");
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

        //afficher_map_drones();

        }

        


    close(s);
    return 0;
}

  
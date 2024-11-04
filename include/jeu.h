#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

// Mettez en commentaire cette ligne si vous êtes sous Windows :
#include <termios.h>

#define TRUE 256
#define FALSE 0

// Mettez en commentaire cette ligne si vous êtes sous Windows :
//static struct termios origtc, newtc;

enum direction {
    NORD = 0,    // ^ (North)
    EST = 1,    // > (East)
    SUD = 2,    // v (South)
    OUEST = 3   // < (West)
};


typedef struct{
    int posl; // ligne actuelle du drone 
    int pl_old; // ligne précédente du drone 
    int posc; // colonne actuelle du drone 
    int pc_old; // colonne précédente du drone 
    char direction; // direction actuelle du drone 
    char* command; 
    int num_drone ; 
} drone_t;

typedef struct{ 
    int **grid;
    int row ; 
    int col; 
} carte_t ; 



void *lire_clavier(void *arg);
void clearScreen();
void parse_config(const char *filename, carte_t *carte, drone_t **drones, int *num_drones);
void generer_map(carte_t *map, double probability, drone_t *drones, int num_drones);
void deplacer_drone(drone_t *drone, carte_t *map) ;
void executer_commandes(drone_t *drone, carte_t *map);
void executer_commande(drone_t *drone, carte_t *map, char commande);
void deplacer_drone(drone_t *drone, carte_t *map) ;
void reculer_drone(drone_t *drone, carte_t *map) ;
void mettre_a_jour_carte(drone_t *drone, carte_t *map) ;
void afficher_map(carte_t *map, drone_t *drone); 
char directionToString(enum direction dir) ;
void afficher_map_drones();
void version_concurrente(char * filename);
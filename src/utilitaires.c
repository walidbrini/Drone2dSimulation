#include "jeu.h"
#include <unistd.h>


void clearScreen()
{
    int n;
    for (n = 0; n < 10; n++)
        printf( "\n\n\n\n\n\n\n\n\n\n" );
}

void parse_config(const char *filename, carte_t *carte, drone_t **drones, int *num_drones) {
    // Parsing FILE 
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(1);
    }

    fscanf(file, "%d %d", &carte->row, &carte->col);

    *drones = malloc(sizeof(drone_t));
    if (*drones == NULL) {
        perror("Memory allocation error");
        exit(1);
    }

    *num_drones = 0;  // Initialize the number of drones

    while (fscanf(file, "%d %d %c %ms", &(*drones)[*num_drones].posl, &(*drones)[*num_drones].posc,
                  &(*drones)[*num_drones].direction, &(*drones)[*num_drones].command) == 4) {

        // Convert  direction enum value
        switch ((*drones)[*num_drones].direction) {
            case 'N': (*drones)[*num_drones].direction = NORD; break;
            case 'E': (*drones)[*num_drones].direction = EST; break;
            case 'S': (*drones)[*num_drones].direction = SUD; break;
            case 'O': (*drones)[*num_drones].direction = OUEST; break;
        }

        (*drones)[*num_drones].pl_old = (*drones)[*num_drones].posl;
        (*drones)[*num_drones].pc_old = (*drones)[*num_drones].posc;
        (*drones)[*num_drones].num_drone = *num_drones ; 
        printf("Drone %d: posl: %d, posc: %d, direction: %d, command: %s\n",
               *num_drones + 1, (*drones)[*num_drones].posl, (*drones)[*num_drones].posc,
               (*drones)[*num_drones].direction, (*drones)[*num_drones].command);

        (*num_drones)++;
        *drones = realloc(*drones, (*num_drones + 1) * sizeof(drone_t));
        if (*drones == NULL) {
            perror("Memory reallocation error");
            exit(1);
        }
    }

    fclose(file);
}

void generer_map(carte_t *map, double probability, drone_t *drones, int num_drones) {
    // Assuming that 0 means walkway , 1 is a wall , and 2 is drone
    //

    srand(time(NULL));
    map->grid = (int **)malloc(map->row * sizeof(int *));
    for (int i = 0; i < map->row; ++i) {
        map->grid[i] = (int *)malloc(map->col * sizeof(int));
    }

    // Initialize all cells as walls
    for (int i = 0; i < map->row; ++i) {
        for (int j = 0; j < map->col; ++j) {
            map->grid[i][j] = 1; 
        }
    }

    for (int i = 0; i < map->row; ++i) {
        for (int j = 0; j < map->col; ++j) {
            int isDronePosition = 0;
            // Check if drone position
            for (int k = 0; k < num_drones; ++k) {
                if (drones[k].posl == i && drones[k].posc == j) {
                    isDronePosition = 1;
                    break;
                }
                
            }
            if (!isDronePosition) {
                double random_value = (double)rand() / RAND_MAX;
                map->grid[i][j] = (random_value < probability) ? 1 : 0;
            }
        }
    }
    
}

// This function works only with one drone use affichier_drones for version_concurrente
void afficher_map(carte_t *map, drone_t *drone) {
    //clearScreen();

    for (int i = 0; i < map->col; ++i) {
        printf("=");
    }
    printf("\n");

    for (int i = 0; i < map->row; ++i) {
        for (int j = 0; j < map->col; ++j) {
            if (map->grid[i][j] == 1) {
                printf("#");
            } else if (map->grid[i][j] == 0) {
                printf(" ");
            } else {
                // Assuming drone->direction is an integer representing the direction
                switch (drone->direction) {
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
            }
        }
        printf("\n");
    }

    for (int i = 0; i < map->col; ++i) {
        printf("=");
    }
    printf("\n");
}

char directionToString(enum direction dir) {
    switch (dir) {
        case NORD:
            return 'N';
        case EST:
            return 'E';
        case SUD:
            return 'S';
        case OUEST:
            return 'O';
        default:
            return 'x';
    }
}

void deplacer_drone(drone_t* drone, carte_t* map) {
    // Save the current position of the drone
    drone->pl_old = drone->posl;
    drone->pc_old = drone->posc;

    // Update the position based on the direction
    switch (drone->direction) {
        case EST:
            drone->posc++;
            break;
        case OUEST:
            drone->posc--;
            break;
        case SUD:
            drone->posl++;
            break;
        case NORD:
            drone->posl--;
            break;
    }

    // Check for collisions and update the map
    mettre_a_jour_carte(drone, map);
}

void reculer_drone(drone_t* drone, carte_t* map) {
    // Save the current position of the drone
    int old_posl = drone->posl;
    int old_posc = drone->posc;

    // Update the position based on the direction
    switch (drone->direction) {
        case EST:
            drone->posc--;
            break;
        case OUEST:
            drone->posc++;
            break;
        case SUD:
            drone->posl--;
            break;
        case NORD:
            drone->posl++;
            break;
    }
    // Check for collisions and update the map
    mettre_a_jour_carte(drone, map);
}

void mettre_a_jour_carte(drone_t* drone, carte_t* map) {
    // Check for collisions 
    if (drone->posl < 0 || drone->posl >= map->row || drone->posc < 0 || drone->posc >= map->col ){
        switch (drone->direction) {
        case EST:
            drone->direction = OUEST;
            break;
        case OUEST:
            drone->direction =  EST; 
            break;
        case SUD:
            drone->direction = NORD ; 
            break;
        case NORD:
            drone->direction = SUD ; 
            break;
    }
    }

    if (drone->posl < 0 || drone->posl >= map->row || drone->posc < 0 || drone->posc >= map->col ||
        map->grid[drone->posl][drone->posc] == 1 ) {
        // Collision detected
        drone->posl = drone->pl_old;
        drone->posc = drone->pc_old;
         
    } else {
        map->grid[drone->pl_old][drone->pc_old] = 0; // Mark old position as free space
        map->grid[drone->posl][drone->posc] = 2; // Assuming 1 represents the drone
    }

    // Update the previous position in the drone structure
    drone->pl_old = drone->posl;
    drone->pc_old = drone->posc;
}

void executer_commandes(drone_t *drone, carte_t *map) {
    char *commands = drone->command;

    for (int i = 0; commands[i] != '\0'; ++i) {
        executer_commande(drone, map, commands[i]);
        printf("Intermediate Drone Position: (%d, %d) and Direction %d \n", drone->posl, drone->posc, drone->direction);
        afficher_map(map, drone);
        //change sleep time
        sleep(1);
    }
}

void executer_commande(drone_t* drone, carte_t* map, char commande) {
    switch (commande) {
        case 'L':
            drone->direction = (drone->direction + 3) % 4;
            break;
        case 'R':
            drone->direction = (drone->direction + 1) % 4;
            break;
        case 'M':
            deplacer_drone(drone, map);
            break;
        case 'B':
            reculer_drone(drone, map);
            break;
    }
}

void generer_map_vc(carte_t *map, double probability, drone_t *drones, int num_drones){
 // Assuming that 0 means walkway , 1 is a wall , and 2 is drone
    

    srand(time(NULL));
    map->grid = (int **)malloc(map->row * sizeof(int *));
    for (int i = 0; i < map->row; ++i) {
        map->grid[i] = (int *)malloc(map->col * sizeof(int));
    }

    // Initialize all cells as walls
    for (int i = 0; i < map->row; ++i) {
        for (int j = 0; j < map->col; ++j) {
            map->grid[i][j] = 1; 
        }
    }

    for (int i = 0; i < map->row; ++i) {
        for (int j = 0; j < map->col; ++j) {
            int isDronePosition = 0;
            // Check if drone position
            for (int k = 0; k < num_drones; ++k) {
                if (drones[k].posl == i && drones[k].posc == j) {
                    isDronePosition = 1;
                    map->grid[i][j]  = 2 ;
                    break;
                }
                
            }
            if (!isDronePosition) {
                double random_value = (double)rand() / RAND_MAX;
                map->grid[i][j] = (random_value < probability) ? 1 : 0;
            }
        }
    }
}

void version_seq() {
    const char *filename = "commande_drone.txt";
    int num_drones = 0;
    carte_t carte;
    drone_t *drones;

    parse_config(filename, &carte, &drones, &num_drones);

    generer_map(&carte, 0.1, drones, num_drones);
    
    
    for (int i = 0; i < num_drones; i++) {
        carte.grid[drones[i].posl][drones[i].posc]  = 2 ; 
        printf("################################NEW DRONE################################\n");
        printf("Initial drone %d position is  %d %d %c\n",i, drones[i].posl, drones[i].posc, directionToString(drones[i].direction));
        afficher_map(&carte, &drones[i]);
        // ca plus grand 
        sleep(2);
        executer_commandes(&drones[i], &carte);
        printf("Final drone %d position is %d %d %c\n",i, drones[i].posl, drones[i].posc, directionToString(drones[i].direction));
        carte.grid[drones[i].posl][drones[i].posc] = 0 ; 
    }

    // Free allocated memory
    //free_drones(drones, num_drones);
}


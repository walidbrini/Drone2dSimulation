
extern carte_t carte;
extern drone_t *drones;
extern int num_drones;


void* execute_drone(void* arg);
void* display_map(void* arg);
void* execute_drone_socket(void* arg) ; 
void sendDrones();

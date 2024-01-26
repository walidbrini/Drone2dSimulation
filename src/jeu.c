#include "jeu.h"
#include <termios.h>
#include <unistd.h>

int main() {
    int version = -1 ; 
    char nom_fichier[40]; 

    printf("Choisir quel version de simulateur à executer 1:Simple 2:Concurrente 3:Concurrente+Server \n "); 
    scanf("%ds", &version); 
    if (version == 1){ 
    version_seq();
    }
    else if (version == 2){ 
    printf("Donner le nom du fichier : ");
    scanf("%s",nom_fichier);
    version_concurrente(nom_fichier);
    }
    else if (version == 3 ){ 
        run_server();
    }
}

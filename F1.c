#include "header.h"
#include <errno.h>



//****** SIMULATION VOITURE


sem_t mutex;            // Pour synchroniser l'accès à la sortie
sem_t mutlect;          // Pour indiquer si une voiture écrit
int red_count = 0;  // Nombre de lecteurs en cours

void simulerVoiture(Voiture *voiture, int minT, int maxT, int lgSect, int lgSectRef) {


    voiture->status = 0; // En course
    voiture->tour = 0;   // Premier tour
    voiture->tempTotal = 0;
    voiture->stand = 0;
    voiture->out = 0;
    voiture->bestLap = 0;

    for (int i = 0; i < 3; i++) {
        voiture->secteur[i] = 0;
        voiture->bestSecteur[i] = 0;
    }
      
    while(voiture->tempTotal < TEMPS_COURSE*100500){ //tant que la voiture a pas fini son temsps de course
        
        sem_wait(&mutex); //les red attendent que le père lise pas
        
        red_count++; //incrémente le nbr car peuvent ecrire à plsueiurs

        if(red_count == 1){ //une fois que le premier est entré , le lecteur peuvent plus lire
            sem_wait(&mutlect);
        }

        sem_post(&mutex); //les red sont reveillées

///////////////////////////////////////////////////////////////////////////////////
// SECTION CRITIQUE


        int random = rand() % 100;
        voiture->tour=voiture->tour+1; // Mise à jour du tour actuel

        // Déterminer le statut de la voiture
        if (random < 2 || (voiture->tour > 0 && voiture->out == 1)) {
            voiture->status = 2; // Crash
            voiture->out = 1;
        } else if (random < 25) {
            voiture->status = 1; // Arrêt au stand
            voiture->stand = 1;
        } else {
            voiture->status = 0; // En course
            voiture->stand = 0;
        }

        // Simulation des secteurs
        int lapTime = 0;
        for (int secteur = 0; secteur < 3; secteur++) {
            if (voiture->out == 0) { // Si la voiture n'est pas hors course
                voiture->secteur[secteur] = genererTempsSecteur(minT, maxT, lgSect, lgSectRef);
                lapTime += voiture->secteur[secteur];

                // Mettre à jour le meilleur temps de ce secteur
                if (voiture->bestSecteur[secteur] == 0 || voiture->secteur[secteur] < voiture->bestSecteur[secteur]) {
                    voiture->bestSecteur[secteur] = voiture->secteur[secteur];
                }
            } else {
                voiture->secteur[secteur] = 0; // Pas de temps enregistré
            }
        }

        // Ajout du temps d'arrêt au stand s'il y a un arrêt
        if (voiture->stand == 1) {
            int pitTime = genererTempsSecteur(MIN_TEMPS_PIT, MAX_TEMPS_PIT, lgSect, lgSectRef);
            lapTime += pitTime;}

        voiture->tempTotal += lapTime; // Mise à jour du temps total
        //printf("id voidture %d et la e temps total %d\n", voiture->num, voiture->tempTotal);

        // Mise à jour du meilleur tour
        if (voiture->bestLap == 0 || (voiture->out == 0 && lapTime < voiture->bestLap)) {
            voiture->bestLap = lapTime;
        }

//////////////////////////////////////////////////////////////////////////////////
        sem_wait(&mutex); // le red fini 
        red_count--;// décrémente 

        if(red_count == 0){sem_post(&mutlect);} //si il y a plus de red , le lecteur se réveille

        sem_post(&mutex);  // les red peuvent se réveiller

        sleep(1);
    }
}




//****** SIMULATION TOURS ET QUALIFICATIONS



void simulateQualification(Voiture *voitures, int nbrVOiture, int *voituresRestantes, int t){
    *voituresRestantes = nbrVOiture;

    for(int qualif = 0; qualif < 3; qualif ++){
        printf("Début de D%d...\n", qualif + 1);

        //afficherTableau(voitures, sizeof(Voiture), t);

        if (qualif < 2) elimination(voitures, voituresRestantes);

        printf("Q%d terminé. Voitures restantes : %d\n\n", qualif + 1, *voituresRestantes);

        if (*voituresRestantes <= 10) break;
        
    }

    printf("Qualification terminée. Classement final établi pour le Grand Prix.\n");

}



//****** MAIN



int main(int argc, char *argv[]) {

    printf("\x1b[8;24;110t"); //PAS SUPPRIMER!!! defini la taille de la fenetre du terminal

////////////////////// SEMAPHORE

    sem_init(&mutex, 0, 1);    // Mutex pour protéger la section critique
    sem_init(&mutlect, 0, 1);  // Mutex pour contrôler l'accès exclusif en écriture

//////////////////// arguments

    if (argc < 7) {
        printf("Usage: %s --type <essaie|qualif> --longueur <longueur_circuit> --jour <jour>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *type_course = NULL;
    int longueur_circuit = 0;
    char *jour = NULL;

    // Lire les arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--type") == 0 && i + 1 < argc) {
            type_course = argv[i + 1];
        } else if (strcmp(argv[i], "--longueur") == 0 && i + 1 < argc) {
            longueur_circuit = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "--jour") == 0 && i + 1 < argc) {
            jour = argv[i + 1];
        }
    }

    if (!type_course || !longueur_circuit || !jour) {
        printf("Erreur : Tous les arguments (--type, --longueur, --jour) doivent être fournis.\n");
        return EXIT_FAILURE;
    }

///////////////////////// INITIALISATION

    // Déterminer le nombre de voitures
    //int NBR_VOITURES = (strcmp(type_course, "qualif") == 0) ? 20 : 5;

    printf("Configuration : Type de course = %s, Longueur du circuit = %d, Jour = %s\n",
           type_course, longueur_circuit, jour);

    key_t key = ftok("f1", 'X'); // Générer une clé unique
    if (key == -1) {
        perror("Erreur de génération de la clé mémoire partagée");
        return EXIT_FAILURE;
    }
    // Création de la mémoire partagée pour les voitures
    int shmid = shmget(key, NBR_VOITURES_MAX * sizeof(Voiture), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("Erreur de création de la mémoire partagée pour les voitures");
        return EXIT_FAILURE;
    }

    Voiture *shm = (Voiture *)shmat(shmid, NULL, 0); //atatchement de la memoire au père
    if (shm == (void *)-1) {
        perror("Erreur d'attachement à la mémoire partagée pour les voitures");
        return EXIT_FAILURE;
    }




//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&



int shmId = shmget(key, 0, 0);  // Essayer de récupérer un segment existant
if (shmId == -1) {
    if (errno == ENOENT) {
        printf("Clé %d n'est pas utilisée.\n", key);
    } else {
        perror("Erreur lors de la vérification de la clé");
    }
} else {
    printf("Clé %d est déjà utilisée par le segment ID %d.\n", key, shmId);
}


////////////////////// INIT BEST

    key_t keyBest = ftok("f1_best", 1);
    if (keyBest == -1) {
    perror("Erreur de génération de la clé mémoire partagée pour les meilleurs temps");
    return EXIT_FAILURE;
    }

    // Création de la mémoire partagée pour les meilleurs temps
    int shmidBest = shmget(keyBest, sizeof(Best), IPC_CREAT | 0666);
    if (shmidBest == -1) {
        perror("Erreur de création de la mémoire partagée pour les meilleurs temps");
        shmdt(shm);
        shmctl(shmid, IPC_RMID, NULL);
        return EXIT_FAILURE;
    }

    Best *madame = (Best *)shmat(shmidBest, NULL, 0);
    if (madame == (void *)-1) {
        perror("Erreur d'attachement à la mémoire partagée pour les meilleurs temps");
        shmdt(shm);
        shmctl(shmid, IPC_RMID, NULL);
        return EXIT_FAILURE;
    }
    

    initialiserVoitures(shm, NBR_VOITURES);

///////////////////////////////// VARIABLES

    srand(time(NULL));
    int vId = 0;
    pid_t pid ;
    int voituresRestantes;



////////////////////////////// FORK VOITURES REDACETEUR

    for (vId = 0; vId < NBR_VOITURES; vId++) {
        pid = fork();
        
        if (pid == 0) {  // Processus fils (voiture)
            // Initialisation du générateur aléatoire pour ce processus fils
            srand(time(NULL) ^ getpid());  

            // Obtenir un pointeur vers la voiture correspondante dans la mémoire partagée
            Voiture *voiture = &shm[vId];
            simulerVoiture(voiture, MIN_TEMPS_SECTEUR, MAX_TEMPS_SECTEUR, TOTAL_PARC, TOTAL_PARC);

            _exit(0);  // Terminer le processus fils proprement
        } 
        else if (pid < 0) {  // Gestion d'une erreur de fork
            perror("Erreur lors du fork");
            return EXIT_FAILURE;
        }
    }

//parking




/////////////////// PERE LECTEUR

    if (pid > 0) {



           //sem_wait(&mutlect);  // Attendre qu'il n'y ait pas d'écrivain actif

////////////////////////////////////////////////
//SECTION CRITIQUE
            
            if (strcmp(type_course, "essaie") == 0) {
                printf("Début des essais libres...\n");
                time_t start = time(NULL);
                time_t finish = start + 13;
                while (time(NULL) < finish) {

                    afficherTableau(shm, NBR_VOITURES,&mutlect);  
                    
                sleep(1);          
            }







////////////////////////////////////////////////
            //sem_post(&mutlect);  // Permettre à une voiture d'écrire à nouveau

            sleep(1);  // Le père attend un peu avant de lire à nouveau
        }
    }


    /* } else if (strcmp(type_course, "qualif") == 0) {
                printf("Début des qualifications...\n");
                while(1){
                for (int t = 0; t < NBR_TOUR; t++){
        
                    
                        simulateQualification(copie, NBR_VOITURES, &voituresRestantes, t); // Effectuer la simulation
                        sleep(2); 

        }
             
    }
*/

    memset(shm, 0, sizeof(Voiture) * NBR_VOITURES); // Remettre à zéro les données de voiture

    // Détachement et suppression de la mémoire partagée
    shmdt(shm);
    shmctl(shmid, IPC_RMID, NULL);

    shmdt(madame);
    shmctl(keyBest, IPC_RMID, NULL);

    sem_close(&mutex);
    sem_close(&mutlect);

    sem_unlink("/mutex");
    sem_unlink("/mutlect");
    return EXIT_SUCCESS;
}


#include "header.h"

//****** UTILITAIRES

long getMin(long a, long b, long c) {
    long min = a;
    if (b < min) min = b;
    if (c < min) min = c;
    return min;
}

//****** FONCTION TEMPS

long genererTempsSecteur(int min, int max, int longueurSecteur, int longueurSecteurRef) {
    int tempsBase = min + rand() % (max - min + 1);
    return (long)tempsBase * longueurSecteur / longueurSecteurRef;
}

//****** FONCTION ETATS VOITURES

char genererEtatVoiture() {
    int random = rand() % 100;
    if (random < 15) return 'O';
    if (random < 25) return 'P';
    return 'R';
}

//****** SIMULATION VOITURE

void simulerVoiture(Voiture *voiture, int minT, int maxT, int lgSect, int lgSectRef) {
    for (int t = 0; t < NBR_TOUR; t++) {

        // Mise à jour de l'état (aléatoire pour cet exemple)
        int random = rand() % 100;
        if (random < 7 || (t > 0 && voiture->tour[t-1].etat == 'O') ) {
            voiture->tour[t].etat = 'O'; // Abandon
        } else if (random < 35) {
            voiture->tour[t].etat = 'P'; // Aux stands
        } else {
            voiture->tour[t].etat = 'R'; // En course
        }

        voiture->tour[t].tempsTotal = 0;
        long tempsSecteur = 0; // Déclaré ici pour être visible partout dans la boucle

        for (int secteur = 1; secteur <= 3; secteur++) {

            tempsSecteur = 0;
            if (voiture->tour[t].etat != 'O' )tempsSecteur = genererTempsSecteur(minT, maxT, lgSect, lgSectRef);
            
            if (secteur == 1) voiture->tour[t].secteur1.temps = tempsSecteur;
            if (secteur == 2) voiture->tour[t].secteur2.temps = tempsSecteur;
            if (secteur == 3) {
                voiture->tour[t].secteur3.temps = tempsSecteur;
                if (voiture->tour[t].etat == 'P') {
                    voiture->tour[t].secteur3.temps = genererTempsSecteur(MIN_TEMPS_PIT, MAX_TEMPS_PIT, lgSect, lgSectRef);
                } 
            }

            voiture->tour[t].tempsTotal += tempsSecteur;
            // printf("the best for auto %d: %ld\n",voiture->numero, tempsSecteur);

            // Met à jour le meilleur temps de secteur
            if (voiture->tour[t].meilleurTempsSecteur == 0 || tempsSecteur < voiture->tour[t].meilleurTempsSecteur) {
                printf("the best : %ld\n", tempsSecteur);
                voiture->tour[t].meilleurTempsSecteur = tempsSecteur;
            }
        }

    }
}


//****** SIMULATION TOURS ET QUALIFICATIONS

void simulateLaps(Voiture *voitures, int nbrVoitures, int timeLimit) {
    time_t startTime = time(NULL);

    while (time(NULL) - startTime < timeLimit) {
        for (int i = 0; i < nbrVoitures; i++) {
            if (voitures[i].meilleurTour.etat != 'O') { // Simule seulement pour les voitures en course
                simulerVoiture(&voitures[i], MIN_TEMPS_SECTEUR, MAX_TEMPS_SECTEUR, TOTAL_PARC, TOTAL_PARC);
            }
        }
    }
}


void simulateQualification(Voiture *voitures, int nbrVoitures) {
    int voituresRestantes = nbrVoitures;

    // Phase Q1 : Élimination des 5 dernières voitures
    printf("Début de Q1...\n");
    simulateLaps(voitures, voituresRestantes, Q1_TIME_LIMIT);
    printf("Résultats après Q1 :\n");
    afficherTableau(voitures, voituresRestantes, 0); // Afficher les résultats après Q1
    elimination(voitures, &voituresRestantes, 5);
    printf("Q1 terminé. Voitures restantes : %d\n\n", voituresRestantes);

    // Phase Q2 : Élimination des 5 dernières voitures restantes
    if (voituresRestantes > 0) {
        printf("Début de Q2...\n");
        simulateLaps(voitures, voituresRestantes, Q2_TIME_LIMIT);
        printf("Résultats après Q2 :\n");
        afficherTableau(voitures, voituresRestantes, 1); // Afficher les résultats après Q2
        elimination(voitures, &voituresRestantes, 5);
        printf("Q2 terminé. Voitures restantes : %d\n\n", voituresRestantes);
    }

    // Phase Q3 : Classement final des 10 meilleures voitures
    if (voituresRestantes > 0) {
        printf("Début de Q3...\n");
        simulateLaps(voitures, voituresRestantes, Q3_TIME_LIMIT);
        printf("Résultats après Q3 :\n");
        afficherTableau(voitures, voituresRestantes, 2); // Afficher les résultats après Q3
        printf("Q3 terminé. Classement final établi pour le Grand Prix.\n");
    }
}

//****** MAIN


#include "header.h"

// Vérification des tailles maximales permises pour la mémoire partagée
#define MAX_SHM_SIZE 65536 // Exemple : 64 KB (modifiable selon vos besoins)

void cleanupSharedMemory(int shmid, void *addr) {
    if (addr != (void *)-1) {
        shmdt(addr);
    }
    if (shmid != -1) {
        shmctl(shmid, IPC_RMID, NULL);
    }
}

int main(int argc, char *argv[]) {
    printf("\x1b[8;24;110t"); // Définir la taille de la fenêtre du terminal

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

    // Déterminer le nombre de voitures
    int nbrVoitures = (strcmp(type_course, "qualif") == 0) ? 20 : 5;

    printf("Configuration : Type de course = %s, Longueur du circuit = %d, Jour = %s\n",
           type_course, longueur_circuit, jour);

    // Calculer la taille nécessaire pour la mémoire partagée
    size_t pageSize = sysconf(_SC_PAGESIZE);
    size_t tailleVoitures = ((NBR_TOUR * NBR_VOITURES_MAX * sizeof(Voiture) + pageSize - 1) / pageSize) * pageSize;

    printf("Taille mémoire alignée demandée : %zu octets\n", tailleVoitures);

    key_t key = ftok("f1", 1); // Générer une clé unique
    if (key == -1) {
        perror("Erreur de génération de la clé mémoire partagée");
        return EXIT_FAILURE;
    }

    int shmid = shmget(key, tailleVoitures, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("Erreur de création de la mémoire partagée pour les voitures");
        return EXIT_FAILURE;
    }

    Voiture *shm = (Voiture *)shmat(shmid, NULL, 0);
    if (shm == (void *)-1) {
        perror("Erreur d'attachement à la mémoire partagée pour les voitures");
        cleanupSharedMemory(shmid, NULL);
        return EXIT_FAILURE;
    }

    // Initialiser les voitures dans la mémoire partagée
    initialiserVoitures(shm, nbrVoitures);

    key_t keyBest = ftok("f1_best", 1);
    if (keyBest == -1) {
    perror("Erreur de génération de la clé mémoire partagée pour les meilleurs temps");
    return EXIT_FAILURE;
    }

    size_t tailleBest = ((sizeof(Best) + pageSize - 1) / pageSize) * pageSize;

    printf("Clé mémoire partagée pour les meilleurs temps : %d\n", keyBest);
    printf("Taille alignée demandée pour Best : %zu octets\n", tailleBest);

    int shmidBest = shmget(keyBest, tailleBest, IPC_CREAT | 0666);
    if (shmidBest == -1) {
        perror("Erreur de création de la mémoire partagée pour les meilleurs temps");
        return EXIT_FAILURE;
    }

    Best *madame = (Best *)shmat(shmidBest, NULL, 0);
    if (madame == (void *)-1) {
        perror("Erreur d'attachement à la mémoire partagée pour les meilleurs temps");
        shmctl(shmidBest, IPC_RMID, NULL); // Nettoyer si nécessaire
        return EXIT_FAILURE;
    }

    srand(time(NULL));

    // Simulation en fonction du type de course
    for (int vId = 0; vId < nbrVoitures; vId++) {
        pid_t pid = fork();
        if (pid == 0) { // Processus enfant
            printf("Processus enfant créé pour la voiture %d (PID: %d)\n", vId, getpid());
            srand(time(NULL) ^ getpid());
            Voiture *voiture = &shm[vId];
            simulerVoiture(voiture, MIN_TEMPS_SECTEUR, MAX_TEMPS_SECTEUR, TOTAL_PARC, TOTAL_PARC);
            _exit(0);
        }
    }

    for (int i = 0; i < nbrVoitures; i++) {
        wait(NULL); // Attendre la fin des processus enfants
    }

    Voiture copie[NBR_VOITURES];
    memcpy(copie, shm, sizeof(Voiture) * NBR_VOITURES);

    if (strcmp(type_course, "essaie") == 0) {
        printf("Début des essais libres...\n");
        while (1) {
            for (int t = 0; t < NBR_TOUR; t++) {
                afficherTableau(copie, NBR_VOITURES, t);
                sleep(2);
            }
        }
    } else if (strcmp(type_course, "qualif") == 0) {
        printf("Début des qualifications...\n");
    }

    // Détachement et suppression de la mémoire partagée
    cleanupSharedMemory(shmid, shm);
    cleanupSharedMemory(shmidBest, madame);

    return EXIT_SUCCESS;
}

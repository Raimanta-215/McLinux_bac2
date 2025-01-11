#ifndef VARIABLE_H
#define VARIABLE_H

//****LIBRAIRIES

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h> // Pour utiliser INT_MAX
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h> //pour open
#include <unistd.h> //pour close et write
#include <string.h>
#include <stdbool.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <semaphore.h>
#include <float.h> // Pour FLT_MAX



//****CONSTANTES

#define NBR_VOITURES 5     // Nombre de voitures dans la course
#define NBR_TOUR 10       // Nombre de tours dans la course
#define MIN_TEMPS_SECTEUR 25000  // Temps minimum pour un secteur en millisecondes
#define MAX_TEMPS_SECTEUR 45000
#define MIN_TEMPS_PIT 2000 
#define MAX_TEMPS_PIT 8000

#define TOUR_MAX MAX_TEMPS_SECTEUR*3 // Temps maximum pour un secteur en millisecondes
#define TOTAL_PARC 305000        // Longueur totale du circuit
#define NOM_FICHIER "essaie-libre.csv"
#define MAX_TOURS 10  // Nombre maximal de tours
#define SHM_KEY_BEST 2121
#define SEM_KEY 54321
#define NBR_VOITURES_MAX 20 // Capacité maximale pour les qualifications


//qualif
#define Q1_TIME_LIMIT 10 //1080  // 18 minutes
#define Q2_TIME_LIMIT  5 //900   // 15 minutes
#define Q3_TIME_LIMIT  2 //720   // 12 minutes
#define NBR_ELIMINATIONS_Q1 5
#define NBR_ELIMINATIONS_Q2 5

//couleurs
#define ROUGE     "\x1b[31m"
#define VERT      "\x1b[32m"
#define BLEU      "\x1b[34m"
#define RESET     "\x1b[0m"


//*****STRUCTURES

// Structure pour stocker le temps d'un secteur
typedef struct {
    long temps;  // Temps en millisecondes pour le secteur
} Secteur;

// Structure pour stocker les informations de chaque tour
typedef struct {
    Secteur secteur1;            // Temps pour le secteur 1
    Secteur secteur2;            // Temps pour le secteur 2
    Secteur secteur3;            // Temps pour le secteur 3
    long tempsTotal;             // Temps total pour le tour (somme des trois secteurs)
    long meilleurTempsSecteur;   // Temps le plus rapide parmi les trois secteurs
    char etat;                   // État de la voiture pour ce tour : 'R' (En course), 'P' (Aux stands), 'O' (Abandon)
} Tour;

// Structure pour stocker les informations de chaque voiture
typedef struct {
    int numero;
    char nom[20];                    // Nom de la voiture (ex : 'A', 'B', 'C', ...)
    Tour tour[NBR_TOUR];         // Tableau de tours pour stocker les informations de chaque tour
    Tour meilleurTour;              // Informations du meilleur tour

} Voiture;


typedef struct {
    Voiture *voitures;
    int tour; // Numéro du tour à utiliser pour le tri
} ContexteTri;


typedef struct {
    Voiture meilleurSecteur1;
    Voiture meilleurSecteur2;
    Voiture meilleurSecteur3;
    Voiture meilleursTemps;
}   Best;


/*struct sembuf sem_wait_op = {0, -1, SEM_UNDO};

struct sembuf sem_signal_op = {0, 1, SEM_UNDO};*/

//*****DEFINITION FONCTIONS

//F1.c
long getMin(long a, long b, long c);
long genererTempsSecteur(int min, int max, int longueurSecteur, int longueurSecteurRef);
//long calculeEcart(Voiture voitures[], int rang);
char genererEtatVoiture();
int comparerVoitures(const void *a, const void *b);
void simulerVoiture(Voiture *voiture, int minT, int maxT, int lgSect, int lgSectRef);
/*void afficherTemps(int tempsEnMs);
void afficherEcartTemps(long ecartTemps);
void afficherResultats(Voiture voitures[], int nbrVoitures);
void afficherMeilleurTemps(Voiture *voitures, int *meilleursTemps, sem_t *sem, int nombreDeTours, int *flags);// Dans header.h
void mettreAJourMeilleursTemps(Best madame, Voiture *voiture, int semid);
void afficherResultatsTempsReel(Best madame);*/



//data.c
void enregistrerData(char *cell, char *nomFich);
char *convertiTemps(int tempsEnMms);
char* intToChar_deux(int nombre);
//void enregistrerTourVoiture(Voiture *voiture, int tourNum, char *nomFichier);
void ajouterEnTetesCSV(char *nomFichier);
void afficherTableau(Voiture *copie, int nbVoitures,int t);
void initialiserVoitures(Voiture *voitures, int nbrVoitures);

//void trierVoituresParTemps(Voiture *copie);
void youTheBest(Voiture *voitures);

void elimination(Voiture *voitures, int *nbrVoitures, int eliminations);
void sauvegarderResultatsQualification(Voiture *voitures, int nbrVoitures, char *nomFichier);








#endif // VARIABLE_H
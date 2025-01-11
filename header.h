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

#define NBR_VOITURES 10    // Nombre de voitures dans la course
#define NBR_TOUR 10  // Nombre de tours dans la course
#define MIN_TEMPS_SECTEUR 25000  // Temps minimum pour un secteur en millisecondes
#define MAX_TEMPS_SECTEUR 45000
#define TOUR_MAX MAX_TEMPS_SECTEUR*3 // Temps maximum pour un secteur en millisecondes
#define TOTAL_PARC 305000        // Longueur totale du circuit
#define NOM_FICHIER "essaie-libre.csv"
#define MAX_TOURS 10  // Nombre maximal de tours
#define SEM_KEY 5432
#define NBR_VOITURES_MAX 20 // Capacité maximale pour les qualifications

#define MIN_TEMPS_PIT 2000 
#define MAX_TEMPS_PIT 8000
#define FINALTOURS 45
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
typedef struct voiture{
    int status;             //status de la voiture : 0 = enCourse, 1 = stand , 2 =crach
    int num;     
    int secteur[3];         //Chaque Secteur
    int bestSecteur[3];     //Best temps pour chaque secteur (n° = index+1)
    int bestLap;            //Best Lap
    int tempTotal;          //Temps de course
    int tour;               //Numéro du tour actuel
    int stand;              //1 = au stand
    int out;                //1 = crach
    int probaStand;         //Probabilité d'un arrèt au stand
}Voiture;


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
char genererEtatVoiture();
void simulerVoiture(Voiture *voiture, int minT, int maxT, int lgSect, int lgSectRef);
void simulateQualification(Voiture *voitures, int nbrVoitures, int *voituresRestantes, int t);




//data.c
void enregistrerData(char *cell, char *nomFich);
char *convertiTemps(int tempsEnMms);
char* intToChar_deux(int nombre);
//void enregistrerTourVoiture(Voiture *voiture, int tourNum, char *nomFichier);
void ajouterEnTetesCSV(char *nomFichier);
void afficherTableau(Voiture *copie, int nbVoitures);
void initialiserVoitures(Voiture *voitures, int nbrVoitures);

//void trierVoituresParTemps(Voiture *copie);
void youTheBest(Voiture *voitures);

void elimination(Voiture *voitures, int *nbrVoitures);
void sauvegarderResultatsQualification(Voiture *voitures, int nbrVoitures, char *nomFichier);
int comparerVoituresParTour(const void *a, const void *b);








#endif // VARIABLE_H
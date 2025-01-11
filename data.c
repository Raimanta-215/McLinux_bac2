//*****fonction pour enregistrer les data dans un csv 

 #include "header.h" //appel pour utiliser mon deuxieme fichier 





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



void enregistrerData(char *cell, char *nomFich){
    
    
    int file = open(nomFich, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (file == -1)
    {
        printf("erreur ouverture");
        return ;
    }
    //alloue de la place pour ma chaine
    char *string = (char *)malloc(strlen(cell));
    strcpy(string, cell); //copie dans string ce qu'il y a dans cell
    strcat(string, ";"); //permet de faire des cellules individuelles
    //printf("%s", string);
    if(write(file, string , strlen(string)) == -1 ){
        printf("erreur de creation du csv");
    }
    free(string);
    close(file);

}

//converti en ascii pcq ca n'existe pas une fonction pour passer de int à char pour que je puisse enregistrer dans le csv
char *convertiTemps(int tempsEnMms){
   // printf("Temps avant conversion: %d\n", tempsEnMms);

    int minutes = tempsEnMms / 60000;
    int secondes = (tempsEnMms % 60000)/ 1000;
    int milliemes = tempsEnMms % 1000;
    char *res = (char *)malloc(12 * sizeof(char));
     //converti temps int to char
    res[0] = (minutes / 10) + '0';
    res[1] = (minutes % 10) + '0';
    res[2] = ':';
    res[3] = (secondes / 10) + '0';
    res[4] = (secondes % 10) + '0';
    res[5] = ':';
    res[6] = (milliemes / 100) + '0';
    res[7] = ((milliemes / 10 )% 10) + '0';
    res[8] = (milliemes % 10) + '0';
    res[9] = '\0';

    return (res);
}

char* intToChar_deux(int nombre){

    static char var[3]; //taille des num de voitures plus le \0
        var[0] = (nombre / 10) + '0'; //conerti premier chiffre
        var[1] = (nombre % 10) + '0'; //converrti deuxieme
        var[2] = '\0';

    return var;
}








void ajouterEnTetesCSV(char *nomFichier) {
    enregistrerData(";Voiture; Tour; TempsTour; MeilleurSecteur; Secteur 1; Secteur 2; Secteur 3;\n", nomFichier);
}

int comparerVoituresParTour(const void *a, const void *b) {
    // ContexteTri n'est plus nécessaire car `tour` est directement utilisé dans chaque voiture
    const Voiture *voitureA = (const Voiture *)a;
    const Voiture *voitureB = (const Voiture *)b;

    // Comparer les temps totaux
    return voitureA->tempTotal - voitureB->tempTotal;
}



void elimination(Voiture *voitures, int *nbVoitures) {
    // Trier les voitures par temps total (croissant) en utilisant un tri à bulles
    for (int i = 0; i < *nbVoitures - 1; i++) {
        for (int j = 0; j < *nbVoitures - i - 1; j++) {
            if (voitures[j].tempTotal > voitures[j + 1].tempTotal) {
                // Échanger les voitures
                Voiture temp = voitures[j];
                voitures[j] = voitures[j + 1];
                voitures[j + 1] = temp;
            }
        }
    }

    // Réduire le nombre de voitures restantes après l'élimination
    *nbVoitures = (*nbVoitures > 5) ? *nbVoitures - 5 : 0;
}

void afficherTableau(Voiture *copie, int nbVoitures) {
    //system("clear");

    qsort(copie, nbVoitures, sizeof(Voiture), comparerVoituresParTour);

    // Identifier le leader pour les écarts
    int tempsLeader = copie[0].tempTotal;

    // Affichage du tableau
    printf("╔════╦═══════════╦════════╦═════════════╦══════════════════╦════════╦════════════════╗\n");
    printf("║Rang║ Numéro    ║ Tour   ║ Temps Total ║ Meilleur Secteur ║  État  ║ Écart de temps ║\n");
    printf("╠════╬═══════════╬════════╬═════════════╬══════════════════╬════════╬════════════════╣\n");

    for (int i = 0; i < nbVoitures; i++) {
        // Convertir les temps pour l'affichage
        char *tempsTotalStr = convertiTemps(copie[i].tempTotal);
        char *meilleurSecteurStr = convertiTemps(
            copie[i].bestSecteur[0] < copie[i].bestSecteur[1] && copie[i].bestSecteur[0] < copie[i].bestSecteur[2]
            ? copie[i].bestSecteur[0]
            : (copie[i].bestSecteur[1] < copie[i].bestSecteur[2]
               ? copie[i].bestSecteur[1]
               : copie[i].bestSecteur[2]));

        int ecartTemps = copie[i].tempTotal - tempsLeader;
        char *ecartStr = convertiTemps(ecartTemps);

        char *etatStr;
            switch (copie[i].status) {
                case 0:
                    etatStr = "RUN";
                    break;
                case 1:
                    etatStr = "PIT";
                    break;
                case 2:
                    etatStr = "OUT";
                    break;
                default:
                    etatStr = "IDK";
            }


        printf("║ %2d ║ %-9d ║ %6d ║ %11s ║ %16s ║ %-6s ║ %12s ║\n",
               i + 1,
               copie[i].num,
               copie[i].tour,
               tempsTotalStr,
               meilleurSecteurStr,
               etatStr,
               ecartStr);

        // Libération de mémoire
        free(tempsTotalStr);
        free(meilleurSecteurStr);
        free(ecartStr);
    }

    printf("╚════╩═══════════╩════════╩═════════════╩══════════════════╩════════╩════════════════╝\n");

}





void sauvegarderResultatsQualification(Voiture *voitures, int nbrVoitures, char *nomFichier) {
    // Ajouter les en-têtes au fichier CSV
    ajouterEnTetesCSV(nomFichier);

    // Sauvegarder les données des voitures
    for (int i = 0; i < nbrVoitures; i++) {
        char data[256];
        sprintf(data, "%d;%d;%d;%d;%d;%d;%d;%d\n",
                i + 1,                        // Position
                voitures[i].num,              // Numéro de la voiture
                voitures[i].tempTotal,        // Temps total de la voiture
                voitures[i].secteur[0],       // Temps secteur 1
                voitures[i].secteur[1],       // Temps secteur 2
                voitures[i].secteur[2],       // Temps secteur 3
                voitures[i].bestLap,          // Meilleur temps au tour
                voitures[i].status);          // Statut de la voiture
        enregistrerData(data, nomFichier);
    }
}


void initialiserVoitures(Voiture *voitures, int nbrVoitures) {
    int numeros[] = {1, 55, 16, 44, 4, 7, 63, 11, 14, 31, 18, 22, 20, 23, 24, 10, 27, 21, 77, 81};

    for (int i = 0; i < nbrVoitures; i++) {
        // Initialisation des numéros
        voitures[i].num = numeros[i];

    }
}

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




int comparerVoituresParTour(const void *a, const void *b) {
    const Voiture *voitureA = (const Voiture *)a;
    const Voiture *voitureB = (const Voiture *)b;

    // Si une voiture est "OUT", elle est considérée comme ayant un temps maximal
    if (voitureA->status == 2 && voitureB->status != 2) return 1;
    if (voitureB->status == 2 && voitureA->status != 2) return -1;

    // Comparer les temps totaux pour les autres cas
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


Voiture copie[NBR_VOITURES];


/////////////////////////////////////////////////////////////////:::::

//AFFICHAGE

void afficherTableau(Voiture *voiture, int nbVoitures, sem_t *mutlect) {
    system("clear");


    sem_wait(mutlect);
    memcpy(copie, voiture, sizeof(Voiture)*NBR_VOITURES);
    sem_post(mutlect);


    qsort(copie, nbVoitures, sizeof(Voiture), comparerVoituresParTour);



    // Identifier le leader pour les écarts
    int tempsLeader = copie[0].secteur[0] + copie[0].secteur[1] + copie[0].secteur[2];

    // Affichage du tableau
    printf("╔════╦═══════════╦════════╦═════════════╦══════════════════╦════════╦════════════════╗\n");
    printf("║Rang║ Numéro    ║ Tour   ║ Temps Total ║ Meilleur Secteur ║  État  ║ Écart de temps ║\n");
    printf("╠════╬═══════════╬════════╬═════════════╬══════════════════╬════════╬════════════════╣\n");

    for (int i = 0; i < nbVoitures; i++) {
        // Calcul du temps pour un tour à partir des secteurs
        int tempsLap = copie[i].secteur[0] + copie[i].secteur[1] + copie[i].secteur[2];
        char *tempsLapStr;

        // Convertir les temps pour l'affichage
        char *meilleurSecteurStr = convertiTemps(
            copie[i].bestSecteur[0] < copie[i].bestSecteur[1] && copie[i].bestSecteur[0] < copie[i].bestSecteur[2]
            ? copie[i].bestSecteur[0]
            : (copie[i].bestSecteur[1] < copie[i].bestSecteur[2]
               ? copie[i].bestSecteur[1]
               : copie[i].bestSecteur[2]));

        int ecartTemps = tempsLap - tempsLeader;
        char *ecartStr;

        if (copie[i].status == 2) {  // OUT
            tempsLapStr = strdup("--:--:---");
            ecartStr = strdup("--:--:---");
        } else {
            tempsLapStr = convertiTemps(tempsLap);
            int ecartTemps = tempsLap - tempsLeader;
            ecartStr = convertiTemps(ecartTemps);
        }


        char *etatStr = (copie[i].status == 0) ? "RUN" :
                        (copie[i].status == 1) ? "PIT" : "OUT";


        printf("║ %s%2d%s ║ %-9d ║ %6d ║ %11s ║ %16s ║ %s%-6s%s ║ %12s   ║\n",
               ROUGE,i + 1,RESET,
               copie[i].num,
               copie[i].tour,
               tempsLapStr,
               meilleurSecteurStr,
               VERT,etatStr,RESET,
               ecartStr);

        // Libération de mémoire
        free(tempsLapStr);
        free(meilleurSecteurStr);
        free(ecartStr);
    }

    printf("╚════╩═══════════╩════════╩═════════════╩══════════════════╩════════╩════════════════╝\n");




}




void initialiserVoitures(Voiture *voitures, int nbrVoitures) {
    int numeros[] = {1, 55, 16, 44, 4, 7, 63, 11, 14, 31, 18, 22, 20, 23, 24, 10, 27, 21, 77, 81};

    for (int i = 0; i < nbrVoitures; i++) {
        // Initialisation des numéros
        voitures[i].num = numeros[i];

    }
}

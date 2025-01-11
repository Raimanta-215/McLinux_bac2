//*****fonction pour enregistrer les data dans un csv 

 #include "header.h" //appel pour utiliser mon deuxieme fichier 


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

int comparerVoituresParTour(const void *a, const void *b, void *contexte) {
    const ContexteTri *ctx = (const ContexteTri *)contexte;
    const Voiture *voitureA = (const Voiture *)a;
    const Voiture *voitureB = (const Voiture *)b;

    int t = ctx->tour; // Récupération du numéro du tour
    return voitureA->tour[t].tempsTotal - voitureB->tour[t].tempsTotal;
}


void ajouterEnTetesCSV(char *nomFichier) {
    enregistrerData(";Voiture; Tour; TempsTour; MeilleurSecteur; Secteur 1; Secteur 2; Secteur 3;\n", nomFichier);
}

void elimination(Voiture *voitures, int *nbrVoitures, int eliminations) {
    // Limitez les éliminations au nombre de voitures restantes
    if (eliminations > *nbrVoitures) {
        eliminations = *nbrVoitures;
    }

    // Tri des voitures par leur temps total (meilleurs temps en haut)
    for (int i = 0; i < *nbrVoitures - 1; i++) {
        for (int j = i + 1; j < *nbrVoitures; j++) {
            if (voitures[i].meilleurTour.tempsTotal > voitures[j].meilleurTour.tempsTotal) {
                Voiture temp = voitures[i];
                voitures[i] = voitures[j];
                voitures[j] = temp;
            }
        }
    }

    // Marquez les voitures éliminées comme "Out" (état = 'O')
    for (int i = *nbrVoitures - eliminations; i < *nbrVoitures; i++) {
        voitures[i].meilleurTour.etat = 'O';  // Out
    }

    // Réduisez le nombre de voitures restantes
    *nbrVoitures -= eliminations;

    // Assurez-vous que nbrVoitures ne devienne pas négatif
    if (*nbrVoitures < 0) {
        *nbrVoitures = 0;
    }
}

void afficherTableau(Voiture *copie, int nbVoitures, int t) {
    system("clear");

    // Préparer le contexte pour le tri
    ContexteTri contexte = {copie, t};

    // Trier les voitures en fonction du temps total pour le tour `t`
    qsort_r(copie, nbVoitures, sizeof(Voiture), comparerVoituresParTour, &contexte);

    // Identifier le leader pour les écarts
    long tempsLeader = copie[0].tour[t].tempsTotal;

    // Affichage du tableau
    printf("╔════╦═══════════╦════════╦═════════════╦══════════════════╦═══════╦════════════════╗\n");
    printf("║Rang║ Nom       ║ Numéro ║ Temps Total ║ Meilleur Secteur ║ État  ║ Écart de temps ║\n");
    printf("╠════╬═══════════╬════════╬═════════════╬══════════════════╬═══════╬════════════════╣\n");

    for (int i = 0; i < nbVoitures; i++) {
        char *tempsTotalStr = convertiTemps(copie[i].tour[t].tempsTotal);
        char *meilleurSecteurStr = convertiTemps(copie[i].tour[t].meilleurTempsSecteur);
        long ecartTemps = copie[i].tour[t].tempsTotal - tempsLeader;
        char *ecartStr = convertiTemps(ecartTemps);

        // Affichage de chaque ligne
        printf( "║ %2d ║ %-9s ║ %6d ║ %11s ║ %16s ║   %s%1c%s   ║ %s      ║\n",
                i + 1,
                copie[i].nom,
                copie[i].numero,
                tempsTotalStr,
                meilleurSecteurStr,
                VERT, copie[i].tour[t].etat, RESET,
                ecartStr);

        // Libération de mémoire
        free(tempsTotalStr);
        free(meilleurSecteurStr);
        free(ecartStr);
    }

    printf("╚════╩═══════════╩════════╩═════════════╩══════════════════╩═══════╩════════════════╝\n");
}


void sauvegarderResultatsQualification(Voiture *voitures, int nbrVoitures, char *nomFichier) {
    ajouterEnTetesCSV(nomFichier);

    for (int i = 0; i < nbrVoitures; i++) {
        char data[128];
        sprintf(data, "%d;%s;%ld;%ld;%ld;%ld\n",
                i + 1,  // Position
                voitures[i].nom,
                voitures[i].meilleurTour.tempsTotal,
                voitures[i].tour[0].secteur1.temps,
                voitures[i].tour[0].secteur2.temps,
                voitures[i].tour[0].secteur3.temps);
        enregistrerData(data, nomFichier);
    }
}


void initialiserVoitures(Voiture *voitures, int nbrVoitures) {
    int numeros[] = {1, 55, 16, 44, 4, 7, 63, 11, 14, 31, 18, 22, 20, 23, 24, 10, 27, 21, 77, 81};
    const char *noms[] = {
        "Max", "Carlos", "Charles", "Lewis", "Lando", "Kimi", "George", "Sergio",
        "Fernando", "Esteban", "Sebastian", "Daniel", "Pierre", "Alex", "Yuki",
        "Nico", "Kevin", "Liam", "Mick", "Logan"
    };

    for (int i = 0; i < nbrVoitures; i++) {
        voitures[i].numero = numeros[i];
        strncpy(voitures[i].nom, noms[i], sizeof(voitures[i].nom) - 1);
        voitures[i].nom[sizeof(voitures[i].nom) - 1] = '\0';

    }

    
}

/*
 * Sujet n� 2
 * Compiler avec -DTRACE_THD pour obtenir les traces sur les threads
 * Rappel : vous ne pouvez apportez des modifications au code
 *          que dans les zones prevues a cet effet
*/

/* Afin d'assurer un parallélisme maximal on peut créer un moniteur de Hoare par type de places disponibles, en effet, mettre en exclusions mutuelles des ressources bien distinctes est une perte de temps
 * 2 spectateur peuvent demander 2 types de place distincts en même temps (càd sans situation de compétition entre les deux).
 *
 *
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

typedef struct {
  int numSpectateur;
  int nbReservations;
} Parametre;

#define NB_SPECT_MAX       20
#define NB_PLACES_MAX     100
#define NB_RESA_MAX        20
#define NB_CATEG_PLC_MAX   10


/********* Debut zone de declaration de vos variable globales ******/
/********* et des fonctions eventuelles utiles            **********/
//

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condResa[NB_CATEG_PLC_MAX][2];
int nbPlacesType[NB_CATEG_PLC_MAX];
int nbCateg = 0;
int nbPlacesGlob = 0;

int priority[NB_CATEG_PLC_MAX];
int prioSet[NB_CATEG_PLC_MAX];
int nbPrioPlaces[NB_CATEG_PLC_MAX];

/********* Fin zone de declaration ajoutees          **************/

/*---------------------------------------------------------------------*/
/* codeErr : code retournee par une primitive
 * msgErr  : message d'erreur personnalise
 * valErr  : valeur retournee par le thread
 */
void thdErreur(int codeErr, char *msgErr, int valeurErr) {
  int *retour = malloc(sizeof(int));
  *retour = valeurErr;
  fprintf(stderr, "%s: %d soit %s \n", msgErr, codeErr, strerror(codeErr));
  pthread_exit(retour);
}

/*---------------------------------------------------------------------*/
// 1er et 2e parametres ajoutes pour la trace
// (vous ne pouvez pas changer le prototype de cette operation)
//
// Synchronisation a ajouter dans cette fonction
//
int reserverPlaces (int numResa, int numSpectateur,
		    int categorieSouhaitee, int nbPlacesSouhaitees) {
  pthread_mutex_lock(&mut);
  printf("Spectateur %d (%lu) : (resa %d) veut reserver %d places de categorie %d \n",
      numSpectateur, pthread_self(),numResa, nbPlacesSouhaitees, categorieSouhaitee);

  while(nbPlacesType[categorieSouhaitee] < nbPlacesSouhaitees) {
    if(!prioSet[categorieSouhaitee]){
      printf("Spectateur %d (%lu) : (resa %d) je serai 1er pour reserver %d places de categorie %d \n",
          numSpectateur, pthread_self(),numResa, nbPlacesSouhaitees, categorieSouhaitee);
      prioSet[categorieSouhaitee] = 1;
      nbPrioPlaces[categorieSouhaitee] = nbPlacesSouhaitees;
      while(nbPlacesType[categorieSouhaitee] < nbPlacesSouhaitees) {
        pthread_cond_wait(&condResa[categorieSouhaitee][1],&mut);
      }
      printf("Spectateur %d (%lu) : (resa %d) je passe 1er pour reserver %d places de categorie %d \n",
          numSpectateur, pthread_self(),numResa, nbPlacesSouhaitees, categorieSouhaitee);
      prioSet[categorieSouhaitee] = 0;
    } else {
      printf("Spectateur %d (%lu) : (resa %d) pas le 1er pour reserver %d places de categorie %d \n",
          numSpectateur, pthread_self(),numResa, nbPlacesSouhaitees, categorieSouhaitee);
      pthread_cond_wait(&condResa[categorieSouhaitee][0],&mut);
    }
  }

  nbPlacesType[categorieSouhaitee] -= nbPlacesSouhaitees;

  pthread_mutex_unlock(&mut);
  return 0;

}

/*---------------------------------------------------------------------*/
// 1er et 2e parametres ajoutes pour la trace
// (vous ne pouvez pas changer le prototype de cette operation)
//
// Synchronisation a ajouter dans cette fonction
//
void libererPlaces (int numResa, int numSpectateur,
		    int categorieReservee, int nbPlacesReservees) {
    pthread_mutex_lock(&mut);
    nbPlacesType[categorieReservee] += nbPlacesReservees;
    printf("Spectateur %d (%lu) : (resa %d) a libere      %d places de categorie %d\n",
	   numSpectateur, pthread_self(), numResa, nbPlacesReservees, categorieReservee);

     if(prioSet[categorieReservee]) {
       pthread_cond_signal(&condResa[categorieReservee][1]);
     } else {
       pthread_cond_signal(&condResa[categorieReservee][0]);
     }
    pthread_cond_signal(&condResa[categorieReservee][0]);
    pthread_mutex_unlock(&mut);
}

/*---------------------------------------------------------------------*/
// Temps a profiter du spectacle
//
// (Vous etes libre de modifier le traitement, pas le prototype)
//
void assisterSpectacle (int numReservation, int numSpectateur,
		        int categorieOccupee, int nbPlacesOccupees) {
  printf("Spectateur %d (%lu) : (resa %d) occupe        %d places de categorie %d\n",
	 numSpectateur, pthread_self(), numReservation, nbPlacesOccupees, categorieOccupee);
  for (int i = 0; i < 100; i++) usleep(rand()%1000+200);
}

/*---------------------------------------------------------------------*/
// Choix du nombre de places et de leur categorie
// (vous ne pouvez pas changer le prototype de cette operation)
//
// ! A remplacer avec de meilleurs choix (aleatoire, ...) et a adapter aux
// nombres de categories et de places maximaux envisages
//
void deciderQuantitePlaces (int *categoriePlaces, int *nombrePlaces) {
    *categoriePlaces = rand()%nbCateg;
    *nombrePlaces    = (rand()%(nbPlacesGlob-1))+1;
}


#ifdef TRACE_THD
int nbThdsRestants;     // Nb de thds encore en activite (pour verifier)
#endif
/*---------------------------------------------------------------------*/
// Thread qui joue le role d'un spectateur
/*---------------------------------------------------------------------*/
void *spectateur (void *arg) {
  int i, etat, nbPlacesDemandees, categoriePlaceDemandee;
  Parametre param = *(Parametre *)arg;

  srand(pthread_self());

#ifdef TRACE_THD
  printf("Spectateur %d (%lu) : commence et fera %d reservations (il reste %d threads)\n",
	  param.numSpectateur, pthread_self(), param.nbReservations, nbThdsRestants);
#endif

  for (i = 0; i < param.nbReservations; i++) {
    deciderQuantitePlaces(&categoriePlaceDemandee, &nbPlacesDemandees);
    reserverPlaces(i+1, param.numSpectateur, categoriePlaceDemandee, nbPlacesDemandees);
    assisterSpectacle(i+1, param.numSpectateur,  categoriePlaceDemandee, nbPlacesDemandees);
    libererPlaces(i+1, param.numSpectateur,  categoriePlaceDemandee, nbPlacesDemandees);
  }
#ifdef TRACE_THD
  nbThdsRestants--;
  printf("Spectateur %d (%lu) : termine (il reste %d threads)\n",
	  param.numSpectateur, pthread_self(), nbThdsRestants);
#endif
  return(NULL);
}

/*---------------------------------------------------------------------*/
//**************************
// Vous n'avez pas le droit de changer l'ordre des parametres
// Vous ajouterez ce qui est necessaire a la synchronisation dans les
// zones marquees
/*---------------------------------------------------------------------*/

int main(int argc, char*argv[]) {
  pthread_t idThd[NB_SPECT_MAX];
  int       nbThd, nbReservations, i, etat, nbPlaces , nbCategoriesPlaces;
  Parametre paramThd[NB_SPECT_MAX];

  if (argc != 5) {
    printf("Usage : %s <Nb spectateurs> <Nb reservations successives>\
    <Nb categorie places> <Nb places de chq categorie>\n", argv[0]);
    exit(1);
  }

  nbThd = atoi(argv[1]);
  if (nbThd > NB_SPECT_MAX)
    nbThd = NB_SPECT_MAX;
#ifdef TRACE_THD
  nbThdsRestants = nbThd;
#endif
  nbReservations = atoi(argv[2]);
  if (nbReservations > NB_RESA_MAX)
    nbReservations = NB_RESA_MAX;

  nbCategoriesPlaces = atoi(argv[3]);
  if (nbCategoriesPlaces > NB_CATEG_PLC_MAX)
    nbCategoriesPlaces = NB_CATEG_PLC_MAX;

  nbPlaces = atoi(argv[4]);
  if (nbPlaces > NB_PLACES_MAX)
    nbPlaces = NB_PLACES_MAX;
  // *********************
  // Ajouts possibles dans cette zone
  // Notamment : initialiser le nombre de places de chaque categorie
  for(int i = 0; i < NB_CATEG_PLC_MAX; i++){
    pthread_cond_init(&condResa[i][0],NULL);
    pthread_cond_init(&condResa[i][1],NULL);
    nbPlacesType[i] = nbPlaces;


    priority[i] = 0;
    prioSet[i] = 0;
    nbPrioPlaces[i] = 0;
  }
  nbCateg = nbCategoriesPlaces;
  nbPlacesGlob = nbPlaces;

  //*********************

  // Lancer les threads spectateurs
  for (i = 0; i < nbThd; i++) {
    paramThd[i].numSpectateur = i;
    paramThd[i].nbReservations = nbReservations;
    if ((etat = pthread_create(&idThd[i], NULL, spectateur, &paramThd[i])) != 0)
      thdErreur(etat, "Creation threads spectateurs", 8);
  }

  // Attente de la fin des threads spectateurs
  for (i = 0; i < nbThd; i++) {
    if ((etat = pthread_join(idThd[i], NULL)) != 0)
      thdErreur(etat, "Join threads spectateurs", 9);
#ifdef TRACE_THD
    printf("Apres join %d\n", i);
#endif
  }

  //**********************
  // Ajouts possibles dans cette zone

  ///*********************


  printf ("\nFin de l'application de gestions de places \n");
  exit(0);
}

/*
 * Sujet n° 2
 * Compiler avec -DTRACE_THD pour obtenir les traces sur les threads
 * Rappel : vous ne pouvez apportez des modifications au code 
 *          que dans les zones prevues a cet effet
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

  printf("Spectateur %d (%lu) : (resa %d) veut reserver %d places de categorie %d \n", 
      numSpectateur, pthread_self(),numResa, nbPlacesSouhaitees, categorieSouhaitee);

}

/*---------------------------------------------------------------------*/
// 1er et 2e parametres ajoutes pour la trace 
// (vous ne pouvez pas changer le prototype de cette operation)
//
// Synchronisation a ajouter dans cette fonction 
//
void libererPlaces (int numResa, int numSpectateur, 
		    int categorieReservee, int nbPlacesReservees) {

    printf("Spectateur %d (%lu) : (resa %d) a libere      %d places de categorie %d\n", 
	   numSpectateur, pthread_self(), numResa, nbPlacesReservees, categorieReservee);
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
  for (int i = 0; i < 100; i++) usleep(rand()%1000);
}

/*---------------------------------------------------------------------*/
// Choix du nombre de places et de leur categorie
// (vous ne pouvez pas changer le prototype de cette operation)
//
// ! A remplacer avec de meilleurs choix (aleatoire, ...) et a adapter aux
// nombres de categories et de places maximaux envisages
// 
void deciderQuantitePlaces (int *categoriePlaces, int *nombrePlaces) {
    *categoriePlaces = 0;
    *nombrePlaces    = 1;    
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
  int       nbThd, nbReservations, i, etat, nbPlaces, nbCategoriesPlaces;
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

/*
 * Producteur-consommateur, base sans synchronisation
 *
 * Compilation : gcc m1_ProdConso_base.c -lpthread [-DOptionDeTrace ...] -o prodconso
 *
 * Options de trace lors de la compilation (-D) :
 * TRACE_BUF : tracer le contenu du buffer
 * TRACE_THD : tracer la creation des threads
 * TRACE_SOUHAIT : tracer ce que veulent faire les producteurs/consommateurs
 * */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

#define NB_PROD_MAX   20
#define NB_CONSO_MAX  20

#define NB_CASES_MAX  20
/*
typedef struct {
  char info[80];
  int  type;          // Message de 2 types (0/1 par exemple)
  int  rangProd;      // Qui a produit le message
} TypeMessage;

typedef struct {
  TypeMessage buffer[NB_CASES_MAX];  // Buffer
  int iDepot;                        // Indice prochain depot
  int iRetrait;			     // Indice prochain retrait
} RessourceCritique;                 // A completer eventuellement pour la synchro

// Variables partagees entre tous
RessourceCritique resCritiques; // Modifications donc conflits possibles
int nbCases;                    // Taille effective du buffer,
                                // Pas de modif donc pas de conflit
                                */
typedef struct {                // Parametre des threads
  int rang;                     // - rang de creation
  int typeMsg;                  // - type de message a deposer/retirer (si besoin)
  int nbFois;
} Parametres;

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
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

/*--------------------------------------------------*/
void initialiserVarPartagees (void) {
  ;
}
int ncp = 0;
int sensCourant = 0;
int nbOnRoad = 0;
int init = 1;
/*--------------------------------------------------
 * Correspondra au service du moniteur vu en TD
 * La synchronisation sera ajoutee dans cette operation
 * */
void demandeAccesVU (int sens, int rangProd) {
  pthread_mutex_lock(&mut);
}

/*--------------------------------------------------
 * Correspondra au service du moniteur vu en TD
 * La synchronisation sera ajoutee dans cette operation
 * */
void libererAccesVU (int sens, int rangConso) {
   pthread_mutex_unlock(&mut);
}

/*--------------------------------------------------*/
void * vehiculeN (void *arg) {
  int i;
  Parametres param = *(Parametres *)arg;

  srand(pthread_self());

  //** Q1 : NB_FOIS_PROD a remplacer par le nouveau parametre du main
  for (i = 0; i < param.nbFois; i++) {
#ifdef TRACE_SOUHAIT
    printf("\t\tVehicule %d : Je veux rouler = %d\n",
         param.rang, param.typeMsg);
#endif

    demandeAccesVU(param.typeMsg, param.rang);
    printf("Vehicule %d sens %d roule dans la voie unique)\n",
         param.rang, param.typeMsg);
    usleep(100);
    libererAccesVU(param.typeMsg, param.rang);
    printf("Vehicule %d sens %d fini de rouler dans la voie unique)\n",
         param.rang, param.typeMsg);
  }
  pthread_exit(NULL);
}

/*--------------------------------------------------*/
int main(int argc, char *argv[]) {
  int i, etat;
  int nbThds, nbProd;
  Parametres paramThds[NB_PROD_MAX + NB_CONSO_MAX];
  pthread_t idThdProd[NB_PROD_MAX];

  if (argc <= 2) {
    printf ("Usage: %s <Nb vehicule <= %d> <Nb fois> \n",
             argv[0], NB_PROD_MAX);
    exit(2);
  }

  nbProd  = atoi(argv[1]);
  if (nbProd > NB_PROD_MAX)
    nbProd = NB_PROD_MAX;
  nbThds = nbProd;

  int nbFoisProd = 10;
  nbFoisProd = atoi(argv[2]);
  // Q1 : ajouter 2 parametres :
  // -  nombre de depots a faire par un producteur
  // -  nombre de retraits a faire par un consommateur

  initialiserVarPartagees();

  /* Creation des nbProd producteurs et nbConso consommateurs */
  for (i = 0; i <  nbThds; i++) {
      paramThds[i].typeMsg = i%2;
      paramThds[i].rang = i;
      paramThds[i].nbFois = nbFoisProd;
      if ((etat = pthread_create(&idThdProd[i], NULL, vehiculeN, &paramThds[i])) != 0)
        thdErreur(etat, "Creation vehiculeN1", etat);
#ifdef TRACE_THD
      printf("Creation thread vehiculeN %lu de rang %d -> %d/%d\n", idThdProd[i], i, paramThds[i].rang, nbProd);
#endif
  }

  /* Attente de la fin des threads */
  for (i = 0; i < nbThds; i++) {
    if ((etat = pthread_join(idThdProd[i], NULL)) != 0)
      thdErreur(etat, "Join threads producteurs", etat);
#ifdef TRACE_THD
    printf("Fin thread producteur de rang %d\n", i);
#endif
  }

#ifdef TRACE_THD
  printf ("\nFin de l'execution du main \n");
#endif

  return 0;
}

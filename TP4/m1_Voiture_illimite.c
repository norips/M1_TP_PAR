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
#include <stdatomic.h>
#define NB_PROD_MAX   20

#define NB_CASES_MAX  20

typedef struct {                // Parametre des threads
  int rang;                     // - rang de creation
  int sensVehicule;                  // - type de message a deposer/retirer (si besoin)
  int nbFois;
} Parametres;

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condType[2];
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

volatile int ncp = 0;
volatile int sensCourant = 0;
volatile int nbOnRoad = 0;
volatile int init = 1;
volatile int nbSleeping[2];
atomic_int counter;

/*--------------------------------------------------*/
void initialiserVarPartagees (void) {
  pthread_cond_init(condType,NULL);
  pthread_cond_init(condType+1,NULL);
  nbSleeping[0] = 0;
  nbSleeping[1] = 0;
  init = 1;
}
/*--------------------------------------------------
 * Correspondra au service du moniteur vu en TD
 * La synchronisation sera ajoutee dans cette operation
 * */
void demandeAccesVU (int sens, int rangProd) {
  if(pthread_mutex_lock(&mut) != 0) {
    perror("Lock");
  }
  #ifdef TRACE_SOUHAIT
  printf("Vehicule %d demandeAccesVU | %d\n",rangProd,atomic_fetch_add_explicit(&counter, 1, memory_order_relaxed));
  #endif
  if(init) {
    sensCourant = sens;
    init = 0;
  }
  while(sensCourant != sens) {
    nbSleeping[sens]++;
    #ifdef TRACE_SOUHAIT
      printf("Vechicule %d sleep in %d | %d\n", rangProd,sens,atomic_fetch_add_explicit(&counter, 1, memory_order_relaxed));
    #endif
    if(pthread_cond_wait(&condType[sens],&mut) != 0) {
      perror("Wait");
    }
    #ifdef TRACE_SOUHAIT
      printf("Vechicule %d wake | %d\n", rangProd,atomic_fetch_add_explicit(&counter, 1, memory_order_relaxed));
    #endif
    nbSleeping[sens]--;
  }
  nbOnRoad++;
  pthread_cond_signal(&condType[sens]);
  if(pthread_mutex_unlock(&mut) != 0) {
    perror("Lock");
  }
}

/*--------------------------------------------------
 * Correspondra au service du moniteur vu en TD
 * La synchronisation sera ajoutee dans cette operation
 * */
void libererAccesVU (int sens, int rangConso) {
  if(pthread_mutex_lock(&mut) != 0) {
    perror("Lock");
  }
   nbOnRoad--;
   if(nbOnRoad == 0) {
     if(nbSleeping[sens] != 0) {
       pthread_cond_signal(&condType[sens]);
     } else {
       sensCourant = 1 - sensCourant;
       if(nbSleeping[sensCourant] == 0){
         init = 1;
       }
       pthread_cond_signal(&condType[sensCourant]);
     }
   }
   if(pthread_mutex_unlock(&mut) != 0) {
     perror("Lock");
   }
}

/*--------------------------------------------------*/
void * vehiculeN (void *arg) {
  int i;
  Parametres param = *(Parametres *)arg;

  srand(pthread_self());

  //** Q1 : NB_FOIS_PROD a remplacer par le nouveau parametre du main
  for (i = 0; i < param.nbFois; i++) {
#ifdef TRACE_SOUHAIT
    printf("\t\tVehicule %d : Je veux rouler = %d | %d\n",
         param.rang, param.sensVehicule, atomic_fetch_add_explicit(&counter, 1, memory_order_relaxed));
#endif



    //Je demande l'accés
    demandeAccesVU(param.sensVehicule, param.rang);

    //JE ROULE
    printf("Vehicule %d sens %d roule dans la voie unique) | %d\n",
         param.rang, param.sensVehicule,atomic_fetch_add_explicit(&counter, 1, memory_order_relaxed));
    usleep(rand()%800+200);



    //Je libere l'accés
    libererAccesVU(param.sensVehicule, param.rang);
    #ifdef TRACE_SOUHAIT
    printf("Vehicule %d sens %d fini de rouler dans la voie unique) | %d\n",
         param.rang, param.sensVehicule,atomic_fetch_add_explicit(&counter, 1, memory_order_relaxed));
    #endif
  }
  pthread_exit(NULL);
}

/*--------------------------------------------------*/
int main(int argc, char *argv[]) {
  int i, etat;
  int nbThds;
  Parametres paramThds[NB_PROD_MAX];
  pthread_t idThdProd[NB_PROD_MAX];

  if (argc <= 2) {
    printf ("Usage: %s <Nb vehicule <= %d> <Nb fois> \n",
             argv[0], NB_PROD_MAX);
    exit(2);
  }

  nbThds  = atoi(argv[1]);
  if (nbThds > NB_PROD_MAX)
    nbThds = NB_PROD_MAX;

  int nbFoisProd = 10;
  nbFoisProd = atoi(argv[2]);
  initialiserVarPartagees();

  /* Creation des nbThds vehicule */
  for (i = 0; i <  nbThds; i++) {
      paramThds[i].sensVehicule = i%2;
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

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


#define NB_THREAD 2
#define NB_THREAD_MAX 10
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condP = PTHREAD_COND_INITIALIZER;

int nbThds;
int nbRendezVous=0;

/*--------------------------------------------------*/
void * producteur (void *arg) {
  pthread_mutex_lock(&mut);


  nbRendezVous++;
  while(nbRendezVous != nbThds) {
    printf("J'attends\n");
    pthread_cond_wait(&condP,&mut);
  }
  printf("Go !\n");
  pthread_cond_signal(&condP);
  pthread_mutex_unlock(&mut);
  pthread_exit(NULL);
}

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
int main(int argc, char *argv[]) {
  int i, etat;
  pthread_t idThdProd[NB_THREAD_MAX];

  if (argc <= 1) {
    printf ("Usage: %s <Nb Thread <= %d>\n",
             argv[0], NB_THREAD);
    exit(2);
  }
  nbThds = atoi(argv[1]);
  if(nbThds > NB_THREAD_MAX) {
    nbThds = NB_THREAD_MAX;
  }

  for (i = 0; i <  nbThds; i++) {
      if ((etat = pthread_create(&idThdProd[i], NULL, producteur, NULL)) != 0)
        thdErreur(etat, "Creation producteur", etat);
  }

  /* Attente de la fin des threads */
  for (i = 0; i < nbThds; i++) {
    if ((etat = pthread_join(idThdProd[i], NULL)) != 0)
      thdErreur(etat, "Join threads producteurs", etat);
  }

  printf ("\nFin de l'execution du main \n");

  return 0;
}

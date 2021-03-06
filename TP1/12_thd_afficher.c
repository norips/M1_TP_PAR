/* nbThread affichent un message a l'ecran
   Parametre du programme : nbThread
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>


#define NB_THREADS_MAX  20
#define NB_MSG          10
#define NB_LIGNES_MAX   10


pthread_mutex_t mut_screen;

/*---------------------------------------------------------------------*/
/* Afficher un message d'erreur en fonction du code erreur obtenu
*/
void thdErreur(int codeErr, char *msgErr, void *codeArret) {
  fprintf(stderr, "%s: %d soit %s \n", msgErr, codeErr, strerror(codeErr));
  pthread_exit(codeArret);
}

/*---------------------------------------------------------------------*/
/* Fonction executee par un thread : afficher un message un certain nombre
  nbMessages de messages a l'ecran, comportant chacun nbLignes lignes
*/
void *thd_afficher (void *arg) {
  int i, j, nbLignes,etat;
  int nbFois = *((int*)arg);
  for (i = 0; i < nbFois; i++) {
    nbLignes = rand() % (NB_LIGNES_MAX - 1) + 1;
    if((etat = pthread_mutex_lock(&mut_screen)) != 0) {
      thdErreur(etat, "Lock mutex", NULL);
    }
    for (j = 0; j < nbLignes; j++) {
      printf("Thread %lu, Message %d Ligne %d \n",  pthread_self(), i, j);
      usleep(10);
    }
    if((etat = pthread_mutex_unlock(&mut_screen)) != 0) {
      thdErreur(etat, "Unlock mutex", NULL);
    }
    usleep(1000);
  }
  /* Se terminer sans renvoyer de compte-rendu */
  free((int*)arg);
  pthread_exit((void *)NULL);
}

/*---------------------------------------------------------------------*/
#define NB_AFFICHAGES 10

int main(int argc, char*argv[]) {
  pthread_t idThdAfficheurs[NB_THREADS_MAX];
  int i, etat, nbThreads;

  if((etat = pthread_mutex_init(&mut_screen,NULL)) != 0) {
    thdErreur(etat, "Creation mutex", NULL);
  }

  if (argc != 2) {
    printf("Usage : %s <Nb de threads>\n", argv[0]);
    exit(1);
  }

  nbThreads = atoi(argv[1]);
  if (nbThreads > NB_THREADS_MAX)
    nbThreads = NB_THREADS_MAX;
  /* Lancer les threads afficheurs */
  for (i = 0; i < nbThreads; i++) {
    int *p_nbFois = malloc(sizeof(int));
    *p_nbFois = nbThreads+3-i;
    if ((etat = pthread_create(&idThdAfficheurs[i], NULL,
                               thd_afficher, p_nbFois)) != 0)
      thdErreur(etat, "Creation afficheurs", NULL);
  }

  /* Attendre la fin des threads afficheur car si le thread principal
    - i.e. le main() - se termine, les threads fils crees meurent aussi */
  for (i = 0; i < nbThreads; i++)
    if ((etat = pthread_join(idThdAfficheurs[i], NULL)) != 0)
      thdErreur(etat, "Join threads afficheurs", NULL);

  printf ("\nFin de l'execution du thread principal \n");
  return EXIT_SUCCESS;
}

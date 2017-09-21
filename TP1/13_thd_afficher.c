/* nbThread affichent un message a l'ecran
   Parametre du programme : nbThread
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define NB_THREADS_MAX  20
#define NB_FOIS         10

// mut_screen Utilisé pour stocker les mutex flip_flop
typedef struct p_args{

  pthread_mutex_t *mut_screen[2];
  int nbFois;
  int threadNum;
} p_args;

/*---------------------------------------------------------------------*/
/* Afficher un message d'erreur en fonction du code erreur obtenu
*/
void thdErreur(int codeErr, char *msgErr, void *codeArret) {
  fprintf(stderr, "%s: %d soit %s \n", msgErr, codeErr, strerror(codeErr));
  pthread_exit(codeArret);
}

/*---------------------------------------------------------------------*/
/* Fonction executee par un thread : afficher un message un certain nombre
  de fois nbFois a l'ecran, nbLignes lignes de messages ou nbLignes et
  genere aleatoirement
  Parametre de creation du thread : nbFois, le nombre d'affichages
*/
void *thd_afficher (void *arg) {
  p_args *args = ((p_args*)arg);
  int i, j, nbLignes,etat;
  int nbFois = NB_FOIS;
  for (i = 0; i < nbFois; i++) {
    nbLignes = nbFois;
    if((etat = pthread_mutex_lock(args->mut_screen[0])) != 0) {
      thdErreur(etat, "Lock mutex", NULL);
    }
    for (j = 0; j < nbLignes; j++) {
      printf("Thread %lu, Message %d Ligne %d \n", pthread_self(), i, j);
      usleep(10);
    }
    if((etat = pthread_mutex_unlock(args->mut_screen[1])) != 0) {
      thdErreur(etat, "Unlock mutex", NULL);
    }
  }

  /* Se terminer sans renvoyer de compte-rendu */
  free((p_args*)arg);
  pthread_exit((void *)NULL);
}

/*---------------------------------------------------------------------*/
#define NB_AFFICHAGES 10

int main(int argc, char*argv[]) {
  pthread_t idThdAfficheurs[NB_THREADS_MAX];

  int i, etat, nbThreads;


  if (argc != 2) {
    printf("Usage : %s <Nb de threads>\n", argv[0]);
    exit(1);
  }

  nbThreads = atoi(argv[1]);
  if (nbThreads > NB_THREADS_MAX)
    nbThreads = NB_THREADS_MAX;

  pthread_mutex_t **mut_tmp = malloc(sizeof(pthread_mutex_t*)*(nbThreads+1));
  for(i = 0; i < nbThreads+1; i++) {
    mut_tmp[i] = malloc(sizeof(pthread_mutex_t));
    if((etat = pthread_mutex_init(mut_tmp[i],NULL)) != 0)
      thdErreur(etat, "Init mutex", NULL);
    if((etat = pthread_mutex_lock(mut_tmp[i])) != 0)
      thdErreur(etat, "Lock mutex", NULL);


  }
  if((etat = pthread_mutex_unlock(mut_tmp[nbThreads])) != 0)
    thdErreur(etat, "Unlock mutex", NULL);
  if((etat = pthread_mutex_destroy(mut_tmp[nbThreads])) != 0)
    thdErreur(etat, "Destroy mutex", NULL);

  free(mut_tmp[nbThreads]);
  mut_tmp[nbThreads] = mut_tmp[0];

  if((etat = pthread_mutex_unlock(mut_tmp[0])) != 0)
    thdErreur(etat, "Unlock mutex", NULL);


  /* Lancer les threads afficheurs */
  for (i = 0; i < nbThreads; i++) {
    p_args *p_args_ = malloc(sizeof(p_args));
    p_args_->nbFois = nbThreads+3;
    p_args_->threadNum = i;
    p_args_->mut_screen[0] = mut_tmp[i];
    p_args_->mut_screen[1] = mut_tmp[i+1];

    if ((etat = pthread_create(&idThdAfficheurs[i], NULL,
                               thd_afficher, p_args_)) != 0)
      thdErreur(etat, "Creation afficheurs", NULL);
  }

  /* Attendre la fin des threads afficheur car si le thread principal
    - i.e. le main() - se termine, les threads fils crees meurent aussi */
  for (i = 0; i < nbThreads; i++)
    if ((etat = pthread_join(idThdAfficheurs[i], NULL)) != 0)
      thdErreur(etat, "Join threads afficheurs", NULL);
  for (i = 0; i < nbThreads; i++) {
    free(mut_tmp[i]);
    mut_tmp[i] = NULL;
  }

  printf ("\nFin de l'execution du thread principal \n");
  return EXIT_SUCCESS;
}

/* nbThread affichent un message a l'ecran
   Parametre du programme : nbThread
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define NB_THREADS_MAX  200
#define NB_MSG          10
#define NB_LIGNES_MAX   10

int shared_var = 0;
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
  int i;
  for(i = 0; i < 100; i++) {
    printf("Thread %lu : Valeur shared_var = %d\n",pthread_self(),shared_var);
  }
  pthread_exit((void *)NULL);
}

void *thd_mod(void *arg) {
  int i;
  for(i = 0; i < 100; i++) {
    shared_var += 1;
  }
  pthread_exit((void *)NULL);
}

void *thd_mod2(void *arg) {
  int i;
  for(i = 0; i < 100; i++) {
    shared_var -= 1;
  }
  pthread_exit((void *)NULL);
}

/*---------------------------------------------------------------------*/
#define NB_AFFICHAGES 10

int main(int argc, char*argv[]) {
  pthread_t idThdAfficheurs[NB_THREADS_MAX];
  pthread_t idThdModifieurs[NB_THREADS_MAX];
  int i, etat, nbThreads;

  if (argc != 2) {
    printf("Usage : %s <Nb de threads>\n", argv[0]);
    exit(1);
  }

  nbThreads = atoi(argv[1]);
  if (nbThreads > NB_THREADS_MAX)
    nbThreads = NB_THREADS_MAX;

  srand(getpid());  /* Initialiser generateur de nombres aleatoires */

  /* Lancer les threads afficheurs */

  for (i = 0; i < nbThreads/2; i++) {
    if ((etat = pthread_create(&idThdModifieurs[i], NULL,
                               thd_mod, NULL)) != 0)
      thdErreur(etat, "Creation modifieur", NULL);
  }

  for (i = nbThreads/2; i < nbThreads-1; i++) {
    if ((etat = pthread_create(&idThdModifieurs[i], NULL,
                               thd_mod2, NULL)) != 0)
      thdErreur(etat, "Creation modifieur 2", NULL);
  }
  i=0;
  if ((etat = pthread_create(&idThdAfficheurs[0], NULL,
                               thd_afficher, NULL)) != 0)
      thdErreur(etat, "Creation afficheurs", NULL);

  /* Attendre la fin des threads afficheur car si le thread principal
    - i.e. le main() - se termine, les threads fils crees meurent aussi */
  if ((etat = pthread_join(idThdAfficheurs[0], NULL)) != 0)
    thdErreur(etat, "Join threads afficheurs", NULL);
  for (i = 0; i < nbThreads-1; i++)
    if ((etat = pthread_join(idThdModifieurs[i], NULL)) != 0)
      thdErreur(etat, "Join threads modifieurs", NULL);

  printf("Shared var %d\n",shared_var);
  printf ("\nFin de l'execution du thread principal \n");
  return EXIT_SUCCESS;
}

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

#define NB_PROD_MAX   20
#define NB_CONSO_MAX  20

#define NB_CASES_MAX  20

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
typedef struct {                // Parametre des threads
  int rang;                     // - rang de creation
  int typeMsg;                  // - type de message a deposer/retirer (si besoin)
  int nbFois;
} Parametres;

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condP = PTHREAD_COND_INITIALIZER;
pthread_cond_t condC[2];
int nbProd;
int nbCons;
int currentCons;
volatile int fifoQueue[2][NB_PROD_MAX];
volatile int head[] = {0,0}, tail[] = {0,0};
void enqueue(int x,int type){
  if(head[type] == ((tail[type] + 1) % NB_PROD_MAX)) {
    printf("ERRORRRRRRR\n");
    exit(EXIT_FAILURE);
  }
  fifoQueue[type][tail[type]] = x;
  tail[type] = (tail[type]+1)%NB_PROD_MAX;
}

int dequeue(int type) {
  if( head[type] == tail[type]){
    printf("ERRORRRRRRR\n");
    exit(EXIT_FAILURE);
  }
  int res = fifoQueue[type][head[type]];
  head[type] = (head[type]+1)%NB_PROD_MAX;
  return res;
}

int top(int type) {
  if( head[type] == tail[type]){
    printf("ERRORRRRRRR\n");
    exit(EXIT_FAILURE);
  }
  return fifoQueue[type][head[type]];
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
void initialiserVarPartagees (void) {
  int i;

  /* Le buffer, les indices et le nombre de cases pleines */
  resCritiques.iDepot = 0;
  resCritiques.iRetrait = 0;
  for (i = 0; i < nbCases; i++) {
    strcpy(resCritiques.buffer[i].info, "Message vide");
    resCritiques.buffer[i].type = 0;
    resCritiques.buffer[i].rangProd = -1;
  }
  for(int j = 0; j < 2; j++)
    for(i = 0; i < NB_PROD_MAX; i++) {
      fifoQueue[j][i] = -1;
    }
  nbProd = 0;
  nbCons = 0;
  pthread_cond_init(condC,NULL);
  pthread_cond_init(condC+1,NULL);
}

/*--------------------------------------------------*/
void afficherBuffer (void) {
  int i;

  printf("[ ");
  for (i = 0; i < nbCases; i++) {
    printf("[T%d] %s (de %d), ",
            resCritiques.buffer[i].type,
            resCritiques.buffer[i].info,
            resCritiques.buffer[i].rangProd);
  }
  printf("]\n");
}

/*--------------------------------------------------*/
void depot (TypeMessage leMessage) {
  strcpy(resCritiques.buffer[resCritiques.iDepot].info, leMessage.info);
  resCritiques.buffer[resCritiques.iDepot].type = leMessage.type;
  resCritiques.buffer[resCritiques.iDepot].rangProd = leMessage.rangProd;
  resCritiques.iDepot = (resCritiques.iDepot + 1) % nbCases;
#ifdef TRACE_BUF
  afficherBuffer();
#endif
}

/*--------------------------------------------------*/
void retrait (TypeMessage *leMessage) {
  strcpy(leMessage->info, resCritiques.buffer[resCritiques.iRetrait].info);
  leMessage->type = resCritiques.buffer[resCritiques.iRetrait].type;
  leMessage->rangProd = resCritiques.buffer[resCritiques.iRetrait].rangProd;
  resCritiques.iRetrait = (resCritiques.iRetrait + 1) % nbCases;
#ifdef TRACE_BUF
  afficherBuffer();
#endif
}

int ncp = 0;
int dernierType = 0;
/*--------------------------------------------------
 * Correspondra au service du moniteur vu en TD
 * La synchronisation sera ajoutee dans cette operation
 */
 void deposer (TypeMessage leMessage, int rangProd) {
   pthread_mutex_lock(&mut);
   while(ncp == nbCases) {
     nbProd++;
     pthread_cond_wait(&condP,&mut);
     nbProd--;
   }
   ++ncp;
   depot(leMessage);
   printf("\tProd %d : Message a ete depose = [T%d] %s (de %d) %d\n",
          rangProd, leMessage.type, leMessage.info, leMessage.rangProd, nbCons);


    if(nbCons != 0){
      pthread_cond_signal(&condC[leMessage.type]);
      pthread_cond_signal(&condC[1-leMessage.type]);
    } else if(ncp != 0) {
      pthread_cond_signal(&condP);
    }
    pthread_mutex_unlock(&mut);
 }

/*--------------------------------------------------
 * Correspondra au service du moniteur vu en TD
 * La synchronisation sera ajoutee dans cette operation
 * */
 void retirer (TypeMessage *unMessage, int rangConso,int type) {
   pthread_mutex_lock(&mut);
   int typeMess = resCritiques.buffer[resCritiques.iRetrait].type;
   while(ncp == 0 || typeMess != type) {
     nbCons++;
     pthread_cond_wait(&condC[type],&mut);
     //Fifo wake up
     /*enqueue(rangConso,type);
     bool end = false;
     while(!end) {
       pthread_cond_wait(&condC[type],&mut);
       if(rangConso == top(type)) {
         end = true;
       } else {
         pthread_cond_signal(&condC[type]);
       }
     }
     dequeue(type);*/
     nbCons--;
     typeMess = resCritiques.buffer[resCritiques.iRetrait].type;
   }
   --ncp;
   retrait(unMessage);
   printf("\t\tConso %d : Message a ete lu = [T%d] %s (de %d)\n",
   rangConso, unMessage->type, unMessage->info, unMessage->rangProd);

   if(nbProd != 0){
     pthread_cond_signal(&condP);
   } else if(ncp != 0) {
     typeMess = resCritiques.buffer[resCritiques.iRetrait].type;
     pthread_cond_signal(&condC[typeMess]);
   }
   pthread_mutex_unlock(&mut);
}

/*--------------------------------------------------*/
void * producteur (void *arg) {
  int i;
  TypeMessage leMessage;
  Parametres param = *(Parametres *)arg;

  srand(pthread_self());

  //** Q1 : NB_FOIS_PROD a remplacer par le nouveau parametre du main
  for (i = 0; i < param.nbFois; i++) {
    sprintf (leMessage.info, "%s %d %s %d", "bonjour num ", i, "de prod ", param.rang);
    leMessage.type = param.typeMsg;
    leMessage.rangProd = param.rang;

#ifdef TRACE_SOUHAIT
    printf("\t\tProd %d : Je veux deposer = [T%d] %s (de %d)\n",
         param.rang, leMessage.type, leMessage.info, leMessage.rangProd);
#endif

    deposer(leMessage, param.rang);

    //usleep(rand()%(100 * param.rang + 100));
  }
  pthread_exit(NULL);
}

/*--------------------------------------------------*/
void * consommateur (void *arg) {
  int i;
  TypeMessage unMessage;
  Parametres *param = (Parametres *)arg;

  srand(pthread_self());

  //** Q1 : NB_FOIS_CONSO a remplacer par le nouveau parametre du main
  for (i = 0; i < param->nbFois; i++) {

#ifdef TRACE_SOUHAIT
    printf("\t\tConso %d : Je veux retirer un message \n", param->rang);
#endif
    retirer(&unMessage, param->rang, param->typeMsg);

    //usleep(rand()%(100 * param->rang + 100));
  }
  pthread_exit(NULL);
}

/*--------------------------------------------------*/
int main(int argc, char *argv[]) {
  int i, etat;
  int nbThds, nbProd, nbConso;
  Parametres paramThds[NB_PROD_MAX + NB_CONSO_MAX];
  pthread_t idThdProd[NB_PROD_MAX], idThdConso[NB_CONSO_MAX];

  if (argc <= 5) {
    printf ("Usage: %s <Nb Prod <= %d> <Nb Conso <= %d> <Nb Cases <= %d> <Nb Fois Prod <= Inf> <Nb Fois Conso <= Inf> \n",
             argv[0], NB_PROD_MAX, NB_CONSO_MAX, NB_CASES_MAX);
    exit(2);
  }

  nbProd  = atoi(argv[1]);
  if (nbProd > NB_PROD_MAX)
    nbProd = NB_PROD_MAX;
  nbConso = atoi(argv[2]);
  if (nbConso > NB_CONSO_MAX)
    nbConso = NB_CONSO_MAX;
  nbThds = nbProd + nbConso;
  nbCases = atoi(argv[3]);
  if (nbCases > NB_CASES_MAX)
    nbCases = NB_CASES_MAX;

  int nbFoisProd = 10;
  int nbFoisConso = 10;
  nbFoisProd = atoi(argv[4]);
  nbFoisConso = atoi(argv[5]);
  // Q1 : ajouter 2 parametres :
  // -  nombre de depots a faire par un producteur
  // -  nombre de retraits a faire par un consommateur

  initialiserVarPartagees();

  /* Creation des nbProd producteurs et nbConso consommateurs */
  for (i = 0; i <  nbThds; i++) {
    if (i < nbProd) {
      paramThds[i].typeMsg = i%2;
      paramThds[i].rang = i;
      paramThds[i].nbFois = nbFoisProd;
      if ((etat = pthread_create(&idThdProd[i], NULL, producteur, &paramThds[i])) != 0)
        thdErreur(etat, "Creation producteur", etat);
#ifdef TRACE_THD
      printf("Creation thread prod %lu de rang %d -> %d/%d\n", idThdProd[i], i, paramThds[i].rang, nbProd);
#endif
    }
    else {
      paramThds[i].typeMsg = i%2;
      paramThds[i].rang = i - nbProd;
      paramThds[i].nbFois = nbFoisConso;
      if ((etat = pthread_create(&idThdConso[i-nbProd], NULL, consommateur, &paramThds[i])) != 0)
        thdErreur(etat, "Creation consommateur", etat);
#ifdef TRACE_THD
      printf("Creation thread conso %lu de rang %d -> %d/%d\n", idThdConso[i-nbProd], i, paramThds[i].rang, nbConso);
#endif
    }
  }

  /* Attente de la fin des threads */
  for (i = 0; i < nbProd; i++) {
    if ((etat = pthread_join(idThdProd[i], NULL)) != 0)
      thdErreur(etat, "Join threads producteurs", etat);
#ifdef TRACE_THD
    printf("Fin thread producteur de rang %d\n", i);
#endif
  }

  for (i = 0; i < nbConso; i++) {
    if ((etat = pthread_join(idThdConso[i], NULL)) != 0)
      thdErreur(etat, "Join threads consommateurs", etat);
#ifdef TRACE_THD
    printf("Fin thread consommateur de rang %d\n", i);
#endif
  }

#ifdef TRACE_THD
  printf ("\nFin de l'execution du main \n");
#endif

  return 0;
}

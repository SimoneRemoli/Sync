#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char term;
/*
26_06_2018:A.
Implementare un programma che riceva in input tramite argv[1] un numero
intero N maggiore o uguale ad 1 (espresso come una stringa di cifre 
decimali), e generi N nuovi thread. Ciascuno di questi, a turno, dovra'
inserire in una propria lista basata su memoria dinamica un record
strutturato come segue:

typedef struct _data{
	int val;
	struct _data* next;
} data; 

I record vengono generati e popolati dal main thread, il quale rimane
in attesa indefinita di valori interi da standard input. Ad ogni nuovo
valore letto avverra' la generazione di un nuovo record, che verra'
inserito da uno degli N thread nella sua lista. 
L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel caso
WinAPI) in modo tale che quando il processo venga colpito esso dovra' 
stampare a terminale il contenuto corrente di tutte le liste (ovvero 
i valori interi presenti nei record correntemente registrati nelle liste
di tutti gli N thread). 
*/


int validator(int, char**);
int check(int, char**);
void* worker(void*);
int val;
long indice = 0;
int array_semaforico;
int semaforo_main;
int valore_ritorno_scanf;

struct nodo
{
	int valore;
	struct nodo* next;
};

struct nodo* testa = NULL;
struct nodo* app = NULL;
struct nodo* nuovo_nodo = NULL;
int numero_threads;

void handle(int);

int main(int argc, char** argv) 
{

	printf("Numero di argomenti passati = %d \n", argc-1);
	if(check(argc, argv)==0)
		return 0;
	pthread_t pid;
	int ret;
	struct sembuf oper;

	array_semaforico = semget(IPC_PRIVATE, numero_threads, IPC_EXCL|IPC_CREAT|0666);
	semaforo_main = semget(IPC_PRIVATE, 1, IPC_EXCL|IPC_CREAT|0666);

	semctl(semaforo_main, 0, SETVAL, 1); //[1]
	for(int i=0;i<numero_threads;i++)
	{
		semctl(array_semaforico, i, SETVAL, 0); //[0][0][0][0][0][0]...
	}

	for(long i=0;i<numero_threads;i++)
	{
		pthread_create( &pid, NULL, worker, (void*) i);
	}
	
	signal(SIGINT, handle);

	while(1)
	{
		oper.sem_num = 0;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo1:	
		ret = semop(semaforo_main, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo1;
		printf("\n");
		printf("\n");


		printf("Inserisci il numero intero da inserire nella lista [*] \n");
		printf(">>");
		//scanf("%d", &val);
		valore_ritorno_scanf = scanf("%d%c", &val, &term);
		printf("Il valore di ritorno della scanf è = %d [*] \n", valore_ritorno_scanf);

		if(valore_ritorno_scanf != 2 || term != '\n')
		{
   			printf("Errore [*], devi digitare ''numeri'' non stringhe o caratteri. [*] \n");
   			fprintf(stderr, "Stopped [*] \n");
   			return 0;
		}
		else
		{
    		printf("Acquisizione del numero effettuata correttamente [*]\n");
		}
    	printf("\n");

    	//ora genero un nuovo record 

    	nuovo_nodo = malloc(sizeof(struct nodo));


		oper.sem_num = indice;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo2:
		ret = semop(array_semaforico, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo2;

		indice = indice + 1;
		if(indice == numero_threads)
			indice = 0;
	}

	return 0;
}
int validator(int argc, char**argv)
{
	if(argc==1)
		goto label;
	if(argc>2)
	{
		printf("Passaggio di argomenti errato (1 solo argomento) [*] \n ");
		goto label;
	}
	if((strtol(argv[1],NULL, 10)<1) && (argc == 2))
	{
		printf("L'argomento deve essere maggiore (o uguale) ad 1 [*] \n");
label:
		return 0;
	}
	if((argc==2) && ((strtol(argv[1],NULL, 10)>=1)))
		return 1;
}

int check(int argc, char**argv)
{
	if(validator(argc, argv))
	{
		numero_threads = strtol(argv[1],NULL, 10);
		printf("Numero di threads attivati = %d \n", numero_threads);
	}
	else
	{
		fprintf(stderr, "Aborted [*] \n");
		return 0;
	}
	printf("---------------------------------------------\n");
}

void* worker(void* arg)
{
	long pid = (long) arg;
	int ret;
	struct sembuf oper;


	while(1)
	{
		oper.sem_num = pid;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo1:
		ret = semop(array_semaforico, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo1;

		printf("Sono il thread numero %ld [*] \n", pid+1);


		nuovo_nodo->valore = val;

		printf("Ho inserito nella lista il valore ------ > %d \n ", val);

   		if(testa == NULL)
    	{
    		app = nuovo_nodo;
    		testa = app;
    	}
    	else
    	{
    		app->next = nuovo_nodo;
    		app = nuovo_nodo;
    	}
    	


		oper.sem_num = 0;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo2:
		ret = semop(semaforo_main, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo2;
	}
}
void handle(int unused)
{
	printf("\n");
	struct nodo* temp = testa;
	while(temp!=NULL)
	{
		printf("|            %d            |\n", temp->valore);
		temp = temp->next;
	}
}
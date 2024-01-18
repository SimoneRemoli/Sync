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

struct nodo
{
	int valore;
	struct nodo* next;
};

struct nodo* testa = NULL;
struct nodo* app = NULL;
struct nodo* nuovo_nodo = NULL;
int val;
int indexx = 0;


int check(int , char**);
int numero_threads; 
void* worker(void*);
int array_semaforico;
int semaforo_main;
void handle(int);


int main(int argc, char **argv)
{
	pthread_t pid;
	struct sembuf oper;
	int ret;
	if(check(argc, argv))
	{
		return 0;
	}

	numero_threads = strtol(argv[1], NULL, 10);
	printf("Numero threads da spawnare = %d \n", numero_threads);
	array_semaforico = semget(IPC_PRIVATE, numero_threads, IPC_CREAT|IPC_EXCL|0666);
	semaforo_main = semget(IPC_PRIVATE, 1, IPC_CREAT|IPC_EXCL|0666);
	semctl(semaforo_main, 0, SETVAL, 1);
	for(int i=0;i<numero_threads;i++)
		semctl(array_semaforico, i, SETVAL, 0); //[0][0][0][0]


	for(long i=0;i<numero_threads;i++)
	{
		pthread_create(&pid, NULL, worker, (void*)i);

	}

	signal(SIGINT, handle);
	while(1)
	{
		
		oper.sem_num = 0;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo1:	
		ret = semop(semaforo_main, &oper, 1);
		if(ret==-1)
			if(errno == EINTR)
				goto redo1;

		printf(">>");
		scanf("%d",&val);
		nuovo_nodo = malloc(sizeof(struct nodo));


		oper.sem_num = indexx;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo2:
		ret = semop(array_semaforico, &oper, 1);
		if(ret==-1)
			if(errno == EINTR)
				goto redo2;


		indexx = indexx + 1;
		if(indexx == numero_threads)
			indexx = 0;

	}

	return 0;
}

int check(int argc, char** argv)
{
	if(argc == 2)
		return 0;
	if(argc != 2)
	{
		fprintf(stderr, "Errore nel passaggio degli argomenti - Devi inserire un numero [*] \n");
		return 1;
	}
}

void* worker(void* pid)
{
	long  i = (long) pid;
	struct sembuf oper;
	int ret;


	while(1)
	{
		oper.sem_num = i;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo1:
		ret = semop(array_semaforico, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo1;

		printf("Threads [%ld] update list [*] \n", i+1);

		nuovo_nodo->valore = val;

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
		oper.sem_flg = 0;
		oper.sem_op = 1;
redo2:
		ret = semop(semaforo_main, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo2;
	}
}

void handle(int unused)
{
	struct nodo* temp = testa;
	while(temp != NULL)
	{
		printf(" \t %d \n", temp->valore);
		temp = temp->next;
	}

}
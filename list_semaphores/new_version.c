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

int numero_threads;
char** file_names;
int check(int, char**);
FILE** source_files;
int descriptor;
char buffer[5][1024];
int semaforo_main;
int array_semaforico;
void* worker(void*);
int counter = 0;
struct nodo
{
	char buffered[1024];
	struct nodo* p_next;
};

struct nodo* testa = NULL;
struct nodo* app = NULL;
void handle(int);

int main(int argc, char **argv)
{
	struct sembuf oper;
	int ret;
	pthread_t pid;
	if(check(argc,argv))
	{
		fprintf(stderr, "Errore [*] - Devi necessariamente passare un argomento [*] \n");
		printf("Aborted [*] \n");
		return 0;
	}
	file_names = argv;
	numero_threads = argc - 1;
	printf("Numero di argomenti passati = %d \n", numero_threads);
	source_files = (FILE**)malloc(sizeof(FILE*)*numero_threads);
	semaforo_main = semget(IPC_PRIVATE, 1, IPC_CREAT|IPC_EXCL|0666);
	array_semaforico = semget(IPC_PRIVATE, numero_threads, IPC_CREAT|IPC_EXCL|0666);
	semctl(semaforo_main, 0, SETVAL, 1); //[1]
	for(int i=0;i<numero_threads;i++)
	{
		semctl(array_semaforico, i, SETVAL, 0); //[0][0][0][0]
	}

	for(int i=0;i<numero_threads;i++)
	{
		printf("File [%d] = %s \n", i+1, file_names[i+1]);
    	descriptor = open(file_names[i+1], O_CREAT|O_RDWR|O_TRUNC, 0666);
		source_files[i] = fdopen(descriptor, "w+");

	}

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
		if(ret == -1)
			if(errno == EINTR)
				goto redo1;

		for(int i=0;i<5;i++)
		{
			printf("Inserisci nella [%d] acquisizione [*] \n", i+1);
			printf(">>");

			if(fgets(buffer[i], 1024, stdin))
				buffer[i][strcspn(buffer[i], "\n")] = 0;


			
		}


		oper.sem_num = counter;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo2:
		ret = semop(array_semaforico, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo2;

		counter = counter + 1;
		if(counter == numero_threads)
			counter = 0;
	}
	return 0;
}
int check(int argc, char **argv)
{
	if(argc==1)
		return 1;
	if(argc>1)
		return 0;
}

void* worker(void* arg)
{
	struct sembuf oper;
	int ret;
	long pid = (long) arg;


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

		printf("\n");
		printf("Sono il thread numero [%ld]\n", pid+1);
		for(int i=0;i<5;i++)
		{
			//printf(" Valore inserito = %s \n", buffer[i]);
			fprintf(source_files[pid], "%s\n", buffer[i]);
			fflush(source_files[pid]);
			struct nodo* nuovo_nodo = malloc(sizeof(struct nodo));
			strncpy(nuovo_nodo->buffered, buffer[i], 1024 );

			if(testa == NULL)
			{
				app = nuovo_nodo;
				testa = app;
			}
			else
			{
				app->p_next = nuovo_nodo;
				app = nuovo_nodo;
			}
		}
		printf("Updated %s [*]\n", file_names[pid+1]);
		printf("\n");



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
		printf("%s\n", temp->buffered);
		temp = temp->p_next;
	}
	printf("\n");
}

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


int file_descriptor;
FILE** source_file;
char **file_names;
int numero_threads;
int check(int,char**);
int array_semaforico;
int semaforo_thread;
char buffer[1024];
int indice = 0;
void* worker(void*);
void handle(int);
int indexeer=0;

int main(int argc, char **argv)
{
	pthread_t pid;
	struct sembuf oper;
	int ret;
	if(check(argc,argv))
	{
		printf(" Aborted [*] \n");
		return 0;
	}
	numero_threads = argc - 1;
	source_file = (FILE**)malloc(sizeof(FILE*)*numero_threads);
	file_names = argv;
	printf("Numero di file e numero dei threads da spawnare = %d \n",numero_threads);

	for(int i=0;i<numero_threads;i++)
	{
		printf("File [%d] creato = %s \n", i+1, file_names[i+1]);
		file_descriptor = open(file_names[i+1], O_CREAT|O_TRUNC|O_RDWR, 0666);
		source_file[i] = fdopen(file_descriptor, "w+");
	}
	array_semaforico = semget(IPC_PRIVATE, numero_threads, IPC_CREAT|IPC_EXCL|0666);
	for(int i=0;i<numero_threads;i++)
		semctl(array_semaforico, i, SETVAL, 0);
	semaforo_thread = semget(IPC_PRIVATE, 1, IPC_CREAT|IPC_EXCL|0666);
	semctl(semaforo_thread, 0, SETVAL, 1);



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
		ret = semop(semaforo_thread, &oper, 1);
		if(ret == 1)
			if(errno == EINTR)
				goto redo1;

		printf(">>");
		if(fgets(buffer, 1024, stdin))
			buffer[strcspn(buffer, "\n")] = 0;

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
int check(int argc, char**argv)
{
	if(argc == 1)
	{
		fprintf(stderr, "Errore [*] - Devi specificare una lista di file come argomento [*] \n");
		return 1;
	}
	if(argc > 1)
		return 0;
}
void* worker(void* arg)
{
	struct sembuf oper;
	int ret; 
	long me = (long) arg;

	while(1)
	{
		oper.sem_num = me;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo1:
		ret = semop(array_semaforico, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo1;

		printf("Thread[%ld] scrive ' %s ' sul file = %s\n", me+1, buffer, file_names[me+1]);
		fprintf(source_file[me], "%s\n", buffer);
		fflush(source_file[me]);

		oper.sem_num = 0;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo2:
		ret = semop(semaforo_thread, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo2;
	}
}
void handle(int)
{
	char pointer[1024];
	for(int i=0;i<numero_threads;i++)
	{
		printf("file numero  = %d \n",i+1);
		fseek(source_file[i], 0, 0);
		while( EOF != fscanf(source_file[i], "%[^\n]\n", pointer))
		{
			printf(" %s \n", pointer);
		}
	}
}
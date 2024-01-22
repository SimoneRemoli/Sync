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

int check(int, char**);
char **file_names;
int numero_threads;
int file_descriptor;
FILE* file;
void* worker(void*);
void* worker2(void*);
void run(int);
pthread_t pid;	
pid_t proc;

char* address_map;

int array_semaforico_processo_A;
int array_semaforico_processo_B;
void handle_child(int);
void handle_parent(int);
char command[256];





int main(int argc, char **argv)
{
	int ret;
	struct sembuf oper;
	if(check(argc,argv))
	{
		printf("Errore - Passaggio degli argomenti non valido [*] \n");
		printf("- Aborted [*] - \n");
		return 0;
	}
	printf("Numero di argomenti passati = %d \n", argc-1);
	file_names = argv;
	printf("File inserito = %s \n", file_names[1]);
	numero_threads = strtol(file_names[2], NULL, 10);
	printf("Numero threads  = %d \n", numero_threads);
	printf("\n");

	file_descriptor = open(file_names[1], O_CREAT|O_RDWR|O_TRUNC, 0666);
	file = fdopen(file_descriptor, "r+");

	sprintf(command, "cat %s", file_names[1]);

	array_semaforico_processo_A = semget(IPC_PRIVATE, numero_threads, IPC_CREAT|IPC_EXCL|0666);
	array_semaforico_processo_B = semget(IPC_PRIVATE, numero_threads, IPC_CREAT|IPC_EXCL|0666);

	address_map = mmap(NULL, 1024, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, 0, 0);

	semctl(array_semaforico_processo_A, 0, SETVAL, 1);//[1][0][0][0][0]..
	for(int i=1;i<numero_threads;i++)
		semctl(array_semaforico_processo_A, i, SETVAL, 0);
	for(int i=0;i<numero_threads;i++)
		semctl(array_semaforico_processo_B, i, SETVAL, 0);//[0][0][0]..

	proc = fork();
	if(proc == 0)
	{
	//figlio
		signal(SIGINT, handle_child);
		run(numero_threads);
	}
	else
	{//padre
		signal(SIGINT, handle_parent);
		for(long i=0;i<numero_threads;i++)
		{
			pthread_create(&pid, NULL, worker, (void*)i);
		}
	}
	if(proc == -1)
	{
		fprintf(stderr, "Errore nella generazione del processo figlio [*] \n");
		return 0;
	}


	while(1)
	{
		pause();
	}










	return 0;
}
int check(int argc, char ** argv)
{
	if(argc==3)
		return 0;
	if(argc != 3)
		return 1;
}

void* worker(void* arg) //threds-A-Process
{
	long me = (long) arg;
	struct sembuf oper;
	int ret;


	while(1)
	{
		oper.sem_num = me;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo1:
		ret = semop(array_semaforico_processo_A, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo1;


		printf("Sono il threads [%ld] del processo A [*] \n", me+1);
		printf(">>");
		if(fgets(address_map, 1024, stdin))
			address_map[strcspn(address_map, "\n")] = 0;

		oper.sem_num = me;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo2:
		ret = semop(array_semaforico_processo_B, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo1;

	}


}
void run(int numero_threads)
{
	for(long i=0;i<numero_threads;i++)
		pthread_create(&pid, NULL, worker2, (void*)i);
	
}

void* worker2(void* arg) //threads- processo- B
{
	long me = (long) arg;
	struct sembuf oper;
	int ret;

	while(1)
	{
		oper.sem_num = me;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo1:
		ret = semop(array_semaforico_processo_B, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo1;

		printf("Sono il threads [%ld] del processo B [*] \n", me+1);
		printf("- Updated file %s [*] - ",file_names[1]);
		fprintf(file, "%s\n", address_map);
		fflush(file);
		printf("\n");
		printf("\n");

		if(me == numero_threads-1)
		{
			oper.sem_num = 0;
			oper.sem_op = 1;
			oper.sem_flg = 0;
redo3:
			ret = semop(array_semaforico_processo_A, &oper, 1);
			if(ret == -1)
				if(errno == EINTR)
					goto redo3;
		}

		oper.sem_num = me+1;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo2:
		ret = semop(array_semaforico_processo_A, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo2;



	}



}
void handle_child(int unused)
{
	printf("[*] - Processo figlio [B] colpito - [*]");
	system(command);
}
void handle_parent(int unused)
{
	printf("Il padre ha ricevuto la segnalazione SIGINT con codice = %d\n", unused);
	printf("Invio della stessa segnalazione verso il processo B [*] \n");
	kill(proc, unused);
}
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
void* worker(void*);
char buffer[1024];
int semaforo_main;
int array_semaforico;
char *nome_file = "output.txt";
int descrittore_file;
FILE* file;
void* output(void*);
pthread_t outputt;
char command[256];
void handle(int);


int main(int argc, char **argv)
{
	pthread_t pid;
	int ret;
	struct sembuf oper;
	if(check(argc,argv)==0)
	{
		fprintf(stderr, "Errore [*] - Devi inserire almeno una stringa - \nAborted[*]\n");
		return 0;
	}
	numero_threads = argc - 1;
	printf("Numero di argomenti passati = Numero di threads attivati = %d \n", numero_threads);
	file_names = argv;
	for(int i=0;i<numero_threads;i++)
	{
		printf("Nome della stringa [%d] = %s \n", i+1, file_names[i+1]);
	}
	//

	semaforo_main = semget(IPC_PRIVATE, 1, IPC_CREAT|IPC_EXCL|0666);
	array_semaforico = semget(IPC_PRIVATE, numero_threads, IPC_CREAT|IPC_EXCL|0666);
	semctl(semaforo_main, 0, SETVAL, 1); //[1]
	for(int i=0;i<numero_threads;i++)
		semctl(array_semaforico, i, SETVAL, 0); //[0][0][0][0][0]
	descrittore_file = open(nome_file, O_CREAT|O_APPEND|O_RDWR, 0666);
	file = fdopen(descrittore_file, "w+");


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

		printf("\n");
		sleep(2);
		printf("Inserisci una stringa:\n");
		printf(">>");
		if(fgets(buffer, 1024, stdin))
			buffer[strcspn(buffer, "\n")] = 0;

		oper.sem_num = 0;
		oper.sem_op = 1;
		oper.sem_flg = 0;

redo2:
		ret = semop(array_semaforico, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo2;

	}


	return 0;
}
int check(int argc, char **argv)
{
	if(argc==1)
		return 0;
	if(argc>1)
		return 1;
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

		sleep(1);
		printf("\n");
		printf("Sono il thread [%ld] e la stringa vale = %s [*] \n", pid, buffer);
		printf("Confronto i caratteri con la stringa passata in argv[%ld]  =  %s \n", pid+1, file_names[pid+1]);

		for(int i=0;i<strlen(buffer);i++)
		{
			//printf("Valore della stringa passata = %c \n", file_names[pid+1][i]);
			for(int j=0;j<strlen(file_names[pid+1]);j++)
			{
				if(buffer[i] == file_names[pid+1][j])
				{
					buffer[i] = '*';
				}
			}
		}

		oper.sem_num = pid+1;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo2:
		ret = semop(array_semaforico, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo2;

		if(pid==numero_threads-1)
		{
			pthread_create(&outputt, NULL, output, NULL);

			oper.sem_num = 0;
			oper.sem_op = 1;
			oper.sem_flg = 0;
redo3:

			ret = semop(semaforo_main, &oper, 1);
			if(ret == -1)
				if(errno == EINTR)
					goto redo3;
		}
	}
}
void* output(void* arg)
{
	fprintf(file, "%s\n", buffer);
	fflush(file);
	printf("\n");
	printf("Updated %s [*] \n",nome_file);
	printf("\n");
}
void handle(int unused)
{
	printf("\n");
	printf("-- SIGINT CTRL + C PRESSED --\n");
	sprintf(command, "cat %s", nome_file);
	system(command);
	printf("\n");
	printf("\n");
}
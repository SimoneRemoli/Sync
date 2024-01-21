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
int semaforo_main;
int semaforo_threads;
int numero_threads;
char buffer[1024];
void* worker(void*);
int file_descriptor;
FILE* file;
void handle(int);



int main(int argc, char **argv)
{
	if(check(argc, argv)==0)
	{
		printf(" - A b o r t e d   [*] \n");
		return 0;
	}
	pthread_t pid;
	int ret;
	struct sembuf oper;
	numero_threads = argc - 2;
	printf("Numero di argomenti inseriti = %d \n", argc-1);
	printf("Numero di threads da spawnare = %d \n", numero_threads);


	printf("\n");
	file_names = argv;
	printf("Nome del file inserito = %s \n", file_names[1]);
	for(int i=2;i<argc;i++)
	{
		printf("Stringa %d inserita = %s \n", (i-2)+1, file_names[i]);
	}

	semaforo_main = semget(IPC_PRIVATE, 1, IPC_CREAT|IPC_EXCL|0666);
	semaforo_threads = semget(IPC_PRIVATE, numero_threads,IPC_CREAT|IPC_EXCL|0666);
	semctl(semaforo_main, 0, SETVAL, 1);
	for(int i=0;i<numero_threads;i++)
		semctl(semaforo_threads, i, SETVAL, 0);
	file_descriptor = open(file_names[1], O_CREAT|O_RDWR|O_TRUNC, 0666);
	file = fdopen(file_descriptor, "w+");


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

		printf(">>");
		if(fgets(buffer, 1024, stdin))
			buffer[strcspn(buffer, "\n")] = 0;

		oper.sem_num = 0;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo2:
		ret = semop(semaforo_threads, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo2;

	}




	return 0;
}
int check(int argc, char** argv)
{
	if(argc == 1)
	{
		fprintf(stderr, "Errore [*] - Devi inserire il nome di un file come argomento e almeno una stringa - [*] \n");
		return 0;
	}
	if(argc == 2)
	{
		fprintf(stderr, "Errore [*] - Devi inserire almeno una stringa - [*] \n ");
		return 0;
	}
	if(argc >= 3)
	{
		printf(" - [*] Passaggio di argomenti corretto [*] - \n");
		return 1;
	}
}

void* worker(void* arg)
{
	long me = (long) arg;
	int ret;
	struct sembuf oper;

	while(1)
	{
		oper.sem_num = me;
		oper.sem_flg = 0;
		oper.sem_op = -1;
redo1:
		ret = semop(semaforo_threads, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo1;

		sleep(1);
		printf("\n");
		printf("- Sono il threads numero [%ld] - \n", me+1);
		printf("- Gestisco la stringa = %s - \n", file_names[me+2]);
		printf("- La stringa acquisita [%s] == stringa gestita [%s] ? \n", buffer, file_names[me+2]);

		if(strcmp(file_names[me+2], buffer)==0)
		{
			printf("- Stringa Uguale [*] - \n");
			for(int i=0;i<strlen(buffer);i++)
				buffer[i] = '*';
		}
		else
		{
			printf("- Stringa non Uguale (!=) - \n");
		}
		printf("La stringa è diventata = %s \n", buffer);




		oper.sem_num = me +1;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo2:
		ret = semop(semaforo_threads, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo2;



		if(me == numero_threads-1)
		{


			fprintf(file, "%s\n", buffer);
			fflush(file);
			printf("-- File %s updated [*] -- \n", file_names[1]);
			printf("\n");

			oper.sem_num = 0;
			oper.sem_op = 1;
			oper.sem_flg = 0;
redo3:
			ret = semop(semaforo_main, &oper, 1);
			if(ret == -1)
				if(errno == EINTR)
					goto redo2;
		}

	}
}
void handle(int unused)
{
	char app[1024];

	printf("\n");
	fseek(file, 0, 0);
	printf("------------------------\n");
	while(EOF != fscanf(file, "%[^\n]\n", app))
	{
		printf("%s\n", app);
	}
	printf("------------------------\n");

}
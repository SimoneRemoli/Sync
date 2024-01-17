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
int file_descriptor;
FILE* file;
char *values;
void run();
int semaforo_main;
int semaforo_processo;
char *values;
void handle(int);

int file_descriptor_2;
FILE* file2;
char *suffix = "shadow_";
char nome_file_shadow[1024];


int main(int argc, char **argv)
{
	pid_t pid;
	struct sembuf oper;
	int ret;
	if(check(argc, argv))
	{
		fprintf(stderr, "Errore - devi passare come argomento il nome di un singolo file [*] \n");
		return 0;
	}
	file_names = argv;
	printf("Nome del file passato = %s [*] \n", file_names[1]);

	for(int i=0;i<strlen(suffix);i++)
	{
		nome_file_shadow[i] = suffix[i];
	}
	for(int i=strlen(suffix), j=0;j<strlen(file_names[1]);i++,j++)
		nome_file_shadow[i] = file_names[1][j];

	printf("Creazione del file = %s \n", nome_file_shadow);

	file_descriptor_2 = open(nome_file_shadow, O_CREAT|O_RDWR|O_TRUNC, 0666);
	file2 = fdopen(file_descriptor_2, "w+");








	file_descriptor = open(file_names[1], O_CREAT|O_RDWR|O_TRUNC, 0666);
	file = fdopen(file_descriptor, "w+");
	semaforo_main = semget(IPC_PRIVATE, 1, IPC_CREAT|IPC_EXCL|0666);
	semaforo_processo = semget(IPC_PRIVATE, 1, IPC_CREAT|IPC_EXCL|0666);
	semctl(semaforo_main, 0, SETVAL, 1);
	semctl(semaforo_processo, 0, SETVAL, 0);
	values = mmap(NULL, 1024, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);


	pid = fork();
	if(pid==0)
	{
		signal(SIGINT, SIG_IGN);
		run();
	}
	if(pid==-1)
	{
		fprintf(stderr, "Errore nella generazione del processo figlio [*] \n");
		return 0;

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
		if(fgets(values, 1024, stdin))
			values[strcspn(values, "\n")] = 0;

		fprintf(file2, "%s\n", values);
		fflush(file2);
		printf("Update %s [*] \n", nome_file_shadow);



		oper.sem_num = 0;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo2:
		ret = semop(semaforo_processo, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo2;
	}

	return 0;
}

int check(int argc, char ** argv)
{
	if(argc == 2)
		return 0;
	if(argc != 2)
		return 1;
}

void run()
{
	struct sembuf oper;
	int ret;

	while(1)
	{
		oper.sem_num = 0;
		oper.sem_op = -1;
		oper.sem_flg = 0;

redo1:
		ret = semop(semaforo_processo, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo1;

		printf("Update %s [*] \n", file_names[1]);
		//printf("Hai inserito il valore = %s \n", values);
		//printf("\n");
		fprintf(file, "%s\n", values);
		fflush(file);


		oper.sem_num = 0;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo2:
		ret = semop(semaforo_main, &oper, 1);
		if(ret==-1)
			if(errno == EINTR)
				goto redo2;



	}
}
void handle(int unused)
{
	printf("\n");
	char buffered[1024];
	sprintf(buffered, "cat %s", file_names[1]);
	system(buffered);
}
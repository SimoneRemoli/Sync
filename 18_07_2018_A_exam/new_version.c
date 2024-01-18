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

char **file_names;
int numero_processi;
int check(int, char**);
FILE** source_files;
int descriptor;
void run(int);
int array_semaforico;
char* value;
void handle(int);

int main(int argc, char **argv)
{
	int ret;
	struct sembuf oper;
	if(check(argc, argv)==0)
	{
		fprintf(stderr, "Errore - Devi passare almeno una stringa come argomento [*] - \n");
		printf("Aborted [*] \n");
		return 0;
	}
	file_names = argv;
	source_files = (FILE**)malloc(sizeof(FILE*)*numero_processi);
	numero_processi = argc - 1;
	printf("Numero dei processi che devono essere attivati = %d [*] \n", numero_processi);
	for(int i=0;i<numero_processi;i++)
	{
		//descriptor = open(file_names[i+1], O_CREAT|O_RDWR|O_TRUNC, 0666);
		//source_files[i] = fdopen(descriptor, "w+"); se non li avessi passati da ARGV avrei decommentato queste due istruzioni
		printf("- File [%d] da cui leggere = %s - \n", i+1, file_names[i+1]);
	}
	array_semaforico = semget(IPC_PRIVATE, numero_processi, IPC_CREAT|IPC_EXCL|0666);
	semctl(array_semaforico, 0, SETVAL, 1);
	for(int i=1;i<numero_processi;i++)
		semctl(array_semaforico, i, SETVAL, 0);//[1][0][0][0][0][0].......
	value = mmap(NULL, 1024, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);



	for(int i=0;i<numero_processi;i++)
	{
		if(fork())
			continue;
		else
			run(i);
	}


	signal(SIGINT,handle);
	while(1)
	{
		pause();
	}
	return 0;
}

int check(int argc, char** argv)
{
	if(argc == 1)
	{
		return 0;
	}
	if(argc >1)
	{
		return 1;
	}
}
void run(int pid)
{
	int i = pid;
	struct sembuf oper;
	int ret;
	int check = 0;
	char pointer[1024];


	signal(SIGINT, SIG_IGN);
	while(1)
	{
		oper.sem_num = i;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo1:
		ret = semop(array_semaforico, &oper, 1);
		if(ret==-1)
			if(errno == EINTR)
				goto redo1;


		sleep(1);
		printf("\n");
		printf(" - Sono il processo numero [%d] - Devo leggere dal file [%s] - \n", i+1, file_names[i+1]);

		source_files[i] = fopen(file_names[i+1], "r+");
		while( EOF != fscanf(source_files[i], "%[^\n]\n", pointer))
		{
			printf("- Area condivisa del processo [%d] = %s - \n", i+1, pointer);
			strcat(value, pointer);
			strcat(value, " ");
		}

		if(i==numero_processi)
		{
			printf("- Tutti i processi hanno letto dal file (premere CTRL + C) - \n");
		}

		oper.sem_num = i + 1;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo2:
		ret = semop(array_semaforico, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo2;
	}
}
void handle(int unused)
{
	printf(" CTRL + C Pressed [*] \n");
	printf(" - -  %s - - \n", value);
}
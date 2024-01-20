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
char ** file_names;
int file_descriptor;
FILE* file;
int semaforo_main;
char buffer[1024];
void handle(int);
int numero_di_stringhe; 
void* worker(void*);
int array_semaforico; 
int contatore = 0;
pthread_t pid;
void run(int);
char *file_backup = "backup";
int desc_backup;
FILE* file_backup_star;

int main(int argc, char **argv)
{
	if(check(argc, argv))
	{
		return 0;
	}
	file_names = argv;
	printf("File ''%s'' creato correttamente. [*] \n", file_names[1]);
	for(int i=2;i<argc;i++)
	{
		printf("Stringa %d inserita = %s [*] \n", (i-2)+1, file_names[i]);
	}

	file_descriptor = open(file_names[1], O_CREAT|O_RDWR|O_TRUNC, 0666);
	file = fdopen(file_descriptor, "w+");
	numero_di_stringhe = argc - 2;
	array_semaforico = semget(IPC_PRIVATE, numero_di_stringhe, IPC_CREAT|IPC_EXCL|0666);
	semctl(array_semaforico, 0 , SETVAL, 1);
	for(int i=1;i<numero_di_stringhe;i++)
		semctl(array_semaforico, i, SETVAL, 0);




	signal(SIGINT, handle);

	while(1)
	{
		printf("Inserisci una stringa da inserire nel file.\n");
		printf(">>");
		if(fgets(buffer, 1024, stdin))
			buffer[strcspn(buffer, "\n")] = 0;
		fprintf(file, "%s\n", buffer);
		fflush(file);
	}








	return 0;
}
int check(int argc, char ** argv)
{
	if(argc == 1)
	{
		fprintf(stderr, "- Non hai inserito nessun argomento [*] - Aborted -\n");
		return 1;
	}
	if(argc > 1)
	{
		printf("Numero di argomenti passati = %d \n", argc-1);
		printf("\n");
		return 0;
	}
}
void handle(int unused)
{
	fseek(file, 0 , 0);
	semctl(array_semaforico, 0 , SETVAL, 1);
	for(int i=1;i<numero_di_stringhe;i++)
		semctl(array_semaforico, i, SETVAL, 0);
	printf("Spawning di %d - processi [*] \n", numero_di_stringhe);
	for(long i=0;i<numero_di_stringhe;i++)
	{
		pthread_create(&pid, NULL, worker, (void*)i);
	}
	/*for(int i=0;i<numero_di_stringhe;i++) //funziona sia con processi che con threads
	{
		if(fork())
			continue;
		else
			run(i);
	}*/

}
void* worker(void* arg)
{
	long me = (long) arg; 
	int ret; 
	struct sembuf oper;
	char buffer[256];
	//printf("Il valore delle stringhe vale = %d \n", numero_di_stringhe);

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

		fseek(file, 0, 0);
		printf("Sono il pthread numero [%ld] - [*] \n", me+1);
		printf("Verifico quante volte la stringa [%s] è presente nel file '%s' [*]  - \n", file_names[me+2], file_names[1]);


		while( EOF != fscanf(file, "%[^\n]\n", buffer))
		{
			//printf("Stringa nel file = %s \n", buffer);
			if(strcmp(buffer, file_names[me+2])==0)
			{
				//printf("Stringa uguale [*] ---> %s = %s ", file_names[pid+2],buffer);
				contatore = contatore + 1;
			}

		}
		printf("Contatore = %d \n", contatore);
		contatore = 0;
		if(me==numero_di_stringhe-1)
		{
			desc_backup = open(file_backup, O_CREAT|O_RDWR|O_APPEND|O_TRUNC, 0666);
			file_backup_star = fdopen(desc_backup, "w+");

			fseek(file, 0, 0);
			while( EOF != fscanf(file, "%[^\n]\n", buffer))
			{
				fprintf(file_backup_star, "%s\n", buffer);
				fflush(file_backup_star);
			} 

		}
		oper.sem_num = me + 1;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo2:
		ret = semop(array_semaforico, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo2;


	}


}

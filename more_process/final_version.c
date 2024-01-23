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
int numero_processi;
char **file_names;
char *mmapped;
void run();
int file_descriptor;
FILE* file;
int semaforo_main;
int array_semaforico;
FILE** source_files;
char buffer[1024];
FILE* file;
int indice=0;
void handle(int);



int main(int argc, char **argv)
{
	struct sembuf oper;
	int ret;
	if(check(argc, argv))
	{
		printf("Aborted [*] \n");
		return 0;
	}
	numero_processi = (argc - 1) / 2;
	printf(" Numero di argomenti passati = %d\n", argc-1);
	printf(" Numero di coppie = numero di processi = %d \n", numero_processi);
	semaforo_main = semget(IPC_PRIVATE, 1, IPC_CREAT|IPC_EXCL|0666);
	array_semaforico = semget(IPC_PRIVATE, numero_processi, IPC_CREAT|IPC_EXCL|0666);
	for(int i=1;i<numero_processi;i++)
		semctl(array_semaforico, i, SETVAL, 0);
	semctl(semaforo_main, 0, SETVAL, 1);

	source_files = (FILE**)malloc(sizeof(FILE*)*numero_processi);


	file_names = argv;
	for(int i=1, j= 1 ;i<argc;j++)
	{
		printf("File numero [%d] inserito = %s\n", j, file_names[i]);
		i = i + 2;
	}
	for(int i=2, j=1;i<argc;j++)
	{
		printf("Stringa numero [%d] inserita = %s\n",j, file_names[i]);
		i = i + 2;
	}
	
	for(int i=0;i<numero_processi;i++)
	{
		if(fork())
			continue;
		else
			run(i);
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
		printf("Hai scritto la stringa %s [*] \n", buffer);

		file_descriptor = open(file_names[indice + (indice + 1)], O_RDWR|O_APPEND, 0666);
		file = fdopen(file_descriptor, "w+");

		fprintf(file, "%s\n", buffer);
		fflush(file);


		oper.sem_num = indice;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo2:
		ret = semop(array_semaforico, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo2;


		/*indice = indice + 1;
		if(indice == numero_processi)
			indice = 0;*/
	}
	return 0;
}

int check(int argc, char** argv)
{
	if(argc % 2 != 0)
	{
		fprintf(stderr, "Passaggio degli argomenti corretto [*] \n");
		return 0;
	}
	if(argc % 2 == 0)
	{
		fprintf(stderr, "Errore - Passaggio argomenti non valido [*] \n");
		return 1;
	}
}
void run(int indice)
{
	struct sembuf oper;
	int ret;
	int me = indice;
	char buffered[1024];
	int contatore = 0;

	file = fopen(file_names[me + ( me + 1)], "w+");

	signal(SIGINT, SIG_IGN);

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

		printf("\n");	

		printf("Sono il processo [%d]. \n", me+1);
		printf("Verifico se nel file ''%s'' è presente la stringa ''%s'' [*] \n", file_names[me + ( me + 1 )], file_names[(me + (me + 1 ))+ 1]);

		source_files[me] = fopen(file_names[me + (me + 1)], "r+"); //gia è fsseekkata all'inizio con fopen

		while( EOF != fscanf(source_files[me], "%[^\n]\n", buffered))
		{
			printf(" Valore = %s\n", buffered);
			if(strcmp(buffered, file_names[(me + (me + 1)) + 1 ])==0)
			{
				contatore = contatore + 1;
			}
		}
		printf("Stringa presente %d volte [*] \n",contatore);
		contatore = 0;
		printf("\n");


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
	struct sembuf oper;
	int ret;
	indice = indice + 1;
	//printf("Indice = %d\n", indice);
	if(indice == numero_processi)
		indice = 0;

		oper.sem_num = indice;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo2:
		ret = semop(array_semaforico, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo2;



}
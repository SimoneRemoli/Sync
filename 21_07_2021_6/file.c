#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
21_07_2021_6CFU

Implementare una programma che riceva in input, tramite argv[], il nome
di un file F e un insieme di N stringhe (con N almeno pari ad 1). Il programa dovra' creare 
il file F e popolare il file con le stringhe provenienti da standard-input. 
Ogni stringa dovra' essere inserita su una differente linea del file.
L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel caso
WinAPI) in modo tale che quando il processo venga colpito si
dovranno  generare N thread concorrenti ciascuno dei quali dovra' analizzare il contenuto
del file F e verificare, per una delle N stringhe di input, quante volte tale stringa 
sia presente nel file. Il risultato del controllo dovra' essere comunicato su standard
output tramite un messaggio. Quando tutti i thread avranno completato questo controllo, 
il contenuto del file F dovra' essere inserito in "append" in un file denominato "backup"
e poi il file F dovrà essere troncato.
*/

char **file_names;
FILE* file;
FILE* file2;
FILE* file3;
int file_descriptor;
int file_descriptor_2;
int file_descriptor_3;
int file_descriptor_append;
FILE* file_append;
void handle(int);
pthread_t pid;
int numero_di_stringhe;
int array_semaforico; 
void* worker(void*);
int contatore = 0;
char *back = "backup";
char *d;

char p[1024];
int u = 0;

int main(int argc, char** argv)
{
	printf("\t Numero di argomenti passati = %d \n", argc);
	numero_di_stringhe = argc - 2;
	int ret;
	struct sembuf oper;
	if(numero_di_stringhe==0)
	{
		printf("Non sono presenti stringhe nel passaggio dei parametri [*] \n");
		fprintf(stderr, "Aborted [*] \n");
		return 0;
	}
	if(argc==1)
	{
		printf("Non hai inserito parametri tramite ARGV[]. \n");
		printf("Aborted [*] \n");
	}
	file_names = argv;
	printf("\t Numero di stringhe passate = %d\n", numero_di_stringhe);
	file_descriptor = open(file_names[1], O_CREAT|O_RDWR|O_TRUNC, 0666);
	file = fdopen(file_descriptor, "w+");
	array_semaforico = semget(IPC_PRIVATE, numero_di_stringhe, IPC_CREAT|IPC_EXCL|0666);
	semctl(array_semaforico, 0, SETVAL,1);
	for(int i=1;i<numero_di_stringhe;i++)
		semctl(array_semaforico, i, SETVAL, 0);//[1][0][0][0][0]...
	//file_descriptor_2 = open(file_names[1], O_CREAT|O_TRUNC|O_RDWR, 0666);
	

	for(int i=0;i<argc-2;i++)
	{
		printf("\t Stringa numero [%d] = %s \n", i+1, file_names[i+2]);
		fprintf(file, "%s\n",file_names[i+2]);
	}
	fseek(file, 0, 0); //rimetto il puntatore all'inizio [Riaggiustamento].
	free(file);

	signal(SIGINT, handle);
	while(1)
	{
		pause();
	}
	return 0;
}
void handle(int unused)
{
	printf("\n");
	printf(".. [*] SIGINT Activated .. [*] \n");
	printf("Codice del gestore = %d \n", unused);
	for(long i=0;i<numero_di_stringhe;i++)
	{
		pthread_create(&pid, NULL, worker, (void*)i);
	}
}
void *worker(void *arg)
{
	u = 0;
	long index = (long) arg;
	struct sembuf oper;
	int ret; 
	char appoggio[numero_di_stringhe][1024];

	while(1)
	{

		oper.sem_num = index;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo1:
		ret = semop(array_semaforico, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo1;

	//	sleep(1);


		file2 = fopen(file_names[1],"r+");

		printf("\n Il Thread [%ld] ha letto la stringa  = %s \t \t | \n", index+1, file_names[index+2]);

		while(EOF != fscanf(file2, "%[^\n]\n", p))
        {
        	strncpy(appoggio[u], p, 1024);
        	u = u + 1;
        }
        u = 0;
        for(int i=0;i<numero_di_stringhe;i++)
        {
        	if(strcmp(appoggio[i],file_names[index+2] ) == 0)
        	{
        		contatore = contatore + 1;
        	}
        }
		free(file2);
		printf("Il thread numero [%ld] ha visto che le occorrenze della parola [%s]  da lui presa in carico \n corrispondono ad un valore pari a = %d [*]\n", index+1, file_names[index+2],contatore);
		contatore = 0;
		index = index + 1;


		oper.sem_num = index;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo2:
		ret = semop(array_semaforico, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo2;



		if(index==numero_di_stringhe)
		{
			index=0;

			//in append;
			file_descriptor_append = open(back, O_CREAT|O_TRUNC|O_RDWR, 0666);
			file_append = fdopen(file_descriptor_append, "w+");

			file3 = fopen(file_names[1], "r+");

			while(EOF != fscanf(file3, "%ms[^\n]\n", &d))
        	{
        		fprintf(file_append, "%s\n", d);
        		fflush(file_append);
        	}
        	free(d);
        	free(file3);
        	free(file_append);

			return 0;
		}
	}
}
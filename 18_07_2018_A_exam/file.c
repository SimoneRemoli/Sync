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


/*Implementare un programma che riceva in input tramite argv[] i pathname 
associati ad N file, con N maggiore o uguale ad 1.

Per ognuno di questi file generi un processo che legga tutte le stringhe contenute in quel file
e le scriva in un'area di memoria condivisa con il processo padre. Si 
supponga per semplicita' che lo spazio necessario a memorizzare le stringhe
di ognuno di tali file non ecceda 4KB. 
Il processo padre dovra' attendere che tutti i figli abbiano scritto in 
memoria il file a loro associato, e successivamente dovra' entrare in pausa
indefinita.
D'altro canto, ogni figlio dopo aver scritto il contenuto del file nell'area 
di memoria condivisa con il padre entrera' in pausa indefinita.
L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel caso
WinAPI) in modo tale che quando il processo padre venga colpito da esso dovra' 
stampare a terminale il contenuto corrente di tutte le aree di memoria 
condivise anche se queste non sono state completamente popolate dai processi 
figli.
*/
void run(int, int);
char **file_names;
int numero_argomenti_effettivi;
FILE **source_files;
int array_semaforico_processi;
char *pointer;
char app[4097];
char *values; //mmap 
void handler(int);
#define PAGE_SIZE 4096

int main(int argc, char **argv)
{
	pointer = malloc(10000);
	file_names = argv;
	struct sembuf oper;
	int ret;
	printf("Numero di argomenti passati = %d \n", argc);
	numero_argomenti_effettivi = argc - 1;
	if(numero_argomenti_effettivi<1)
	{
		fprintf(stderr,"Errore nel passaggio degli argomenti [*] \n");
		return 0;
	}
	printf("Numero di file passati = %d \n", numero_argomenti_effettivi);
	for(int i=0;i<argc-1;i++)
		printf("File numero [%d] = %s \n", i+1, file_names[i+1]);
	source_files = (FILE**)malloc(sizeof(FILE*)*numero_argomenti_effettivi);
	values = mmap(NULL,PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, 0,0 ); //regione di memoria condivisa


	array_semaforico_processi = semget(IPC_PRIVATE, numero_argomenti_effettivi, IPC_CREAT|IPC_EXCL|0666);
	semctl(array_semaforico_processi, 0, SETVAL, 1);
	for(int i=1;i<numero_argomenti_effettivi;i++)
	{
		semctl(array_semaforico_processi, i, SETVAL, 0); //[1][0][0][0][0]....
	}

	for(int i=0;i<numero_argomenti_effettivi;i++)
	{
		if(fork())
			continue;
		else
			run(i,argc);
	}

	signal(SIGINT, handler);
	
	while(1)
	{
		pause();

	}


	return 0;
}
void run(int i, int argc)
{
	struct sembuf oper;
	char pointer[4096];
	int ret;
	int pid = i;

	while(1)
	{

		oper.sem_num = i;
		oper.sem_op = -1;
		oper.sem_flg = 0;

redo1:	
		ret = semop(array_semaforico_processi, &oper, 1);
		if(ret==-1)
			if(errno == EINTR)
				goto redo1;


		printf("Sono il processo numero %d [*] \n", i+1);
		printf("Devo leggere le stringhe dal file %s [*] \n", file_names[i+1]);
		sleep(1);
		source_files[i] = fopen(file_names[i+1], "r+");

		while(EOF != fscanf(source_files[i],"%[^\n]\n", pointer))
		{
			printf("\n");
			printf("\t Leggo la stringa '' %s '' dal file[%d] [*] \n ", pointer,i+1);
			sprintf(app, " %s",pointer);
			strcat(values, app);
		}


		oper.sem_num = i+1;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo2:
		ret = semop(array_semaforico_processi, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo2;

	}
}

void handler(int unused)
{
	printf("\n");
	printf("\t Siamo nel gestore [*] \n");
	printf("\t CTRL + C press ... \n");
	printf("  %s \n ", values);
}
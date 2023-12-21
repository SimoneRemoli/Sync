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
18-09-2019

Implementare una programma che riceva in input, tramite argv[], un insieme di
stringhe S_1 ..... S_n con n maggiore o uguale ad 1. 

Per ogni stringa S_i dovra' essere attivato un thread T_i.
Il main thread dovra' leggere indefinitamente stringhe dallo standard-input.
Ogni stringa letta dovra' essere resa disponibile al thread T_1 che dovra' 
eliminare dalla stringa ogni carattere presente in S_1, sostituendolo con il 
carattere 'spazio'.
Successivamente T_1 rendera' la stringa modificata disponibile a T_2 che dovra' 
eseguire la stessa operazione considerando i caratteri in S_2, e poi la passera' 
a T_3 (che fara' la stessa operazione considerando i caratteri in S_3) e cosi' 
via fino a T_n. 

T_n, una volta completata la sua operazione sulla stringa ricevuta da T_n-1, dovra'
passare la stringa ad un ulteriore thread che chiameremo OUTPUT il quale dovra' 
stampare la stringa ricevuta su un file di output dal nome output.txt.
Si noti che i thread lavorano secondo uno schema pipeline, sono ammesse quindi 
operazioni concorrenti su differenti stringhe lette dal main thread dallo 
standard-input.

L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel caso
WinAPI) in modo tale che quando il processo venga colpito esso dovra' 
stampare il contenuto corrente del file output.txt su standard-output.
*/
char **file_names;
char buffer[1024];
int numero_di_stringhe_effettivamente_passate;
void* worker(void*);
int semaforo_main;
int array_semaforico;
int indexx=0;
char app[1024];
int indice=0;
pthread_t outputt;
void* output(void*);
int descrittore;
FILE* file;
char *file_name="output.txt";
void sprint(int);

int main(int argc, char** argv)
{
	pthread_t pid;
	struct sembuf oper;
	int ret;
	file_names = argv;
	printf("Numero di argomenti passati = %d \n", argc);
	numero_di_stringhe_effettivamente_passate = argc - 1;

	if(argc<2)
	{
		fprintf(stderr, "Errore nel passaggio degli argomenti [*] \n");
		printf("Aborted [*] \n");
		return 0;
	}
	semaforo_main = semget(IPC_PRIVATE, 1, IPC_CREAT|IPC_EXCL|0666);
	array_semaforico = semget(IPC_PRIVATE, numero_di_stringhe_effettivamente_passate, IPC_CREAT|IPC_EXCL|0666);
	semctl(semaforo_main, 0, SETVAL, 1);//[1]
	for(int i=0;i<numero_di_stringhe_effettivamente_passate;i++)
	{
		semctl(array_semaforico, i, SETVAL, 0);//[0][0][0][0][0][0]
	}

	printf("Numero di stringhe effettivamente passate = %d \n", numero_di_stringhe_effettivamente_passate);

	for(long i=0;i<numero_di_stringhe_effettivamente_passate;i++)
	{
		pthread_create(&pid, NULL, worker, (void*)i);
	}



	signal(SIGINT, sprint);
	while(1)
	{

		oper.sem_num=0; //parte il main 
		oper.sem_op=-1;
		oper.sem_flg=0;
redo1:
		ret= semop(semaforo_main,&oper,1);
		if(ret==-1)
			if(errno==EINTR)
				goto redo1;

		sleep(1);
		printf("Inserisci la stringa da acquisire [*] \n");
		printf(">>");
		if(fgets(buffer, 1024, stdin))
			buffer[strcspn(buffer,"\n")] = 0;


		oper.sem_flg = 0;
		oper.sem_op = 1;
		for(long i=0;i<numero_di_stringhe_effettivamente_passate;i++)
		{
			oper.sem_num=i;
redo2:
		ret = semop(array_semaforico, &oper, 1);
		if(ret==-1)
			if(errno==EINTR)
				goto redo2;
		}
	}
	return 0;
}

void* worker(void *arg)
{
	long pid = (long) arg;
	struct sembuf oper;
	int ret;
	int count=0;

	while(1)
	{
		oper.sem_num = pid; //si blocca il primo processo
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo1:
		ret = semop(array_semaforico, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo1;

		if(pid==count)
		{
			printf("Stringa resa disponibile al Thread [%ld] = %s \n", pid, buffer);
		
			for(int i=0;i<strlen(buffer);indice++,i++)
			{
				printf(" app[%d] = %c  e  buffer[%d] = %c \n", indice, app[indice], i, buffer[i]);
				app[indice] += buffer[i];
			}
			for(int i=0;i<strlen(app);i++)
				app[i]='_';
			printf("Stringa modificata [%ld] = %s \n", pid, app);
		
			if(pid==numero_di_stringhe_effettivamente_passate-1)
			{
				printf("Siamo all'ultimo thread \n");
				pthread_create(&outputt, NULL, output, NULL);
			}
		}

		oper.sem_num = 0;
		oper.sem_op  = 1;
		oper.sem_flg = 0;
redo2:
		ret = semop(semaforo_main, &oper, 1);
		if(ret==-1)
			if(errno==EINTR)
				goto redo2;


		count = count + 1;
		if(count == numero_di_stringhe_effettivamente_passate)
			count = 0;
	}
}
void* output(void *arg)
{
	descrittore = open(file_name, O_CREAT|O_RDWR|O_TRUNC, 0666);
	file = fdopen(descrittore, "w+");
	fprintf(file, "%s \n", app);
	fflush(file);
	fclose(file);
	close(descrittore);
}
void sprint(int unused)
{
	char buffering[256];
	sprintf(buffering, "cat %s", file_name);
	system(buffering);
}
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
#define PAGE_SIZE 4096
#define NUM_PAGE 100
/*
7_09_2018

Implementare un'applicazione che riceva in input tramite argv[] il 
nome di un file F ed una stringa indicante un valore numerico N maggiore
o uguale ad 1.
L'applicazione, una volta lanciata dovra' creare il file F ed attivare 
N thread. Inoltre, l'applicazione dovra' anche attivare un processo 
figlio, in cui vengano attivati altri N thread. 
I due processi che risulteranno attivi verranno per comodita' identificati
come A (il padre) e B (il figlio) nella successiva descrizione.

Ciascun thread del processo A leggera' stringhe da standard input. 
Ogni stringa letta dovra' essere comunicata al corrispettivo thread 
del processo B tramite memoria condivisa, e questo la scrivera' su una 
nuova linea del file F. Per semplicita' si assuma che ogni stringa non
ecceda la taglia di 4KB. 

L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel caso
WinAPI) in modo tale che quando il processo A venga colpito esso dovra' 
inviare la stessa segnalazione verso il processo B. Se invece ad essere 
colpito e' il processo B, questo dovra' riversare su standard output il 
contenuto corrente del file F.

*/
FILE *file;
int file_descriptor;
void* worker(void*);
void* worker2(void*);
int numero_threads;
char *values;
int array_semaforico_thread_processo_A;
int array_semaforico_thread_processo_B;
int sigint_code;
void sprintff(int);
char **argomenti;
pid_t pidd;
void parent_handler(int);

int main(int argc, char** argv)
{
	pthread_t pid;	

	printf("Numero di argomenti passati = %d [*] \n", argc);
	argomenti = argv;
	numero_threads = strtol(argomenti[2],NULL,10);
	if((argc>3)||(argc<=2))
	{
		fprintf(stderr, "Errore nel passaggio degli argomenti [*] \n");
		printf("Aborted [*] \n");
	}
	printf("Nome del file = %s \n",argomenti[1]);
	printf("Numero di thread che devono essere attivati = %d \n", numero_threads);
	array_semaforico_thread_processo_A = semget(IPC_PRIVATE, numero_threads, IPC_CREAT|IPC_EXCL|0666);
	array_semaforico_thread_processo_B = semget(IPC_PRIVATE, numero_threads, IPC_CREAT|IPC_EXCL|0666);

	for(int i=0;i<numero_threads;i++)
		semctl(array_semaforico_thread_processo_B,i,SETVAL,0);//[0][0][0][0][0]

	semctl(array_semaforico_thread_processo_A,0,SETVAL,1);//[1][0][0][0][0]
	for(int i=1;i<numero_threads;i++)
		semctl(array_semaforico_thread_processo_A,i,SETVAL,0);//[1][0][0][0][0]

	values = mmap(NULL,PAGE_SIZE*NUM_PAGE, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, 0,0 );
	file_descriptor = open(argomenti[1], O_RDWR|O_CREAT|O_TRUNC, 0666);
	file = fdopen(file_descriptor, "w+");


	pidd = fork();
	if(pidd==0)
	{
		signal(SIGINT,sprintff);
		for(long i=0;i<numero_threads;i++)
		{
			pthread_create(&pid, NULL, worker2, (void*) i );
		}
	}
	else
	{
		signal(SIGINT,parent_handler);
		for(long i=0;i<numero_threads;i++)
		{
			pthread_create(&pid, NULL, worker, (void*)i);
		}
	}

	while(1)
	{
		pause();
	}

	return 0;
}

void *worker(void* arg)
{
	struct sembuf oper;
	int ret;
	long indice = (long)arg;
	while(1)
	{
		oper.sem_num = indice;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo1:
		ret = semop(array_semaforico_thread_processo_A, &oper, 1); //non bloccante
		if(ret==-1)
			if(errno==EINTR)
				goto redo1;
//---------------------------------------------------
		printf("\n");
		printf("Il thread del processo a[%ld] ti ->ordina<- di inserire una stringa [*]. \n", indice);
		printf(">>");

		if(fgets(values, 4096, stdin))
			values[strcspn(values,"\n")] = 0;
//---------------------------------------------------


		oper.sem_num = indice;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo2:
		ret = semop(array_semaforico_thread_processo_B, &oper, 1);
		if(ret==-1)
			if(errno==EINTR)
				goto redo2;

		if(indice == numero_threads-1)
		{
			indice = 0;			
		}
	}
}
void *worker2(void*arg)
{
	struct sembuf oper;
	int ret;
	long indice = (long)arg;

	while(1)
	{

		oper.sem_num = indice;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo1:
		ret = semop(array_semaforico_thread_processo_B, &oper, 1);
		if(ret==-1)
			if(errno==EINTR)
				goto redo1;

		printf("\n");
		printf("----------------------\n");
		printf("Valore letto dal thread b[%ld] = %s [*] \n",indice,values);
		printf("----------------------\n");
		printf("\n");


		indice = indice + 1;
		fprintf(file, "%s\n", values);
		fflush(file);


		oper.sem_num = indice;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo2:
		ret = semop(array_semaforico_thread_processo_A, &oper, 1);
		if(ret==-1)
			if(errno==EINTR)
				goto redo2;

		if(indice==numero_threads)
		{
			indice = 0;
			oper.sem_num = indice;
			oper.sem_op = 1;
			oper.sem_flg = 0;
redo3:
			ret = semop(array_semaforico_thread_processo_A, &oper, 1); 
			if(ret==-1)
				if(errno==EINTR)
					goto redo3;
		}
	}
}

void sprintff(int unused)
{
	printf("\n");
	printf("\n");
	printf("\tColpito il processo figlio [*] \n");
	printf("\tArrivato il segnale = %d \n", unused);

	char buffer[1024];
	sprintf(buffer, "cat %s", argomenti[1]);
	system(buffer);

}
void parent_handler(int signo){
	printf("Il padre ha ricevuto il segnale  %d - forwarding to child (pid %d)\n",signo,pidd);
	kill(pidd,signo);
}
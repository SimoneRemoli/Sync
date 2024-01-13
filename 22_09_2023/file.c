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
22 settembre 2023.

Scrivere un programma che riceva in input tramite arvg[1] il nome di un file F.
Il programma, attivo in un processo P, dovra' attivare un nuovo processo P' che
dovra' creare il file F (o se esistente dovra' troncarlo).
Il processo P dovra' poi attivare un thread T che legga linee dallo standard input
e le trasferisca al processo P', il quale dovra' scriverle sul file F.
Il trasferimento delle linee verso il processo P' dovra' avvenire seguendo l'ordine di 
lettura delle linee dallo standard input. Il canale di comunicazione per il
trasferimento di dati e' a scelta dello studente.

L'applicazione dovra' gestire il segnale  SIGINT (o CTRL_C_EVENT nel caso WinAPI) 
in modo tale che quando il processo P venga colpito dovra' essere attivato
un nuovo thread equivalente a T, che dovra' anche lui leggere linee dallo 
standard input e trasferirle al processo P', sempre rispettando che l'ordine 
di lettura delle linee dallo standard input corrisponda all'ordine con cui i 
dati sono traferiti al processo P'.  Il numero massimo di thread equivalenti a T
che potranno essere attivati e' 4, quindi la gestione della segnalazione dovra'
tenere in conto di questo limite. Se invece la segnalazione colpisse il processo P',
non dovra' avere alcun effetto sulla sua esecuzione.
*/
char *file_name;
int file_descriptor;
void* worker(void*);
void* work2(void*);
char buffer[1024];
int semaforo_processo;
int semaforo_thread;
int semaforo_thread_1;
char *mmmap_string;
FILE* file;
void handle(int);
long counter = 0;


int main(int argc, char** argv)
{
	signal(SIGINT, handle);
	struct sembuf oper;
	int ret;
	pid_t pid; 
	pthread_t tid;

	file_name = argv[1];
	if(argc!=2)
	{
		printf("Devi passare come argomento solo ''1 file'' [*] \n");
		fprintf(stderr, "Aborted [*] \n" );
		return 0;
	}

	printf("Numero di argomenti passati = %d \n", argc-1);
	printf("Nome del file passato = %s \n", file_name);
	semaforo_processo = semget(IPC_PRIVATE, 1, IPC_CREAT|IPC_EXCL|0666);
	semaforo_thread = semget(IPC_PRIVATE, 1, IPC_CREAT|IPC_EXCL|0666);
	semaforo_thread_1 = semget(IPC_PRIVATE, 1, IPC_CREAT|IPC_EXCL|0666);
	semctl(semaforo_processo, 0, SETVAL, 0);
	semctl(semaforo_thread, 0, SETVAL, 1); // parte prima il thread
	semctl(semaforo_thread_1, 0, SETVAL, 1);
//----------------------------------------------------------------------------------------------
	mmmap_string = mmap(NULL, 1024*1024, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, 0, 0);
//----------------------------------------------------------------------------------------------


	pid = fork();
	if(pid==-1)
	{
		fprintf(stderr, "Errore nella generazione del figlio [*] - Spawning Aborted [*] \n");
		return 0;
	}
	if(pid==0) //child process P'
	{
		file_descriptor = open(file_name, O_CREAT|O_RDWR|O_TRUNC, 0666);
		file = fdopen(file_descriptor, "w+");

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



			//printf("Hai inserito la stringa %s \n ", mmmap_string);
			fprintf(file, "%s\n", mmmap_string);
			fflush(file);


			oper.sem_num = 0;
			oper.sem_op = 1;
			oper.sem_flg = 0;
redo2:

		ret = semop(semaforo_thread, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo2;
		}
	}
	else
	{
		pthread_create(&tid, NULL, worker, NULL);
	}

	while(1)
	{
		pause();
	}
	return 0;
}

void* worker(void* arg) //thread T (deve leggere linee e renderle disponibile al processo P')
{
	struct sembuf oper;
	int ret;

	while(1)
	{
		oper.sem_num = 0;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo1:	
		ret = semop(semaforo_thread, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo1;


		//----------
		if(fgets(mmmap_string,1024,stdin))
			mmmap_string[strcspn(mmmap_string,"\n")] = 0;
		//----------

		oper.sem_num = 0;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo2:
		ret = semop(semaforo_processo, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo2; 
	}
}

void handle(int)
{
	counter = counter + 1;
	pthread_t new_thread;
	if(counter<=4)
	{
		printf("Threads [%ld] attivato - Spawning [*]  \n", counter);
		pthread_create(&new_thread, NULL, work2, (void*)counter);
	}
	else
		printf("Limite massimo dei threads raggiunto - NO SPAWNING [*] \n");
}
void* work2(void *arg) //thread T'
{
	struct sembuf oper;
	int ret;

	long me = (long) arg;
	while(1)
	{
		oper.sem_num = 0;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo1:	
		ret = semop(semaforo_thread_1, &oper, 1);
		if(ret == -1)
			if(errno = EINTR)
				goto redo1;


		printf(" Sono il thread numero [%ld] - Spawning [*] \n", me);

		if(fgets(mmmap_string, 1024, stdin))
			mmmap_string[strcspn(mmmap_string, "\n")] = 0;


		oper.sem_num = 0;
		oper.sem_op = 1;
		oper.sem_flg = 0;

redo2:
		ret = semop(semaforo_processo, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo2;

	}
}
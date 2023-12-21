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
/*SPECIFICATION TO BE IMPLEMENTED:
Implementare un programma che riceva in input tramite argv[] i pathname 
associati ad N file, con N maggiore o uguale ad 1. Per ognuno di questi
file generi un thread (quindi in totale saranno generati N nuovi thread 
concorrenti). 
Successivamente il main-thread acquisira' stringhe da standard input in 
un ciclo indefinito, ed ognuno degli N thread figli dovra' scrivere ogni
stringa acquisita dal main-thread nel file ad esso associato.
L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel caso 
WinAPI) in modo tale che quando uno qualsiasi dei thread dell'applicazione
venga colpito da esso dovra' stampare a terminale tutte le stringhe gia' 
immesse da standard-input e memorizzate nei file destinazione.*/

// 18-7-2018 B
//>> ./a.out file1 file2 file3 file4 ... 

char** file_names;
int array_semaforico, singolo_semaforo;
char stringa[128];
char *path_name;
void* worker(void *);
void print(int);

int main(int argc, char **argv)
{
	int num_threads;
	pthread_t tid;
	int ret;
	file_names = argv;
	num_threads = argc-1; 
	struct sembuf oper;

	if(argc<=1)
		printf("Errore passaggio argomenti [*] \n");
	for(long i=0;i<num_threads;i++)
	{
		ret = pthread_create(&tid, NULL, worker, (void*) i + 1);
	}

	signal(SIGINT,print);

	singolo_semaforo = semget(IPC_PRIVATE, 1, IPC_CREAT|IPC_EXCL|0666);
	array_semaforico = semget(IPC_PRIVATE, num_threads, IPC_CREAT| IPC_EXCL|0666);
	ret = semctl(singolo_semaforo, 0, SETVAL, num_threads);



	for(int i=0;i<num_threads; i++)
	{
		ret = semctl(array_semaforico, i, SETVAL, 0);
	}
	while(1)
	{

		oper.sem_num = 0;
		oper.sem_op = -num_threads;
		oper.sem_flg = 0;
redo1:
		ret = semop(singolo_semaforo, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo1; 
		printf("Inserisci la stringa: ");
		ret = scanf("%s", stringa);

		oper.sem_op = 1;
		oper.sem_flg = 0;
		for(int i=0;i<num_threads;i++)
		{
			oper.sem_num = i; 
redo2:
		ret = semop(array_semaforico, &oper, 1);
		if(ret==-1)
			if(errno == EINTR)
				goto redo2;
		}
	}
	return 0;
}
void *worker(void* arg)
{
	struct sembuf oper;
	long indice = (long) arg;
	int ret;
	long thread_corrente = indice-1; 
	path_name = file_names[thread_corrente + 1];
	int file_descriptor = open(path_name, O_RDWR|O_CREAT|O_TRUNC, 0666);
	if(file_descriptor == -1)
		fprintf(stderr, "Errore nell'apertura del file [*] \n");
	while(1)
	{
		oper.sem_num = thread_corrente;
		oper.sem_op = -1;
		oper.sem_flg = 0; //default.
redo1:
		ret = semop(array_semaforico, &oper, 1); //-1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 
		if(ret == -1)
			if(errno==EINTR)
				goto redo1;

redo2:
		write(file_descriptor, stringa, strlen(stringa));

		oper.sem_num = 0;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo3:
		ret = semop(singolo_semaforo, &oper,1);
		if(ret==-1)
			if(errno == EINTR)
				goto redo3;
	}
	return NULL;
}
void print(int unused)
{
	char buffer[128];
	sprintf(buffer, "cat %s", path_name);
	system(buffer);
	return;
}

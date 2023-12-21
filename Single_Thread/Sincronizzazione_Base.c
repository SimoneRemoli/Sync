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
#define N 1024

/*
Implementare un programma in cui un thread scrive sul file in maniera continua e il main thread legge da file
anch'esso in maniera continua.
Organizzazione semaforica e sincronizzazione. 
Partenza: 
Semaforo_Thread impostato a 1.
Semaforo_Main impostato a 0.
*/

FILE *file; 
char *nome_del_file = "file";
char buffer[N];
void *worker(void*);
void print();
int semaforo_thread, semaforo_main; 

int main(int argc, char** argv) 
{
	pthread_t tid; 
	int file_descriptor, ret;
	file_descriptor = open(nome_del_file, O_CREAT| O_RDWR, 0666);
	file = fdopen(file_descriptor, "r+");
	struct sembuf oper;
	semaforo_thread = semget(IPC_PRIVATE,1, IPC_CREAT|IPC_EXCL|0666);
	semaforo_main = semget(IPC_PRIVATE,1, IPC_CREAT|IPC_EXCL|0666);
	semctl(semaforo_thread,0,SETVAL,1);
	semctl(semaforo_main, 0, SETVAL, 0);
	pthread_create(&tid, NULL, worker, NULL);

	while(1)
	{
		oper.sem_num = 0;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo1:
		ret = semop(semaforo_main, &oper, 1);
		if(ret==-1)
			if(errno==EINTR)
				goto redo1;

		write(file_descriptor, buffer, strlen(buffer));

		if(ret != -1)
			printf("Stringa %s scritta correttamente  [*] \n", buffer);

		print();
		oper.sem_num = 0;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo2:
		ret = semop(semaforo_thread, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo2;
	}
	return 0;
}

void *worker(void *a)
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

		printf("\n Inserisci il valore da acquisire : \n");
		if(fgets(buffer, N, stdin))
			buffer[strcspn(buffer, "\n")] = 0;
		printf("Hai inserito la stringa = %s \n", buffer);

		oper.sem_num = 0;
		oper.sem_op = 1;
		oper.sem_flg = 0;

redo2:
		ret = semop(semaforo_main, &oper, 1);
		if(ret==-1)
			if(errno ==EINTR)
				goto redo2;
	}

}
void print()
{
	char buffer[128];
	sprintf(buffer, "cat %s", nome_del_file);
	system(buffer);
	return;
}




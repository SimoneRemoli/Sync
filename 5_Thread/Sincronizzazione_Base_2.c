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
#include <sys/stat.h>
#define N 1024
/*
Il programma dispone della presenza del main thread e l'ausilio di ulteriori 5 threads che vengono spawnati a Run-Time.
In totale il software lavora con 6 thread e due strutture semaforiche per gestire l'atomicità delle operazioni.
Utilizzo di Semaphores V di UNIX. (System V);
Il programma è di seguito così strutturato:
- Il thread 1 scrive su un file1;
- Il thread 2 scrive su un file2;
- Il thread 3 legge dal file1;
- Il thread 4 legge dal file2;
- Il thread 5 effettua il "merge" del contenuto dei due file e ne visualizza il contenuto.

All'arrivo del segnale SIGINT (CTRL + C) il contenuto del file "merged" viene riversato completamente su STDOUT.
*/
char *file_1 = "file1";
char *file_2 = "file2";
char *file_merged = "file_merged";
int fd1, fd2, fd3_merged, dim1, dim2;
void passing();
pthread_t tid;
void *worker(void *);
void print(char *);
int descrittore_semaforico, descrittore_semaforo_main;
FILE *fptr1, *fptr2;
char buffer[N];
char azzera1[N], azzera2[N];
void printer(int);

int main(int argc, char** argv) 
{
	signal(SIGINT, printer);
	struct sembuf oper;
	fd1 = open(file_1, O_CREAT| O_RDWR, 0666);
	fd2 = open(file_2, O_CREAT| O_RDWR, 0666);
	fd3_merged = open(file_merged, O_CREAT|O_WRONLY|O_TRUNC, 0666);
	descrittore_semaforico = semget(IPC_PRIVATE,5,IPC_CREAT|IPC_EXCL|0666);
	descrittore_semaforo_main = semget(IPC_PRIVATE,5,IPC_CREAT|IPC_EXCL|0666);
	semctl(descrittore_semaforico,0, SETVAL, 1);
	semctl(descrittore_semaforico,1, SETVAL, 0);
	semctl(descrittore_semaforico,2, SETVAL, 0);
	semctl(descrittore_semaforico,3, SETVAL, 0);
	semctl(descrittore_semaforico,4, SETVAL, 0);
	semctl(descrittore_semaforo_main,0, SETVAL, 0);
	int ret;
	passing();

	while(1)
	{
		oper.sem_num = 0;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo1:
		ret = semop(descrittore_semaforo_main, &oper, 1);
		if(ret==-1)
			if(errno == EINTR)
				goto redo1;
		oper.sem_num = 1;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo2:
		ret = semop(descrittore_semaforico, &oper, 1);
		oper.sem_num = 0;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo3:
		ret = semop(descrittore_semaforo_main, &oper, 1);
		if(ret==-1)
			if(errno == EINTR) //operazione bloccante 
				goto redo3;
		oper.sem_num = 2;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo4:
		ret = semop(descrittore_semaforico, &oper, 1); 
		oper.sem_num= 0;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo5:
		ret = semop(descrittore_semaforo_main, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo5; //operazione bloccante 
		oper.sem_num = 3;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo6:
		ret = semop(descrittore_semaforico, &oper, 1);
		oper.sem_num  = 0;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo7:
		ret = semop(descrittore_semaforo_main, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo7; //operazione bloccante 
		oper.sem_num = 4;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo8:
		ret = semop(descrittore_semaforico, &oper, 1);
		oper.sem_num  = 0;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo9:
		ret = semop(descrittore_semaforo_main, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo9; //operazione bloccante 
		oper.sem_num = 0;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo10:
		ret = semop(descrittore_semaforico, &oper, 1);
	}
	return 0;
}
void passing()
{
	for(long i=0;i<5;i++)
		pthread_create(&tid, NULL, worker, (void*)i );
}

void *worker(void *arg)
{
	struct sembuf oper;
	int ret;
	long value = (long)arg;
while(1)
{
	if(value+1 == 1)
	{
		oper.sem_num = 0;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo1:
		ret = semop(descrittore_semaforico, &oper, 1);
		if(ret==-1)
			if(errno == EINTR)
				goto redo1; //non bloccante 
		printf("Inserisci il valore da scrivere nel %s : \n ", file_1);
		if(fgets(buffer, 1024, stdin))
			buffer[strcspn(buffer, "\n")] = 0;
		write(fd1, buffer, strlen(buffer));
		printf("Hai scritto il valore :  %s \n ", buffer);
		oper.sem_num = 0;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo2:
		ret = semop(descrittore_semaforo_main, &oper, 1); //rilascio token al main
	} 
	if(value+1 ==2)
	{
		oper.sem_num = 1;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo3:
		ret = semop(descrittore_semaforico, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo3;
		printf("Inserisci il valore da scrivere nel %s : \n ", file_2);
		if(fgets(buffer, 1024, stdin))
			buffer[strcspn(buffer, "\n")] = 0;
		write(fd2, buffer, strlen(buffer));
		printf("Hai scritto il valore : %s \n ", buffer);
		oper.sem_num = 0;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo4:
		ret = semop(descrittore_semaforo_main, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo4; //rilascio il gettone al main 
	}
	if(value+1==3)
	{
		oper.sem_num = 2;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo5:
		ret = semop(descrittore_semaforico, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo5; 
		print(file_1);
		oper.sem_num = 0;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo6:
		ret = semop(descrittore_semaforo_main, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo6; //rilascio il gettone al main 
	}

	if(value+1==4)
	{
		oper.sem_num = 3;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo7:
		ret = semop(descrittore_semaforico, &oper, 1);
		if(ret==-1)
			if(errno == EINTR)
				goto redo7;
		print(file_2);
		oper.sem_num = 0;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo8:
		ret = semop(descrittore_semaforo_main, &oper, 1);
	}
	if(value+1==5)
	{
		oper.sem_num = 4;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo9:
		ret = semop(descrittore_semaforico, &oper, 1);
		if(ret ==-1)
			if(errno == EINTR)
				goto redo9;
		fptr1 = fopen(file_1, "r");
		fptr2 = fopen(file_2, "r");
		while(fgets(buffer, N, fptr1))
  			write(fd3_merged, buffer, strlen(buffer));
		while(fgets(buffer, N, fptr2))
			write(fd3_merged, buffer, strlen(buffer));
		printf("Ho effettuato il merged del file : %s \n", file_merged);
		print(file_merged);
		fflush(fptr1);
		fflush(fptr2);
		fclose(fptr1);
		fclose(fptr2);
		oper.sem_num = 0;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo10:
		ret = semop(descrittore_semaforo_main, &oper, 1);
	}
  }
}
void print(char *f)
{
	char buffer[128];
	puts("\n");
	puts("Stampo : ");
	sprintf(buffer, "cat %s", f);
	system(buffer);
	puts("\n");
	return;
}
void printer(int unused)
{
	sprintf(buffer, "cat %s", file_merged);
	system(buffer);
	exit(-1);
}




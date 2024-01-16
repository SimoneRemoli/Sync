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
int validate(int);
int numero_processi;

int *value;
void run(int);
int semaforo_main;
int array_semaforico;
int contatore = 1;
int file_descriptor;
FILE* file;
void handle(int);
char buffered[1024];



int main(int argc, char **argv)
{	
	int ritorno;
	int ret;
	struct sembuf oper;
	ritorno = check(argc,argv);
	if(validate(ritorno)==0)
		return 0;
	numero_processi = strtol(argv[1], NULL, 10);
	printf("Numero di argomenti passati = %d\n", argc);
	printf("Numero di processi attivi = %d [*] \n", numero_processi);
	semaforo_main = semget(IPC_PRIVATE, 1, IPC_PRIVATE|IPC_CREAT|0666);
	array_semaforico = semget(IPC_PRIVATE, numero_processi, IPC_EXCL|IPC_CREAT|0666);
	semctl(semaforo_main, 0, SETVAL, 0);
	semctl(array_semaforico, 0, SETVAL, 1);
	for(int i=1;i<numero_processi;i++)
		semctl(array_semaforico, i, SETVAL, 0);

	file_descriptor = open("file", O_CREAT|O_TRUNC|O_RDWR, 0666);
	file = fdopen(file_descriptor, "w+");
	sprintf(buffered, "cat %s", "file");

	value = mmap(NULL, 1024, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);

	for(int i=0;i<numero_processi;i++)
	{
		if(fork())
			continue;
		else
			run(i);
	}


	signal(SIGINT, handle);

	while(1) //processo padre
	{
		oper.sem_num = 0;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo1:
		ret = semop(semaforo_main, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo1;

		printf("Valore che hai inserito nella MMAP = %d \n", *value);
		fprintf(file, "%d ", *value);
		fflush(file);


		oper.sem_num = contatore;
		oper.sem_op = 1;
		oper.sem_flg = 0;

redo2:
		ret = semop(array_semaforico, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo2;


		printf("Contatore = %d \n", contatore);
		contatore = contatore + 1;
		if(contatore == numero_processi)
			contatore = 0;


	}

	return 0;
}
int check(int argc, char **argv)
{
	if(argc!=2)
		return 0;
	if(argc == 2)
	{
		if(strtol(argv[1],NULL, 10) < 1)
		{
			return 1;
		}
		if(strtol(argv[1],NULL, 10) >= 1)
		{
			return 2;
		}
	}

}
int validate(int ritorno)
{
	if(ritorno == 0)
	{
		fprintf(stderr, "Errore. Devi inserire un solo argomento [*] \n");
		return 0;
	}
	if(ritorno == 1)
	{
		fprintf(stderr, "Errore, l'argomento deve essere maggiore (o uguale) ad 1 [*] \n");
		return 0;
	}
	if(ritorno == 2)
	{
		return 1;
	}


}
void run(int pid)
{
	struct sembuf oper;
	int i = pid;
	int ret;
	int valore;
	signal(SIGINT, SIG_IGN);


	while(1)
	{
		oper.sem_num = i;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo1:
		ret = semop(array_semaforico, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo1;

		printf("Sono il processo [%d] - inserire il valore intero da passare al processo padre [*] \n",i+1);
		printf(">>");
		scanf("%d",&valore);
		*value = valore;


		oper.sem_num = 0;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo2:
		ret = semop(semaforo_main, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo2;
	}
}
void handle(int unused)
{
	printf("\t - CTRL + C Pressed [*] - \n");
	char buffer[1024];

	fseek(file, 0, 0);
	while(EOF != fscanf(file, "%s", buffer))
	{
		printf("Valore del file = %s \n", buffer); //lettura dei valori anche se fisicamente non sono messi uno sotto l'altro
	}
	/*
	Ho adottato un modo alternativo invece di usare la sprintf
	*/
	system(buffered);
}
/*SPECIFICATION TO BE IMPLEMENTED:
Implementare una programma che riceva in input, tramite argv[], il nome
di un file F. Il programa dovra' creare il file F e popolare il file
con lo stream priveniente da standard-input. Il programma dovra' generare
anche un ulteriore processo il quale dovra' riversare il contenuto  che 
viene inserito in F su un altro file denominato shadow_F, tale inserimento
dovra' essere realizzato in modo concorrente rispetto all'inserimento dei dati su F.

L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel caso
WinAPI) in modo tale che quando un qualsiasi processo (parent o child) venga colpito si
dovra' immediatamente emettere su standard-output il contenuto del file 
che il processo child sta popolando. 

Qualora non vi sia immissione di input, l'applicazione dovra' utilizzare 
non piu' del 5% della capacita' di lavoro della CPU.


26-6-2021-9cfu
*/
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
#define N 1024
#define PAGE_SIZE 4096
#define NUM_TARGET_PAGES 1000

char *filename;
char *new_file = "shadow_F";
int file_descriptor;
int new_file_descriptor; 
char command[N];
int semaforo_main, semaforo_son; 
char *values;

//suggerimento: deve esserci una memoria condivisa per far coumicare i due processi 

void run();
void printer();

int main(int argc, char** argv)
{
	int proc;
	int ret;
	struct sembuf oper;
	if(argc!=2)
	{
		fprintf(stderr, "Errore nella lettura degli argomenti [*] \n");
		printf("Aborted [*] \n ");
		sleep(2);
		exit(EXIT_FAILURE);
	}

	//-------------------------------Cuore Portante del codice---------------------------------------------------
	values = mmap(NULL,PAGE_SIZE*NUM_TARGET_PAGES, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, 0, 0);
	//--------------------------------Non toccare mai la MMAP  --------------------------------------------------



	filename = argv[1];
	printf("\n");
	printf("\tNome del file vecchio  = %s\n ", filename);
	printf("\tNome del file nuovo    = %s\n ", new_file);
	printf("\n");

	file_descriptor = open(filename, O_CREAT|O_RDWR, 0666);
	new_file_descriptor = open(new_file, O_CREAT|O_RDWR, 0666);
	semaforo_son = semget(IPC_PRIVATE,1,IPC_CREAT|IPC_EXCL|0666);
	semaforo_main = semget(IPC_PRIVATE,1,IPC_CREAT|IPC_EXCL|0666);
	semctl(semaforo_main, 0, SETVAL, 1);	
	semctl(semaforo_son, 0, SETVAL, 0);	


	sprintf(command, "cat %s", new_file);
	signal(SIGINT, printer);


	proc = fork();
	if(proc == -1)
	{
		printf("Fork() error [*] \n");
		exit(EXIT_FAILURE);
	}
	if(proc==0)
		run();

	while(1) //main
	{
		oper.sem_num = 0;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo1:
		ret = semop(semaforo_main, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo1; //passa 

		if(fgets(values, N, stdin))
			values[strcspn(values, "\n")] = 0;
		printf("Valore %s letto - [*] \n", values);

		ret = write(file_descriptor, values, strlen(values));

		if(ret == -1)
			printf("Errore sulla scrittura del file [*] \n ");
		else
			printf("Valore %s scritto correttamente nel vecchio file  - Successfully [*] \n", values); // vecchio ok, scrive

		oper.sem_num = 0;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo2:
		ret = semop(semaforo_son, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo2; //non bloccante, l'operazione passa tranquillamente 
	}
	return 0;
}

void run()
{
	int ret;
	struct sembuf oper;
	signal(SIGINT, SIG_IGN);

	while(1)
	{
		oper.sem_num = 0;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo1:
		ret = semop(semaforo_son, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo1; //bloccante 
		ret = write(new_file_descriptor, values, strlen(values));
		if(ret == -1)
			printf("Errore sulla scrittura del file [*] \n ");
		else
			printf("Valore %s scritto correttamente nel nuovo file  - Successfully [*] \n", values);

		oper.sem_num = 0;
		oper.sem_op = 1;
		oper.sem_flg = 0;

redo2: 	
		ret = semop(semaforo_main, &oper, 1);
		if(ret==-1)
			if(errno == EINTR)
				goto redo2;
	}
}

void printer(int unused)
{
	system(command);
	return;

}

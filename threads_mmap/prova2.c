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
#define PAGE_SIZE 1024
#define N_pages 10

void run();
void printer(int);
int *address_mmap;
int semaforo_per_processi, semaforo_daddy;
char command[1024];


/*Implementare un programma che riceva in input tramite argv[2] un numero
intero N maggiore o uguale ad 1 (espresso come una stringa di cifre 
decimali), e generi N nuovi processi. Ciascuno di questi leggera' in modo 
continuativo un valore intero da standard input, e lo comunichera' al
processo padre tramite memoria condivisa. Il processo padre scrivera' ogni
nuovo valore intero ricevuto su di un file, come sequenza di cifre decimali. 
I valori scritti su file devono essere separati dal carattere ' ' (blank).
Il pathname del file di output deve essere comunicato all'applicazione 
tramite argv[1].
L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel caso
WinAPI) in modo tale che se il processo padre venga colpito il contenuto
del file di output venga interamente riversato su standard-output.
Nel caso in cui non vi sia immissione in input, l'applicazione non deve 
consumare piu' del 5% della capacita' di lavoro della CPU.*/

//26_06_2018 B

int main(int argc, char **argv)
{
	FILE *file;
	struct sembuf oper;
	int ret; 
	int numero_processi;
	char *num_proc = argv[2];
	char *nome_file = argv[1];
	numero_processi = strtol(num_proc, NULL, 10);
	int file_descriptor; 

	if(argc < 3)
		fprintf(stderr, "Errore nel passaggio degli argomenti [*] \n");

	address_mmap = mmap(NULL, N_pages*PAGE_SIZE, PROT_WRITE|PROT_READ, MAP_SHARED| MAP_ANONYMOUS, 0, 0);

	if(address_mmap==NULL)
		printf("Errore sul frammento di memoria condivisa [*] \n ");

	semaforo_per_processi = semget(IPC_PRIVATE,1,IPC_CREAT|IPC_EXCL|0666);
	semaforo_daddy = semget(IPC_PRIVATE, 1,IPC_CREAT|IPC_EXCL|0666);
	ret = semctl(semaforo_daddy,0, SETVAL, 0); //si blocca
	ret = semctl(semaforo_per_processi, 0, SETVAL, 1);


	signal(SIGINT,printer); 
	file_descriptor = open(nome_file, O_CREAT|O_RDWR, 0666);
	file = fdopen(file_descriptor, "r+");
	sprintf(command,"cat %s", nome_file);

	for(int i=0;i<numero_processi;i++)
	{
		if(fork())
			continue;
		else
			run();
	}


	
	while(1)
	{

		oper.sem_num = 0;
		oper.sem_op = -1;
		oper.sem_flg = 0;

redo1:
		ret = semop(semaforo_daddy, &oper, 1); //operazione bloccante
		if(ret == -1)
			if(errno==EINTR)
				goto redo1;
		//se parto perprimo, io padre mi fermo


redo2:

		//ora leggo dalla shared memory e scrivo sul file 
		ret = fprintf(file, "%d ", *address_mmap);
		if(ret!=-1)
			printf("Dati scritti correttamente [*] \n");
		fflush(file);

		oper.sem_num = 0;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo3:
	
		ret = semop(semaforo_per_processi, &oper, 1);
		if(ret==-1)
			if(errno == EINTR)
				goto redo3;		
	}

	return 0;
}

void run()
{
	//signal(SIGINT, SIG_IGN);
	int ret;
	struct sembuf oper;
	int valore;
	signal(SIGINT,SIG_IGN);
	while(1)
	{

		scanf("%d",&valore);

		oper.sem_num = 0;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo1:
		ret = semop(semaforo_per_processi, &oper, 1);
		if(ret==-1)
			if(errno==EINTR)
				goto redo1; //passa
		printf("Valore letto : %d\n", valore);

		*address_mmap = valore;

		oper.sem_num=0;
		oper.sem_op =1;
		oper.sem_flg =0;

		ret = semop(semaforo_daddy, &oper, 1);
		

	}
}

void printer(int sign)
{
	system(command);
	return;
}

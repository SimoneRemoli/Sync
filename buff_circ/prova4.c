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

/*Implementare una programma che riceva in input, tramite argv[], il nomi
di N file (con N maggiore o uguale a 1).

Per ogni nome di file F_i ricevuto input dovra' essere attivato un nuovo thread T_i.
Il main thread dovra' leggere indefinitamente stringhe dallo standard-input 
e dovra' rendere ogni stringa letta disponibile ad uno solo degli altri N thread
secondo uno schema circolare.

Ciascun thread T_i a sua volta, per ogni stringa letta dal main thread e resa a lui disponibile, 
dovra' scriverla su una nuova linea del file F_i. 

L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel caso
WinAPI) in modo tale che quando il processo venga colpito esso dovra' 
riversare su standard-output e su un apposito file chiamato "output-file" il 
contenuto di tutti i file F_i gestiti dall'applicazione 
ricostruendo esattamente la stessa sequenza di stringhe (ciascuna riportata su 
una linea diversa) che era stata immessa tramite lo standard-input.

In caso non vi sia immissione di dati sullo standard-input, l'applicazione dovra' utilizzare 
non piu' del 5% della capacita' di lavoro della CPU.



17_10_2019

*/

//>>./a.out file1 file2 file3 argc = 4

#define N 1024
FILE* output_file; 
FILE** source_files; 
char **buffers; 
char **file_names; 
int numero_threads; 
void *worker(void *);
void printer(int);
int semaforo_main, semaforo_threads;
char *file_output_name = "output_file";

int main(int argc, char **argv)
{
	int ret; 
	pthread_t tid; 
	struct sembuf oper;
	int index = 0;
	char *p; 
	if(argc<2)
	{
		fprintf(stderr, "Errore nella lettura dei file [*] \n");
	}
	file_names = argv;
	numero_threads = argc - 1;
	source_files = (FILE**)malloc(sizeof(FILE*)*numero_threads);
	buffers = (char**)malloc(sizeof(char*)*numero_threads);
	//file_desc_finale = open(file_output_name, O_CREAT|O_WRONLY|O_TRUNC, 0666);
	semaforo_main = semget(IPC_PRIVATE,1,IPC_CREAT|IPC_EXCL|0666);
	semaforo_threads = semget(IPC_PRIVATE,numero_threads,IPC_CREAT|IPC_EXCL|0666);
	semctl(semaforo_main,0, SETVAL, 1);
	for(int i=0;i<numero_threads;i++)
		semctl(semaforo_threads,i,SETVAL,0);
	/*
	[1] Semaforo main
	[0][0][0][0] Semafori threads
	*/

	signal(SIGINT, printer);
	for(long i = 0;i<numero_threads; i++)
	{
		ret = pthread_create(&tid, NULL, worker, (void*)i);
	}

	while(1)
	{
		//loop
		oper.sem_num = 0;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo1:
		ret = semop(semaforo_main, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo1;
		//passa
		printf("Inserisci la stringa da acquisire nel %s [*] \n ", file_names[index+1]);
		ret = scanf("%ms",&p);
		buffers[index] = p;

		oper.sem_op = 1;
		oper.sem_flg = 0;
		oper.sem_num = index; //sblocco solo 1 thread che scrive nel suo file 

redo2:
			
		ret = semop(semaforo_threads, &oper, 1);
		if(ret == -1)
		{
			if(errno == EINTR)
				goto redo2;
		}
	/*index = index + 1;
	if(index == numero_threads)
		index = 0;*/
	index = (index+1)%numero_threads;
	}
	return 0;
}

void *worker(void *a)
{
	FILE* target_file; // in + 
	long indice = (long) a;
	/*file_names[indice+1] è il nome del file singolo.*/
	target_file = fopen(file_names[indice+1],"w+"); //in + 
	int ret;
	struct sembuf oper;
	while(1)
	{
		oper.sem_num = indice;
		oper.sem_op = -1;
		oper.sem_flg = 0;

redo1:
		ret = semop(semaforo_threads, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo1; //bloccante 
redo2:
		printf("Verrà scritta la stringa %s [*] \n", buffers[indice]);
		//ret = write(file_descriptor, buffer, strlen(buffer));
		fprintf(target_file,"%s\n",buffers[indice]);//each string goes to a new line //IN + 
		fflush(target_file); // IN + 
		

		oper.sem_num = 0;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo3:
		ret = semop(semaforo_main, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo3;
	}
}
void printer(int signo)
{
	printf("\n");

	/*if(check==false)
	{

		for(int i = 0;i<numero_threads; i++)
		{
			printf("Prendo da %s \n ", file_names[i+1]);
			fptr = fopen(file_names[i+1], "r");
			while(fgets(buffered, N, fptr))
  				write(file_desc_finale, buffered, strlen(buffered));
  			fptr = NULL;
		}
		check = true;
	}
	if(check)
	{
		sprintf(command, "cat %s", file_output_name);
		system(command);
	}*/
	char *p;
	int i;
	int ret; 
	for(i=0;i<numero_threads;i++)
	{
		source_files[i] = fopen(file_names[i+1],"r"); //source files è doppio 
		printf("Nome del %i file = %s \n",i+1,file_names[i+1]);
	}
	output_file = fopen("output_file","w+");
	i = 0;
	while(1)
	{

		ret = fscanf(source_files[i],"%ms",&p);
		if(ret == EOF)
			break;
		printf("%s\n",p);
		fflush(stdout);
		fprintf(output_file,"%s\n",p);
		fflush(output_file);
		free(p);
		i = (i+1)%numero_threads;
	}
	return;
}
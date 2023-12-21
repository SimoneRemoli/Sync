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

23_01_2019
SPECIFICATION TO BE IMPLEMENTED:
Implementare una programma che riceva in input, tramite argv[], il nome
di un file F ed N stringhe S_1 .. S_N (con N maggiore o uguale
ad 1.

Per ogni stringa S_i dovra' essere attivato un nuovo thread T_i, che fungera'
da gestore della stringa S_i.
Il main thread dovra' leggere indefinitamente stringhe dallo standard-input. 
Ogni nuova stringa letta dovra' essere comunicata a tutti i thread T_1 .. T_N
tramite un buffer condiviso, e ciascun thread T_i dovra' verificare se tale
stringa sia uguale alla stringa S_i da lui gestita. In caso positivo, ogni
carattere della stringa immessa dovra' essere sostituito dal carattere '*'.
Dopo che i thread T_1 .. T_N hanno analizzato la stringa, ed eventualmente questa
sia stata modificata, il main thread dovra' scrivere tale stringa (modificata o non)
su una nuova linea del file F. 
In altre parole, la sequenza di stringhe provenienti dallo standard-input dovra' 
essere riportata su file F in una forma 'epurata'  delle stringhe S1 .. SN, 
che verranno sostituite da strighe  della stessa lunghezza costituite esclusivamente
da sequenze del carattere '*'.
Inoltre, qualora gia' esistente, il file F dovra' essere troncato (o rigenerato) 
all'atto del lancio dell'applicazione.

L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel caso
WinAPI) in modo tale che quando il processo venga colpito esso dovra' 
riversare su standard-output il contenuto corrente del file F.

*/

char **file_names;
int numero_di_stringhe_effettivamente_passate;
void* worker(void*);
char buffer_condiviso[1024];
int array_semaforico;
int semaforo_main;
int indexx = 0;
int check_equal(char **,int);
int file_descriptor;
char *nome_file="epurata";
FILE* file;
void sprint(int);
char buffer_command[156];

int main(int argc, char** argv)
{
	signal(SIGINT, sprint);
	pthread_t pid;
	struct sembuf oper;
	int ret;

	file_descriptor = open(nome_file, O_RDWR|O_CREAT|O_TRUNC, 0666);
	file = fdopen(file_descriptor, "w+");


	printf("Numero di argomenti passati = %d \n", argc);
	if(argc<2)
	{
		fprintf(stderr, "Errore nel passaggio degli argomenti [*] \n");
		printf("Aborted [*] \n");
		return 0;
	}
	file_names = argv;
	numero_di_stringhe_effettivamente_passate = argc - 1;
	printf("Numero di stringhe effettivamente passate = %d \n", numero_di_stringhe_effettivamente_passate);
	array_semaforico = semget(IPC_PRIVATE, numero_di_stringhe_effettivamente_passate, IPC_CREAT|IPC_EXCL|0666);
	semaforo_main = semget(IPC_PRIVATE, 1, IPC_CREAT|IPC_EXCL|0666);
	semctl(semaforo_main, 0, SETVAL, 1);
	for(int i=0;i<numero_di_stringhe_effettivamente_passate;i++)
	{
		semctl(array_semaforico, i, SETVAL, 0);
	}

	for(long i=0;i<numero_di_stringhe_effettivamente_passate;i++)
	{
		pthread_create(&pid, NULL, worker, (void*)i);
	}
	while(1)
	{
		oper.sem_num =0;
		oper.sem_op = -1; //sblocco il main (parte,non bloccante)
		oper.sem_flg = 0;

redo1:

		ret = semop(semaforo_main,&oper,1);
		if(ret==-1)
			if(errno==EINTR)
				goto redo1;



		printf("\n");
		printf("Inserisci la stringa >> ");
		if(fgets(buffer_condiviso, 1024, stdin))
		{
			buffer_condiviso[strcspn(buffer_condiviso, "\n")] = 0;
		}



		oper.sem_num = indexx; //sblocco il thread i-esimo
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo2:
		ret = semop(array_semaforico, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo2;

		oper.sem_num =0;
		oper.sem_op = -1; //blocco il main ()
		oper.sem_flg = 0;

redo3:

		ret = semop(semaforo_main,&oper,1);
		if(ret==-1)
			if(errno==EINTR)
				goto redo3;

	printf("Buffer condiviso = %s \n", buffer_condiviso); 
	printf("------------------------------------------------------------------------------------\n");
	fprintf(file, "%s\n", buffer_condiviso);
	fflush(file);
	indexx = indexx + 1;
		if(indexx==numero_di_stringhe_effettivamente_passate)
		{
			indexx=0;
		}
	}
	return 0;
}

void* worker(void *arg)
{
	long pid = (long) arg;
	struct sembuf oper;
	int ret;

	while(1)
	{


	oper.sem_num = pid;
	oper.sem_op = -1; //il thread i-esimo viene bloccato 
	oper.sem_flg = 0;

redo1:

	ret = semop(array_semaforico,&oper,1);
	if(ret==-1)
		if(errno==EINTR)
			goto redo1;


	printf("\n");
	printf("------------------------------------------------------------------------------------\n");
	printf("La stringa che gestisce il thread corrente è : %s [*] \n", file_names[pid+1]);
	printf("Confronto tra la stringa acquisita  %s  e la stringa che gestisce  %s  [*] \n", buffer_condiviso, file_names[pid+1]); 
	printf("Dimensione della stringa acquisita = %ld \n Dimensione della stringa gestita = %ld \n", strlen(buffer_condiviso), strlen(file_names[pid+1])); //nette

	if(check_equal(file_names, pid) == 1)
	{
		fprintf(stdout, "Stringa acquisita e stringa gestita sono uguali. Check OK [*] \n");
		for(int i=0;i<strlen(buffer_condiviso);i++)
		{
			buffer_condiviso[i] = '*';
		}
		printf("La stringa acquisita viene sostituita nel seguente modo : %s \n", buffer_condiviso);
	}
	else
		printf("Stringhe non uguali. (!=) \n");



	oper.sem_num = 0; //sblocca il main 
	oper.sem_op = 1;
	oper.sem_flg = 0;

redo3:
	ret = semop(semaforo_main, &oper, 1);
	if(ret==-1)
		if(errno == EINTR)
			goto redo3;



	oper.sem_num = 0;
	oper.sem_op = 1;
	oper.sem_flg = 0;

redo2:

	ret = semop(semaforo_main,&oper,1);
	if(ret==-1)
		if(errno==EINTR)
			goto redo2;
	}
}

int check_equal(char **file_names, int pid)
{
	int i = 0;
	if(strlen(buffer_condiviso)!=strlen(file_names[pid+1]))
		return -1;
	while(i<strlen(buffer_condiviso))
	{
		if(buffer_condiviso[i]==file_names[pid+1][i])
		{
			i++;
		}
		else
		{
			return -1;
		}	
	}
	if(i==strlen(file_names[pid+1]))
	{
		return 1;
	}
	else
		return -1;
	return -1;
}
void sprint(int unused)
{
	sprintf(buffer_command, "cat %s", nome_file);
	printf("\n ---------\n");
	printf("\t \t |");
	printf("\n");
	system(buffer_command);
	printf("\n");
	printf("\t \t |");
	printf("\n ---------\n");
}

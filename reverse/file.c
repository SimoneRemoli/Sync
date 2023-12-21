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
21_01_2020

SPECIFICATION TO BE IMPLEMENTED:
Implementare una programma che ricevendo in input tramite argv[] una stringa S
esegua le seguenti attivita'.
Il main thread dovra' attivare due nuovi thread, che indichiamo con T1 e T2.
Successivamente il main thread dovra' leggere indefinitamente caratteri dallo 
standard input, a blocchi di 5 per volta, e dovra' rendere disponibili i byte 
letti a T1 e T2. 
Il thread T1 dovra' inserire di volta in volta i byte ricevuti dal main thread 
in coda ad un file di nome S_diretto, che dovra' essere creato. 
Il thread T2 dovra' inserirli invece nel file S_inverso, che dovra' anche esso 
essere creato, scrivendoli ogni volta come byte iniziali del file (ovvero in testa al 
file secondo uno schema a pila).

L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel caso
WinAPI) in modo tale che quando il processo venga colpito esso dovra' 
calcolare il numero dei byte che nei due file hanno la stessa posizione ma sono
tra loro diversi in termini di valore. Questa attivita' dovra' essere svolta attivando 
per ogni ricezione di segnale un apposito thread.

In caso non vi sia immissione di dati sullo standard input, l'applicazione dovra' 
utilizzare non piu' del 5% della capacita' di lavoro della CPU.
*/
int newv=0;
char **file_names;
#define NUM_THREADS 2
int array_semaforico;
int semaforo_main;
void* worker(void*);
char buffer[5][1024];
int indexx = 0;
int file_descriptor_coda;
FILE* file_coda;
int file_descriptor_testa;
char diretto[1024];
char inverso[1024];
char *dir="_diretto";
char *inv ="_inverso";
void crea_nome(char*,char*);
void handle(int);
void* final(void*);
FILE* file_testa;
int statico = 1;
int o = 0;
int u = 0;
char **buffering;
char app[100000][1024];
FILE* apertura_diretto;
FILE* apertura_inverso;
void opens(char*, FILE*, char*, FILE*);
int contatore_hanno_la_stessa_posizione_ma_diversi_in_termini_di_valore = 0;


int main(int argc, char** argv)
{

	struct sembuf oper;
	pthread_t pid;
	file_names = argv;
	int ret;

	printf("Numero di argomenti passati = %d \n", argc);
	if((argc>2)||(argc<=1))
	{
		fprintf(stderr, "Errore nel passaggio degli argomenti [*] \n");
		sleep(1);
		printf("Aborted \n");
		return 0;
	}
	printf("Il nome del file passato è = %s \n", file_names[1]);

	crea_nome(dir,diretto);
	crea_nome(inv,inverso);
	
	printf("Nome del file diretto = %s \n", diretto);
	printf("Nome del file inverso = %s \n", inverso);


	file_descriptor_coda = open(diretto, O_RDWR|O_CREAT, 0666);
	file_coda = fdopen(file_descriptor_coda, "r+");

	//file_testa = 

	//printf("Valore di ritorno della fseek = %d \n ", fseek(file_testa, 0, SEEK_END));


	semaforo_main = semget(IPC_PRIVATE, 1, IPC_CREAT|IPC_EXCL|0666);
	array_semaforico = semget(IPC_PRIVATE, NUM_THREADS, IPC_CREAT|IPC_EXCL|0666);
	semctl(semaforo_main, 0, SETVAL, 1); //[1]
	for(int i=0;i<NUM_THREADS;i++)
	{
		semctl(array_semaforico, i, SETVAL, 0); //[0][0][0][0][0]...
	}


	for(long i=0;i<NUM_THREADS;i++)
	{
		pthread_create(&pid, NULL, worker, (void*)i);
	}


	signal(SIGINT, handle);
	while(1)
	{
		oper.sem_num=0;
		oper.sem_op = -1;
		oper.sem_flg=0;
redo1:	
		ret = semop(semaforo_main, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo1;


			//work
		sleep(2);
		printf("\n");
		for(int i=0;i<5;i++)
		{
			printf("Inserisci il dato numero [%d] ", i+1);
			printf(">>");

			if(fgets(buffer[i], 1024, stdin))
				buffer[i][strcspn(buffer[i], "\n")] = 0;
		}



		oper.sem_op = 1;
		oper.sem_flg = 0;
		for(long i=0;i<NUM_THREADS;i++)
		{
			oper.sem_num = i;
		
redo2:
		ret = semop(array_semaforico, &oper, 1); // sblocco il thread-esimo
		if(ret == -1)
			if(errno == EINTR)
				goto redo2;
		}
	}

	return 0;
}
void *worker(void *arg)
{
	long pid = (long) arg;
	struct sembuf oper;
	int ret;
	int counter = 0;
	//char **c = calloc (5*100000,sizeof(char*));
	while(1)
	{
		oper.sem_num = pid;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo1:
		ret = semop(array_semaforico, &oper, 1); //OPerazione threadesima bloccante
		if(ret==-1)
			if(errno == EINTR)
				goto redo1;




		//work here (thread sbloccato)

	
			sleep(1);
			printf("\n");
			if(pid==0)
			{
				file_descriptor_testa = open(inverso,O_RDWR|O_CREAT, 0666);
				file_testa = fdopen(file_descriptor_testa, "r+");
				printf("Spawning thread = %ld \n",pid);
				for(int i=0;i<5;i++)
				{
					strncpy(app[o], buffer[i], 1024);
					o = o + 1;
				}
				for(int i=(5*statico)-1;i>=0;i--)
				{
					fflush(file_testa);
					fprintf(file_testa,"%s\n",app[i]);
					//printf("Valore che verrà scritto = %s \n", app[i]);
					fflush(file_testa);
				}

				statico = statico + 1;
			}
			printf("\n");
			if(pid==1)
			{
				printf("Spawning thread = %ld \n",pid);
				for(int i=0;i<5;i++)
				{
					fflush(file_coda);
					fprintf(file_coda, "%s\n", buffer[i]);
					fflush(file_coda);
					
				}
			}

		oper.sem_num = 0;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo2:
		ret = semop(semaforo_main, &oper, 1);
		if(ret==-1)
			if(errno == EINTR)
				goto redo2;

		counter = counter + 1;
		if(counter==2)
			counter = 0;

	}
}

void crea_nome(char* suffisso, char *buffer)
{
	for(int i=0;i<strlen(file_names[1]);i++)
	{
		buffer[i] = file_names[1][i];
	}
	for(int i=strlen(file_names[1]), j=0;j<strlen(suffisso);i++,j++)
	{
		buffer[i]=suffisso[j];
	}
}

void handle(int unused)
{
	pthread_t tid;
	pthread_create(&tid, NULL, final, NULL);

}
void* final(void *arg)
{
	char *d;
	char *i;
	printf("\n");
	printf("SIGINT start [*] \n ");
	printf("\n");
    opens(diretto, apertura_diretto, inverso, apertura_inverso);




}
void opens(char *nome_file_diretto, FILE *apertura_diretto, char *nome_file_inverso, FILE *apertura_inverso)
{
	o = 0;
	u = 0;
	char appoggio1[statico*5][1024];
	char appoggio2[statico*5][1024];
	printf("\t------------------------\n");
	printf("\tApertura del file diretto %s . \n", nome_file_diretto);
	char d[2048];
	char i[2048];
	apertura_diretto = fopen(nome_file_diretto,"r");
	printf("\n");
    while(EOF != fscanf(apertura_diretto, "%[^\n]\n", d))
    {
         printf("\t%s\n", d);
         strncpy(appoggio1[o], d, 1024);
         o = o + 1;

    }

	printf("\tApertura del file inverso : %s . \n", nome_file_inverso);
	apertura_inverso = fopen(nome_file_inverso,"r");
	printf("\n");
    while(EOF != fscanf(apertura_inverso, "%[^\n]\n", i))
    {
         printf("\t%s\n", i);
         strncpy(appoggio2[u], i, 1024);
         u = u + 1;

    }
    printf("Parte il check che calcola il numero dei byte che nei due file \n hanno la stessa posizione ma sono tra loro diversi in termini di valore [*] \n");
    for(int i=0;i<(statico-1)*5;i++)
    {
    	if(strlen(appoggio1[i])==(strlen(appoggio2[i])))
    	{
    		contatore_hanno_la_stessa_posizione_ma_diversi_in_termini_di_valore = contatore_hanno_la_stessa_posizione_ma_diversi_in_termini_di_valore + 1;
    	}
    }

    printf(" Numero di byte che hanno la stessa posizione ma differenti come valore = %d [*] \n", contatore_hanno_la_stessa_posizione_ma_diversi_in_termini_di_valore);
    contatore_hanno_la_stessa_posizione_ma_diversi_in_termini_di_valore = 0;
    printf("\n");
   	printf("\t------------------------\n");

}
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

Simone Remoli.

19_09_2018
Implementare un programma che riceva in input tramite argv[] i pathname
associati ad N file (F1 ... FN), con N maggiore o uguale ad 1.
Per ognuno di questi file generi un thread che gestira' il contenuto del file.
Dopo aver creato gli N file ed i rispettivi N thread, il main thread dovra'
leggere indefinitamente la sequenza di byte provenienti dallo standard-input.
Ogni 5 nuovi byte letti, questi dovranno essere scritti da uno degli N thread
nel rispettivo file. La consegna dei 5 byte da parte del main thread
dovra' avvenire secondo uno schema round-robin, per cui i primi 5 byte
dovranno essere consegnati al thread in carico di gestire F1, i secondi 5
byte al thread in carico di gestire il F2 e cosi' via secondo uno schema
circolare.

L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel caso
WinAPI) in modo tale che quando il processo venga colpito esso dovra',
a partire dai dati correntemente memorizzati nei file F1 ... FN, ripresentare
sullo standard-output la medesima sequenza di byte di input originariamente
letta dal main thread dallo standard-input.
*/

char **filenames; 
FILE **source_files;
FILE **source_files_2;
void *worker(void*);
char buffer[5][2048];
int semaforo_main;
int array_semaforico;
void create_file(int);
long indice = 0;
int numero_thread;
void handle(int);
int contatore = -1;

struct nodo
{
	struct nodo *next;
	char value[2048];
};
struct nodo* testa = NULL;
struct nodo* app = NULL;

int main(int argc, char** argv)
{
	pthread_t tid;
	int ret;
	struct sembuf oper;
	filenames = argv;
	numero_thread = argc - 1;
	printf("\tNumero di argomenti passati = %d \n", argc);
	printf("\tNumero dei file passati = %d \n ", numero_thread);
	source_files = (FILE**)malloc(sizeof(FILE*)*numero_thread);
	source_files_2=(FILE**)malloc(sizeof(FILE*)*numero_thread);

	semaforo_main = semget(IPC_PRIVATE, 1, IPC_CREAT|IPC_EXCL|0666);
	array_semaforico = semget(IPC_PRIVATE, numero_thread, IPC_CREAT|IPC_EXCL|0666);
	semctl(semaforo_main, 0, SETVAL, 1);//[1]
	for(int i=0;i<numero_thread;i++)
		semctl(array_semaforico,i,SETVAL,0);//[0][0][0][0]..

	create_file(numero_thread);

	for(long i=0;i<numero_thread;i++)
	{
		pthread_create(&tid, NULL, worker,(void*) i);
	}

	signal(SIGINT, handle);

	while(1)
	{
		oper.sem_num = 0;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo1:
		ret = semop(semaforo_main,&oper,1);
		if(ret==-1)
			if(errno==EINTR)
				goto redo1;

		
		printf("\n");
		contatore = contatore + 1;
		for(int i=0;i<5;i++)
		{
			printf("Inserisci byte numero [%d] \n",i+1);
			printf(">>");
			if(fgets(buffer[i],2048, stdin))
			{
				if((strlen(buffer[i]))!=2)
				{
					printf("Errore [*]. Devi leggere 1 byte solamente [*]!! \n");
					sleep(1);
					printf("Riprova \n");
					contatore = contatore - 1;
					i = i - 1;
					sleep(1);
				}
				else if((strlen(buffer[i]))==2)
				{
					buffer[i][strcspn(buffer[i], "\n")] = 0;
					struct nodo* nuovo_nodo = malloc(sizeof(struct nodo));
					strncpy(nuovo_nodo->value, buffer[i], 2048);
					if(testa == NULL)
					{
						app = nuovo_nodo;
						testa = app;
					}
					else
					{
						app->next = nuovo_nodo;
						app = nuovo_nodo;
					}
				}
			}
		}


		oper.sem_num = indice;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo2:	
		ret = semop(array_semaforico,&oper,1);
		if(ret==-1)
			if(errno==EINTR)
				goto redo2;


		indice = indice + 1;
		if(indice == numero_thread)
			indice = 0;

	}
	return 0;
}

void create_file(int numero_thread)
{
	for(int i=0;i<numero_thread;i++)
	{
		source_files[i] = fopen(filenames[i+1], "w+");
		printf("\tFile numero %d = %s \n ", i+1, filenames[i+1]);
	}
}
void *worker(void *arg)
{
	struct sembuf oper;
	int ret;
	long indice = (long) arg;

	while(1)
	{

		oper.sem_num = indice;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo1:	
		ret = semop(array_semaforico,&oper,1);
		if(ret==-1)
			if(errno == EINTR)
				goto redo1;


		for(int i=0;i<5;i++)
		{
			printf("Valore letto dal thread [%ld] = %s \n", indice, buffer[i]);
			fprintf(source_files[indice], "%s\n", buffer[i]);
			fflush(source_files[indice]);
		}

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
void handle(int unused)
{
	printf("\n"),
	printf("\t.. [*] Sigint Activated [*] .. \n");
   	struct nodo * temp = testa;
	while(temp!=NULL)
	{
		printf(" | %s \n",temp->value);
		temp = temp -> next;
	}
	printf("\n");
}
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

FILE** source_files;

FILE* output_file;
int check(int, char**);
char **file_names;
void views();
int numero_dei_threads_che_dovranno_essere_attivati;
void *worker(void*);
int semaforo_main;
int array_semaforico;
char buffer[256];
long contatore = 0;
void handle(int);
int file_descriptor;
char *nome_file = "output_file";
int contatore_acquisizioni;

struct nodo
{
	char value[256];
	struct nodo* next;
};

struct nodo* testa = NULL;
struct nodo* app = NULL;

int main(int argc, char **argv)
{
	pthread_t pid;
	int ret;
	struct sembuf oper;
	printf("Numero di argomenti passati = %d \n ", argc);
	if(check(argc, argv))
		return 0;
	printf("Procedura di lettura dei file inseriti [*] \n");
	file_names = argv;
	numero_dei_threads_che_dovranno_essere_attivati = argc-1;

	views(numero_dei_threads_che_dovranno_essere_attivati);

	source_files = (FILE**)malloc(sizeof(FILE*)*numero_dei_threads_che_dovranno_essere_attivati);
	for(int i=0;i<numero_dei_threads_che_dovranno_essere_attivati;i++)
		source_files[i] = fopen(file_names[i+1], "w+");



	semaforo_main =  semget(IPC_PRIVATE, 1, IPC_CREAT|IPC_EXCL|0666);
	array_semaforico = semget(IPC_PRIVATE, numero_dei_threads_che_dovranno_essere_attivati, IPC_CREAT|IPC_EXCL|0666);
	semctl(semaforo_main, 0, SETVAL, 1);
	for(int i=0;i<numero_dei_threads_che_dovranno_essere_attivati;i++)
		semctl(array_semaforico, i, SETVAL, 0);



	for(long i=0;i<numero_dei_threads_che_dovranno_essere_attivati;i++)
		pthread_create(&pid, NULL, worker, (void*)i);


	signal(SIGINT, handle);

	while(1)//main thread
	{
		oper.sem_num = 0;
		oper.sem_op = -1;
		oper.sem_flg = 0;


redo1:
		ret = semop(semaforo_main, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo1;


		if(fgets(buffer, 256, stdin))
			buffer[strcspn(buffer, "\n")] = 0;

		contatore_acquisizioni = contatore_acquisizioni + 1;
		printf("Acquisizione numero %d effettuata [*] \n", contatore_acquisizioni);

		struct nodo* nuovo_nodo = malloc(sizeof(struct nodo));
		strncpy(nuovo_nodo->value, buffer, 256);

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

		oper.sem_num = contatore;
		oper.sem_op = 1;
		oper.sem_flg = 0;

redo2:

		ret = semop(array_semaforico, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo2;


		contatore = contatore + 1;
		if(contatore == numero_dei_threads_che_dovranno_essere_attivati)
			contatore = 0;
	}
	return 0;
}
int check(int argc, char** argv)
{
	if(argc < 2)
	{
		fprintf(stderr, "Errore, devi passare come argomento almeno un file [*] \n");
		return 1;
	}
	else
		return 0;
}
void views()
{
	for(int i=0;i<numero_dei_threads_che_dovranno_essere_attivati;i++)
	{
		printf("File numero [%d] inserito = %s \n", i+1, file_names[i+1]);

	}
}

void* worker(void* arg)
{
	int ret;
	long pid = (long) arg;
	struct sembuf oper;

	while(1)
	{
		oper.sem_num = pid;
		oper.sem_op = -1;
		oper.sem_flg = 0;

redo1:
		ret = semop(array_semaforico, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo1;

		printf("Valore letto dal thread numero [%ld] = %s [*] \n", pid+1, buffer);
		fprintf(source_files[pid], "%s\n", buffer);
		fflush(source_files[pid]);

		oper.sem_num = 0;
		oper.sem_flg = 0;
		oper.sem_op = 1;

redo2:
		ret = semop(semaforo_main, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo2;
	}

}
void handle(int unused)
{
	int i = 0;
	char pointer[1024];
	file_descriptor = open(nome_file, O_RDWR|O_CREAT, 0666);
	output_file = fdopen(file_descriptor, "w+");



	struct nodo* temp = testa;


	for(int i=0;i<numero_dei_threads_che_dovranno_essere_attivati;i++)
	{
		fseek(source_files[i], 0,0);
	}

	while(temp != NULL)
	{
		printf("%s \n ", temp->value);
		fprintf(output_file, "%s\n", temp->value);
		fflush(output_file);
		temp = temp->next;
	}


	printf("\n");
	printf("Oppure [*] ... \n");
	printf("\n");
	i = 0;


	while(EOF != fscanf(source_files[i], "%[^\n]\n", pointer))
	{
		printf("Valore del pointer = %s \n", pointer);
		fprintf(output_file, "%s\n", pointer);
		fflush(output_file);
		i = i + 1;
		if(i == numero_dei_threads_che_dovranno_essere_attivati)
			i = 0;


	}
	// NOTA: Nel file di output ovviamente si vedrà un duplicato poichè anche la lista carica i suoi 
	// dati nel file. 
	/*
	Basta levare lo scorrimento della lista e funziona
	*/










}
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
#define NUMERO_THREADS 2

void *worker(void*);
int check(int, char**);
char caratteri[5][1024];
char app[1000][1024];
char c;
int semaforo_main;
int semaforo_threads;
char *stringa_passata;
char* file_diretto = "_diretto";
char *file_inverso = "_inverso";
int file_descriptor_diretto;
int file_descriptor_inverso;
FILE* file_diretto_star;
FILE* file_inverso_star;
int j = 0;
int l = 0;
char values[256];
char diretto[2048];
char inverso[2048];
void crea_nome(char*,char*);
void handle(int);
int statico = 0;
void *worker2(void *);
int contatore = 0;
int check1 = 0;

int main(int argc, char **argv)
{
	pthread_t pid;
	struct sembuf oper;
	int ret;
	printf("Numero di argomenti passati = %d \n ", argc);
	if(check(argc, argv))
	{
		fprintf(stderr, "Errore nel passaggio degli argomenti [*] - Devi inserire solo un argomento - [*] \n" );
		return 0;
	}
	stringa_passata = argv[1];
	printf("Argomento passato = %s \n ", stringa_passata);
	semaforo_main = semget(IPC_PRIVATE, 1, IPC_CREAT|IPC_EXCL|0666);
	semaforo_threads = semget(IPC_PRIVATE, NUMERO_THREADS, IPC_CREAT|IPC_EXCL|0666);
	crea_nome(diretto,file_diretto);
	crea_nome(inverso,file_inverso);
	
	printf("Nome del file diretto = %s \n", diretto);
	printf("Nome del file inverso = %s \n", inverso);

	file_descriptor_diretto = open(diretto, O_CREAT|O_RDWR, 0666);
	file_diretto_star = fdopen(file_descriptor_diretto, "w+");
	file_descriptor_inverso = open(inverso, O_RDWR|O_CREAT|O_TRUNC, 0666); //ogni volta è aggiornato
	file_inverso_star = fdopen(file_descriptor_inverso, "w+");

	semctl(semaforo_main, 0, SETVAL, 1); // [1]
	for(int i=0;i<NUMERO_THREADS;i++)
		semctl(semaforo_threads, i, SETVAL, 0); // [0][0]

	for(long i=0;i<NUMERO_THREADS;i++)
	{
		pthread_create(&pid, NULL, worker, (void*)i);
	}

	signal(SIGINT, handle);
	while(1)
	{
		oper.sem_num = 0;
		oper.sem_flg = 0;
		oper.sem_op = -1;
redo1:	
		ret = semop(semaforo_main, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo1;


		sleep(1);
		for(int i=0;i<5;i++)
		{
			if(check1)
			{
				printf("\n \t Errore - devi inserire solo un carattere [*] - Riprova. \n \t ");
				i = 0;
				check1 = 0;
			}
			printf("Inserisci il [%d] carattere da passare ai threads [*] \n", i+1);
			printf(">>");
			if(fgets(caratteri[i], 1024, stdin))
			{
				if((strlen(caratteri[i]) == 1))
					goto label;
				if(strlen(caratteri[i])!=2)
				{
					if(i==0)
					{
						check1 = 1;
					}
					if(i!=0)
					{
						printf("\n \t Errore - devi inserire solo un carattere [*] - Riprova. \n \t ");
						i = i - 1;
					}
				}
				else if(((strlen(caratteri[i])==2)))
				{
					goto label;
				}
label:
				caratteri[i][strcspn(caratteri[i], "\n")] = 0;
			}																

		}

		oper.sem_flg = 0;
		oper.sem_op = 1;

		for(int i=0;i<NUMERO_THREADS;i++)
		{
			oper.sem_num = i;
redo2:
			ret = semop(semaforo_threads, &oper, 1);
			if(ret==-1)
				if(errno == EINTR)
					goto redo2;
		}
	}
	return 0;
}
int check(int argc, char ** argv)
{
	if((argc < 2)||(argc > 2))
		return 1;
	if(argc==2)
		return 0;
}

void* worker(void *arg)
{
	struct sembuf oper;
	int ret;
	long pid = (long) arg;

	while(1)
	{
		oper.sem_num = pid;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo1:

		ret = semop(semaforo_threads, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo1;

		if(pid==0)//t1
		{
			for(int i=0;i<5;i++)
			{
				fprintf(file_diretto_star, "%s", caratteri[i]);
				fflush(file_diretto_star);
			}
			printf("Update - File Diretto [*] \n");
			fprintf(file_diretto_star, "\n");

		}
		if(pid == 1)//t2
		{
			statico = statico + 1;
			//printf("Statico = %d \n", statico);

			for(int i=0;i<5;i++)
			{
				//printf("Valore di caratteri = %s \n", caratteri[i]);
				strcat(app[l], caratteri[i]); //man mano concateno la stringa
				//printf("Valore di app = %s \n", app[0]); 

			}
			l = l + 1;
			file_descriptor_inverso = open(inverso, O_RDWR|O_CREAT|O_TRUNC, 0666); //ogni volta è aggiornato
			file_inverso_star = fdopen(file_descriptor_inverso, "w+");
			for(int i=statico-1;i>=0;i--)
			{
				//printf("Valore di app[i] = %s \n", app[i]);
				fflush(file_inverso_star);
				fprintf(file_inverso_star, "%s\n", app[i]);
				fflush(file_inverso_star);
			}
			printf("Update - File Inverso [*] \n");

		}
		oper.sem_num = 0;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo2:
		ret = semop(semaforo_main, &oper, 1);
		if(ret == -1)
			if(errno ==EINTR)
				goto redo2;
	}

}

void crea_nome(char *nome_file_diretto, char *file)
{
	for(int i=0;i<strlen(stringa_passata);i++)
	{
		nome_file_diretto[i] = stringa_passata[i];
	}
	for(int i=strlen(stringa_passata),j=0;j<strlen(file);i++,j++)
	{
		nome_file_diretto[i] = file[j];
	}
}
void handle(int unused)
{
	printf("CTRL + C Pressed - [*] \n");
	pthread_t pid;
	pthread_create(&pid, NULL, worker2, NULL);
}
void* worker2(void* arg)
{
	fseek(file_diretto_star, 0,SEEK_SET);
	fseek(file_inverso_star, 0,SEEK_SET);

	printf("Il thread è stato attivato [*] \n");
	char appoggio_diretto[statico][1024];
	char appoggio_inverso[statico][1024];
	char pointer1[2048];
	char pointer2[2048];
	int o = 0;
	int u = 0;

	while(EOF != fscanf(file_diretto_star, "%[^\n]\n", pointer1))
	{
		//printf("Valore del pointer1 = %s \n ",pointer1);
		strcpy(appoggio_diretto[o],pointer1);
		o = o + 1;
	}
	while(EOF != fscanf(file_inverso_star, "%[^\n]\n", pointer2))
	{
		//printf("Pointer 2 = %s \n", pointer2);
		strcpy(appoggio_inverso[u], pointer2);
		u = u + 1;
	}
	printf("- Contenuto del file diretto [*] -  \n");
	for(int i=0;i<statico;i++)
	{
		printf("\t %s \n", appoggio_diretto[i]);
		printf("DIM = %ld \n", strlen(appoggio_diretto[i]));
	}
	printf("- Contenuto del file inverso [*] -  \n");
	for(int i=0;i<statico;i++)
	{
		printf("\t  %s \n", appoggio_inverso[i]);
		printf("DIM = %ld \n", strlen(appoggio_inverso[i]));
	}
	printf("\n");
	printf("- - - - - - - - - - - - \n");
	printf("\n");
	printf("- Allineamenti file - [*] \n");
	printf("\n");
	for(int i=0;i<statico;i++)
	{
		printf("\t  %s   %s  \t [%d]  \n", appoggio_inverso[i], appoggio_diretto[i],i);
		if((strlen(appoggio_diretto[i]) == strlen(appoggio_inverso[i])))
		{
			if(strcmp(appoggio_diretto[i],appoggio_inverso[i])!=0)
			{
				printf("valore = %s e %s \n", appoggio_diretto[i], appoggio_inverso[i]);
				contatore = contatore + 1;
				printf("Posizione = %d \n", i);
			}
		}
	}
	printf("\t Numero dei bytes che nei due file hanno la stessa posizione ma sono diversi in termini di ''valore'' = %d [*] \n",contatore);
	contatore = 0;
}
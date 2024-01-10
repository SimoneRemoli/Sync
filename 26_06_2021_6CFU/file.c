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


/*

26_06_2021_6CFU
Implementare una programma che riceva in input, tramite argv[], il nome
di un file F. Il programma dovra' creare il file F e popolare il file
con lo stream proveniente da standard-input. Il programma dovra' generare
anche un ulteriore processo il quale dovra' riversare il contenuto del file F
su un altro file denominato shadow_F, inserendo mano a mano in shadow_F soltanto 
i byte che non erano ancora stati prelevati in precedenza da F. L'operazione 
di riversare i byte di F su shadow_F dovra' avvenire con una periodicita' di 10
secondi.  

L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel caso
WinAPI) in modo tale che quando un qualsiasi processo (parent o child) venga colpito si
dovra' immediatamente riallineare il contenuto del file shadow_F al contenuto
del file F sempre tramite operazioni che dovra' eseguire il processo child.
*/

int check(int,char**);
int go_check(int,char**);
char *nome_file;
int file_descriptor, file_descriptor2;
FILE* file;
FILE* file2;
void process();
char buffer[2048];
char *address_map;
int check_in = 0;
char appoggio[1024];
char array[1024];
void handle(int);

int main(int argc, char** argv) 
{
	pid_t processo;
	int ret;
	printf("Numero di argomenti passati = %d\n ", argc-1);
	if(go_check(argc,argv))
		return 0;
	printf("File creato correttamente [*]\n");
	file_descriptor = open(nome_file, O_CREAT|O_TRUNC|O_RDWR, 0666);
	file = fdopen(file_descriptor, "w+");
	file_descriptor2 = open("_shadow_F", O_CREAT|O_TRUNC|O_RDWR, 0666);
	file2 = fdopen(file_descriptor2, "w+");
	address_map = mmap(NULL, 1024, PROT_WRITE|PROT_READ, MAP_SHARED|MAP_ANONYMOUS, 0, 0);

	processo = fork();
	if(processo==0)
	{
		process();
	}
	if(processo == -1)
		fprintf(stderr, "Errore nella generazione del processo [*]\n");

	signal(SIGINT, handle);

	while(1)
	{
		while(check_in==0)
		{

			printf("Inserisci dei byte da acquisire : \n");
			printf(">>");

			if(fgets(appoggio, 1024, stdin))
				appoggio[strcspn(appoggio, "\n")] = 0;
			if(strlen(appoggio)!=1)
			{
				printf("Devi inserire 1 byte [*] - Retry. \n");
				check_in = 0;
			}
			if(strlen(appoggio)==1)
			{
				strcat(address_map , &appoggio[0]);
				check_in = 1;
				printf("Valore scritto nel file = %c\n", appoggio[0]);
				//strcat(address_map, &appoggio[0]);
				fprintf(file, "%c\n", appoggio[0] ); //già c'è il \n
				fflush(file);
			}
		}
		check_in = 0;
	}
	return 0;
}
int check(int argc, char** argv)
{
	if(argc == 2)
		return 1;
	else
		return 0;
}
int go_check(int argc, char** argv)
{
	if(check(argc,argv))
	{
		printf("Numero di argomenti passati corretto [*]\n");
		nome_file = argv[1];
		printf("Nome del file passato = %s [*]\n", nome_file);
		return 0;
	}
	else
	{
		printf("Devi passare solo come argomento il nome di un file [*]\n");
		fprintf(stderr, "Aborted [*]\n" );
		return 1;
	}
}
void process()
{
	while(1)
	{
		sleep(10);
		for(int i=0;i<strlen(address_map);i++)
		{
			printf("Valore scritto = %c [*] \n", address_map[i]);
			fprintf(file2, "%c\n", address_map[i] ); //già c'è il \n
	   		fflush(file2);
		}
	    strcpy(address_map,"");
	}
}
void handle(int unused)
{
	printf("Riallineamento di [%ld] bytes. \n", strlen(address_map));


	for(int i=0;i<strlen(address_map);i++)
	{
		printf("Valore scritto = %c [*] \n", address_map[i]);			
		fprintf(file2, "%c\n", address_map[i] ); //già c'è il \n
	   	fflush(file2);
	}
	strcpy(address_map,"");
	exit(-1);
}

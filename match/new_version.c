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
#include <stdbool.h>
#define N 1024

/*Scrivere un programma che riceva in input tramite argv[] il nome di un file
F e una stringa S contenente un numero arbitrario di caratteri. 
Il main thread dell'applicazione dovra' creare il file F e poi leggere indefinitamente 
stringhe dallo standard input per poi scriverle, una per riga, all'interno del file.

Qualora venga ricevuto il segnale SIGINT (o CTRL_C_EVENT nel caso WinAPI), dovra'
essere lanciato un nuovo thread che riporti il contenuto del file F all'interno di un 
altro file con lo stesso nome, ma con suffisso "_shadow", sostituendo tutte le stringhe
che coincidono con la stringha S ricevuta dall'applicazione tramite argv[] con 
una stringha della stessa lunghezza costituita da una sequenza di caratteri '*'.
Il lavoro di questo thread dovra' essere incrementale, ovvero esso dovra' riportare 
sul file shadow solo le parti del file originale che non erano state riportate in 
precedenza. Al termine di questa operazione, il thread dovra' indicare su standard 
output il numero di stringhe che sono state sostituite in tutto.*/

//17_09_2021

char **argomenti;
char *file_name; 
char *stringa;
int file_descriptor;
FILE* file;
char buffer[1024];
void handle(int);
char *new_file;
void *worker(void*);
int file_descriptor_2;
FILE** filez;
FILE* file_shadow;


int main(int argc, char **argv)
{
	printf("Numero di argomenti passati = %d [*] \n", argc-1);
	if(argc!=3)
	{
		fprintf(stderr, "Errore nel passaggio degli argomenti [*] \n");
		return 0;
	}
	argomenti = argv;
	file_name = argomenti[1];
	stringa = argomenti[2];

	file_descriptor = open(file_name, O_CREAT|O_RDWR|O_TRUNC, 0666);
	file = fdopen(file_descriptor, "w+");

	filez = (FILE**)malloc(sizeof(FILE*)*2);

	signal(SIGINT, handle);
	while(1)
	{
		if(fgets(buffer, 1024, stdin))
			buffer[strcspn(buffer, "\n")] = 0;

		fprintf(file, "%s\n", buffer);
		fflush(file);
	}
	return 0;
}
void handle(int unused)
{
	char *file_shadow_completo;

	pthread_t pid;
	//creiamo il suffisso _shadow
	char *suffisso = "_shadow";
	char new_file_shadow[strlen(suffisso)+strlen(file_name)];
	printf("%ld %ld", strlen(suffisso), strlen(file_name));
	new_file = file_name;

	for(int i=0;i<strlen(new_file);i++)
	{
		new_file_shadow[i] = new_file[i];
	}
	for(int i=strlen(new_file_shadow),j=0;i<strlen(suffisso)+strlen(new_file_shadow);i++,j++)
	{
		new_file_shadow[i] = suffisso[j];
	}

	printf("Nome del nuovo file = %s \n", new_file_shadow);
	file_shadow_completo = &new_file_shadow[0];

	pthread_create(&pid, NULL, worker, file_shadow_completo);

	pthread_join(pid,NULL);
	exit(-1);
}
void* worker(void *stringaa)
{
	int check = 0;
	char pointer[1024];
	int j = 0;
	char *nuovo_file_shadow = (char*)stringaa;
	printf("Nome del nuovo file con il suffisso = %s \n", nuovo_file_shadow);
	printf("Nome del vecchio file senza il suffisso = %s \n", file_name);

	//file_descriptor_2 = open(file_name, O_CREAT|O_TRUNC|O_RDWR, 0666);
	file_descriptor_2 = open(nuovo_file_shadow, O_CREAT|O_TRUNC|O_RDWR, 0666);
	file_shadow = fdopen(file_descriptor_2, "r+");

	filez[0] = fopen(file_name, "r+");

	while(EOF != fscanf(filez[0], "%[^\n]\n", pointer))
	{
		printf("Leggo la stringa dal file = %s \n", pointer);
		for(int i=0;i<strlen(pointer);i++)
		{
			check = 0;
			//printf("pointer[%d] = %c \n", i, pointer[i]);
			if((pointer[i] == stringa[j])&&(strlen(pointer)==strlen(stringa)))
			{
				j = j + 1;
				if(j==strlen(stringa))
				{
					//printf("stringa uguale \n");
					check = 1;
				}
			}
			if((pointer[i] != stringa[j])&&(j<strlen(stringa)))
			{
				check = 0;
			}
			if(strlen(pointer)!=strlen(stringa))
				check = 0;
		}
		if(check==1)
		{
			printf("Stringa uguale a %s !\n ", stringa);
			for(int i=0;i<strlen(stringa);i++)
				pointer[i]='*';
			fprintf(file_shadow, "%s\n", pointer);
			fflush(file_shadow);

		}
		if(check == 0)
		{
			printf("Stringa non uguale a %s  \n", stringa);
			fprintf(file_shadow, "%s\n", pointer);
			fflush(file_shadow);
		}
		j = 0;
	}
}
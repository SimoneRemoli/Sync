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

void gestore(int);
void gestion(int);
char *nome_file;
char command[1024];
char command2[1024];
int status;
int descrittore_semaforico; 
unsigned long int ID;
FILE *ex_desc;
char *stringa;
int main(int argc, char **argv)
{
	struct sembuf oper;
	descrittore_semaforico = semget(IPC_PRIVATE,1,IPC_CREAT|IPC_EXCL|0666);
	semctl(descrittore_semaforico, 0, SETVAL, 2);	
	char buffer[1024];

	if(argc<3)
	{
		fprintf(stderr, "Errore nella lettura degli argomenti [*] \n");
		return 0;
	}
	nome_file = argv[1];
	stringa = argv[2];
	sprintf(command, "cat %s", nome_file);
	int ret;
	int file_descriptor; 
	file_descriptor = open(nome_file, O_RDWR|O_CREAT, 0666);

	if(file_descriptor== -1)
	{
		printf("Errore nella creazione del file [*] \n ");
		printf("Chiusura applicazione [*] \n");
		return 0;
	}
	else
		printf("File aperto correttamente [*] \n");
	signal(SIGINT,gestion);
	ID = pthread_self();
	printf(" ID thread = %ld \n ", ID);

	while(1) 
	{
		oper.sem_num = 0;
		oper.sem_op = -1;
		oper.sem_flg = 0;

		semop(descrittore_semaforico, &oper, 1);
//----------------------------------------------------------------
		fprintf(stdout, "Inserisci una stringa: ");
		if(fgets(buffer, 1024, stdin))
			buffer[strcspn(buffer, "\n")] = 0;
		write(file_descriptor, buffer, strlen(buffer));
		printf("Hai scritto = %s \n",buffer);
//----------------------------------------------------------------
		oper.sem_num = 0;
		oper.sem_op =  1;
		oper.sem_flg = 0;
		semop(descrittore_semaforico, &oper, 1); //non bloccante
	}
	return 0;
}

void gestore(int fd) 
{
	system(command);
	exit(-1);
	return;
}
void *funzione(void *arg)
{
	char buffer[N];
	struct sembuf oper;
	int ret;
	char *new_name_shadow = (char*) arg;
	int new_desc;
	printf("\n Nome del nuovo file  = %s\n", new_name_shadow);
	ex_desc = fopen(nome_file, "r");
	new_desc = open(new_name_shadow, O_CREAT|O_WRONLY|O_TRUNC, 0666);
	int lunghezza_stringa_da_confrontare = strlen(stringa); //argv
	int j = 0,i=0;
	bool check = false;
	while(fgets(buffer, N, ex_desc))
	{
		for(i=0;i<N;i++)
			{
				check = false;
				if(buffer[i]==stringa[j])
				{
					
					if(j==lunghezza_stringa_da_confrontare-1)
					{
						for(int h=i-j;h<i+1;h++)
						{
							buffer[h] = '*';
						}
						check = true;
					}
					
					if(check)
						j = 0;
					else
						j = j + 1;
				}
				else
				{
					j = 0;
				}
			}
  			write(new_desc, buffer, strlen(buffer));
	}
	printf(".....\n");
	system(command);
	printf("--------\n");
	sprintf(command2, "cat %s", new_name_shadow);
	system(command2);
	printf("------- \n");
	fclose(ex_desc);
	close(new_desc);
}

void gestion(int fd) 
{
	pthread_t t;
	int ret; 
	struct sembuf oper;
	char *str = "_shadow";
	char new_name_shadow[1024];
	for(int i=0;i<strlen(nome_file);i++)
	{
		new_name_shadow[i] = nome_file[i];
	}

	for(int i=(strlen(new_name_shadow)),j=0;i<strlen(new_name_shadow)+strlen(str);i++,j++)
		new_name_shadow[i] = str[j];

	char *pointer = &new_name_shadow[0];
	oper.sem_num = 0;
	oper.sem_op = -1;
	oper.sem_flg = 0;
redo1:
	ret = semop(descrittore_semaforico, &oper,1);
	if(ret==-1) //bloccante 
		if(errno==EINTR)
			goto redo1;
	
	pthread_create(&t, NULL, funzione, pointer);
	pthread_join(t,(void**)&status);
	printf("Aborted - All successfully  [*] \n");
	exit(-1);
	return;
}













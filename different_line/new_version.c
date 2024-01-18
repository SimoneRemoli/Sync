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

int check(int, char**);
int numero_argomenti_effettivi;
int numero_processi_concorrenti;
int file_descriptor;
FILE* file;
char buffer[1024];
int array_semaforico;
char *back = "backup";
FILE* file_back;
int file_descriptor_backup;

char **file_names;
void handle(int);
void run(int);
int contatore_inverse = 0;

int main(int argc, char **argv)
{

	struct sembuf oper;
	int ret;
	if(check(argc,argv)==0)
	{
		fprintf(stderr, "Aborted [*] \n");
		return 0;
	}
	numero_argomenti_effettivi = argc - 1;
	printf("Numero di argomenti effettivi = %d \n", numero_argomenti_effettivi);
	file_names = argv;
	printf("Nome del file = %s \n", file_names[1]);
	for(int i=2;i<numero_argomenti_effettivi+1;i++)
	{
		printf("Stringa [%d] = %s\n", (i-2)+1, file_names[i]);
	}
	numero_processi_concorrenti = numero_argomenti_effettivi - 1;
	array_semaforico = semget(IPC_PRIVATE, numero_processi_concorrenti, IPC_EXCL|IPC_CREAT|0666);
	semctl(array_semaforico, 0, SETVAL, 1);
	for(int i=1;i<numero_processi_concorrenti;i++)
	{
		semctl(array_semaforico, i, SETVAL, 0); 
	}
	file_descriptor = open(file_names[1], O_CREAT|O_RDWR|O_TRUNC, 0666);
	file = fdopen(file_descriptor, "w+");
	printf(" - %s creato correttamente [*] - \n",file_names[1]);



	signal(SIGINT, handle);
	while(1)
	{
		printf("\n");
		printf("Inserisci una stringa [*] \n");
		printf(">>");
		if(fgets(buffer, 1024, stdin))
			buffer[(strcspn(buffer,"\n"))] = 0;

		sleep(1);
		printf("- Update %s [*] - ", file_names[1]);
		fprintf(file, "%s\n", buffer);
		fflush(file);

	}




	return 0;
}


int check(int argc, char **argv)
{
	if(argc == 1)
		return 0;
	if(argc==2)
	{
		fprintf(stderr, "Errore [*] - Devi inserire anche delle stringhe [*] \n" );
		return 0;
	}
	if(argc > 2)
		return 1;

}
void handle(int unused)
{
	printf("\n");
	printf("- CTRL + C pressed [*] - \n");
	printf("Numero di processi concorrenti = %d \n", numero_processi_concorrenti);

	for(int i=0;i<numero_processi_concorrenti;i++)
	{
		if(fork())
			continue;
		else
			run(i);
	}

}

void run(int pid)
{
	int process_id = pid;
	struct sembuf oper;
	int ret;
	char buffer_inverso[1024];
	char pointer[1024];
	fseek(file, 0, 0);

	while(1)
	{
		fseek(file, 0, 0);
		oper.sem_num = process_id;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo1:
		ret = semop(array_semaforico, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo1;

		sleep(1);
		printf("\n");
		printf("- Sono il processo numero [%d] - \n", pid+1);
		printf("- Verifico se la inversa di %s è presente nel file %s [*] - \n", file_names[pid+2], file_names[1]);

		for(int i=0,j=strlen(file_names[pid+2])-1;i<strlen(file_names[pid+2]);i++,j--)
		{
			buffer_inverso[i] = file_names[pid+2][j];
		}
		printf("La stringa inversa è = %s \n", buffer_inverso);



		while(EOF != fscanf(file, "%[^\n]\n", pointer))
		{
			printf(" \t \t  %s \n", pointer);
			if(strcmp(buffer_inverso,pointer)==0)
			{
				printf(" - %s = %s - \n", buffer_inverso, pointer);
				contatore_inverse = contatore_inverse + 1;
			}

		}
		printf("-- RealTime ''contatore inverse'' = %d -- \n", contatore_inverse);

		if(pid==numero_processi_concorrenti-1)
		{
			printf("\n");
			printf("\t - Tutti i processi hanno effettuato il controllo [*] - ");
			file_descriptor_backup = open(back, O_CREAT|O_RDWR|O_APPEND, 0666);
			file_back = fdopen(file_descriptor_backup, "w+");

			fseek(file, 0, 0);

			while( EOF != fscanf(file, "%[^\n]\n", pointer))
			{
				fprintf(file_back, "%s\n", pointer);
				fflush(file_back);
			}
			open(file_names[1], O_TRUNC, 0666);
			printf("- File %s [TRUNC] - [*] \n", file_names[1]);
		}




		printf("\n");

		oper.sem_num = process_id + 1;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo2:
		ret = semop(array_semaforico, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo2;
	}




}
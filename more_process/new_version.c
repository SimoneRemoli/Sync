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
char ** file_names;
int contatore = 0;
int conta = 0;
int coppie = 1;
void run(int);
char buffer[1024];
int semaforo_main;
int array_semaforico; 
int proc_next = 0;
FILE** star_file;
FILE* file; 
int descrittore;
void handle(int);

int main(int argc, char **argv)
{
	int returno;
	int ret;
	struct sembuf oper;
	returno = check(argc, argv);
	if(returno == 0)
	{
		fprintf(stderr, "Errore. Devi passare coppie di stringe a due a due [*] \n" );
		return 0;
	}
	printf("Numero di argomenti coinvolti = %d \n", argc-1);
	file_names = argv;
	for(int i=1;i<argc;i++)
	{
		
		if(i%2!=0)
		{
			printf("\n");
			printf("Coppia numero %d [*] \n", coppie);
			printf("\t Nome del file [%d] = %s\n", contatore, file_names[i]);
			contatore = contatore + 1;
			coppie = coppie + 1;
		}
		if(i%2==0)
		{
			printf("\t Nome della stringa [%d] = %s\n", conta, file_names[i]);
			conta = conta + 1;
		}
		
	}

	printf("\n - Attivazione di ->% d <- processi [*] - \n", contatore);
	star_file = (FILE**)malloc(sizeof(FILE*)*contatore);
	semaforo_main = semget(IPC_PRIVATE, 1, IPC_CREAT|IPC_EXCL|0666);
	semctl(semaforo_main, 0, SETVAL, 1);//[1]
	array_semaforico = semget(IPC_PRIVATE, contatore, IPC_CREAT|IPC_EXCL|0666);
	for(int i=0;i<contatore;i++)
		semctl(array_semaforico, i, SETVAL, 0);//[0][0][0]




	for(int i=0;i<contatore;i++)
	{
		if(fork())
			continue;
		else
			run(i);
	}

	signal(SIGINT, handle);

	while(1)
	{
		oper.sem_num = 0;
		oper.sem_op = -1;
		oper.sem_flg = 0;

redo1:
		ret = semop(semaforo_main, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo1;

		printf("\n");
		printf("Inserisci la stringa da scrivere sul file  %s [*] \n",file_names[proc_next+(proc_next+1)]);
		printf(">>");
		if(fgets(buffer, 1024, stdin))
			buffer[strcspn(buffer, "\n")] = 0;



		descrittore = open(file_names[proc_next+(proc_next+1)], O_RDWR|O_APPEND, 0666);
		file =  fdopen(descrittore, "r+");
		fprintf(file, "%s\n", buffer);
		fflush(file);
		
		printf("\t Attiverò il processo  = %d \n", proc_next);




		oper.sem_num = proc_next;
		oper.sem_op = 1;
		oper.sem_flg = 0;

redo2:
		ret = semop(array_semaforico, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo2;

	/*	proc_next = proc_next + 1;
		if(proc_next == contatore )
			proc_next = 0;
	*/

	}



	return 0;
}
int check(int argc, char** argv)
{
	if(argc%2==0)
		return 0;
	if(argc%2!=0)
	{
		if(argc==1)
			return 0;
		if(argc>1)
			return 1;
	}

}
void run(int i)
{
	int pid = i;
	struct sembuf oper;
	int ret;
	char pointer[1024];
	int contatore_stringa=0;

	//bisogna creare i file
	fopen(file_names[pid+(pid+1)], "w+");

	signal(SIGINT, SIG_IGN);

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


		printf("\n");
		printf("Sono il processo [%d] - \n", pid);
		printf("Apertura file  %s [*]\n", file_names[pid+(pid+1)]);
		printf("Bisogna confrontare la stringa passata  [%s] con le stringe inserite nel file  [%s] [*] \n", file_names[pid+(pid+2)],file_names[pid+(pid+1)]);
		star_file[pid] = fopen(file_names[pid+(pid+1)], "r+");

		while(EOF != fscanf(star_file[pid], "%[^\n]\n", pointer))
		{
			//printf("Valore nel file = %s \n ", pointer); 
			if(strcmp(pointer, file_names[pid+(pid+2)])==0)
				contatore_stringa = contatore_stringa + 1;
		}
		printf("La stringa passata compare nel file %d volte [*] \n", contatore_stringa);
		contatore_stringa = 0;

		oper.sem_num = 0;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo2:
		ret = semop(semaforo_main, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo2;



	}
}
void handle(int unused)
{
	printf("- SIGINT ACTVATED - [*] \n");
	proc_next = proc_next + 1;
	printf("proc = %d \n", proc_next);
	if(proc_next == contatore)
		proc_next = 0;
}
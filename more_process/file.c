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
I deeply thank the warm help of my little kitty, so tender and so sweet, who has always supported me in carrying out this program.
I love you <3 .

Start.

07_09_2021
Scrivere un programma che riceva in input tramite argv[] N coppie di stringhe 
con N maggiore o uguale a 1, in cui la prima stringa della coppia indica il 
nome di un file. 

Per ogni coppia di strighe dovra' essere attivato un processo che dovra' creare 
il file associato alla prima delle stringhe della coppia e poi ogni 5 secondi 
dovra' effettuare il controllo su quante volte la seconda delle stringhe della 
coppia compare nel file, riportando il risultato su standard output.


Il main thread del processo originale dovra' leggere lo stream da standard input in 
modo indefinito, e dovra' scrivere i byte letti in uno dei file identificati 
tramite i nomi passati con argv[]. 

La scelta del file dove scrivere dovra' 
avvenire in modo circolare a partire dal primo file identificato tramite argv[], 
e il cambio del file destinazione dovra' avvenire qualora venga ricevuto il 
segnale SIGINT (o CTRL_C_EVENT nel caso WinAPI).



Se il segnale SIGINT (o CTRL_C_EVENT nel caso WinAPI) colpira' invece uno degli 
altri processi, questo dovra' riportare il contenuto del file che sta gestendo 
su standard output.
*/


char** file_names;
FILE ** source_file;
void run(int,int);
void process(int,int);
int semaforo;
int semaforo_processo;
char **array;
int descrittore;
FILE *file;
void handle(int, siginfo_t *, void *);
void sprintf_file(int, siginfo_t *, void *);
int indexx;
int counter = 0;
int contatore_gestione_segnale = 0;
char buffer[1024];

int main(int argc, char** argv)
{
	sigset_t set;
	struct sigaction act;

	sigfillset(&set);

  	act.sa_sigaction = handle; 
 	act.sa_mask =  set;
 	act.sa_flags = SA_SIGINFO;
 	act.sa_restorer = NULL;
 	sigaction(SIGINT,&act,NULL);

 	sigprocmask(SIG_BLOCK,&set,NULL);


	char *p;
	int i,j;
	struct sembuf oper;
	int proc;
	int indice;
	indice = indexx;
	file_names = argv;

	int ret;

	printf("Numero di argomenti passati = %d\n", argc);

	if((argc%2==0)||(argc==1))
	{
		fprintf(stderr, "Errore nel passaggio degli argomenti. Coppie non scritte [*] \n");
		printf("Aborted [*] \n");
		return 0;
	}
	source_file = (FILE**)malloc(sizeof(FILE*)*(argc-1)/2);
	semaforo = semget(IPC_PRIVATE, 1, IPC_CREAT|IPC_EXCL|0666);
	semctl(semaforo, 0, SETVAL, 1); 
	semaforo_processo = semget(IPC_PRIVATE, (argc-1), IPC_CREAT|IPC_EXCL|0666); //array_semaforico
	semctl(semaforo_processo, 0, SETVAL,0); 


	for(i=0;i<(argc-1)/2;i++) 
	{
		if(fork())
			continue;
		else
			process(i,argc);
	}
	while(1) //main thread deve scrive dentro sti file
	{
		oper.sem_num=0;
		oper.sem_op = -1;
		oper.sem_flg=0;

redo1:
		ret = semop(semaforo, &oper, 1); //non bloccante 
		if(ret==-1)
			if(errno==EINTR)
				goto redo1;


		sigpending(&set);
		if(sigismember(&set,SIGINT))
		{ //solo se premo CTRL + C 
			  contatore_gestione_segnale = contatore_gestione_segnale + 1;
	 		  sigemptyset(&set);
	 		  sigaddset(&set,SIGINT);
			  sigprocmask(SIG_UNBLOCK,&set,NULL);
			  sigprocmask(SIG_BLOCK,&set,NULL);
		}
		if(contatore_gestione_segnale==(((argc-1)/2)))
		{
			indexx = 0; //cuore
			contatore_gestione_segnale = 0;
		}
		descrittore = open(file_names[indexx+(indexx+1)], O_RDWR|O_APPEND, 0666);
		file = fdopen(descrittore, "r+");
		printf("\n Inserisci la stringa da inserire nel file : %s\n", file_names[indexx+(indexx+1)]); //qui dobbiamo scrivere dentro al file 
		printf(">>");
		scanf("%ms", &p);
		fprintf(file, "%s", p);
		fflush(file);

		indice = indexx;

		oper.sem_num=indice; //sblocco in ordine i processi
		oper.sem_op =1;
		oper.sem_flg=0;

redo2:
		ret = semop(semaforo_processo, &oper, 1); //non bloccante 
		if(ret==-1)
			if(errno==EINTR)
				goto redo2;
		//-----------------------------------------------------------------------------------------------------------		
	}
	return 0;
}
void process(int pid,int argc)
{
	sigset_t set;
	int ret;
	struct sembuf oper;
	char *p;
	int i=0;
	int j=0;
	char *stringa_buff;
	fopen(file_names[pid+(pid+1)],"w+");

	sigfillset(&set);
 	sigprocmask(SIG_BLOCK,&set,NULL);


	while(1)
	{
		//leggi da file
		oper.sem_num=pid;
		oper.sem_op = -1;
		oper.sem_flg=0;

redo1:
		ret = semop(semaforo_processo, &oper, 1); // bloccante 
		if(ret==-1)
			if(errno==EINTR)
				goto redo1;

		sigpending(&set);
		if(sigismember(&set,SIGINT))
		{
			printf("\n");
			sprintf(buffer, "cat %s", file_names[pid+(pid+1)]);
			printf("\n----------\n");
			printf("Contenuto del %s che correntemente è gestito :  [*] \n", file_names[pid+(pid+1)]);
			system(buffer);
			printf("\n----------\n");
			printf("\n");
	 		sigemptyset(&set);
	 		sigaddset(&set,SIGINT);
			sigprocmask(SIG_UNBLOCK,&set,NULL);
	 		sigprocmask(SIG_BLOCK,&set,NULL);

		}
		printf("\n");


//--------------------------------------------------------------------------------
		source_file[pid] = fopen(file_names[pid+(pid+1)],"r");
		printf("Nome del file appena letto: %s \n", file_names[pid+(pid+1)]);
		fscanf(source_file[pid],"%ms",&p);
		printf(" Valore letto = %s\n", p);
		printf("Il valore %s verrà confrontato con il valore %s .\n", p, file_names[pid+(pid+2)]);
		stringa_buff = file_names[pid+(pid+2)];

		for(int i=0;i<strlen(p);i++)
		{
			if(p[i] == stringa_buff[j])
			{
				if(j==strlen(stringa_buff)-1)
				{
					//printf("\n La stringa è presente [*] \n ");
					counter = counter + 1;
					j=-1;
				}
				j = j + 1;
			}
		}
		printf("Numero occorrenze: %d  [*] \n", counter);
		printf("\n");
		counter = 0;
		fflush(stdout);
		free(p);
		
//--------------------------------------------------------------------------------

		oper.sem_num=0;
		oper.sem_op =1;
		oper.sem_flg=0;

redo2:
		ret = semop(semaforo, &oper, 1); //non bloccante 
		if(ret==-1)
			if(errno==EINTR)
				goto redo2;

	}
	exit(-1);
}
void handle(int signal, siginfo_t * a, void *b)
{
	indexx = indexx + 1;
}

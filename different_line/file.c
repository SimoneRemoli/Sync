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


/*SPECIFICATION TO BE IMPLEMENTED:

21-07-2021 9 CFU.

Implementare una programma che riceva in input, tramite argv[], il nome
di un file F e un insieme di N stringhe (con N almeno pari ad 1). Il programa dovra' creare 
il file F e popolare il file con le stringhe provenienti da standard-input. 
Ogni stringa dovra' essere inserita su una differente linea del file.


L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel caso
WinAPI) in modo tale che quando il processo venga colpito si
dovranno  generare N processi concorrenti ciascuno dei quali dovra' analizzare il contenuto
del file F e verificare, per una delle N stringhe di input, quante volte la inversa di tale stringa 
sia presente nel file. Il risultato del controllo dovra' essere comunicato su standard
output tramite un messaggio. Quando tutti i processi avranno completato questo controllo, 
il contenuto del file F dovra' essere inserito in "append" in un file denominato "backup"
e poi il file F dova' essere troncato.

Qualora non vi sia immissione di input o di segnali, l'applicazione dovra' utilizzare 
non piu' del 5% della capacita' di lavoro della CPU.
*/

int j = 0;
char **file_index;
void print_string(int);
int write_on_file(int);
FILE* file;
FILE *file2;
FILE *back_desc;
FILE *file_lett;
int descrittore;
void gestion(int);
int numero_stringhe_effettivamente_passate;
void process(int);
int semaforo_main;
int array_semaforico;
int indice = 0;
char buffer[1024];
int counter = 0;
char buffer_inversa[1024];
char new_buffer[1024];
int contatore_inverse=0;
char *file_backup = "backup";
int desc_back;
char *pointer;
int descrittore2;
char buffer_nuovo[1024];

int main(int argc, char** argv)
{
	int ret;
	struct sembuf oper;
	numero_stringhe_effettivamente_passate = argc-2;

	semaforo_main = semget(IPC_PRIVATE, 1, IPC_CREAT|IPC_EXCL|0666);
	semctl(semaforo_main, 0, SETVAL, numero_stringhe_effettivamente_passate);//[N]
	array_semaforico = semget(IPC_PRIVATE, numero_stringhe_effettivamente_passate, IPC_CREAT|IPC_EXCL|0666);
	for(int i=0;i<numero_stringhe_effettivamente_passate;i++)
	{
		semctl(array_semaforico, i, SETVAL, 0); //[0][0][0][0]...
	}


	if((argc == 0)||(argc == 1)||(argc == 2))
	{
		printf("Errore nel passaggio degli argomenti [*] \n ");
		printf("Aborted [*] \n");
		return 0;

	}
	else
	{
		printf("\t ------------------------------------\n");
		printf("\t Numero di argomenti passati = %d \n ", argc);
		printf("\t Numero di stringhe passate  = %d \n", argc-2);
		printf("\n");


	}
	file_index = argv;
	print_string(argc);
	printf("\n");
	signal(SIGINT, gestion);
	while(1)
	{

		oper.sem_num = 0;
		oper.sem_op = -numero_stringhe_effettivamente_passate;
		oper.sem_flg = 0;
redo1:
		ret = semop(semaforo_main, &oper, 1); //non bloccante, semaforo main settato ad n
		if(ret==-1)
			if(errno == EINTR)
				goto redo1;

		oper.sem_op = 1;
		oper.sem_flg = 0;
		oper.sem_num = indice;

redo2:
			
		ret = semop(array_semaforico, &oper, 1);
		if(ret==-1)
			if(errno==EINTR)
				goto redo2;

		printf("indice = %d e numero_stringhe_effettivamente_passate = %d\n",indice,numero_stringhe_effettivamente_passate);
		if(indice == numero_stringhe_effettivamente_passate)
		{

			desc_back = open(file_backup, O_CREAT|O_RDWR|O_APPEND, 0666);
			back_desc = fdopen(desc_back, "w+");
			descrittore2 = open(file_index[argc-(argc-1)], O_CREAT|O_RDWR, 0666);//1
			file_lett = fdopen(descrittore2, "w+");
			while (EOF != fscanf(file_lett, "%1024[^\n]\n", buffer_nuovo))//tutti leggono tutto
    		{
    			fprintf(back_desc, "%s\n", buffer_nuovo);
				fflush(back_desc);
    		}
    		free(file_lett);
    		free(back_desc);
    		free(file);
    		free(file2);

    		descrittore2 = open(file_index[argc-(argc-1)],O_TRUNC, 0666);//1
			return 0;
		}
		indice = indice + 1;
	}
	printf("continua");
	return 0;
}
void print_string(int argc)
{
	char buffer[1024];
	for(int i=0;i<argc-2;i++)
	{
		printf("\t La stringa passata numero %d è = %s [*] \n ", i+1, file_index[i+2]);
	}
	printf("\t ------------------------------------\n");
	if(write_on_file(argc))
	{
		fprintf(stdout, "\t Dati scritti correttamente nel file : ""%s"" [*] \n",file_index[argc-(argc-1)]);
		sprintf(buffer, "cat %s",file_index[argc-(argc-1)]);
		printf("\t Esecuzione automatica del comando >> %s  .[*]\n", buffer);
		system(buffer);
		printf("Press CTRL+C (^C) N.B: Ad intervalli regolari di un secondo verranno spawnati %d processi ...", numero_stringhe_effettivamente_passate);
	}
	return;
}
int write_on_file(int argc)
{
	int ret;
	//file = fopen(file_index[argc-(argc-1)], "w+"); //in alternativa poteva essere utilizzata solo questa istruzione, invece che di 1 e 2 
	descrittore = open(file_index[argc-(argc-1)], O_CREAT|O_RDWR, 0666);//1
	file = fdopen(descrittore, "w+");//2
	for(int i=0;i<argc-2;i++)
	{
		ret = fprintf(file, "%s \n", file_index[i+2] );
		if(ret==-1)
			return -1;
		fflush(file);
	}
	return 1;
}
void gestion(int unused)
{
	for(int i=0;i<numero_stringhe_effettivamente_passate;i++)
	{
		if(fork())
			continue;
		else
			process(i);
	}
	return;
}
void process(int pid)
{
	//printf("Process Id = %d \n", pid);
	struct sembuf oper;
	int ret;
	char *p;


	while(1)
	{
		oper.sem_num = pid;
		oper.sem_op = -1;
		oper.sem_flg = 0;

redo1:
		ret = semop(array_semaforico, &oper, 1); //bloccante
		if(ret == -1)
			if(errno == EINTR)
				goto redo1;


//-----------------------------------------------------------
		sleep(1);
		printf("\n (Spawning [*]) --> Pid attivato = %d \n", pid);
		file = fopen(file_index[1], "r+");  
		while (EOF != fscanf(file, "%1024[^\n]\n", buffer))//tutti leggono tutto
    	{
    		if(counter==pid)
    		{
        	 	printf("Valore trovato dal processo numero %d =  %s\n",pid,buffer);
        	 	printf("Dimensione della stringa = %ld\n",  strlen(buffer)-1);
        	 	//troviamo la inversa
        	 	for(int i=strlen(buffer)-2,j=0;j<strlen(buffer)-1;i--,j++)
        	 	{
        	 		buffer_inversa[j] = buffer[i];
        	 	}
        	 	file2 = fopen(file_index[1], "r+");  
        	 	printf("Valore della stringa inversa = %s\n", buffer_inversa);
        	 	printf("Dimensione della stringa inversa = %ld\n", strlen(buffer_inversa));

        	 	while (EOF != fscanf(file2, "%1024[^\n]\n", new_buffer))//tutti leggono tutto
    			{
    				for(int i=0;i<strlen(buffer_inversa);i++)
    				{
    					if(buffer_inversa[i]==new_buffer[j])
    					{

    						j = j + 1;
    						if(j==strlen(buffer_inversa))
    						{
    							j = 0;
    							contatore_inverse++;
    						}
    					}
    					else
    					{
    						i = strlen(buffer_inversa); //esce dal ciclo
    						j = 0;
    					}
    				}	
    			}
				printf("Occorrenze inverse di %s, ossia %s,  nel file %s ---> %d [*]\n", buffer, buffer_inversa, file_index[1], contatore_inverse);
				contatore_inverse = 0;
    		}
    		counter = counter + 1;
    	}
//-----------------------------------------------------------

		oper.sem_num = 0;
		oper.sem_op = numero_stringhe_effettivamente_passate;
		oper.sem_flg = 0;
redo2:
		ret = semop(semaforo_main, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo2;

	}
}

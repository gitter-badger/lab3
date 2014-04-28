#include <iostream>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>

#define products 10
#define max_rand 20
#define min_rand 5
#define minv 1
#define maxv 100
#define empty 0
#define full 1
#define proda_state 2
#define konsb_state 3
#define critic 4



using namespace std;

int semafor,pids[4],pamiec;
string objects[]={"ProdA","ProdB","KonsA","KonsB"};
key_t key;
union semun {
	int val;
	struct semid_ds *buf;
	unsigned short int *array;
	struct seminfo *__buf;
};
int *ptr_size;
int *ptr_heap;

void proda();
void prodb();
void konsa();
void down(int sem_nr);
void up(int sem_nr);
void text_push(int nr, int num);
string get_name();
int get_id();
void aaa(int a)
{
	sleep(1);
}
void show_heap()
{
	//down(critic);
	cout<<"[STOS] ma "<<*ptr_size<<" elementow: ";
	for(int i=0;i<*ptr_size;i++)
		cout<<*(ptr_heap+i*4)<<" ";
	cout<<get_name()<<endl;
	//up(critic);
}

int wartosc()
{
	int out=0;
	for(int i=0;i<*ptr_size;i++)
		out+=*(ptr_heap+i*4);
	return out;
}

void push(int a)
{
	while(!(semctl(semafor,1,GETVAL)))
	{
		cout<<"[STOS] jest pelny "<<get_name()<<endl;
		sleep(3);

	}
	down(critic);
	*(ptr_heap+((*ptr_size)*4))=a;
	(*ptr_size)++;
	if(*ptr_size==1)
		up(empty);
	if(*ptr_size==9)
		down(full);
	cout<<"[STOS] WartoscPush = "<<wartosc()<<endl;
	if(wartosc()>19 && semctl(semafor,2,GETVAL))
		down(proda_state);
	cout<<get_name()<<" zostawil size  "<<*ptr_size<<endl;
	show_heap();
	sleep(1);
	up(critic);
}

void push_a(int a)
{
	while( (!(semctl(semafor,1,GETVAL))) && (!(semctl(semafor,2,GETVAL))))
	{
		cout<<get_name<<" czeka"<<endl;
	}
	down(critic);
	*(ptr_heap+((*ptr_size)*4))=a;
	(*ptr_size)++;
	if(*ptr_size==1)
		up(empty);
	if(*ptr_size==9)
		down(full);
	cout<<"[STOS] WartoscPushA = "<<wartosc()<<endl;
	if(wartosc()>19 && semctl(semafor,2,GETVAL))
		down(proda_state);
	cout<<get_name()<<" zostawil size"<<*ptr_size<<endl;
	show_heap();
	sleep(1);
	up(critic);
}

int pop()
{
	cout<<get_name()<<" stara sie zdjac ze stosu"<<endl;
	while(!(semctl(semafor,0,GETVAL)))
	{
		cout<<"[STOS] jest pusty"<<get_name()<<endl;
		sleep(3);
	}
	down(critic);
	if(*ptr_size==9)
		up(full);
	(*ptr_size)--;
	if(*ptr_size==0)
		down(empty);
	cout<<"[STOS] WartoscPoP = "<<wartosc()<<endl;
	if(wartosc()<20)
		up(proda_state);
	int out = *(ptr_heap+((*ptr_size)*4));
	cout<<get_name()<<" zdjal ze stosu i zostawil size "<<*ptr_size<<endl;
	show_heap();
	up(critic);
	return out;
}

void down(int sem_nr)
{
	union semun arg;
	int s;
	arg.val=0;
	while(1)
	{
		s=semctl(semafor,sem_nr,GETVAL);
		if( (s==1 && ( get_id()!=0  ) )||(s==1 && get_id()==0 && semctl(semafor,2,GETVAL) ) )
		{
			semctl(semafor,sem_nr,SETVAL,arg);
			cout<<"[SEMAFOR] o nr "<<sem_nr<<" zostal opuszczony przez ["<<get_name()<<"]"<<endl;
//			arg.val=0;
//			semctl(semafor,sem_nr,SETVAL,arg);
			sleep(1);
			break;
		}
		else
		{
			cout<<"[SEMAFOR] o nr "<<sem_nr<<" jest opuszczony, czekam ["<<get_name()<<"]"<<endl;
		}
		sleep(3);
	}

}

void up(int sem_nr)
{
	union semun arg;
	if(semctl(semafor,sem_nr,GETVAL)==0)
	{
		arg.val=1;
		semctl(semafor,sem_nr,SETVAL,arg);
		cout<<"[SEMAFOR] zostal podniesiony semafor o nr "<<sem_nr<<" przez "<<get_name()<<endl;
	}
	else
		cout<<"jakies dziwne DANE!!LALASDASLDASLDASDLA"<<sem_nr<<endl;
}

int losuj(int min,int max)
{
	srand(time(NULL)^(getpid()<<16));
	return (rand()%(max-min+1)+min);
}

void child(int n)
{
	int pid = getpid();
	pids[n]=pid;
	if(n==1)
	{
		cout<<"[ProdB] wlaczyl sie do akcji z pid = "<<pid<<endl;
		prodb();
	}
	else if(n==2)
	{
		konsa();
	}
	else if(n==0)
	{
		proda();
	}
	else if(n==3)
	{

	}
}

void init()
{
	union semun arg;
	arg.val=1;
	for(int i=1;i<5;i++)
		semctl(semafor,i,SETVAL,arg);
	arg.val=0;
	semctl(semafor,0,SETVAL,arg);

	pamiec = shmget(2014,40,IPC_CREAT | 0666);
	ptr_size = (int *)shmat(pamiec,0,0);
	ptr_heap = (int *)ptr_size+4;
	for(int i=0;i<10;i++)
	{
		*(ptr_size+i*4)=0;
	}
}

int main()
{
	int pid,status=0;
	union semun argument;
	unsigned short values[1];
	if((key = ftok(".",'a'))==-1)
	{
		cout<<"Nie utworzono klucza "<<errno<<endl;
		exit(1);
	}
	else
		cout<<"Klucz utworzono"<<endl;

	if((semafor=semget(key,5,IPC_CREAT | 0660))==-1)
	{
		cout<<"Nie utworzono semafora, powod: "<<errno<<endl;
		exit(1);
	}
	else
		cout<<"Utworzono semafor o id: "<<semafor<<endl;
	init();
	sleep(5);
	for(int i=0;i<4;i++)
	{
		int p=fork();
		if(p>=0)
		{
			if(p==0)
			{//child process

				if((pamiec = shmget(2014,40,IPC_CREAT))==-1)
				{
					cout<<"Pamiec nie zostala zaalokowana, error: "<<errno<<endl;
				}
				ptr_size=(int *)shmat(pamiec,0,0);
				ptr_heap=(int *)ptr_size+4;
				child(i);
				exit(1);
			}
			else
			{//parrent process
//				cout<<"Utworzono proces rodzic"<<endl;
			}
		}
		sleep(1);
	}

	while((pid=wait(&status)) >0)
	{
//		cout<<"proces o nr "<<pid<<" zakonczony ze statusem "<<status<<endl;
	}

	return 0;
}

void proda()
{
	for(int i=0;i<products;i++)
	{
//		while(!(semctl(semafor,2,GETVAL)));
//		cout<<endl<<endl<<"weszlem1"<<endl<<endl;
		int val=losuj(minv,maxv);
		push_a(val);
		text_push(0,val);
//		show_heap();
//		sleep(losuj(1,10));
		cout<<i+1<<" "<<get_name()<<endl;
		sleep(losuj(1,10));
	}

}

void prodb()
{
	for(int i=0;i<products;i++)
	{
		int val=losuj(minv,maxv);
		push(val);
		text_push(1,val);
//		show_heap();
//		sleep(losuj(1,10));
		cout<<i+1<<" "<<get_name()<<endl;
		sleep(losuj(1,10));
	}
}

void konsa()
{
	for(int i=0;i<products;i++)
	{
		int out = pop();
		cout<<get_name()<<" zdjal ze stosu liczbe: "<<out<<endl;
//		show_heap();
		cout<<get_name()<<" ma size "<<*ptr_size<<endl;
//		sleep(losuj(1,10));
		cout<<i+1<<" "<<get_name()<<endl;
		sleep(losuj(1,10));
	}
}

void text_push(int nr, int num)
{
	cout<<get_name()<<" wrzucil na stos liczbe: "<<num<<endl;
}

string get_name()
{
	int pid=getpid();
	for(int i=0;i<4;i++)
		if(pid==pids[i])
			return "["+objects[i]+"]";
}
int get_id()
{
	for(int i=0;i<4;i++)
		if(getpid()==pids[i])
			return i;
}

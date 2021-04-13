/*
derleme: gcc -o process part2.c -pthread -lrt 
çalıştırma: ./process 1000 5 i1 i2 i3 i4 i5
*/

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <sys/shm.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <sys/mman.h>
#include <math.h>
#include <time.h> 
#include <wait.h>
#include "bstree.h"

#define MAXFILENAMESIZE 8
#define SHAREDMEMSIZE 10000
#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                                   } while (0)

struct shmbuf {
               sem_t  readWriteSema;            
               size_t cnt;
               int K;
               int maxValues[SHAREDMEMSIZE]; 
           };

const char* MemoryMapname = "PART2"; 
int *processList;
char **fileList;
int  numOfFiles=0;
int numOfNodesInTree;

sem_t wait2End;

 void throwRunCommandError(){
      printf("\tInvalid Run Command ERROR!!!!\n");
      printf("\tPlease run program like :\n");
      printf("\tpart1 K N infile1 ... infileN\n") ; 
      printf("\tRemember File Names can be max 8 char lenght\n") ; 
      exit(1);

 }

void insertInorder(int number2Add, int arr[],int size){
 int i,j;

   for(i=0;i<size && arr[i]<number2Add && arr[i] != -1 ;i++);
//i=2
   if(i != size){
      for(j=size;j>i;j--)
      	arr[j] = arr[j-1];
      
   }
   if(arr[i]!= number2Add)
      arr[i]= number2Add;
 }

 void processWork(int myID){
   printf("Process #%d started: \n", myID );

    FILE *fptr = NULL;
    int readed,breakIt=0;
    int shm_fd; 
    int r;
    int enb=0;
    srand(time(NULL));   
    
    shm_fd = shm_open("PART2", O_RDWR , 0); //Paylaşılan hafıza okumak için açılır

    struct shmbuf *MMptr = mmap(0,(SHAREDMEMSIZE*sizeof(int))  ,PROT_READ | PROT_WRITE , MAP_SHARED, shm_fd, 0); //MMptr hafızadaki bilgilere erişir
       
     if (MMptr == MAP_FAILED)
           errExit("mmap");

   while(myID<numOfFiles && breakIt == 0){
       if (sem_wait(&MMptr->readWriteSema) == -1)
           errExit("sem_wait");       

       if (fptr == NULL){
           if ((fptr = fopen(fileList[myID], "r")) == NULL) {//dosyayı aç
               printf("Error! opening file in Process %d Filename: %s\n",myID,fileList[myID]);
               exit(1);
           }
           printf("Process %d opened File: %s \n", myID ,fileList[myID]);
       }

       if( fscanf(fptr, "%d", &readed) == 1){//dosyadan sayıları al değişkene at
            printf("Process %d readed from File: %s - Value %d \n", myID ,fileList[myID],readed);
            
	    if(readed>enb){
	   	enb=readed;}//okunan sayı en büyük mü kontrolü yap
            if (MMptr->cnt < MMptr->K ){//dosyanın son değerine gelene kadar 1 arttır
                    MMptr->cnt = MMptr->cnt + 1;}//dosyanın içindeki sayıyıyı tutmuş olur
       }else {//dosyadaki sayılar bitince
            //printf("xxxxxxxxxxxxxxxxxxx\n");
            insertInorder(enb,  MMptr->maxValues,MMptr->cnt);//enb kıyaslamasına at
            fclose(fptr);
            fptr=NULL;
            breakIt =1;
       }
       if (sem_post(&MMptr->readWriteSema) == -1)
           errExit("sem_post");   
           
       r = rand()%2 ;	// 0 ve RAND_MAX. Arasinda integer değer dondurur
       sleep(r);
   }
   printf("Process #%d Finished his Job  \n", myID );
 }


int main(int argc, char *argv[]){
  int numOfProcess;
  int i,j,k,l,m;
  pid_t child;
  
  if (argc < 3){
      throwRunCommandError();
  }

  numOfNodesInTree  = atoi(argv[1]);
  numOfProcess      = atoi(argv[2]);
  numOfFiles        = argc - 3;
    printf("\t > numOfNodesInTree(K): %d \n",numOfNodesInTree);
    printf("\t > numOfProcess(N)    : %d \n",numOfProcess);
    printf("\t > numOfFiles         : %d \n",numOfFiles);

  fileList = (char **)malloc(numOfFiles * sizeof(char *)); 
  for (i=0; i<numOfFiles; i++) //dosya isimlerinin boyutunu ayarlıyoruz
       fileList[i] = (char *)malloc(MAXFILENAMESIZE * sizeof(char)); 

  for (i=3; i<argc; i++)//dosya isimlerini atıyoruz
      strncpy(fileList[i-3],argv[i],MAXFILENAMESIZE);

  for (i=0; i<numOfFiles; i++)//terminalde dosya isimlerini yazdırıyoruz
      printf("\t Files %d is : %s \n",i,fileList[i]);

  processList = (int *)malloc(numOfProcess * sizeof(int)); 
  for (i=0; i<numOfProcess; i++) 
      processList[i] = -1;

  sem_init(&wait2End, 0, 0);
     
  int shm_fd; 
  shm_fd = shm_open(MemoryMapname, O_CREAT | O_RDWR,S_IRUSR | S_IWUSR);
  //paylaşılan bellek nesnesi oluştur okuma yazma izni

  ftruncate(shm_fd, (SHAREDMEMSIZE*sizeof(int)) ); 
  //paylaşılan bellek nesnesinin boyutunu ayarlamak için mesela 1000 tane int sayı var 

  struct shmbuf *MMptr = mmap(0, (SHAREDMEMSIZE*sizeof(int) ), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);//paylaşılan belleğe erişme

  MMptr->cnt = 0;//K sayısına ulaşana kadar 1 arttırılıcak
  MMptr->K  = numOfNodesInTree;

  sem_init(&MMptr->readWriteSema, 1, 1);
 
  for(i = 0; i< SHAREDMEMSIZE; i++)
       MMptr->maxValues[i] = -1;

  for (i = 0; i< numOfProcess; i++){
      child = fork();
   
      if(child < 0){
          printf("fork failed.\n");
          exit(1);
      } else if ( child == 0){
          processWork(i);
          sleep(0.5);
          exit(1);  
      }
      for(j = 0;j<numOfProcess ;j++){
          if(processList[j] == -1)
            break;
       }
       printf("Main Creates proces PID: %d - Index: %d\n",child, j);
       processList[j] = child;   
  }
 sleep(0.5);
 
  printf(" Main Process waiting chields to end. !\n");

  pid_t  wpid;
  int status = 0;
  while ((wpid = wait(&status)) > 0);

  for(i = 0 ; i< MMptr->cnt ;i++){
    if(MMptr->maxValues[i]!=-1)
      printf(" %d ", MMptr->maxValues[i]);}
  printf("\n");

   shm_unlink(MemoryMapname);
   printf("This is Main Process END !\n");
   return 0;
}

#include "Factory.h" 

//Factory to create philosophers for the philosophers problem

extern void main_Philosopher();


void main_Factory(int argc, char* argv[]) {

  //Fetch input parameters

  void* addr = &main_Philosopher;
  int n = atoi(argv[0]);
  char nc[2];
  nc[0] = argv[0][0];
  nc[1] = argv[0][1];
  int C[20];
  char arg[5][2] = {'0'};
  arg[0][0] = '0';
  arg[0][1] = '0';
  if (n > 9) {
      arg[1][0] = '1';
  } else {
      arg[1][0] = '0';
  }
  arg[1][1] = (n%10) + '0';
  arg[2][0] = '0';
  arg[2][1] = '0';


  if(n>0 && n < 20) {
  
    //Init shared mem
    
     char p[16];
     int pnt = share_init();
     arg[2][1] = pnt + '0';

    for(int i=0; i<20; i++) C[i]=0;
    C[0] = 1;
    while(semaphore());
    write(STDOUT_FILENO, ">> ", 3);
    share(SHARE_WRITE, pnt, C, 20);

    //Loop to create philosophers
    for(int i=0; i<n; i++) {
    //Assign arguments
      if( i < 10){
          arg[0][0] =  '0';
          arg[0][1] = i + '0';
      } else {
          arg[0][0] = '1';
          arg[0][1] = (i - 10) + '0';
      }

      //Create philosopher
      pid_t pid  = fork();
      exec(addr, arg);

      //Wait till philosopher has init
      while(C[19] ==0){
        while(semaphore());
        share(SHARE_READ, pnt, C, 20); 
      }
      setPriority(10);

      C[19]=0;

      //Clean shared mem
      while(semaphore());
      share(SHARE_WRITE, pnt, C, 20);
      setPriority(10);

      write(STDOUT_FILENO, ">> ", 3);
    }
 
    C[0] = 0;

    //Tell philosophers to go
    while(semaphore());
    share(SHARE_WRITE, pnt, C, 20);
    setPriority(10);

    int sleep = 0;

    //Write out the state of shared mem after time interval
    while(true) {
      sleep = curr_timer() + 10;
      while(curr_timer() < sleep);

      while(semaphore());
      share(SHARE_READ, 1, C, 20);

      for(int i=0; i<n; i++) {
        p[i] = C[i] + '0';
      }
      write(STDOUT_FILENO, p, n);
    }
  
  }


  exit( EXIT_SUCCESS );
}
 

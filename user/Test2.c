#include "Test2.h"

void main_Test2(int argc, char* argv[]) {

  int C[20];
  
  for(int i=0; i<20; i++) C[i] = 0;
  
  write(STDOUT_FILENO, "3", 1);
  
  while(semaphore());
  share(SHARE_WRITE, 0, C, 20);


  write(STDOUT_FILENO, "4", 1);

  exit( EXIT_SUCCESS );
}


    
    

#include "Test.h"

void main_Test(int argc, char* argv[]) {

  int C[20];
  int pnt = share_init();

  for(int i=0; i<20; i++) C[i] = 1;

  if(C[0] ==1 )write(STDOUT_FILENO, "Y", 1);

  while(semaphore());
  share(SHARE_WRITE, pnt, C, 20);

  C[0] = 2;
  if(C[0] ==2) write(STDOUT_FILENO, "Y", 1);

  while(semaphore());
  share(SHARE_READ, pnt, C, 20);

  if(C[0] ==1 )write(STDOUT_FILENO, "Y", 1);

  exit( EXIT_SUCCESS );
}


    
    

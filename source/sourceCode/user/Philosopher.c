#include "Philosopher.h"

//'Pick up'
bool pickup(int index, int n, int pnt) {

  int C[20]; 
  char p[5];

  //Read shared mem
  share(SHARE_READ, pnt, C, 21);
  while(semaphore());
  p[0] = index + '0';

  //If can eat, assigns shared mem and return true, return false is not.
  if(C[index] == 0 && C[(index+1) % n] == 0) {
      C[index] = 1;
      C[(index+1) % n] = 1;
      share(SHARE_WRITE, pnt, C, 20);
      return true;
  }

  share(SHARE_WRITE, pnt, C, 20);
  return false;
}

//'Put down chopstick'
void putdown(int index, int n, int pnt) {

  int C[20]; 

  //Read mem and put down chopsticks and write
  share(SHARE_READ, pnt, C, 20); 
  while(semaphore());

  C[index] = 0;
  C[(index+1)%n] = 0;

  share(SHARE_WRITE, pnt, C, 20);
  
  return;
}

//'think' for some time
void think(int n) {

  int sleep = curr_timer() + 2*n +6;
  while(curr_timer() < sleep);

  return;
}

//'eat' for some time
void eat(int n) {

  int sleep = curr_timer() + 2*n +1;
  while(curr_timer() < sleep);

  return;

}

void main_Philosopher(int argc, char* argv[]) {

  int C[20];

  //Fetch arguments
  char* d = (char *) argv;
  char a[2];
  char b[2];
  char c[2]; 
  a[0] = d[0];
  a[1] = d[1];
  b[0] = d[2];
  b[1] = d[3];
  c[0] = d[4];
  c[1] = d[5];

  write(STDOUT_FILENO, a, 2);
  write(STDOUT_FILENO, ":", 1);
  write(STDOUT_FILENO, b, 2);
  write(STDOUT_FILENO, ":", 1);
  write(STDOUT_FILENO, c, 2);
  write(STDOUT_FILENO, " ", 2);

  //Init parameters
  int index = a[1] - '0';
  if(a[0] == '1') index += 10;
  int n = b[1] - '0';
  if(b[0] == '1') n+= 10;
  int pnt = c[1] - '0';
  if(c[0] == '1') pnt += 10;

 
  //Read shared mem and write out ready bit
  while(semaphore());
  share(SHARE_READ, pnt, C, 20); 
 
  C[19] =1;

  while(semaphore());
  share(SHARE_WRITE, pnt, C, 20); 
  yield();

  //Wait for go bit 
  while(C[0] ==1){
    yield();
    while(semaphore());
    share(SHARE_READ, pnt, C, 20); 
  }
  setPriority(0);

  write(STDOUT_FILENO, "go ", 3);

  int k=0;
  char* m;
  char* o;
  int l =0;

  //Loop to think -> pickup -> eat -> putdown
  while(k<100) {
  for(int i=0; i<3; i++) m[i] = C[i] + '0';
    k++;
    write(STDOUT_FILENO, "think", 5);
    think(index);
    while(semaphore());
    bool flag = pickup(index, n, pnt);

    if(flag) {
        write(STDOUT_FILENO, "eat", 3);
        eat(index);
        while(semaphore());
        putdown(index, n, pnt);
        write(STDOUT_FILENO, "putdown ", 8);
    } else {
        write(STDOUT_FILENO, "can't -> ", 9);
        setPriority(5);
    }
  }
  exit( EXIT_SUCCESS );
}

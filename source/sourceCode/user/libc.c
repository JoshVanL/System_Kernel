#include "libc.h"

int  atoi( char* x        ) {
  char* p = x; bool s = false; int r = 0;

  if     ( *p == '-' ) {
    s =  true; p++;
  }
  else if( *p == '+' ) {
    s = false; p++;
  }

  for( int i = 0; *p != '\x00'; i++, p++ ) {
    r = s ? ( r * 10 ) - ( *p - '0' ) :
            ( r * 10 ) + ( *p - '0' ) ;
  }

  return r;
}

void itoa( char* r, int x ) {
  char* p = r; int t, n;

  if( x < 0 ) {
    p++; t = -x; n = 1;
  }
  else {
         t = +x; n = 1;
  }

  while( t >= n ) {
    p++; n *= 10;
  }

  *p-- = '\x00';

  do {
    *p-- = '0' + ( t % 10 ); t /= 10;
  } while( t );

  if( x < 0 ) {
    *p-- = '-';
  }

  return;
}



void yield() {
  asm volatile( "svc %0     \n" // make system call SYS_YIELD
              :
              : "I" (SYS_YIELD)
              : );

  return;
}

int write( int fd, const void* x, size_t n ) {
  int r;

  asm volatile( "mov r4, %2 \n" // assign r0 = fd
                "mov r5, %3 \n" // assign r1 =  x
                "mov r6, %4 \n" // assign r2 =  n
                "svc %1     \n" // make system call SYS_WRITE
                //"mov %0, r4 \n" // assign r  = r0
              : "=r" (r) 
              : "I" (SYS_WRITE), "r" (fd), "r" (x), "r" (n)
              : "r4", "r5", "r6" );

  return r;
}

int  read( int fd,       void* x, size_t n ) {
  int r;

  asm volatile( "mov r0, %2 \n" // assign r0 = fd
                "mov r1, %3 \n" // assign r1 =  x
                "mov r2, %4 \n" // assign r2 =  n
                "svc %1     \n" // make system call SYS_READ
                "mov %0, r0 \n" // assign r  = r0
              : "=r" (r) 
              : "I" (SYS_READ),  "r" (fd), "r" (x), "r" (n) 
              : "r0", "r1", "r2" );

  return r;
}

int fork() {
  int r;

  asm volatile( "svc %1     \n" // make system call SYS_FORK
                "mov %0, r0 \n" // assign r  = r0 
              : "=r" (r) 
              : "I" (SYS_FORK)
              : "r0" );

  return r;
}

void exit( int x ) {
  asm volatile( "mov r0, %1 \n" // assign r0 =  x
                "svc %0     \n" // make system call SYS_EXIT
              :
              : "I" (SYS_EXIT), "r" (x)
              : "r0" );

  return;
}

void exec( const void* x, void* arg ) {
  asm volatile( "mov r0, %1 \n" // assign r0 = x
                "mov r1, %2 \n" //assign r1 = arg
                "svc %0     \n" // make system call sys_exec
              :
              : "i" (SYS_EXEC), "r" (x), "r" (arg)
              : "r0", "r1" );

  return;
}

int kill( int pid, int x ) {
  int r;

  asm volatile( "mov r0, %2 \n" // assign r0 =  pid
                "mov r1, %3 \n" // assign r1 =    x
                "svc %1     \n" // make system call sys_kill
                "mov %0, r0 \n" // assign r0 =    r
              : "=r" (r) 
              : "i" (SYS_KILL), "r" (pid), "r" (x)
              : "r0", "r1" );

  
  return r;
}

int share_init() {
  int r;

  asm volatile( "svc %1     \n" // make system call SYS_WRITE
                "mov %0, r0 \n" // assign r  = r0
              : "=r" (r) 
              : "I" (SYS_SHARE_INIT)
              : "r0" );

  return r;
}

int share( int fd, int pnt, int* x, int n) {
  int r;

  asm volatile( "mov r8, %2 \n" // assign r0 = fd
                "mov r9, %3 \n" // assign r1 =  pnt
                "mov r10, %4 \n" // assign r3 = x
                "mov r11, %5 \n" // assign r4 =  n
                "svc %1     \n" // make system call SYS_WRITE
                "mov %0, r0 \n" //  assign r  = r0
              : "=r" (r)           
              : "I" (SYS_SHARE), "r" (fd), "r" (pnt), "r" (x), "r" (n)
              : "r8", "r9", "r10",  "r11" );
                                   
  return r;                        
}                                  

int semaphore() {
    int r;
  asm volatile( "svc %1     \n" // make system call sys_
                "mov %0, r0 \n" // assign r  = r0
              : "=r" (r) 
              : "I" (SYS_SEM_DOWN)
              : "r0" );

  return r;
}

int curr_timer() {
    int r;
  asm volatile( "svc %1     \n" // make system call sys_
                "mov %0, r0 \n" // assign r  = r0
              : "=r" (r) 
              : "i" (SYS_TIME)
              : "r0" );

  return r;
}
                                   
int console_command() {
    int r;
  asm volatile( "svc %1     \n" // make system call sys_
                "mov %0, r0 \n" // assign r  = r0
              : "=r" (r) 
              : "i" (SYS_CONS_COM)
              : "r0" );

  return r;
}

int killall() {
    int r;
  asm volatile( "svc %1     \n" // make system call sys_
                "mov %0, r0 \n" // assign r  = r0
              : "=r" (r) 
              : "i" (SYS_KILL_ALL)
              : "r0" );

  return r;
}

int processes_count() {
  int r;
  asm volatile( "svc %1     \n" // make system call sys_
                "mov %0, r0 \n" // assign r  = r0
              : "=r" (r) 
              : "i" (SYS_PS_COUNT)
              : "r0" );

  return r;
}

int processes_pid(int n) {
  int r;
  asm volatile( "mov r0, %2 \n"  //assign r0 = n
                "svc %1     \n" // make system call sys_
                "mov %0, r0 \n" // assign r  = r0
              : "=r" (r) 
              : "i" (SYS_PS_PID), "r" (n)
              : "r0", "r1" );

  return r;
}

int setPriority(int n) {
  int r;
  asm volatile( "mov r0, %2 \n"  //assign r0 = n
                "svc %1     \n" // make system call sys_
                "mov %0, r0 \n" // assign r  = r0
              : "=r" (r) 
              : "i" (SYS_SET_PRI), "r" (n)
              : "r0", "r1" );

  return r;
}

int console_writeLCD(char* x, int n) {
  int r;
  asm volatile( "mov r8, %2 \n"  //assign r0 = x
                "mov r9, %3 \n" // assign r1 = n
                "svc %1     \n" // make system call sys_
                "mov %0, r0 \n" // assign r  = r0
              : "=r" (r) 
              : "i" (SYS_CON_LCD_WRI), "r" (x), "r" (n)
              : "r8", "r9" );

  return r;
}

int console_readLCD(char* x) {
  int r;
  asm volatile( "mov r8, %2 \n"  //assign r5 = x
                "svc %1     \n" // make system call sys_
                "mov %0, r0 \n" // assign r  = r5
              : "=r" (r) 
              : "i" (SYS_CON_LCD_REA), "r" (x)
              : "r8" );

  return r;
}

#include "hilevel.h"

//Global variables

pcb_t pcb[ 50  ], *current = NULL;
int priority[ 50 ];
uint32_t count=0;
uint32_t icurrent = 0;
uint32_t pidNum = 1;
uint32_t timer =0;
uint32_t active_pids[50];

bool entered = false;
bool nextUpper = false;

int consoleBuffer=0;
char inputBuffer[200];
int cursorPos[2] = {300,300};

extern void     main_P3();
extern void     main_P4();
extern void     main_P5();
extern void     main_console();
extern uint32_t tos_console(); 

extern uint32_t tos_shared();
uint32_t* sharred_current = (uint32_t*) (tos_shared);


//Priority based scheduler
void scheduler( ctx_t* ctx  ) {
  bool changed=false;
  int32_t next = 0;
  int hiPriority = priority[0];

  for (uint32_t i = 1; i < count; i++) {
    if (priority[i] > hiPriority) {
        hiPriority = priority[i];
        next = i;
        changed=true;
    }
  }

  for (uint32_t i = 0; i < count; i++) priority[i] +=1;

  priority[next] = 0;

  memcpy( &pcb[ icurrent  ].ctx, ctx, sizeof( ctx_t  )  );
  memcpy( ctx, &pcb[ next ].ctx, sizeof( ctx_t  )  );
  current = &pcb[ next ];
  icurrent = next;

  if(changed) {
    char x[2];
    uint32_t num = pcb[ icurrent].pid %10;
    x[0] = '0' +  num;
    num = (pcb[ icurrent].pid - num)/10;
    x[1] = '0' + num;

    PL011_putc( UART0, '\n', true ); 
    PL011_putc( UART0, x[1], true ); 
    PL011_putc( UART0, x[0], true ); 
    PL011_putc( UART0, ':', true ); 
    PL011_putc( UART0, ' ', true ); 
    carriageReturn(1);
    drawLetter(x[1], 1);
    drawLetter(x[0], 1);
    drawString(": ", 2, 1);
  }
	
  return;
}


//Print 00s, console command made
void printZero() {

  PL011_putc( UART0, '\n', true ); 
  PL011_putc( UART0, '0', true ); 
  PL011_putc( UART0, '0', true ); 
  PL011_putc( UART0, '$', true ); 
  PL011_putc( UART0, ' ', true ); 
  carriageReturn(1);
  drawString("00$ ", 4, 1);

  return;
}

//Change running process to console
void change_toConsole ( ctx_t* ctx) {
  memcpy( &pcb[ icurrent  ].ctx, ctx, sizeof( ctx_t  )  );
  memcpy( ctx, &pcb[ 0 ].ctx, sizeof( ctx_t  )  );
  current = &pcb[ 0 ];
  icurrent = 0;
  printZero();
  return;
}



//Initiate inpterups
void hilevel_handler_rst(  ctx_t* ctx ) {

  /* Configure the mechanism for interrupt handling by
  *
  * - configuring timer st. it raises a (periodic) interrupt for each 
  *   timer tick,
  * - configuring GIC st. the selected interrupts are forwarded to the 
  *   processor via the IRQ interrupt signal, then
  * - enabling IRQ interrupts.
  */
  
  TIMER0->Timer1Load  = 0x00020000; // select period = 2^20 ticks ~= 1 sec
  TIMER0->Timer1Ctrl  = 0x00000002; // select 32-bit   timer
  TIMER0->Timer1Ctrl |= 0x00000040; // select periodic timer
  TIMER0->Timer1Ctrl |= 0x00000020; // enable          timer interrupt
  TIMER0->Timer1Ctrl |= 0x00000080; // enable          timer
  
  
  GICC0->PMR          = 0x000000F0; // unmask all            interrupts
  GICD0->ISENABLER1  |= 0x00000010; // enable timer          interrupt
  GICD0->ISENABLER1 |= 0x00300000; // enable PS2          interrupts
  GICC0->CTLR         = 0x00000001; // enable GIC interface
  GICD0->CTLR         = 0x00000001; // enable GIC distributor

  renderInit();
  
  memset( &pcb[ 0  ], 0, sizeof( pcb_t  )  );
  pcb[ 0  ].pid      = 0;
  pcb[ 0  ].ctx.cpsr = 0x50;
  pcb[ 0  ].ctx.pc   = ( uint32_t  )( &main_console  );
  pcb[ 0  ].ctx.sp   = ( uint32_t  )( &tos_console   );

  priority[0] = 1;
  
  int_enable_irq(); //current = &pcb[ 0  ]; memcpy( ctx, &current->ctx, sizeof( ctx_t  )  );
  current = &pcb[ 0  ]; 
  memcpy( ctx, &current->ctx, sizeof( ctx_t  )  );

  active_pids[0] =0;

  count=1;

  PL011_putc( UART0, '\n', true ); 
  for(uint32_t i=0; i<20; i++) PL011_putc( UART0, '~', true ); 
  for(uint32_t i=0; i<26; i++) drawLetter('~', 1);


  // Write example red/green/blue test pattern into the frame buffer.

  icurrent =0;

  return;
}


//Pointer to the heap for use programs
extern uint32_t _heap_start;
void* tos_userProgram = &_heap_start;

//Execute a user program
void do_Exec (ctx_t* ctx ) {
  void* (*prog) = ( void* )( ctx->gpr[ 0 ] );
  char** (stra) = (char**) (ctx->gpr[1]);
  uint32_t argc = 0;
  while(stra[argc] != NULL) argc++;
  char* argv[argc];
  for(int i=0; i<3; i++)argv[i] = stra[i];

  active_pids[count] = pidNum;
  memset( &pcb[ count  ], 0, sizeof( pcb_t  )  );
  pcb[ count  ].pid      = pidNum;
  pcb[ count  ].parent   = current->pid;
  pcb[ count  ].ctx.cpsr = 0x50;
  pcb[ count  ].ctx.pc   = ( uint32_t  )( prog  );
  pcb[ count  ].ctx.sp   = ( uint32_t  )( tos_userProgram );    
  pcb[ count  ].ctx.gpr[0]   = ( uint32_t  )( argc );    
  pcb[ count  ].ctx.gpr[1]   = ( uint32_t  )( argv );    
  pidNum++;
  count++;

  memcpy( &pcb[ icurrent  ].ctx, ctx, sizeof( ctx_t  )  );
  memcpy( ctx, &pcb[ count-1 ].ctx, sizeof( ctx_t  )  );
  current = &pcb[ count-1 ];
  icurrent = count-1;

  char x[2];
  uint32_t num = pcb[ icurrent].pid %10;
  x[0] = '0' +  num;
  num = (pcb[ icurrent].pid - num)/10;
  x[1] = '0' + num;

  PL011_putc( UART0, '\n', true ); 
  PL011_putc( UART0, x[1], true ); 
  PL011_putc( UART0, x[0], true ); 
  PL011_putc( UART0, ':', true ); 
  PL011_putc( UART0, ' ', true ); 
  carriageReturn(1);
  drawLetter(x[1], 1);
  drawLetter(x[0], 1);
  drawString(": ", 2, 1);

  priority[count] = 2;
  return;
}


//Exit a program called by the program
void do_Exit (ctx_t* ctx ) {
  for (uint32_t i = icurrent; i<count; i++) {
    //pcb[i].pid = i;
    memcpy( &pcb[ i  ].pid, &pcb[ i+1 ].pid, sizeof( ctx_t  )  ); 
    priority[i] = priority[i+1]; 
    active_pids[i] = active_pids[i+1];
  } 

  count--;
  icurrent = 0;
  memcpy( ctx, &pcb[ icurrent  ].ctx, sizeof( ctx_t  )  );
  current = &pcb [ icurrent ];
  PL011_putc( UART0, 'H', true );
  drawLetter('H', 1);

  return;
}

//Kill a user program, called by the console
void do_Kill( ctx_t* ctx ) {
  uint32_t P;
  bool valid = false;
  uint32_t pid = ctx->gpr[ 0 ];
  for(uint32_t i = 1; i <count; i++) {
    if (pid == pcb[ i ].pid) {       
      valid = true;
      P = i;
      break;
    }
  }
  if(valid) {
    for (uint32_t i = P; i<count; i++) {
      memcpy( &pcb[ i  ].pid, &pcb[ i+1 ].pid, sizeof( ctx_t  )  );
      memcpy( &pcb[ i  ].ctx, &pcb[ i+1 ].ctx, sizeof( ctx_t  )  );
      priority[i] = priority[i+1];
      active_pids[i] = active_pids[i+1];
    }
    count--;
    icurrent = 0;
    PL011_putc( UART0, 'H', true );
    drawLetter('H', 1);
  } else {
    PL011_putc( UART0, 'E', true );
    drawLetter('E', 1);
  }
  return;
}

//Kill all use programs
void do_KillAll (ctx_t* ctx) {
  for(uint32_t i=count-1; i>0; i--) {
      ctx->gpr[ 0 ] = active_pids[i];
      do_Kill( ctx);
  }
  PL011_putc( UART0, 'A', true );
  drawLetter('A', 1);

  return;
}

    

bool flag_share = false;
uint32_t share_loc[20] = {0};

//High level supervisor interupt handler
void hilevel_handler_svc( ctx_t* ctx, uint32_t id ) {
  /* Each time execution reaches this point, we are tasked with handling
   * a supervisor call (aka. software interrupt, i.e., a trap or system 
   * call).
   */

  //Switch case to determin what was called and handle apropriatly
  switch( id  ) {
    case 0x00 : { // 0x00 => yield()
      scheduler( ctx  );
      break;
    }
    case 0x01 : { // 0x01 => write( fd, x, n  )
     int   fd = ( int    )( ctx->gpr[ 4  ]  );
     char*  x = ( char*  )( ctx->gpr[ 5  ]  );
     int    n = ( int    )( ctx->gpr[ 6  ]  );

   
     if (fd == 1) { //write to console
       drawString(x, n, 1);
       for( int i = 0; i < n; i++  ) {
  	     PL011_putc( UART0, *x++, true  );
       }
       ctx->gpr[ 4  ] = n;
     }   
     break;
    }
    case 0x02 : { //Read()
      break;
    }
    case 0x03 : { //Fork()
      PL011_putc( UART0, 'F', true );
      drawLetter('F', 1);
      tos_userProgram += 0x00010000; 
      break;
    }
    case 0x04 : {  //Exit()
      PL011_putc( UART0, ' ', true );
      PL011_putc( UART0, 'X', true );
      drawString(" X", 2, 1);
      do_Exit( ctx );
      break;
    }     
    case 0x05 : { //EXEC()
      if (&ctx->gpr[0] != NULL) {
        drawLetter('E', 1);
        PL011_putc( UART0, 'E', true );
        do_Exec(ctx);
      }
      break;
    }
    case 0x06 : { //KILL()
      PL011_putc( UART0, 'K', true );
      drawLetter('K', 1);
      pid_t p = ctx->gpr[0];
      do_Kill(ctx);
      for(int i=count-1; i>-1; i--) {
          if(pcb[active_pids[i]].parent == p) {
              ctx->gpr[0] = pcb[active_pids[i]].pid;
              do_Kill(ctx);
          }

      }
      break;
    }
    case 0x07 : { //Share init
      sharred_current += 0x00010000;
      share_loc[0] +=1;
      share_loc[share_loc[0]] = (uint32_t) sharred_current;
      ctx->gpr[0] = share_loc[0];
      break;
    }
    case 0x08 : { //Share() Read-Write
      priority[icurrent] = 20;
      int   fd = ( int    )( ctx->gpr[ 8  ]  );
      uint32_t pnt = ( uint32_t  )( ctx->gpr[ 9  ]  );
      uint32_t*  x = ( uint32_t*  )( ctx->gpr[ 10  ]  );
      uint32_t  n = ( uint32_t  )( ctx->gpr[ 11  ]  );

      uint32_t* curr = (uint32_t*) (share_loc[pnt]);
      //uint32_t* curr = (uint32_t*) (sharred_current);
    
      if (fd == 0) {
         memcpy(&curr[0], x,  n*sizeof(int));
         priority[icurrent] = 0;
      }
      if (fd == 1) {
        memcpy(x, &curr[0], n*sizeof(int));
        priority[icurrent] = 0;
      }
      flag_share = false;
      break;
    }
    case 0x09 : { //Semaphore
      if (flag_share) {
          ctx->gpr[0] = 1;
          if (priority[icurrent] >1) priority[icurrent]-=1;
      }
      else {
          flag_share = true;
          ctx->gpr[0] = 0;
          priority[icurrent] += 10;
     }
      break;
    }
    case 0x10 : { //Return timer val
      ctx->gpr[0] = timer;
      break;
    }
    case 0x11 : { //Console has made command
      change_toConsole(ctx);
      break;
    }
    case 0x12 : { //Kill all processes
      PL011_putc( UART0, 'K', true );
      drawLetter('K', 1);
      do_KillAll(ctx);
      break;
    }
    case 0x13 : { //Get processes count
      ctx->gpr[0] = count;
      break;
    }
    case 0x14 : { //Get processes pid

      int n = ctx->gpr[0];
      ctx->gpr[0] = active_pids[n];

      break;
    }
    case 0x15 : { //Set current processs priority

      int n = ctx->gpr[0];
      priority[icurrent] = n;
      break;
    }
    case 0x16 : { //Console writes to LCD
      //memcpy( &pcb[ icurrent  ].ctx, ctx, sizeof( ctx_t  )  );
      //memcpy( ctx, &pcb[ 0 ].ctx, sizeof( ctx_t  )  );
      current = &pcb[ 0 ];
      icurrent = 0;
      char* c = (char *) ctx->gpr[8];
      int x = (int) ctx->gpr[9];
      drawString(c, x, 0);
      consoleBuffer = 0;
      break;
    }
    case 0x17 : { //Console reads from keys in LCD
      char* c = (char *) ctx->gpr[8]; 
      char* n = "*";
      if(entered) {
        if(consoleBuffer == 0) {
            inputBuffer[0] = '\n';
            consoleBuffer = 1;
        }
        carriageReturn(0);
        memcpy(c, inputBuffer, consoleBuffer*sizeof(char));
        entered = false;
        consoleBuffer=0;
      } else {
        memcpy(&c, &n, sizeof(char));
      }
      break;
    }
    default   : { // 0x?? => unknown/unsupported
      PL011_putc( UART0, 'E', true );
      PL011_putc( UART0, 'R', true );
      drawString("ER", 2, 1);
      break;
    }
  
  }

  return;
}

int mouseCode = 0;
int mouseChangeX;
int mouseChangeY;
int mouseState;
bool mouseLocked = false;

//Interupt handler for Keyboard/Mouse and Timer
void hilevel_handler_irq( ctx_t* ctx ) {
  // Step 2: read  the interrupt identifier so we know the source.

  uint32_t id = GICC0->IAR;

  //Timer interupt
  if( id == GIC_SOURCE_TIMER0 ) {
    TIMER0->Timer1IntClr = 0x01;
    timer++;
    scheduler(ctx);
  }


  //Keyboard interupt
  if     ( id == GIC_SOURCE_PS20 ) {
      int x = PL050_getc( PS20 );
      char c = decode(x);
      memcpy( &pcb[ icurrent  ].ctx, ctx, sizeof( ctx_t  )  );
      memcpy( ctx, &pcb[ 0 ].ctx, sizeof( ctx_t  )  );
      current = &pcb[ 0 ];
      icurrent = 0;
      if(c == '#') consoleBuffer = deleteLetter(consoleBuffer, 0);
      else if(c == '+') entered = true;
      else if(c == '^') nextUpper = true;
      else if(c == '/') nextUpper = false;
      else if(c != '~') {
          if (nextUpper) {
              c -= 32;
          }
          drawLetter(c, 0);
          inputBuffer[consoleBuffer] = c;
          consoleBuffer++;
    }
  }

  //Mouse Interupt for movement and click
  else if( id == GIC_SOURCE_PS21 ) {
    uint8_t x = PL050_getc( PS21 );
    if (x == 0x09 && mouseCode == 0) {
        //mouse clicked
        int n = mouseClicked();
        if (n > 0 && n < 4) {
            change_toConsole( ctx );
            PL011_putc( UART0, 'F', true );
            drawLetter('F', 1);
            tos_userProgram += 0x00010000; 
            if (n == 1) {
                ctx->gpr[0] = (uint32_t) &main_P3;
            } else if (n == 2) {
                ctx->gpr[0] = (uint32_t) &main_P4;
            } if (n == 3) {
                ctx->gpr[0] = (uint32_t) &main_P5;
            }
            drawLetter('E', 1);
            do_Exec(ctx);
        } else if(n == 4) {
            change_toConsole( ctx );
            drawString( "\npids [", 7, 0);
            char c[2];
            if(count< 10) {
                c[0] = count + '0';
                drawString(c, 1, 0);
            } else {
                c[0] = (count/10) + '0';
                c[1] = (count % 10) + '0';
                drawString(c, 2, 0);
            }
            drawString ( "] : 0 ", 5, 0);

            int p;
            for(int i =1; i<count; i++) {
              p = active_pids[i];
              drawString(" ", 1, 0);
              if(p< 10) {
                  c[0] = p + '0';
                  drawString(c, 1, 0);
              } else {
                  c[0] = (p/10) + '0';
                  c[1] = (p % 10) + '0';
                  drawString(c, 2, 0);
              }
             
            }
            drawString("\n\n", 2, 0);
            drawString("shell$ ", 7, 0);
            consoleBuffer = 0;
        } else if (n == 5) {
            change_toConsole( ctx );
            drawLetter('K', 1);
            do_KillAll(ctx);
        }
            
    } 
    if(mouseCode == 0) {
        mouseCode += 1;
        mouseState = x;
    } else if (mouseCode == 1) {
        mouseCode += 1;
        mouseChangeY = x - ((mouseState << 4) & 0x100);
    } else if (mouseCode == 2) {
        mouseCode = 0;
        mouseChangeX = x - ((mouseState << 3) & 0x100);
        cursorPos[0] -= mouseChangeX;
        cursorPos[1] += mouseChangeY;
        if(cursorPos[0] > 590) cursorPos[0] = 590;
        if(cursorPos[0] < 0) cursorPos[0] = 0;
        if(cursorPos[1] > 795) cursorPos[1] = 795;
        if(cursorPos[1] < 0) cursorPos[1] = 0;
    }

    drawCursor(cursorPos[0], cursorPos[1]);
  }
  GICC0->EOIR = id;

  return;
}

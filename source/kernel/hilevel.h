#ifndef __HILEVEL_H
#define __HILEVEL_H

// Include functionality relating to newlib (the standard C library).

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <string.h>

#include <stdlib.h>


// Include functionality relating to the   kernel.

#include "lolevel.h"
#include     "int.h"
#include    "font.h"
#include    "decoder.h"
#include "render.h"

//Include functionality relating to the platform.

#include   "GIC.h"
#include "PL011.h"
#include "SP804.h"
#include "PL050.h"
#include "PL111.h"
#include "SYS.h"

typedef int pid_t;

typedef struct {
      uint32_t cpsr, pc, gpr[ 13  ], sp, lr;

} ctx_t;

typedef struct {
      pid_t pid;
        ctx_t ctx;
      pid_t parent;

} pcb_t;


#endif

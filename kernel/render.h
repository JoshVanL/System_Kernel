#ifndef __RENDER_H
#define __RENDER_H

#include "hilevel.h"





// Include functionality relating to the   kernel.

#include "render.h"

//Include functionality relating to the platform.

#include "PL011.h"
#include "PL050.h"

void drawCursor(int x, int y);

int mouseClicked();

void carriageReturn(int type);

void renderInit();

void drawLetter(char c, int type);

int deleteLetter(int consoleBuffer, int type);

void drawString(char* c, int n, int type);

int forkButton();

#endif

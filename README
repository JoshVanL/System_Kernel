I have implemented a systems kernel targeted at the RealView Platform Baseboard for Cortext-A8 which is emulated by QEMU.
It features 
- Dynamic creation and termination of user programs
- Priority based scheduler of programs
- Inter Process Communication (shared memory) demonstrated by a program that solves the dining philosophers problem
- GUI interface via the emulated LCD screen, displays both program output and console terminal. Buttons and mouse cursor are also included.
- Bit mapped font to display text on LCD screen
- Handles both user keyboard input and mouse input via PS/2 controllers

----------------------------------------------------------------------------

The following toolchain is required:

gcc-linaro-5.1-2015.08-x86_64_arm-eabi

This can be found at:
https://releases.linaro.org/components/toolchain/binaries/5.1-2015.08/arm-eabi/

This toolchain should be located at:

/usr/local/gcc-linaro-5.1-2015.08-x86_64_arm-eabi

Defined at sourceCode/Makefile line:17

Qemu is also required.

----------------------------------------------------------------------------
Run the following commands in 3 separate terminals in source code directory:

make launch-qemu
make launch-gdb
nc 127.0.0.1 1235

Once connected enter the command:
c
or 
continue
into gdb terminal. This should display the emulated LCD screen.
----------------------------------------------------------------------------


Available commands:
--------------------------

$ fork P
- Where P is one of the user programs: {P3, P4, P5, Factory} e.g fork P3

- Note Factory implements a program to solve the dining philosophers problem via shared memory by forking each philosopher. 

$ fork Factory N

- where N is the number of philosophers to run in program
- e.g fork Factory 16

: The buttons P3, P4 and P5 will fork those programs

$ kill N
- Where N is the pid number of the program to kill, e.g kill 4

$ killall
- Kills all running programs
: KA button also performs this

$ ps
- List all currently running program's pids
: PS button also performs this

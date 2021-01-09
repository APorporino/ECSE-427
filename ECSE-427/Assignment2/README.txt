To use the SUT library one can add the sut.c file in the compilation of their file.
For example if a program was called program1, the example makefile to build it would be:

program1: program.c sut.c
    gcc -pthread program.c sut.c

Alternatively, the library can be compiled into object code and then finally linked to the user prgram.
you can generate the object code by running: `make sut`
This object code file can then be compiled with the user program, however the flag -pthread must be used!!!

For example a compiling of a user program called program2 can be done like:

make sut                                // This genrates object code file sut.o
make program2                           // This generate object code file program2.o
gcc -pthread program2.o sut.o           //this generates a.out executable

----------------------------------------------------------------------------------------------------------------


- Note on the queue:
    Throught developement, one bug that kept coming up as wheneevr storing contexts, or sturcts in the queue,
    everytime an element was added it would overwrite all the other elements.
    A workaround that I used for this is the simply store ints in the queue and have these ints represent
    the index of an array that stores aall the appropriate contexts. This idea was also used in the YAU example given.

- Simple logic:
    Since the assignment description states that only one input output operation will be performed at once,
    the logic I used simplified the passing of tasks to the IEXEC kernel thread. Instead of directly passing the function
    to be executed in the queue, I use certain global booleans to determine which fucntion is to be run.
    This allows the queue to be extremly simple and again avoids the bug that kept showing up of overwritting.

- Note on sut_yield with input and output: 
    Calling sut_yield after a read, write, or open will cause that thread to wait twice the amount of time. 
    Since any input output operation essentially performs a yield once that specific operatinon is done. 
    It essentially performs a yield because once the task is done it is added to the end of the task queue.
    To avoid this simply remove the sut_yield after an input/output operation

- Time delay;
    The time delay is a variable in sut.h called SLEEP_TIME and can be changed to whatever speed is desired.

- Testing read/write:
    To test read and write commands I used the backend server I created from assignment one since I understood the funcitonality
    of it completely and can directly see the messages being sent and received. 
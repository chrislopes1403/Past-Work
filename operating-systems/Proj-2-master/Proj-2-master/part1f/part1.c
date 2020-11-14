/*
\\\\\\\\\\\\                                                              >
==========================================================================>>>>
////////////                                                              >
STUDENTS    : Zachary Berrhill, Chris Lopes, James Roquebert.
CLASS       : COP 4610 Intro To Operating Systems.
ASSIGNMENT  : Project 2, Part I
DATE        : 03/04/2020
*/
#include <unistd.h>
#include <sys/syscall.h>
int main(void) {
int i =0;
        while(i<5)
        {
        syscall(SYS_write, 1, "hello, world!\n", 14);
        i++;
        }
  return 0;
}



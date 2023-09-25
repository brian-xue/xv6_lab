/*
Write a simple version of the UNIX xargs program for xv6: its arguments describe a command to run, it reads lines from the standard input, and it runs the command for each line, appending the line to the command's arguments. Your solution should be in the file user/xargs.c.
The following example illustrates xarg's behavior:    
$ echo hello too | xargs echo bye     bye hello too     $   Note that the command here is "echo bye" and the additional arguments are "hello too", making the command "echo bye hello too", which outputs "bye hello too".Please note that xargs on UNIX makes an optimization where it will feed more than argument to the command at a time. We don't expect you to make this optimization. To make xargs on UNIX behave the way we want it to for this lab, please run it with the -n option set to 1. For instance
    $ (echo 1 ; echo 2) | xargs -n 1 echo     1     2     $   Some hints:
●Use fork and exec to invoke the command on each line of input. Use wait in the parent to wait for the child to complete the command.
●To read individual lines of input, read a character at a time until a newline ('\n') appears.
●kernel/param.h declares MAXARG, which may be useful if you need to declare an argv array.
●Add the program to UPROGS in Makefile.
●Changes to the file system persist across runs of qemu; to get a clean file system run make clean and then make qemu.
xargs, find, and grep combine well:
  $ find . b | xargs grep hello   will run "grep hello" on each file named b in the directories below ".".To test your solution for xargs, run the shell script xargstest.sh. Your solution is correct if it produces the following output:
  $ make qemu   ...   init: starting sh   $ sh < xargstest.sh   $ $ $ $ $ $ hello   hello   hello   $ $

*/
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"


int another_gets(char* buf, int max) {
  int i = 0;
  char c;
  int res = 0;
  while (i < max - 1) {
    if (read(0, &c, 1) != 1) {
      res = -1;
      break;
    }
    
    if (c == '\n') {
      break;
    }
    buf[i++] = c;
  }
  buf[i] = '\0';
  return res;
}

int main(int argc,char* argv[])
{
  if (argc < 2) {
    fprintf(2, "usage: xargs command\n");
    exit(1);
  }
  char buf[MAXARG];
  int n;
  while ((n = another_gets(buf, sizeof(buf))) >= 0) {
    if(fork()==0)
    {
        for(int i = 0;i< argc-1;i++)
        {
            argv[i] = argv[i+1];
        }
        argv[argc-1] = buf;
        exec(argv[0],argv);
        exit(1);
    }
    wait(0);
  }
  return 0;
}
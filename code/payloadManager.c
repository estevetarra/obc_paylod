#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> /* for fork */
#include <sys/types.h> /* for pid_t */
#include <sys/wait.h> /* for wait */
#include<time.h>

//state is a shared variable with UAR
char state;

char runExperiment(char const * experimentName) {
  /*Spawn a child to run the program.*/
     pid_t pid=fork();
     if (pid==0) { /* child process */
         //execl(experimentName, NULL, NULL);
         system(experimentName);
         exit(123); /* only if execv fails */
     }
     else { /* pid!=0; parent process */
         waitpid(pid,0,0); /* wait for child to exit */
     }
     return '0';
}

int main(int argc, char const *argv[]) {
  state = 'W';
  while(state == 'W'){
    //poll for I
    state = 'I';
  }
  runExperiment("sleep 3");
  state = 'G';
  printf( "G\n");
  runExperiment("sleep 5");
  state = 'O';

  printf( "O\n");
  runExperiment("sleep 4");

  printf( "F\n");
  state = 'F';
  return 0;
}

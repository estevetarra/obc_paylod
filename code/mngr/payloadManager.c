#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> /* for fork */
#include <sys/types.h> /* for pid_t */
#include <sys/wait.h> /* for wait */
#include <signal.h> /* for kill */
#include <time.h>

//state is a shared variable with UAR
char state;

//timeout in seconds
int runExperiment(char const * experimentName, int timeout) {
  /*Spawn a child to run the program.*/
     pid_t pid=fork();
     if (pid==0) { /* child process */
         //execl(experimentName, NULL, NULL);
         system(experimentName);
         exit(123); /* only if execv fails */
     }
     else { /* pid!=0; parent process */
         int startTime = (int)time(NULL);
         int currentTime = (int)time(NULL);
         int status;
         int wpid = waitpid(pid,&status,WNOHANG);
         while ( (currentTime - startTime) < timeout && wpid == 0)
         {
           wpid = waitpid(pid,&status,WNOHANG);
           currentTime = (int)time(NULL);
         }
         if (wpid == 0) {
           //The experiment has run for too much time
           int ret = kill(pid,0);
           if ( ret == 0 )   printf( "Killed succesfuly \n");
         }
       }
     return 0;
}

int main(int argc, char const *argv[]) {
  state = 'W';
  while(state == 'W') {
    //poll for I
    state = 'I';
  }
  runExperiment("sleep 8", 7);
  state = 'G';
  printf( "G\n");
  runExperiment("sleep 3", 4);
  state = 'O';

  printf( "O\n");
  runExperiment("sleep 4", 5);

  printf( "F\n");
  state = 'F';
  return 0;
}

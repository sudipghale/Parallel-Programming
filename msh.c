/*
    Name: Sudip Ghale
    Id: 1001557881
    CSE-3320-001
    Lab-01
*/
// The MIT License (MIT)
// Copyright (c) 2016, 2017 Trevor Bakker
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS  10   // Mav shell only supports TEN arguments

/*
this fun takes string array called token as a parameter.
this fun, calls execvp fun only if the command is not cd, history or doens starts with char '!'
*/
void callExec(char *token[MAX_NUM_ARGUMENTS]);
/*
this fun takes string arrays called token, and history, int histocunt and pids array claase pidsArray, and itrs index.
this fun calls the child fork and on success it call the execvp funtion and executes
the command
*/
void callFork(char *token[MAX_NUM_ARGUMENTS],char *history[MAX_NUM_ARGUMENTS], int historyCount, pid_t pidsArray[15], int* pidsIndex);
//if SIGINT AND SIGTSTP are caught then it prints Signal Cougt and accepts next command
static void sigHandler(int sig);


int main(int argc, char *argv[])
{

  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
  pid_t pidsArray[15]; // arry to store last 15 process IDS
  int pidsIndex=0; // starting index for pids array

  // history string array of 50, to store all the command up to 50 histocunt is used to keep track of history
  char *history[50]; // string array to store upto 50 command history
  int i=0, historyCount=0; //starting index for history array


  while( 1 )
  {
    // Print out the msh prompt
    printf ("msh> ");

    struct sigaction act;

    memset(&act, '\0', sizeof(act));
    act.sa_handler = &sigHandler; // calls sigHandler fun

    if(sigaction(SIGINT, &act, NULL) <0) // if the pressed CTRL+C, then SIGINT is paased through act to stop the currrently working process
    {
      perror("sigaction: \n");
      return 1;
    }

    if(sigaction(SIGTSTP, &act, NULL) <0)// if the pressed CTRL+C, then SIGINT is paased through act to suspend the currrently working process

    {
      perror("sigaction: \n");
      return 1;
    }
    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input

    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];
    int   token_count = 0;
    int index=1;

    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;
    char *working_str  = strdup( cmd_str );

//if the command is not null, then it is stored into the history array.
    if(strcmp(working_str,"\n"))
    {
         history[historyCount]= strdup(working_str);
      	 historyCount++;
    }

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) &&
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );

      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }

//if the command is null, then we continue to take another command
    if(!token[0])
    {
	    continue;
    }
// if the command is exit or quit then we exit with value 0
    if(strcmp(token[0],"exit")==0 || strcmp(token[0],"quit")==0)
    {
	    exit(0);
    }
//if the command is listpids, then we print the last 15 process pids id
//we set index to 0, so that it doesnt execute the command again
    if(strcmp(token[0],"listpids")==0 || strcmp(token[0],"showpids")==0)
    {
      for (i=0; i <pidsIndex; i++)
      {
        printf("%d: %d\n",i,pidsArray[i] );

      }
      index = 0 ;

    }
// if the command if bg, then we resume the last process using SIGCONT
    if(strcmp(token[0],"bg")==0)
    {
      kill(pidsArray[pidsIndex-1], SIGCONT);
      index = 0;
    }
// if the first char is '!', then we know its executing from the history

    if (token[0][0]=='!')
    {
      int histComand=0, count=0;
      char *stringHistory;
//firs we remove the '1' and then we change that into int value and stored in histComand
      memmove(token[0],token[0]+1, strlen(token[0]));
      histComand = atoi(token[0]);

//we copy the index history string into stringHistory
      stringHistory= strdup(history[histComand]);
// the string is tokenized for the exectution
      while ( ( (arg_ptr = strsep(&stringHistory, WHITESPACE ) ) != NULL) &&
                (count<MAX_NUM_ARGUMENTS))
      {
        token[count] = strndup( arg_ptr, MAX_COMMAND_SIZE );

        if( strlen( token[count] ) == 0 )
        {
          token[count] = NULL;
        }
          count++;
      }
// we call the callFork fun
      callFork(token, history, historyCount, pidsArray, &pidsIndex);
// store the pids id whever we call fork

      index=0; // set the index 0, so the callFork fun doenst execute twice



    }


    free( working_root ); // free the momory of the working_root


// the callFork will only be called if the value of index is 1
  if(index)
    {
      callFork(token, history, historyCount, pidsArray,&pidsIndex);
    }

 }

  return 0;
}
/*
if SIGINT AND SIGTSTP are caught then it prints Signal Cougt and accepts next command
*/
static void sigHandler(int sig)
{
  printf("Signal Cought%d\n",sig );
}

// this fun, calls execvp fun only if the command is not cd, history or doens starts with char '!'
void callExec(char *token[MAX_NUM_ARGUMENTS])
{
  if(strcmp(token[0],"cd")!=0 && strcmp(token[0],"history")!=0 && token[0][0]!='!')
    {

      if(execvp(token[0], token) < 0) // prints command not found if the commans is not found by execvp function
         {

            printf("%s: command not found\n",token[0]);
         }
        exit( EXIT_SUCCESS);
    }
}

// this fun calls the child fork and on success it call the execvp funtion and executes
// the command
void callFork(char *token[MAX_NUM_ARGUMENTS],char *history[MAX_NUM_ARGUMENTS], int historyCount, pid_t pidsArray[15], int* pidsIndex)
{
      int i =0;
      pid_t childPid = fork(); // forks the process and returns the pid_t

            if(childPid >0) // the positive value returned by fork() is the process id which is stored in pidsArray
            {
                      pidsArray[*pidsIndex] = childPid;
                      (*pidsIndex)++;
            }

      if( childPid == -1) // error messeage if the fork fails
      {
          perror("fork failed!");
  	      exit(-2);
      }

      if ( childPid == 0 )// fork is success
      {
        callExec(token); // calls the callExec fun

        if(strcmp(token[0],"history")==0) // if the command is history, then it prints the history form the history array
        {
          for( i =0; i <historyCount; i++)
           {
             printf("%d: %s\n",i,history[i]);
             continue;
           }

        }

  	  //end the child process so that parent can continue
  	      kill(getpid(),SIGKILL);
      }
      else
      {
  	    int status;

  	    waitpid(childPid, &status, 0); // parent process waits until the child process terminates
  	    if(strcmp(token[0],"cd")==0) // if the command is cd, then parent process executes it calling chdir() function
        {
  		    chdir(token[1]);
  	    }
      }
}

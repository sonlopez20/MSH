/*
*

*
*/

// The MIT License (MIT)
// 
// Copyright (c) 2016, 2017 Trevor Bakker 
// 
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
// used code provided by professor from GitHub to read in strings and tokenize them 
// also looked at fork.c and sigint.c from blackboard
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 10     // Mav shell will support ten arguments


static void handle_signal (int sig)
{
	//when it detects the signal the program will do nothing and continue
}

int main()
{
	
  int status;
  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
	//flags needed for signals
  struct sigaction act;
	memset (&act, '\0', sizeof(act));
	act.sa_handler = &handle_signal;
	if (sigaction(SIGINT , &act, NULL) < 0) 
	{
	   perror ("sigaction: ");
	   return 1;
	}
	if (sigaction(SIGTSTP , &act, NULL) < 0) 
	{
	   perror ("sigaction: ");
	   return 1;
	}
	// initializing values I will need throughout the entire program
  int i = 0;
	int showpids [15];
	static int s;
	pid_t pid;

  while( 1 )
  {
    // Print out the msh prompt
    printf ("msh> ");
    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );
		
    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];
		char* history[50];
    int token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;                                                                                        
    char *working_str  = strdup( cmd_str );      
		// copying unparsed string to the history          
    history[i] = strdup(cmd_str);
    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;
    // Tokenize the input strings with whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
			else
			{
				if(history[i] == NULL)
				{
					// will ignore any empty line in the string
				}
				else
				{
				  i++; //otherwise it will increment the count in history
			  }
			}
        token_count++;
    }
		
		if (token[0]==NULL)
		{
			continue;
			//if they dont enter anything then we will just continue in the while loop
		}	
		while (token[0][0]=='!')
		{
		  int number= atoi(&token[0][1]);
			if (number>(i-2))
			{
				//index is one ahead and the current one is not yet in history so subtracting by 2
				//if the number requested is not in history then we assume their is no such command and let user know
				printf("Command not in history\n");
				break;
			}
			else
			{
				// copy the unparsed string from history onto token 
				//then delete the whitespace and tokenize it so we can use it once again
				token_count = 0;
				token[0] = strdup(history[number]);
				token[0][strlen(token[0])-1]='\0';
				arg_ptr = strtok(token[0], " " );
				while (arg_ptr != NULL)
				{
					//will continue to read in the command until it is empty
				  token[token_count] = arg_ptr;
					token_count++;
					arg_ptr = strtok(NULL," ");
				}	
			}
			break;
		}
		if (strcmp(token[0],"history")==0)
		{
			// prints out array that contains history
			// using j as counter to not affect i which is keeping count if the number of commands
		  int j;
			for (j = 0; j< i; j++)
			{
				printf("%d : %s",j,history[j]);
			}
			if (i >49)
			{
				//making sure it doesnt exceed 50 commands in history will reset to 0 for it to restart
				i = 0;
			}
			continue;
		}
		if (strcmp(token[0],"showpids")==0)
		{
			// prints out array that contains pids
			// using c as counter to not affect s which is keeping count if the number of pids
			int c = 0;
			for (c = 0; c < s; c++)
			{
				printf("%d : %d\n",c,showpids[c]);
			}
			if (s > 14)
			{
				//making sure it doesnt exceed 15 pids in history will reset to 0 for it to restart
				s = 0;
			}
			continue;
		}
		if (!strcmp(token[0],"bg"))
		{
			//handlling the signal
	    kill(pid,SIGCONT);
			continue;
		}
		else if(!strcmp(token[0],"exit") || !strcmp(token[0],"quit"))
		{
			//if the user enters quit or exit we will quit the program leaving with a status of 0 
			status = 0;
		  exit(0);
		}
		else 
		{
			//otherwise we will check the four directories for the command until found if found
			pid = fork();
			showpids [s] = pid;
			s++;
			if(pid == -1)
			{
				// When fork() returns -1, an error happened.
		    perror("fork failed: ");
		    exit( EXIT_FAILURE );
			}
			else if (pid == 0)
			{
				//When fork() returns 0, we are in the child process
				//concatenate the strings before using them to make it easier
				char bin[255];
				strcat(bin,"/bin/");
				strcat(bin,token[0]);
				char usrbin[255];
				strcat(usrbin,"/usr/bin/");
				strcat(usrbin,token[0]);
				char local[255];
				strcat(local,"/usr/local/bin/");
				strcat(local,token[0]);
				//check all working directories and if command is not found then print why 
				//execv automativally does this
				execv (token[0],token); //current directory
				execv (bin,token);//bin
				execv (usrbin,token);//usr/bin
				execv (local,token);//local/usr/bin
			  printf("%s : Command not found.\n",token[0]);
			  fflush (NULL);
				exit ( EXIT_SUCCESS);
			}
			else
			{
			  //When fork() returns a positive number we are in the parent process
				chdir(token[1]);					
				waitpid(pid, &status,0);
				fflush (NULL);	
			}
		}		
    free( working_root );
  }
  return 0;
}

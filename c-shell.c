/**
 * Simple shell interface starter kit program.
 * Operating System Concepts
 * Mini Project1
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>


#define MAX_LINE		80 /* 80 chars per line, per command */

int main(void)
{
    char *args[MAX_LINE/2 + 1];	/* command line (of 80) has max of 40 arguments */
    char str[100]; //Input from STDIN
    int should_run = 1; //If the loop should repeat
    int words = 0; //Number of words in the input
    char prev[100]; //The stored previous input
    char strcopy[100]; //The copy of the original input from STDIN before strtok
    int hasHistory = 0; //Flag if the current Shell has had a previous input
    int outFlag = 0; //Flag if the output needs to be exported into a file
    int inFlag = 0; //Flag if the input needs to be read for the command
    int flagPos = 0; //Postion of the out/in Flag
    int pipeFlag = 0; //Flag if the command has a pipe "|"
    int pipePos = 0; //Postion of the pipe "|"

    while (should_run){ 
        printf("mysh:~$ ");
        /**
          * After reading user input, the steps are:
          * (1) fork a child process
          * (2) the child process will invoke execvp()
          * (3) if command includes &, parent and child will run concurrently
          */
	fgets(str, sizeof str, stdin); //Gets input
	int length = strlen(str);
	if (!strcmp(str,"\n")) { //If the input is just a new line, then continue looking for input
		continue;
	}
	strcpy(strcopy,str); //Copy of the original input
	char *token = strtok(str, "  \n");
	while (token != NULL) { //Seperating the words from the spaces 
		args[words] = token;
		token = strtok(NULL, " \n");	
		if (!strcmp(args[words],">")) { //If there is an outFlag then its position is recorded and its flag is raised
			outFlag = 1;
			flagPos = words;
		} else if (!strcmp(args[words],"<")) { //If there is an inFlag then its position is recorded and its flag is raised
			inFlag = 1;	
			flagPos = words;	
		}
		if (!strcmp(args[words],"|")) { //If there is a pipe then its position is recodred and its flag is raised
			pipeFlag = 1;
			pipePos = words;
		}
		words++;
	}
	args[words] = NULL; //Ending the final value of the input with a NULL so that when we search we know that the NULL terminals and signals the end of the input
	if (!strcmp(args[0], "!!")) { //Checks to see if "!!" is inputted
		if (!hasHistory) { //If it does not have previous history then this is outputted
			printf("No commands in history.\n"); 
		} else {
			strcpy(str,prev); //If it does then the previous input is copied into the current input
			words = 0;
			char *token = strtok(str, "  \n");
			while (token != NULL) { //We then use strtok to get the words from the previous string and split them into words for execution
				args[words] = token;
				token = strtok(NULL, " \n");	
				words++;
			}
			strcpy(strcopy, prev);
		}
	} 
	if (!strcmp(args[0], "exit")) { //If the user wants to exit then the loop stops and it exits
		should_run = 0;
		break;	
	} else if (!strcmp(args[0], "cd")) { //If the user wants to change directory, then it changes the directory of the parent
		chdir(args[1]);
	} else {
		if (!strcmp(args[words-1], "&")) { //If "&" is in the input then we dont wait for the child process and the parent just continues
			args[words-1] = NULL;
			int pid = fork();
			if (pid==0) {
				setpgid(0, 0); //Sets child id to 0, it runs in the backgroup
				execvp(args[0],args);
			} else {
				printf("Your command is now running in the backgroud\n");
				continue;
			}
		} else if (outFlag || inFlag) { //If there is an outFlag or inFlag
			if (outFlag) { //If outFlag
				char inCommand[100];
				strcpy(inCommand,args[0]); //Copies the first command
				for (int i=1;i<flagPos;i++) { //If there are other commands pass the first one and before the flag postion, ">", then copy that as well
					strcat(inCommand," ");
					strcat(inCommand,args[i]);
				}
				FILE *outCommand = popen(inCommand,"r"); //Create a file that is reading from the command
				FILE *outPut = fopen(args[flagPos+1], "w"); //Create a file with the same name as the argument name from the input
				char temp[100];
				if (outCommand == NULL) {
					printf("Failed Command.");
				}
				//If the command doesnt work or doesnt exist then this error pops up
				while (fgets(temp, sizeof temp, outCommand)!=NULL) {
					fputs(temp,outPut);
				}
				//Puts the output of the executed command into the file
				pclose(outCommand);
				fclose(outPut);
				//Closes both files 
				outFlag = 0; //Lower Flag
			} else if (inFlag) { //If inFlag is raised
				char inCommand[100];
				strcpy(inCommand,args[0]); //Copy the first command of the input
				if (flagPos == 2) { //If there are additional commands of the input then add that was well
					for (int i=1;i<flagPos;i++) {
						strcat(inCommand," ");
						strcat(inCommand,args[i]);
					}
				} else {
					strcat(inCommand," ");
				}
				strcat(inCommand, args[flagPos+1]); //Add the file name to the end of command
				FILE *outCommand = popen(inCommand, "r"); //Open the file and read its inputs
				if (outCommand == NULL) {	
					printf("Failed Command.");
				} //If the command doesnt work the output that it was failed
				char temp[100];
				while (fgets(temp, sizeof temp, outCommand)!=NULL) {
					printf("%s",temp);
				} //Get the values from the file after the execution of the command and output
				inFlag = 0; //Lower Flag
			}
		} else if (pipeFlag) { //If pipeFlag has been raised
			char *FirstCommand[MAX_LINE/2 + 1]; //The first command is before the pipe
			char *SecondCommand[MAX_LINE/2 + 1]; //The second command is after the pipe
			for(int i=0;i<pipePos;i++) { //Iterate through the words before the pipe and place them as the first set of commands
				FirstCommand[i] = args[i];
			}
			FirstCommand[pipePos] = NULL; //End the first set of commands with NULL so that the program knows when the command set is over
			int posTemp=0;
			for (int j=pipePos;j<words;j++) { //Iterate through the words after the pipe and place them as the second set of commands
				SecondCommand[j-pipePos] = args[j+1];
				posTemp++;
			}
			SecondCommand[posTemp] = NULL; //End the second set of commands with NULL so that the program knows when the command set is over
			int pipes[2]; //Creating pipes
			pipe(pipes);
			int pid = fork(); //Fork the process
			if (pid==0) { //If this is the first child
				close(pipes[1]); //Close output
				dup2(pipes[0],0); //Reroute Input
				close(pipes[0]);
				execvp(SecondCommand[0], SecondCommand); //The executed command will now reroute its output
				exit(0);
			} else {
				int pid2 = fork();
				if (pid2 == 0) { //If this is the second child
					close(pipes[0]); //Close input 
					dup2(pipes[1],1); //Reroute output
					close(pipes[1]);
					execvp(FirstCommand[0], FirstCommand); //The executed command will now reroute its output
					exit(0);
				} else {
					close(pipes[0]); //The parent will close both pipes and wait for both children to exit
					close(pipes[1]);
					waitpid(-1, NULL, 0);
					waitpid(-1, NULL, 0);
					pipeFlag = 0; //The pipeFlag is lowered
				}
			}
		} else {
			int pid = fork(); //If it is none of the above then just normally execute the command in a child
			if (pid==0) {
				execvp(args[0],args);
				exit(0);
			} else {
				waitpid(-1, NULL, 0);
			}
		}
	}
	strcpy(prev,strcopy); //Copy input into the history
	hasHistory = 1; //The program now has a previous input
	words = 0; //Reset the number of words in this input
        fflush(stdout); //Flush out all the output
    }
    return 0;
}


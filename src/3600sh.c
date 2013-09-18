/**
 * CS3600, Spring 2013
 * Project 1 Starter Code
 * (c) 2013 Alan Mislove
 *
 * You should use this (very simple) starter code as a basis for 
 * building your shell.  Please see the project handout for more
 * details.
 */

#include "3600sh.h"

#define USE(x) (x) = (x)

int main(int argc, char*argv[]) {
  	// Code which sets stdout to be unbuffered
  	// This is necessary for testing; do not change these lines
  	USE(argc);
  	USE(argv);
  	setvbuf(stdout, NULL, _IONBF, 0); 
  
  	// Main loop that reads a command and executes it
	while (1) {         
    	// Issuing prompt
    	char *input = "";
		int ret = do_prompt(&input);  
                //char ** args = {"ls", NULL};
                char ** args;
                args = (char **) calloc(2, sizeof(char *));
                char * ls = "ls";
                char * n = NULL;
                args[0] = ls;
                args[1] = n;
    
		if (ret) {
			return 1;
		}
		else {
			do_parse(input);
			if (!strcmp(input, "exit\n")) {
				free(input);
				break;	
			}
                        else if (!strcmp(input, "/bin/ls\n")) {
                          do_exec("/bin/ls", args);
                        }
			free(input);
		}

		
  }

  // Exit function
  do_exit();

  return 0;
}

// Function which prompts the user for input
//
int do_prompt(char **input) {
	// Return code variable
	int ret;

	// Getting the username
	// Max length = 32 bytes
	char *user;
	user = getlogin();
	// Checking for errors
	if (user == NULL) {	
		return 1;
	}

	// Getting the hostname
	// Max length according to posix = 255 bytes
	char *host = (char *)calloc(256, sizeof(char));
	ret = gethostname(host, 256);
	host[255] = '\0';
	// Checking for errors
	if (ret) {
		return 1;
	}

	// Getting the current working directory
	// Max length of path is 4096
	char *dir = (char *)calloc(256, sizeof(char));
	// Checking for errors
	if (getcwd(dir, 256) != dir) {	
		return 1;
	}
	dir[255] = '\0'; 

	// Printing prompt with variables given
	printf("%s@%s:%s> ", user, host, dir);
	
	// Free calloced buffers
	free(host);
	free(dir);
        
	// read in user input
  	int c;
  	char * temp = "";
  	do {
    	c = getc(stdin);
    	if (strlen(*input) == 0) { // if this is the first char of input
      		*input = (char *) calloc(2, sizeof(char)); // allocate space for input string
      		(*input)[0] = (char) c; // write char and null terminator to input string
      		(*input)[1] = '\0';
    	} else { // otherwise we have previously allocated memory
      		temp = (char *) calloc(strlen(*input) + 1, sizeof(char)); // move input to temp
      		strcpy(temp, *input);
      		free(*input);
      		*input = (char *) calloc(strlen(temp) + 2, sizeof(char)); // add more space to input
      		strcpy(*input, temp); // copy back over input string
      		(*input)[strlen(temp)] = c; // add newly read char
      		(*input)[strlen(temp) + 1] = '\0'; // add null terminator
      		free(temp);
    	}
  	} while (c != '\n');
  	// input will include \n char on end

  	// DEBUG
	//printf("%s", *input);
       
	return 0;
}

// Function which parses the user input from a string to usable data
//
int do_parse(char *input) {
	// Compiling regular expression
	regex_t r;
	char * regex_text = "[ \t]*[A-Za-z0-9_]+[ \t\n]+";
	int ret = regcomp (&r, regex_text, REG_EXTENDED|REG_NEWLINE);
	if (ret) // Returns if error with regex compilation
		return 1;

	char **args = (char **) calloc(1, sizeof(char *));
	if (args == NULL)
		return 1;
	int argc = 0;
	args[0] = (char *) NULL;

	char * pointer = input;

	while (1) {
		regmatch_t match[1];
		int noMatchFound = regexec(&r, pointer, 1, match, 0);
		if (noMatchFound) {
			printf("End\n");
			break;
		}

		argc++;
		char * temp = "";
		char * matchString = "";
		int i = 0;
		for (i = 0; match[0].rm_so + i < match[0].rm_eo; i++) {
			if (strlen(matchString) == 0) { // if this is the first char of match
      			matchString = (char *) calloc(2, sizeof(char)); // allocate space for input string
      			matchString[0] = *(pointer + match[0].rm_so + i); // write char and null terminator to input string
      			matchString[1] = '\0';
    		} else { // otherwise we have previously allocated memory
      			temp = (char *) calloc(strlen(matchString) + 1, sizeof(char)); // move input to temp
      			strcpy(temp, matchString);
      			free(matchString);
      			matchString = (char *) calloc(strlen(temp) + 2, sizeof(char)); // add more space to input
      			strcpy(matchString, temp); // copy back over input string
      			if (*(pointer + match[0].rm_so + i) == '\n')
      				matchString[strlen(temp)] = '\0';
      			else
      				matchString[strlen(temp)] = *(pointer + match[0].rm_so + i); // add newly read char
      			matchString[strlen(temp) + 1] = '\0'; // add null terminator
      			free(temp);
    		}
		}
		pointer += match[0].rm_eo;

		char **tempArgs = (char **) calloc(argc - 1, sizeof(char*)); // move args to temp
      	for (i = 0; i < argc - 1; i++)
      		tempArgs[i] = args[i];
      	free(args);
      	args = (char **) calloc(argc + 1, sizeof(char*)); // move args back to array
      	for (i = 0; i < argc - 1; i++)
      		args[i] = tempArgs[i];
      	args[i] = matchString;
      	args[i+1] = (char *) NULL;
      	free(tempArgs);

		printf("%s\n", matchString);

	}

	regfree(&r);
	return 0;
}

// Function which calls exec
//
int do_exec(char *path, char **argl) {
  //int cur_pid = getpid(); // get current pid
  //int parent_pid = getppid();
  int child_pid;

  // fork child process
  if ((child_pid = fork()) < 0) { // if child process fails to fork
    perror("Error: fork() Failure\n");
    return 1;
  }
  if (child_pid == 0) { // fork() == 0 for child process
    //cur_pid = getpid();
    //parent_pid = getppid();
    execv(path, argl); // exec user program
    perror("Error: execv() Failure\n"); // will not get to error if successful
    return errno;
  }
  else { // parent process
    wait(NULL); // wait for child process to exit
  }
  return 0;
}

// Function which exits, printing the necessary message
//
void do_exit() {
	printf("So long and thanks for all the fish!\n");
	exit(0);
}

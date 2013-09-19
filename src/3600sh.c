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
	
		if (ret) {
			return 1;
		}
		else {
			char **args;

			int backgroundProc = 0;

			ret = do_parse_input(input, &args, &backgroundProc);
			if (ret == 1) {
				printf("%s\n", "ERROR");
			} else if (ret == 2) {
				printf("%s\n", "Error: Unrecognized escape sequence.");
			} else if (ret == 3) {
				printf("%s\n", "Error: Invalid syntax.");
			}
			free(input);

			// If no args, continue loop
			if (args[0] == NULL) {
				free_args(args);
				continue;
			}
			
			//printf("%s %d\n", "Background proc:", backgroundProc);
			//debug_print_args(args);

			if (!strcmp(args[0], "exit")) {
				// Need to also support EOF
				// Handle \n EOF
				free_args(args);
				break;	
			} else {
				//char * path = calloc(50, sizeof(char));
				//strcpy(path, "");
				do_exec(args, backgroundProc);
				
				//free(path);
				free_args(args);
			}
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
			temp = *input;
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
// Return Codes:
//	0 - Good
// 	1 - Generic Error
//	2 - Escape Sequence Error 
// 	3 - & Syntax Error
int do_parse_input(char *input, char ***args, int *background) {
	// Compiling regular expression
	regex_t r;
	char * regex_text = "[ \t]*[^ \t\n]+[ \t\n]+"; //[-A-Za-z0-7_&/\\]
	int ret = regcomp (&r, regex_text, REG_EXTENDED|REG_NEWLINE);
	if (ret) // Returns if error with regex compilation
		return 1;

	// Creating args string pointer array
	*args = (char **) calloc(1, sizeof(char *));
	if (*args == NULL)
		return 1;
	// Creates arg count
	int argc = 0;
	// Last argument is null pointer
	(*args)[0] = (char *) NULL;
	// Setting background to no
	*background = 0;

	// Cursor pointer
	char * pointer = input;
	int i = 0;
	int combineWithPrev = 0;

	while (1) {
		regmatch_t match[1];
		// Searching for match with regex
		int noMatchFound = regexec(&r, pointer, 1, match, 0);
		if (noMatchFound) {
			// If no match is found, break loop
			break;
		}

		// If here, match was found so increment match
		argc++;
		char * temp = "";
		char * matchString = "";
		int combine = combineWithPrev;
		combineWithPrev = 0;

		for (i = 0; match[0].rm_so + i < match[0].rm_eo; i++) {
			if (*background)
				return 3;

			int length = 0;
			if (strlen(matchString) == 0) { // if this is the first char of match
				matchString = (char *) calloc(2, sizeof(char)); // allocate space for input string
			} else { 
				// otherwise we have previously allocated memory
				temp = matchString;
				matchString = (char *) calloc(strlen(temp) + 2, sizeof(char)); // add more space to input
				strcpy(matchString, temp); // copy back over input string
				length = strlen(temp);
				free(temp);
			}

			switch (*(pointer + match[0].rm_so + i)) {
				case '\n':
					matchString[length] = '\0';
					break;
				case '\\':
					// Deal with escape characters
					i++;
					if (!(match[0].rm_so + i < match[0].rm_eo)) {
						// End of line, get the hell out of there
						return 2;
					} else {
						switch (*(pointer + match[0].rm_so + i)) {
							// Next char is available
							case '\\':
								matchString[length] = '\\';
								break;
							case ' ':
								matchString[length] = ' ';
								combineWithPrev = 1;
								break;
							case 't':
								matchString[length] = '\t';
								break;
							case '&':
								matchString[length] = '&';
								break;
							default:
								return 2;
						}
					} 
					break;
				case '&':
					matchString[length] = '&';
					*background = 1;
					break;
				default:
					matchString[length] = *(pointer + match[0].rm_so + i); // add newly read char
					break;
			}
				
			matchString[length + 1] = '\0'; // add null terminator
			
		}



		// Adds string to string pointer array
		if (combine && *(pointer-2) == '\\') {
			char *tempPointer = (*args)[argc-2];
			(*args)[argc-2] = (char *)calloc(strlen((*args)[argc-2]) + sizeof(matchString) + 1, sizeof(char));
			strcpy((*args)[argc-2], tempPointer);
			free(tempPointer);
			strcat((*args)[argc-2], matchString);
			argc--;
		} else {
			char **tempArgs = *args;
			*args = (char **) calloc(argc + 1, sizeof(char*)); // move args back to array
			for (i = 0; i < argc - 1; i++)
				(*args)[i] = tempArgs[i];
			(*args)[i] = matchString;
			(*args)[i+1] = (char *) NULL;
			free(tempArgs);
		}

		// Increments pointer to end of matched string + 1
		pointer += match[0].rm_eo;

	}

	regfree(&r);

	// Fix & at end
	if (*background) {
		if (strlen((*args)[argc-1]) == 1) {
			// Delete term
			argc--;
			char **tempAmp = *args;
			*args = (char **) calloc(argc + 1, sizeof(char*)); // move args back to array
			int j = 0;
			for (j = 0; j < argc; j++)
				(*args)[j] = tempAmp[j];
			(*args)[j] = (char *) NULL;
		} else {
			// Delete last byte
			(*args)[argc-1][strlen((*args)[argc-1])-1] = '\0';
		}
	}

	return 0;
}


// Function which calls exec
//
int do_exec(char **argl, int background) {
	int cur_pid = getpid(); // get current pid
	//int parent_pid = getppid();
	int child_pid;

	// fork child process
	if ((child_pid = fork()) < 0) { // if child process fails to fork
		perror("Error: fork() Failure\n");
		return 1;
	}
	if (child_pid == 0) { // fork() == 0 for child process
		cur_pid = getpid();
		//parent_pid = getppid();
		//strcat(path, argl[0]);
		execvp(*argl, argl); // exec user program
		perror("Error: execv() Failure\n"); // will not get to error if successful
		//kill((pid_t)cur_pid, SIGQUIT);
                exit(errno);
                //return errno;
	}
	else { // parent process
          if (!background) {
		wait(NULL); // wait for child process to exit
          }
	}

	return 0;
}

// Function which exits, printing the necessary message
//
void do_exit() {
	printf("So long and thanks for all the fish!\n");
	exit(0);
}

// Frees argument string pointer array
//
void free_args(char **args) {
	int i;
	for (i = 0; args[i] != NULL; i++) {
		free(args[i]);
	}    
	free(args);
}

void debug_print_args(char **args) {
	int i = 0;
	printf("%s\n", "Printing Args");
	for (i = 0; args[i] != NULL; i++) {
		printf("\t%s\n", args[i]);
	}
}

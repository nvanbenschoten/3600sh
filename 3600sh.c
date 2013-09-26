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
		int eof = 0;
		int ret = do_prompt(&input, &eof);  
		if (ret == 1) {
			return 1;
		} else if (ret == 2) {
			break;
		}

		// Parsing input
		char **args;
		int backgroundProc = 0;
		ret = do_parse_input(input, &args, &backgroundProc);
		free(input);
		if (ret == 1) {
			return 1;	
		} else if (ret == 2) {
			if (eof) {
				break;
			} else {
				continue;
			}
		}
		
		// If no args, continue loop
		if (args[0] == NULL) {
			free_args(args);
			continue;
		}

		if (!strcmp(args[0], "exit")) {
			free_args(args);
			break;	
		} else {
			// Cals do_exec
			do_exec(args, backgroundProc);
			
			free_args(args);
			if (eof) {
				// If end of file, end
				break;
			}
		}
		
  }

  // Exit function
  do_exit();

  return 0;
}

// Function which prompts the user for input
// Return codes:
//   1 - Program failure, exit immediately
//   2 - Program exit after printing exit
int do_prompt(char **input, int *eof) {
	// Return code variable
	int ret;

	// Getting the username
	char *user = getenv("USER");
	if (user == NULL) {
		printf("%s\n", "Error: Username acquisition failed.");
		return 1;
	}
	printf("%s@", user);

	// Getting the hostname
	// Max length according to posix = 255 bytes
	char *host = (char *)calloc(256, sizeof(char));
	if (host == NULL) {
		// Checking memory allocation
		printf("%s\n", "Error: Memory allocation failed.");
		return 1;
	}
	ret = gethostname(host, 256);
	if (ret) {
		printf("%s\n", "Error: Hostname acquisition failed.");
		return 1;
	}
	host[255] = '\0';

	// Getting the current working directory
	// Max length of path is 4096
	char *dir = (char *)calloc(4096, sizeof(char));
	if (dir == NULL) {
		// Checking memory allocation
		printf("%s\n", "Error: Memory allocation failed.");
		return 1;
	}
	// Checking for errors
	if (getcwd(dir, 4096) != dir) {	
		printf("%s\n", "Error: Current working directory acquisition failed.");
		return 1;
	}
	dir[4095] = '\0'; 

	// Printing prompt with variables given
	printf("%s:%s> ", host, dir);
	
	// Free calloced buffers
	free(host);
	free(dir);
		
	// read in user input
	int c;
	char * temp = "";
	do {
		c = getc(stdin);
		if (strlen(*input) == 0) { 
			// if this is the first char of input
			if (c == EOF) {
				// If first and end of file
				if (feof(stdin)) {
					*eof = 1;
					// Return and exit properly
					return 2;
				} else {
					printf("%s\n", "Error: Read failed from getc.");
					return 1;
				}
			}
			// allocate space for input string
			*input = (char *) calloc(2, sizeof(char)); 
			if (input == NULL) {
				// Checking memory allocation
				printf("%s\n", "Error: Memory allocation failed.");
				return 1;
			}
			(*input)[0] = (char) c; // write char and null terminator to input string
			(*input)[1] = '\0';
		} else { 
			// otherwise we have previously allocated memory
			if (c == EOF) {
				// If EOF but not first
				if (feof(stdin)) {
					*eof = 1;
					temp = *input;
					// add more space to input so that it can be terminated
					*input = (char *) calloc(strlen(temp) + 2, sizeof(char)); 
					if (input == NULL) {
						// Checking memory allocation
						printf("%s\n", "Error: Memory allocation failed.");
						return 1;
					}
					strcpy(*input, temp); // copy back over input string
					(*input)[strlen(temp)] = '\n'; // add newly read char
					(*input)[strlen(temp) + 1] = '\0'; // add null terminator
					free(temp);
					return 0;
				} else {
					printf("%s\n", "Error: Read failed from getc.");
					return 1;
				}
			}
			temp = *input;
			*input = (char *) calloc(strlen(temp) + 2, sizeof(char)); // add more space to input
			if (input == NULL) {
				// Checking memory allocation
				printf("%s\n", "Error: Memory allocation failed.");
				return 1;
			}
			strcpy(*input, temp); // copy back over input string
			(*input)[strlen(temp)] = c; // add newly read char
			(*input)[strlen(temp) + 1] = '\0'; // add null terminator
			free(temp);
		}
	} while (c != '\n' && c != EOF);
	// Input will include \n char on end
	   
	return 0;
}

// Function which parses the user input from a string to usable data
// Return Codes:
//	0 - Good
// 	1 - Exit error
//	2 - Reprompt error
int do_parse_input(char *input, char ***args, int *background) {
	// Compiling regular expression
	regex_t r;
	// Regex set to one or more non space characters then at least on space character
	char * regex_text = "[ \t]*[^ \t\n]+[ \t\n]+";
	int ret = regcomp (&r, regex_text, REG_EXTENDED|REG_NEWLINE);
	if (ret) {
		// Returns if error with regex compilation
		printf("%s\n", "Error: Regex compilation failed.");
		return 1;
	}

	// Creates arg count
	int argc = 0;
	// Creating args array string pointer array
	*args = (char **) calloc(argc + 1, sizeof(char *));
	if (args == NULL) {
		// Checking memory allocation
		printf("%s\n", "Error: Memory allocation failed.");
		return 1;
	}

	// Last argument is null pointer
	(*args)[0] = (char *) NULL;
	// Setting background to no
	*background = 0;

	// Cursor pointer
	char * pointer = input;
	int i = 0;

	// Combine with prev will be used if 
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
			// Iterates through match token
			if (*background && (*(pointer + match[0].rm_so + i)) != '\n' && (*(pointer + match[0].rm_so + i)) != ' ' && (*(pointer + match[0].rm_so + i)) != '\t') {
				// Character after &
				printf("%s\n", "Error: Invalid syntax.");
				return 2;
			}	

			int prevlength = 0;

			if (strlen(matchString) == 0) { 
				// If this is the first char of match
				matchString = (char *) calloc(2, sizeof(char)); 
				// Allocate space for input string
				if (matchString == NULL) {
					// Checking memory allocation
					printf("%s\n", "Error: Memory allocation failed.");
					return 1;
				}
			} else { 
				// Otherwise we have previously allocated memory
				temp = matchString;
				matchString = (char *) calloc(strlen(temp) + 2, sizeof(char)); 
				// Add more space to input
				if (matchString == NULL) {
					// Checking memory allocation
					printf("%s\n", "Error: Memory allocation failed.");
					return 1;
				}
				// Copy back over input string
				strcpy(matchString, temp); 
				// Set prevlength variable
				prevlength = strlen(temp);
				free(temp);
			}

			switch (*(pointer + match[0].rm_so + i)) {
				case '\n':
					// Ends string
					matchString[prevlength] = '\0';
					break;
				case '\\':
					// Deal with escape characters
					// Needs to increment index to account for dealing with next char
					i++;
					if (!(match[0].rm_so + i < match[0].rm_eo)) {
						// End of line, get out of there with error
						printf("%s\n", "Error: Unrecognized escape sequence.");
						return 2;
					} else {
						switch (*(pointer + match[0].rm_so + i)) {
							// Next char is available
							case '\\':
								// Sets char to backslash
								matchString[prevlength] = '\\';
								break;
							case ' ':
								// Sets char to space
								matchString[prevlength] = ' ';
								combineWithPrev = 1;
								break;
							case 't':
								// Sets char to \t
								matchString[prevlength] = '\t';
								break;
							case '&':
								// Sets char to &
								matchString[prevlength] = '&';
								break;
							default:
								// Anything else, error
								printf("%s\n", "Error: Unrecognized escape sequence.");
								return 2;
						}
					} 
					break;
				case '&':
					// Adds to end of string and sets background flag
					matchString[prevlength] = '&';
					*background = 1;
					break;
				case ' ':
					// Do nothing with spaces, should not occur due to regex except at end of token
					break;
				case '\t':
					// Do nothing with \t, should not occur due to regex except at end of token
					break;
				default:
					// Sets string char to char in token
					matchString[prevlength] = *(pointer + match[0].rm_so + i); // add newly read char
					break;
			}
				
			// add null terminator to next character	
			matchString[prevlength + 1] = '\0';
		}

		// Adds string to string pointer array
		if (combine && *(pointer-2) == '\\') {
			// If token needs to be combines with previous
			char *tempPointer = (*args)[argc-2];
			// Reallocates previous token with length of each token and room for null terminator
			(*args)[argc-2] = (char *)calloc(strlen(tempPointer) + strlen(matchString) + 1, sizeof(char));
			if ((*args)[argc-2] == NULL) {
				// Checking memory allocation
				printf("%s\n", "Error: Memory allocation failed.");
				return 1;
			}
			strcpy((*args)[argc-2], tempPointer);
			free(tempPointer);
			strcat((*args)[argc-2], matchString);
			argc--;
		} else {
			char **tempArgs = *args;
			// Needs to reallocate arg array with size for argc plus room for null pointer
			*args = (char **) calloc(argc + 1, sizeof(char*));
			if (*args == NULL) {
				// Checking memory allocation
				printf("%s\n", "Error: Memory allocation failed.");
				return 1;
			}
			// Copies args back from temp arg array
			for (i = 0; i < argc - 1; i++)
				(*args)[i] = tempArgs[i];
			(*args)[i] = matchString;
			(*args)[i+1] = (char *) NULL;
			free(tempArgs);
		}

		// Increments pointer to end of matched string + 1
		pointer += match[0].rm_eo;

	}

	// Frees regular expression
	regfree(&r);

	// Fix & at end
	if (*background) {
		if (strlen((*args)[argc-1]) == 1) {
			// Delete term if alone
			argc--;
			char **tempAmp = *args;
			*args = (char **) calloc(argc + 1, sizeof(char*)); // move args back to array
			if (*args == NULL) {
				// Checking memory allocation
				printf("%s\n", "Error: Memory allocation failed.");
				return 1;
			}
			int j = 0;
			for (j = 0; j < argc; j++)
				(*args)[j] = tempAmp[j];
			(*args)[j] = (char *) NULL;
		} else {
			// Delete last byte if in token
			(*args)[argc-1][strlen((*args)[argc-1])-1] = '\0';
		}
	}

	return 0;
}


// Function which calls exec
//
int do_exec(char **argl, int backgroundProc) {
	//int cur_pid = getpid(); // get current pid
	//int parent_pid = getppid();
	int child_pid;
	int ret;

	char * arg;
	unsigned int i = 0;
	unsigned int j = 0;
	arg = argl[i];
        unsigned int len = 0;
        //int redir = 0;
        int old_i = dup(STDIN_FILENO);
        int old_o = dup(STDOUT_FILENO);
        int old_e = dup(STDERR_FILENO);

        while (argl[len] != NULL) {
          len++;
        }
        len++;

        while (arg != NULL) { 
                if (!strcmp(arg, "<")) { // if we need to redirect stdin
                        int f = open(argl[i+1], O_RDONLY);
                        if (f == -1) {
                          printf("Error: Unable to open redirection file.\n");
                          return 1;
                        }
                        dup2(f, STDIN_FILENO);
                        j = i;
                        free(argl[i]);
                        free(argl[i+1]);
                        while (j < len - 2) { 
                          argl[j] = argl[j+2];
                          j++;
                        }
                        i--;
                }
                else if (!strcmp(arg, ">")) { // if we need to redirect stdout
                        int f = open(argl[i+1], O_RDWR|O_CREAT|O_TRUNC, 0777);
                        if (f == -1) {
                          printf("Error: Unable to open redirection file.\n");
                          return 1;
                        }
                        dup2(f, STDOUT_FILENO);
                        j = i;
                        free(argl[i]);
                        free(argl[i+1]);
                        while (j < len - 2) { 
                          argl[j] = argl[j+2];
                          j++;
                        }
                        i--;
                } 
                else if (!strcmp(arg, "2>")) { // if we need to redirect stderr
                        int f = open(argl[i+1], O_RDWR|O_CREAT|O_TRUNC, 0777);
                        if (f == -1) {
                          printf("Error: Unable to open redirection file.\n");
                          return 1;
                        }
                        dup2(f, STDERR_FILENO);
                        j = i;
                        free(argl[i]);
                        free(argl[i+1]);
                        while (j < len - 2) { 
                          argl[j] = argl[j+2];
                          j++;
                        }
                        i--;
                }
                i++;
                arg = argl[i];
        }

	// fork child process
	if ((child_pid = fork()) < 0) { // if child process fails to fork
		perror("Error: fork() Failure\n");
		return 1;
	}
	if (child_pid == 0) { // fork() == 0 for child process
				// process arguments for redirections
		//cur_pid = getpid();
		//parent_pid = getppid();
		//strcat(path, argl[0]);
		//debug_print_args(argl);
		ret = execvp(*argl, argl); // exec user program
                dup2(old_i, STDIN_FILENO);
                dup2(old_o, STDOUT_FILENO);
                dup2(old_e, STDERR_FILENO);
		if (ret == -1) {
			printf("Error: Command not found.\n");
		}
		else if (ret == EPERM) {
			printf("Error: Permission denied.\n");
		}
		else {
			printf("Error: %d\n", ret);
                        perror("Error");
		}
		//perror("Error: execv() Failure\n"); // will not get to error if successful
		// Open foo.txt get fd
		// Dup 2 to get stuff
		//execvp(*argl, argl); // exec user program
		//perror("Error: execv() Failure\n"); // will not get to error if successful
		//kill((pid_t)cur_pid, SIGQUIT);
		exit(errno);
		//return errno;
	}
	else { // parent process
		if (!backgroundProc) {
			wait(NULL); // wait for child process to exit
                        }
                dup2(old_i, STDIN_FILENO);
                dup2(old_o, STDOUT_FILENO);
                dup2(old_e, STDERR_FILENO);
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
	// Frees args up to null
	for (i = 0; args[i] != NULL; i++) {
		free(args[i]);
	}  
	// Frees null pointer
	free(args[i]);  
	// Frees args array pointer
	free(args);
}

void debug_print_args(char **args) {
	// Prints args with quotes around them
	int i = 0;
	printf("%s\n", "Printing Args");
	for (i = 0; args[i] != NULL; i++) {
		printf("\t\"%s\"\n", args[i]);
	}
	printf("\t%s\n", args[i]);
}

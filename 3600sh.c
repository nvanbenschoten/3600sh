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
			free_args(args);
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
			// Exit
			free_args(args);
			break;	
		} else {
			// Cals do_exec
			ret = do_exec(args, backgroundProc);
			if (ret) {
				return 1;
			}
			
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
//	 0 - Good
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
//	 0 - Good
//	 1 - Exit error
//	 2 - Reprompt error
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
		regfree(&r);
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
				regfree(&r);
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
					regfree(&r);
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
					regfree(&r);
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
						regfree(&r);
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
								regfree(&r);
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
				regfree(&r);
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
				regfree(&r);
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
			// Frees null pointer
			free((*args)[argc]);
			argc--;
			free((*args)[argc]);
			(*args)[argc] = (char *) NULL;
		} else {
			// Delete last byte if in token
			(*args)[argc-1][strlen((*args)[argc-1])-1] = '\0';
		}
	}

	return 0;
}


// Function which calls exec
// Return Codes:
//	 0 - Good
//	 1 - Exit error
int do_exec(char **argl, int backgroundProc) {
	int child_pid;
	int ret;
	char * arg;
	unsigned int i = 0; // counter
	unsigned int j = 0; // counter
	arg = argl[i];
	unsigned int len = 0; // length of argl

	// Creates temporary file descriptors
	int old_i = dup(STDIN_FILENO);
	int old_o = dup(STDOUT_FILENO);
	int old_e = dup(STDERR_FILENO);
	int fd_i = STDIN_FILENO;
	int fd_o = STDOUT_FILENO;
	int fd_e = STDERR_FILENO;

	// Creates flags for multiple same type redirection
	int flag_i = 0;
	int flag_o = 0;
	int flag_e = 0;

	// count provided number of arguments
	while (argl[len] != NULL) {
	  len++;
	}
	len++;

	while (arg != NULL) { // while there are more arguments to parse
		if (!strcmp(arg, "<")) { // if we need to redirect stdin
			// if there is invalid redirection syntax
			if (argl[i+1] == NULL || !strcmp(argl[i+1], "<") 
					|| !strcmp(argl[i+1], ">") || !strcmp(argl[i+1], "2>")
					|| !strcmp(argl[i+1], "&")) {
				reset_redirection(old_i, old_o, old_e);
				printf("Error: Invalid syntax.\n");
				return 1;
			}
			// else if there is more invalid redirection syntax
			else if (len - i > 3 && !(!strcmp(argl[i+2], "<")
					|| !strcmp(argl[i+2], ">") || !strcmp(argl[i+2], "2>")
					|| !strcmp(argl[i+2], "&") )) {
				reset_redirection(old_i, old_o, old_e);
				printf("Error: Invalid syntax.\n");
				return 1;
			}
			if (flag_i) {
				reset_redirection(old_i, old_o, old_e);
				printf("Error: Invalid syntax.\n");
				return 1;
			}
			fd_i = open(argl[i+1], O_RDONLY, 0777); // open the input file
			if (fd_i == -1) {
				reset_redirection(old_i, old_o, old_e);
				printf("Error: Unable to open redirection file.\n");
				return 1;
			}
			dup2(fd_i, STDIN_FILENO); // set STDIN to input file
			j = i;
			free(argl[i]);
			free(argl[i+1]);
			while (j < len - 2) { 
				argl[j] = argl[j+2]; // modify argument list for further parsing
				j++;
			}
			i--; // update vars for modified list
			len = len - 2;
			flag_i++;
		}
		else if (!strcmp(arg, ">")) { // if we need to redirect stdout
			// if there is invalid redirection syntax
			if (argl[i+1] == NULL || !strcmp(argl[i+1], "<") 
					|| !strcmp(argl[i+1], ">") || !strcmp(argl[i+1], "2>")
					|| !strcmp(argl[i+1], "&")) {
				reset_redirection(old_i, old_o, old_e);
				printf("Error: Invalid syntax.\n");
				return 1;
			}
			// else if there is more invalid redirection syntax
			else if (len - i > 3 && !(!strcmp(argl[i+2], "<")
					|| !strcmp(argl[i+2], ">") || !strcmp(argl[i+2], "2>")
					|| !strcmp(argl[i+2], "&") )) {
				reset_redirection(old_i, old_o, old_e);
				printf("Error: Invalid syntax.\n");
				return 1;
			}
			if (flag_o) {
				reset_redirection(old_i, old_o, old_e);
				printf("Error: Invalid syntax.\n");
				return 1;
			}
			fd_o = open(argl[i+1], O_RDWR|O_CREAT|O_TRUNC, 0777); // open output file
			if (fd_o == -1) {
				reset_redirection(old_i, old_o, old_e);
				printf("Error: Unable to open redirection file.\n");
				return 1;
			}
			dup2(fd_o, STDOUT_FILENO);
			j = i;
			free(argl[i]);
			free(argl[i+1]);
			while (j < len - 2) { 
				argl[j] = argl[j+2]; // modify argument list for further parsing
				j++;
			}
			i--; // update vars for modified list
			len = len - 2;
			flag_o++;
		} 
		else if (!strcmp(arg, "2>")) { // if we need to redirect stderr
				// if there is invalid redirection syntax
			if (argl[i+1] == NULL || !strcmp(argl[i+1], "<")
					|| !strcmp(argl[i+1], ">") || !strcmp(argl[i+1], "2>")
					|| !strcmp(argl[i+1], "&")) {
				reset_redirection(old_i, old_o, old_e);
				printf("Error: Invalid syntax.\n");
				return 1;
			}
			// else if there is more invalid redirection syntax
			else if (len - i > 3 && !(!strcmp(argl[i+2], "<")
					|| !strcmp(argl[i+2], ">") || !strcmp(argl[i+2], "2>")
					|| !strcmp(argl[i+2], "&") )) {
				reset_redirection(old_i, old_o, old_e);
				printf("Error: Invalid syntax.\n");
				return 1;
			}
			if (flag_e) {
				reset_redirection(old_i, old_o, old_e);
				printf("Error: Invalid syntax.\n");
				return 1;
			}
			fd_e = open(argl[i+1], O_RDWR|O_CREAT|O_TRUNC, 0777); // open error file
			if (fd_e == -1) {
				reset_redirection(old_i, old_o, old_e);
				printf("Error: Unable to open redirection file.\n");
				return 1;
			}
			dup2(fd_e, STDERR_FILENO);
			j = i;
			free(argl[i]);
			free(argl[i+1]);
			while (j < len - 2) { 
				argl[j] = argl[j+2]; // modify argument list for further parsing
				j++;
			}
			i--; // update vars for modified list
			len = len - 2;
			flag_e++;
		}
		i++;
		arg = argl[i];
	}

	// fork child process
	if ((child_pid = fork()) < 0) { // if child process fails to fork
		reset_redirection(old_i, old_o, old_e);
		perror("Error: fork() Failure\n");
		return 1;
	}
	if (child_pid == 0) { // fork() == 0 for the child process
		// process arguments for redirections
		ret = execvp(*argl, argl); // exec user program
		// if execvp does not exit normally
		// Resete file descriptors
		reset_redirection(old_i, old_o, old_e);
		if (errno == EPERM || errno == EACCES) { // Permission denied
			printf("Error: Permission denied.\n");
		}
		else if (ret == -1) { // if we had an error return code
			printf("Error: Command not found.\n");
		}
		else { // we had some other "bad" return code
			perror("Error");
		}
		exit(errno); // exit with error code
	}
	else { // else we are in the parent process
		if (!backgroundProc) { // if we are not running the process in the background
			wait(NULL); // wait for child process to exit
		}
		// reset our file descriptors
		reset_redirection(old_i, old_o, old_e);

		// Close files
		if (fd_i != STDIN_FILENO) {
			close(fd_i);
		}
		if (fd_o != STDOUT_FILENO) {
			close(fd_o);
		}
		if (fd_e != STDERR_FILENO) {
			close(fd_e);
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
	// Frees args up to null
	for (i = 0; args[i] != NULL; i++) {
		free(args[i]);
	}  
	// Frees null pointer
	free(args[i]);  
	// Frees args array pointer
	free(args);
}

// Resets file descriptors for stdin, stdout, & stderr with previous values
//
void reset_redirection(int old_i, int old_o, int old_e) {
	// reset our file descriptors
	dup2(old_i, STDIN_FILENO);
	dup2(old_o, STDOUT_FILENO);
	dup2(old_e, STDERR_FILENO);
}

// Prints the list of arguments in a readable way
//
void debug_print_args(char **args) {
	// Prints args with quotes around them
	int i = 0;
	printf("%s\n", "Printing Args");
	for (i = 0; args[i] != NULL; i++) {
		printf("\t\"%s\"\n", args[i]);
	}
	printf("\t%s\n", args[i]);
}

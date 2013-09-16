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
    char * input;
		input = do_prompt();  
    
		if (!strcmp(input, "exit\n")) {
			break;	
		}
  }

  // Exit function
  do_exit();

  return 0;
}

// Function which prompts the user for input
//
char *do_prompt() {
	// Return code variable
	int ret;

	// Getting the username
	// Max length = 32 bytes
	char *user;
	user = getlogin();
	// Checking for errors
	if (user == NULL) {	
		printf("Error 1\n");
		return NULL;
	}

	// Getting the hostname
	// Max length according to posix = 255 bytes
	char *host = (char *)calloc(256, sizeof(char));
	ret = gethostname(host, 256);
	host[255] = '\0';
	// Checking for errors
	if (ret) {
		printf("Error 2\n");
		return NULL;
	}

	// Getting the current working directory
	char *dir = (char *)calloc(100, sizeof(char));
	// Checking for errors
	if (getcwd(dir, 100) != dir) {	
		printf("Error 3\n");
		return NULL;
	}
	dir[99] = '\0'; 

	// Printing prompt with variables given
	printf("%s@%s:%s> ", user, host, dir);
	
	// Free calloced buffers
	free(host);
	free(dir);
        
	// read in user input
  int c;
  char * input = "";
  char * temp = "";
  do {
    c = getc(stdin);
    if (strlen(input) == 0) { // if this is the first char of input
      input = (char *) calloc(2, sizeof(char)); // allocate space for input string
      input[0] = (char) c; // write char and null terminator to input string
      input[1] = '\0';
    }
    else { // otherwise we have previously allocated memory
      temp = (char *) calloc(strlen(input) + 1, sizeof(char)); // move input to temp
      strcpy(temp, input);
      free(input);
      input = (char *) calloc(strlen(temp) + 2, sizeof(char)); // add more space to input
      strcpy(input, temp); // copy back over input string
      input[strlen(temp)] = c; // add newly read char
      input[strlen(temp) + 1] = '\0'; // add null terminator
      free(temp);
    }
  } while (c != '\n');
  // input will include \n char on end

  printf("%s", input);
       
	return input;

}

// Function which exits, printing the necessary message
//
void do_exit() {
	printf("So long and thanks for all the fish!\n");
	exit(0);
}

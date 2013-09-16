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
    // You should issue the prompt here
    do_prompt();  
    // You should read in the command and execute it here
    
    // You should probably remove this; right now, it
    // just exits
    do_exit();
  }

  return 0;
}

// Function which prompts the user for input
//
void do_prompt() {
	int ret;

	char *user;	
	user = getlogin();

	if (user == NULL) {	
		printf("Error 1\n");
		return;
	}

	char host[100];
	host[99] = '\0';
	ret = gethostname(host, strlen(host) - 1);

	if (ret) {
		printf("Error 2\n");
		return;
	}

	char *dir = (char *)malloc(100 * sizeof(char));
	if (getcwd(dir, strlen(dir) - 1) != dir) {	
		printf("Error 3\n");
		return;
	}

	printf("%s@%s:%s> ", user, host, dir);
}

// Function which exits, printing the necessary message
//
void do_exit() {
	printf("So long and thanks for all the fish!\n");
	exit(0);
}

/*
 * CS3600, Spring 2013
 * Project 1 Starter Code
 * (c) 2013 Alan Mislove
 *
 * You should use this (very simple) starter code as a basis for
 * building your shell.  Please see the project handout for more
 * details.
 */

#ifndef _3600sh_h
#define _3600sh_h

#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <regex.h>

void do_exit();
int do_prompt(char **input, int *eof);
int do_parse_input(char *input, char ***args, int *background);
int do_exec(char **argl, int background, pid_t **pids);

void free_args(char **args);
void reset_redirection(int old_i, int old_o, int old_e);

void debug_print_args(char **args);

#endif 

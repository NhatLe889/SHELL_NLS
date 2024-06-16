#pragma once
#include "declare_func.h"
#include "looping.h"

extern char *command_list[];

int num_command();

int (*command_func[])(char **);

//Declare more command below:

int command_cd(char** parse_input);

int command_exit(char** parse_input);

int command_help(char** parse_input);

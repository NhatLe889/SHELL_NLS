#pragma once
#include "looping.h"
#include "nls_command.h"

char* shell_initilization();

char** shell_parse(char* process_command);

int shell_starting(char** parse_input);

int support_shell_command(char** parse_input);

#pragma once
#include "looping.h"
#include "nls_command.h"

char* shell_initilization();

char** shell_parse(char *process_command, int *pipe_signal, int *pipe_position);

int shell_starting(char **parse_input);

int support_shell_command(char **parse_input);

void split_piping(char **parse_input, int pipe_position, char ***left_half, char ***right_half);

int piping_execute(char **left_half, char**right_half);

int support_piping(char **left_half, char**right_half);